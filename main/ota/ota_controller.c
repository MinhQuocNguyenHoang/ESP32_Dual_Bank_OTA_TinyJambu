/**
 * @file ota_controller.c
 * @brief OTA update controller implementation over the UART packet protocol.
 * @details Implements the START/DATA/END/ABORT state machine, streams CRC32
 *          over received firmware bytes, and delegates flash operations to
 *          OTA_WRITER.
 */

#include "ota_controller.h"

#include <inttypes.h>
#include <string.h>
#include "crc_until.h"
#include "esp_log.h"
#include "freertos/task.h"
#include "nvs_store.h"
#include "ota_writer.h"

/* ========================================================================= */
/*                          STATIC TYPE DEFINITIONS                          */
/* ========================================================================= */

typedef struct {
    ota_writer_context_t writer;
    ota_controller_config_t config;
    ota_controller_state_t state;
    uint32_t expected_image_size;
    uint32_t expected_crc32;
    uint32_t calculated_crc32;
    uint32_t crc32_state;
    uint32_t bytes_received;
    uint16_t expected_seq;
    bool initialized;
    bool session_active;
} ota_controller_context_t;

/* ========================================================================= */
/*                          STATIC GLOBAL VARIABLES                          */
/* ========================================================================= */

static const char *TAG = "OTA_CONTROLLER";
static ota_controller_context_t s_ota_controller = {0};

/* ========================================================================= */
/*                          STATIC HELPER FUNCTIONS                          */
/* ========================================================================= */

/**
 * @brief Loads a uint32_t value from a little-endian byte buffer.
 * @param[in] data Pointer to four-byte input buffer.
 * @return uint32_t Decoded unsigned 32-bit value.
 */
static uint32_t ota_controller_load_u32_le(const uint8_t *data)
{
    uint32_t value = 0U;

    if (data != NULL)
    {
        value = ((uint32_t)data[0]) |
                ((uint32_t)data[1] << 8) |
                ((uint32_t)data[2] << 16) |
                ((uint32_t)data[3] << 24);
    }

    return value;
}

/**
 * @brief Resets the active OTA transfer metadata.
 * @param None
 * @return None
 */
static void ota_controller_reset_session(void)
{
    memset(&s_ota_controller.writer, 0, sizeof(s_ota_controller.writer));
    s_ota_controller.expected_image_size = 0U;
    s_ota_controller.expected_crc32 = 0U;
    s_ota_controller.calculated_crc32 = 0U;
    s_ota_controller.crc32_state = crc_util_crc32_init();
    s_ota_controller.bytes_received = 0U;
    s_ota_controller.expected_seq = 0U;
    s_ota_controller.session_active = false;
}

/**
 * @brief Converts an ESP-IDF error code to a UART protocol NACK status.
 * @param[in] err ESP-IDF error code.
 * @return uart_proto_status_t Protocol status code.
 */
static uart_proto_status_t ota_controller_error_to_status(esp_err_t err)
{
    uart_proto_status_t status = UART_PROTO_STATUS_INTERNAL_ERROR;

    if (err == ESP_ERR_INVALID_CRC)
    {
        status = UART_PROTO_STATUS_CRC_ERROR;
    }
    else if (err == ESP_ERR_INVALID_SIZE)
    {
        status = UART_PROTO_STATUS_LENGTH_ERROR;
    }
    else if (err == ESP_ERR_INVALID_STATE)
    {
        status = UART_PROTO_STATUS_STATE_ERROR;
    }
    else if (err == ESP_ERR_INVALID_ARG)
    {
        status = UART_PROTO_STATUS_STATE_ERROR;
    }
    else
    {
        status = UART_PROTO_STATUS_INTERNAL_ERROR;
    }

    return status;
}

/**
 * @brief Sends ACK when ret is OK, otherwise sends NACK with matching status.
 * @param[in] seq Packet sequence number.
 * @param[in] ret Packet processing result.
 * @param[in] nack_status NACK status when ret is not ESP_OK.
 * @return esp_err_t ESP_OK if response was sent, otherwise an ESP-IDF error code.
 */
static esp_err_t ota_controller_send_result(uint16_t seq, esp_err_t ret, uart_proto_status_t nack_status)
{
    esp_err_t response_ret = ESP_OK;

    if (ret == ESP_OK)
    {
        response_ret = uart_proto_send_ack(seq, UART_PROTO_STATUS_OK);
    }
    else
    {
        response_ret = uart_proto_send_nack(seq, nack_status);
    }

    return response_ret;
}

/**
 * @brief Handles a START_OTA command packet.
 * @param[in] packet Decoded START_OTA packet.
 * @return esp_err_t ESP_OK on success, otherwise an ESP-IDF error code.
 */
static esp_err_t ota_controller_handle_start(const uart_proto_packet_t *packet)
{
    uint32_t image_size = 0U;
    uint32_t expected_crc32 = 0U;
    esp_err_t ret = ESP_OK;

    if (packet == NULL)
    {
        ret = ESP_ERR_INVALID_ARG;
    }
    else if (s_ota_controller.session_active)
    {
        ret = ESP_ERR_INVALID_STATE;
    }
    else if (packet->len != OTA_CONTROLLER_START_PAYLOAD_SIZE)
    {
        ret = ESP_ERR_INVALID_SIZE;
    }
    else
    {
        image_size = ota_controller_load_u32_le(&packet->payload[0]);
        expected_crc32 = ota_controller_load_u32_le(&packet->payload[4]);

        ESP_LOGI(TAG,
                 "START OTA: image_size=%" PRIu32 " expected_crc32=0x%08" PRIx32,
                 image_size,
                 expected_crc32);

        ret = ota_writer_begin(&s_ota_controller.writer, image_size);
        if (ret == ESP_OK)
        {
            s_ota_controller.state = OTA_CONTROLLER_STATE_RECEIVING;
            s_ota_controller.expected_image_size = image_size;
            s_ota_controller.expected_crc32 = expected_crc32;
            s_ota_controller.calculated_crc32 = 0U;
            s_ota_controller.crc32_state = crc_util_crc32_init();
            s_ota_controller.bytes_received = 0U;
            s_ota_controller.expected_seq = (uint16_t)(packet->seq + 1U);
            s_ota_controller.session_active = true;
        }
        else
        {
            s_ota_controller.state = OTA_CONTROLLER_STATE_FAILED;
            ESP_LOGE(TAG, "FAIL: ota_writer_begin failed: %s", esp_err_to_name(ret));
        }
    }

    return ret;
}

/**
 * @brief Handles a DATA command packet.
 * @param[in] packet Decoded DATA packet.
 * @return esp_err_t ESP_OK on success, otherwise an ESP-IDF error code.
 */
static esp_err_t ota_controller_handle_data(const uart_proto_packet_t *packet)
{
    uint32_t bytes_after_write = 0U;
    uint32_t percent = 0U;
    esp_err_t ret = ESP_OK;

    if (packet == NULL)
    {
        ret = ESP_ERR_INVALID_ARG;
    }
    else if ((!s_ota_controller.session_active) ||
             (s_ota_controller.state != OTA_CONTROLLER_STATE_RECEIVING))
    {
        ret = ESP_ERR_INVALID_STATE;
    }
    else if (packet->seq != s_ota_controller.expected_seq)
    {
        ret = ESP_ERR_INVALID_RESPONSE;
    }
    else if (packet->len == 0U)
    {
        ret = ESP_ERR_INVALID_SIZE;
    }
    else if (((uint32_t)packet->len) > (s_ota_controller.expected_image_size - s_ota_controller.bytes_received))
    {
        ret = ESP_ERR_INVALID_SIZE;
    }
    else
    {
        ret = ota_writer_write(&s_ota_controller.writer, packet->payload, packet->len);
        if (ret == ESP_OK)
        {
            s_ota_controller.crc32_state = crc_util_crc32_update(s_ota_controller.crc32_state,
                                                                 packet->payload,
                                                                 packet->len);
            s_ota_controller.bytes_received += packet->len;
            s_ota_controller.expected_seq = (uint16_t)(s_ota_controller.expected_seq + 1U);

            bytes_after_write = s_ota_controller.bytes_received;
            if (s_ota_controller.expected_image_size > 0U)
            {
                percent = (uint32_t)(((uint64_t)bytes_after_write * 100ULL) /
                                     (uint64_t)s_ota_controller.expected_image_size);
            }

            ESP_LOGI(TAG,
                     "WRITING: %" PRIu32 "/%" PRIu32 " bytes (%" PRIu32 "%%)",
                     bytes_after_write,
                     s_ota_controller.expected_image_size,
                     percent);
        }
        else
        {
            s_ota_controller.state = OTA_CONTROLLER_STATE_FAILED;
            ESP_LOGE(TAG, "FAIL: ota_writer_write failed: %s", esp_err_to_name(ret));
        }
    }

    return ret;
}

/**
 * @brief Handles an END_OTA command packet.
 * @param[in] packet Decoded END_OTA packet.
 * @return esp_err_t ESP_OK on success, otherwise an ESP-IDF error code.
 */
static esp_err_t ota_controller_handle_end(const uart_proto_packet_t *packet)
{
    esp_err_t ret = ESP_OK;

    if (packet == NULL)
    {
        ret = ESP_ERR_INVALID_ARG;
    }
    else if ((!s_ota_controller.session_active) ||
             (s_ota_controller.state != OTA_CONTROLLER_STATE_RECEIVING))
    {
        ret = ESP_ERR_INVALID_STATE;
    }
    else if (packet->seq != s_ota_controller.expected_seq)
    {
        ret = ESP_ERR_INVALID_RESPONSE;
    }
    else if (packet->len != 0U)
    {
        ret = ESP_ERR_INVALID_SIZE;
    }
    else if (s_ota_controller.bytes_received != s_ota_controller.expected_image_size)
    {
        ret = ESP_ERR_INVALID_SIZE;
    }
    else
    {
        s_ota_controller.state = OTA_CONTROLLER_STATE_VERIFYING;
        ESP_LOGI(TAG, "VERIFY: calculating firmware CRC32");

        s_ota_controller.calculated_crc32 = crc_util_crc32_finish(s_ota_controller.crc32_state);
        if (s_ota_controller.calculated_crc32 != s_ota_controller.expected_crc32)
        {
            ret = ESP_ERR_INVALID_CRC;
            ESP_LOGE(TAG,
                     "FAIL: CRC32 mismatch calculated=0x%08" PRIx32 " expected=0x%08" PRIx32,
                     s_ota_controller.calculated_crc32,
                     s_ota_controller.expected_crc32);
            (void)ota_writer_abort(&s_ota_controller.writer);
            ota_controller_reset_session();
            s_ota_controller.state = OTA_CONTROLLER_STATE_FAILED;
        }
        else
        {
            ret = nvs_store_set_ota_pending();
            if (ret == ESP_OK)
            {
                ret = ota_writer_finish(&s_ota_controller.writer, true);
            }

            if (ret == ESP_OK)
            {
                ESP_LOGI(TAG, "SUCCESS: OTA image verified and boot partition updated");
                ota_controller_reset_session();
                s_ota_controller.state = OTA_CONTROLLER_STATE_SUCCESS;
            }
            else
            {
                ESP_LOGE(TAG, "FAIL: ota_writer_finish failed: %s", esp_err_to_name(ret));
                (void)nvs_store_clear_ota_pending();
                ota_controller_reset_session();
                s_ota_controller.state = OTA_CONTROLLER_STATE_FAILED;
            }
        }
    }

    return ret;
}

/**
 * @brief Handles an ABORT command packet.
 * @param[in] packet Decoded ABORT packet.
 * @return esp_err_t ESP_OK on success, otherwise an ESP-IDF error code.
 */
static esp_err_t ota_controller_handle_abort(const uart_proto_packet_t *packet)
{
    esp_err_t ret = ESP_OK;

    if (packet == NULL)
    {
        ret = ESP_ERR_INVALID_ARG;
    }
    else if (packet->len != 0U)
    {
        ret = ESP_ERR_INVALID_SIZE;
    }
    else
    {
        ESP_LOGW(TAG, "ABORT: host requested OTA abort");
        if (s_ota_controller.session_active)
        {
            ret = ota_writer_abort(&s_ota_controller.writer);
        }

        ota_controller_reset_session();
        s_ota_controller.state = (ret == ESP_OK) ? OTA_CONTROLLER_STATE_IDLE : OTA_CONTROLLER_STATE_FAILED;
    }

    return ret;
}

/**
 * @brief Dispatches a decoded UART packet to the matching OTA command handler.
 * @param[in] packet Decoded UART protocol packet.
 * @return esp_err_t ESP_OK on success, otherwise an ESP-IDF error code.
 */
static esp_err_t ota_controller_handle_packet(const uart_proto_packet_t *packet)
{
    esp_err_t ret = ESP_OK;

    if (packet == NULL)
    {
        ret = ESP_ERR_INVALID_ARG;
    }
    else if (packet->cmd == UART_PROTO_CMD_START_OTA)
    {
        ret = ota_controller_handle_start(packet);
    }
    else if (packet->cmd == UART_PROTO_CMD_DATA)
    {
        ret = ota_controller_handle_data(packet);
    }
    else if (packet->cmd == UART_PROTO_CMD_END_OTA)
    {
        ret = ota_controller_handle_end(packet);
    }
    else if (packet->cmd == UART_PROTO_CMD_ABORT)
    {
        ret = ota_controller_handle_abort(packet);
    }
    else
    {
        ret = ESP_ERR_INVALID_ARG;
    }

    return ret;
}

/* ========================================================================= */
/*                          PUBLIC FUNCTION DEFINITIONS                      */
/* ========================================================================= */

esp_err_t ota_controller_init(const ota_controller_config_t *config)
{
    esp_err_t ret = ESP_OK;

    if (config == NULL)
    {
        ret = ESP_ERR_INVALID_ARG;
    }
    else
    {
        memset(&s_ota_controller, 0, sizeof(s_ota_controller));
        s_ota_controller.config = *config;
        s_ota_controller.state = OTA_CONTROLLER_STATE_IDLE;
        ota_controller_reset_session();

        ret = uart_proto_init(&config->uart_config);
        if (ret == ESP_OK)
        {
            s_ota_controller.initialized = true;
            ESP_LOGI(TAG, "OTA controller initialized");
        }
        else
        {
            s_ota_controller.state = OTA_CONTROLLER_STATE_FAILED;
            ESP_LOGE(TAG, "FAIL: uart_proto_init failed: %s", esp_err_to_name(ret));
        }
    }

    return ret;
}

esp_err_t ota_controller_process_once(void)
{
    uart_proto_packet_t packet = {0};
    uart_proto_status_t nack_status = UART_PROTO_STATUS_INTERNAL_ERROR;
    esp_err_t ret = ESP_OK;
    esp_err_t response_ret = ESP_OK;

    if (!s_ota_controller.initialized)
    {
        ret = ESP_ERR_INVALID_STATE;
    }
    else
    {
        ret = uart_proto_receive_packet(&packet, s_ota_controller.config.packet_timeout_ticks);
        if (ret == ESP_OK)
        {
            ret = ota_controller_handle_packet(&packet);
            if (ret == ESP_ERR_INVALID_RESPONSE)
            {
                nack_status = UART_PROTO_STATUS_SEQ_ERROR;
            }
            else
            {
                nack_status = ota_controller_error_to_status(ret);
            }

            response_ret = ota_controller_send_result(packet.seq, ret, nack_status);
            if ((ret == ESP_OK) && (response_ret != ESP_OK))
            {
                ret = response_ret;
            }

            if ((ret == ESP_OK) &&
                (packet.cmd == UART_PROTO_CMD_END_OTA) &&
                (s_ota_controller.state == OTA_CONTROLLER_STATE_SUCCESS) &&
                (s_ota_controller.config.reboot_on_success))
            {
                ESP_LOGI(TAG, "REBOOT: restarting after successful OTA");
                vTaskDelay(s_ota_controller.config.reboot_delay_ticks);
                ota_writer_restart();
            }
        }
        else if (ret != ESP_ERR_TIMEOUT)
        {
            nack_status = ota_controller_error_to_status(ret);
            response_ret = uart_proto_send_nack(packet.seq, nack_status);
            if (response_ret != ESP_OK)
            {
                ret = response_ret;
            }
        }
        else
        {
            /* Timeout is a normal idle condition for non-blocking app loops. */
        }
    }

    return ret;
}

ota_controller_state_t ota_controller_get_state(void)
{
    ota_controller_state_t state = OTA_CONTROLLER_STATE_FAILED;

    state = s_ota_controller.state;

    return state;
}

void ota_controller_deinit(void)
{
    if (s_ota_controller.session_active)
    {
        (void)ota_writer_abort(&s_ota_controller.writer);
    }

    uart_proto_deinit();
    memset(&s_ota_controller, 0, sizeof(s_ota_controller));
}
