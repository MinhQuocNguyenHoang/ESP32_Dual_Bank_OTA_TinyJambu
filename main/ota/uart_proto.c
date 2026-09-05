/**
 * @file uart_proto.c
 * @brief UART packet protocol implementation for ESP32 OTA update.
 * @details Handles UART driver setup, packet framing/parsing, CRC16 validation,
 *          and ACK/NACK response transmission. OTA flash writing is intentionally
 *          kept outside this layer.
 */

#include "uart_proto.h"
#include "crc_until.h"

#include <string.h>
#include "esp_log.h"
#include "freertos/queue.h"

/* ========================================================================= */
/*                          CONSTANTS AND DEFINITIONS                        */
/* ========================================================================= */

#define UART_PROTO_HEADER_SIZE 7U
#define UART_PROTO_CRC_SIZE 2U
#define UART_PROTO_ACK_CMD 0x79U
#define UART_PROTO_NACK_CMD 0x1FU
#define UART_PROTO_RESPONSE_PAYLOAD_SIZE 1U
#define UART_PROTO_RX_CHUNK_SIZE 128U

static const char *TAG = "UART_PROTO";

/* ========================================================================= */
/*                          STATIC GLOBAL VARIABLES                          */
/* ========================================================================= */

static uart_port_t s_uart_num = UART_NUM_2;
static QueueHandle_t s_uart_event_queue = NULL;
static bool s_initialized = false;

/* ========================================================================= */
/*                          STATIC HELPER FUNCTIONS                          */
/* ========================================================================= */

/**
 * @brief Loads a uint16_t value from a little-endian byte buffer.
 * @param[in] data Pointer to two-byte input buffer.
 * @return uint16_t Decoded unsigned 16-bit value.
 */
static uint16_t uart_proto_load_u16_le(const uint8_t *data)
{

    uint16_t value = 0U;

    if (data != NULL)
    {
        value = (uint16_t)((uint16_t)data[0] | ((uint16_t)data[1] << 8));
    }

    return value;
}

/**
 * @brief Stores a uint16_t value into a little-endian byte buffer.
 * @param[out] data Pointer to two-byte output buffer.
 * @param[in] value Unsigned 16-bit value to encode.
 * @return None
 */
static void uart_proto_store_u16_le(uint8_t *data, uint16_t value)
{
    if (data != NULL)
    {
        data[0] = (uint8_t)(value & 0x00FFU);
        data[1] = (uint8_t)((value >> 8) & 0x00FFU);
    }
}

/**
 * @brief Checks whether a command byte is a supported host-to-device command.
 * @param[in] cmd Raw command byte from the received packet.
 * @return bool True if command is valid, false otherwise.
 */
static bool uart_proto_is_valid_cmd(uint8_t cmd)
{

    bool valid = false;

    if ((cmd == (uint8_t)UART_PROTO_CMD_START_OTA) ||
        (cmd == (uint8_t)UART_PROTO_CMD_DATA) ||
        (cmd == (uint8_t)UART_PROTO_CMD_END_OTA) ||
        (cmd == (uint8_t)UART_PROTO_CMD_ABORT))
    {
        valid = true;
    }

    return valid;
}

/**
 * @brief Calculates remaining timeout ticks for packet receive operation.
 * @param[in] start_tick Tick count captured when receive started.
 * @param[in] timeout_ticks Total receive timeout.
 * @param[out] remaining_ticks Pointer to store remaining timeout ticks.
 * @return bool True if time remains, false if timeout expired.
 */
static bool uart_proto_get_remaining_ticks(TickType_t start_tick,
                                           TickType_t timeout_ticks,
                                           TickType_t *remaining_ticks)
{

    TickType_t now_ticks = 0U;
    TickType_t elapsed_ticks = 0U;
    bool has_time = true;

    if (remaining_ticks == NULL)
    {
        has_time = false;
    }
    else if (timeout_ticks == portMAX_DELAY)
    {
        *remaining_ticks = portMAX_DELAY;
    }
    else
    {
        now_ticks = xTaskGetTickCount();
        elapsed_ticks = now_ticks - start_tick;
        if (elapsed_ticks >= timeout_ticks)
        {
            *remaining_ticks = 0U;
            has_time = false;
        }
        else
        {
            *remaining_ticks = timeout_ticks - elapsed_ticks;
        }
    }

    return has_time;
}

/**
 * @brief Decodes and validates a complete UART protocol frame.
 * @param[in] frame Pointer to raw frame bytes.
 * @param[in] frame_len Number of bytes in the complete frame.
 * @param[out] packet Pointer to decoded packet output.
 * @return esp_err_t ESP_OK on valid packet, otherwise an ESP-IDF error code.
 */
static esp_err_t uart_proto_decode_frame(const uint8_t *frame, uint32_t frame_len, uart_proto_packet_t *packet)
{

    uint16_t payload_len = 0U;
    uint16_t received_crc16 = 0U;
    uint16_t calculated_crc16 = 0U;
    uint32_t expected_frame_len = 0U;
    esp_err_t ret = ESP_OK;

    if ((frame == NULL) || (packet == NULL))
    {
        ret = ESP_ERR_INVALID_ARG;
    }
    else if (frame_len < (UART_PROTO_HEADER_SIZE + UART_PROTO_CRC_SIZE))
    {
        ret = ESP_ERR_INVALID_SIZE;
    }
    else if ((frame[0] != UART_PROTO_MAGIC_0) || (frame[1] != UART_PROTO_MAGIC_1))
    {
        ret = ESP_ERR_INVALID_RESPONSE;
    }
    else if (!uart_proto_is_valid_cmd(frame[2]))
    {
        ret = ESP_ERR_INVALID_ARG;
    }
    else
    {
        payload_len = uart_proto_load_u16_le(&frame[5]);
        expected_frame_len = UART_PROTO_HEADER_SIZE + (uint32_t)payload_len + UART_PROTO_CRC_SIZE;

        if (payload_len > UART_PROTO_MAX_PAYLOAD_SIZE)
        {
            ret = ESP_ERR_INVALID_SIZE;
        }
        else if (frame_len != expected_frame_len)
        {
            ret = ESP_ERR_INVALID_SIZE;
        }
        else
        {
            received_crc16 = uart_proto_load_u16_le(&frame[UART_PROTO_HEADER_SIZE + payload_len]);
            calculated_crc16 = crc_util_crc16_ccitt(frame, UART_PROTO_HEADER_SIZE + payload_len);

            memset(packet, 0, sizeof(*packet));
            packet->cmd = (uart_proto_cmd_t)frame[2];
            packet->seq = uart_proto_load_u16_le(&frame[3]);
            packet->len = payload_len;
            packet->received_crc16 = received_crc16;
            packet->calculated_crc16 = calculated_crc16;
            packet->crc_valid = (received_crc16 == calculated_crc16);

            if (payload_len > 0U)
            {
                memcpy(packet->payload, &frame[UART_PROTO_HEADER_SIZE], payload_len);
            }

            if (!packet->crc_valid)
            {
                ret = ESP_ERR_INVALID_CRC;
            }
        }
    }

    return ret;
}

/**
 * @brief Sends a compact ACK/NACK response frame to the PC host.
 * @details Response frame format:
 *          [MAGIC0:1][MAGIC1:1][RESP:1][SEQ:2][LEN:2][STATUS:1][CRC16:2].
 * @param[in] response_cmd ACK or NACK response command byte.
 * @param[in] seq Packet sequence number being acknowledged or rejected.
 * @param[in] status Response status code.
 * @return esp_err_t ESP_OK on success, otherwise an ESP-IDF error code.
 */
static esp_err_t uart_proto_send_response(uint8_t response_cmd, uint16_t seq, uart_proto_status_t status)
{

    uint8_t frame[UART_PROTO_HEADER_SIZE + UART_PROTO_RESPONSE_PAYLOAD_SIZE + UART_PROTO_CRC_SIZE] = {0};
    uint16_t crc16 = 0U;
    int written = 0;
    esp_err_t ret = ESP_OK;

    if (!s_initialized)
    {
        ret = ESP_ERR_INVALID_STATE;
    }
    else
    {
        frame[0] = UART_PROTO_MAGIC_0;
        frame[1] = UART_PROTO_MAGIC_1;
        frame[2] = response_cmd;
        uart_proto_store_u16_le(&frame[3], seq);
        uart_proto_store_u16_le(&frame[5], UART_PROTO_RESPONSE_PAYLOAD_SIZE);
        frame[UART_PROTO_HEADER_SIZE] = (uint8_t)status;

        crc16 = crc_util_crc16_ccitt(frame, UART_PROTO_HEADER_SIZE + UART_PROTO_RESPONSE_PAYLOAD_SIZE);
        uart_proto_store_u16_le(&frame[UART_PROTO_HEADER_SIZE + UART_PROTO_RESPONSE_PAYLOAD_SIZE], crc16);

        written = uart_write_bytes(s_uart_num, (const char *)frame, sizeof(frame));
        if (written != (int)sizeof(frame))
        {
            ESP_LOGE(TAG, "Failed to send UART response frame");
            ret = ESP_FAIL;
        }
    }

    return ret;
}

/* ========================================================================= */
/*                          PUBLIC FUNCTION DEFINITIONS                      */
/* ========================================================================= */

esp_err_t uart_proto_init(const uart_proto_config_t *config)
{

    esp_err_t ret = ESP_OK;
    uart_config_t uart_config = {0};

    if (config == NULL)
    {
        ret = ESP_ERR_INVALID_ARG;
    }
    else
    {
        uart_config.baud_rate = config->baud_rate;
        uart_config.data_bits = UART_DATA_8_BITS;
        uart_config.parity = UART_PARITY_DISABLE;
        uart_config.stop_bits = UART_STOP_BITS_1;
        uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
        uart_config.source_clk = UART_SCLK_DEFAULT;

        s_uart_num = config->uart_num;

        ret = uart_param_config(s_uart_num, &uart_config);
        if (ret == ESP_OK)
        {
            ret = uart_set_pin(s_uart_num,
                               config->tx_pin,
                               config->rx_pin,
                               UART_PIN_NO_CHANGE,
                               UART_PIN_NO_CHANGE);
        }

        if (ret == ESP_OK)
        {
            ret = uart_driver_install(s_uart_num,
                                      (int)config->rx_buffer_size,
                                      (int)config->tx_buffer_size,
                                      (int)config->event_queue_size,
                                      &s_uart_event_queue,
                                      0);
        }

        if ((ret == ESP_OK) || (ret == ESP_ERR_INVALID_STATE))
        {
            s_initialized = true;
            ret = ESP_OK;
            ESP_LOGI(TAG,
                     "UART protocol initialized: UART%d TX=%d RX=%d baud=%d",
                     (int)s_uart_num,
                     (int)config->tx_pin,
                     (int)config->rx_pin,
                     config->baud_rate);
        }
    }

    return ret;
}

esp_err_t uart_proto_receive_packet(uart_proto_packet_t *packet, TickType_t timeout_ticks)
{

    uart_event_t event = {0};
    uint8_t frame[UART_PROTO_MAX_FRAME_SIZE] = {0};
    uint8_t rx_buf[UART_PROTO_RX_CHUNK_SIZE] = {0};
    uint32_t frame_index = 0U;
    uint32_t expected_frame_len = 0U;
    uint32_t bytes_remaining = 0U;
    uint32_t bytes_to_read = 0U;
    TickType_t start_tick = 0U;
    TickType_t remaining_ticks = 0U;
    int read_len = 0;
    int32_t i = 0;
    esp_err_t ret = ESP_OK;
    bool packet_complete = false;

    if (packet == NULL)
    {
        ret = ESP_ERR_INVALID_ARG;
    }
    else if ((!s_initialized) || (s_uart_event_queue == NULL))
    {
        ret = ESP_ERR_INVALID_STATE;
    }
    else
    {
        memset(packet, 0, sizeof(*packet));
        start_tick = xTaskGetTickCount();

        while ((ret == ESP_OK) && (!packet_complete))
        {
            if (!uart_proto_get_remaining_ticks(start_tick, timeout_ticks, &remaining_ticks))
            {
                ret = ESP_ERR_TIMEOUT;
            }
            else if (xQueueReceive(s_uart_event_queue, &event, remaining_ticks) != pdTRUE)
            {
                ret = ESP_ERR_TIMEOUT;
            }
            else if (event.type == UART_DATA)
            {
                bytes_remaining = (uint32_t)event.size;

                while ((bytes_remaining > 0U) && (ret == ESP_OK))
                {
                    bytes_to_read = (bytes_remaining > UART_PROTO_RX_CHUNK_SIZE) ? UART_PROTO_RX_CHUNK_SIZE : bytes_remaining;
                    read_len = uart_read_bytes(s_uart_num, rx_buf, bytes_to_read, 0);

                    if (read_len < 0)
                    {
                        ret = ESP_FAIL;
                    }
                    else if (read_len == 0)
                    {
                        bytes_remaining = 0U;
                    }
                    else
                    {
                        bytes_remaining -= (uint32_t)read_len;

                        for (i = 0; (i < read_len) && (ret == ESP_OK) && (!packet_complete); i++)
                        {
                            if (frame_index == 0U)
                            {
                                if (rx_buf[i] == UART_PROTO_MAGIC_0)
                                {
                                    frame[frame_index] = rx_buf[i];
                                    frame_index++;
                                }
                            }
                            else if (frame_index == 1U)
                            {
                                if (rx_buf[i] == UART_PROTO_MAGIC_1)
                                {
                                    frame[frame_index] = rx_buf[i];
                                    frame_index++;
                                }
                                else if (rx_buf[i] == UART_PROTO_MAGIC_0)
                                {
                                    frame[0] = rx_buf[i];
                                    frame_index = 1U;
                                }
                                else
                                {
                                    frame_index = 0U;
                                }
                            }
                            else
                            {
                                frame[frame_index] = rx_buf[i];
                                frame_index++;

                                if (frame_index == UART_PROTO_HEADER_SIZE)
                                {
                                    expected_frame_len = UART_PROTO_HEADER_SIZE +
                                                         (uint32_t)uart_proto_load_u16_le(&frame[5]) +
                                                         UART_PROTO_CRC_SIZE;

                                    if (expected_frame_len > UART_PROTO_MAX_FRAME_SIZE)
                                    {
                                        ret = ESP_ERR_INVALID_SIZE;
                                        frame_index = 0U;
                                    }
                                }

                                if ((expected_frame_len > 0U) && (frame_index == expected_frame_len))
                                {
                                    ret = uart_proto_decode_frame(frame, expected_frame_len, packet);
                                    if (ret == ESP_OK)
                                    {
                                        packet_complete = true;
                                    }
                                    else
                                    {
                                        frame_index = 0U;
                                        expected_frame_len = 0U;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else if ((event.type == UART_FIFO_OVF) || (event.type == UART_BUFFER_FULL))
            {
                uart_flush_input(s_uart_num);
                xQueueReset(s_uart_event_queue);
                ret = ESP_ERR_INVALID_STATE;
            }
            else if ((event.type == UART_PARITY_ERR) || (event.type == UART_FRAME_ERR))
            {
                ret = ESP_ERR_INVALID_CRC;
            }
            else
            {
                /* Ignore non-data UART events and continue waiting within timeout. */
            }
        }
    }

    return ret;
}

esp_err_t uart_proto_send_ack(uint16_t seq, uart_proto_status_t status)
{
    esp_err_t ret = ESP_OK;

    ret = uart_proto_send_response(UART_PROTO_ACK_CMD, seq, status);

    return ret;
}

esp_err_t uart_proto_send_nack(uint16_t seq, uart_proto_status_t status)
{
    esp_err_t ret = ESP_OK;

    ret = uart_proto_send_response(UART_PROTO_NACK_CMD, seq, status);

    return ret;
}

void uart_proto_deinit(void)
{

    if (s_initialized)
    {
        uart_driver_delete(s_uart_num);
        s_uart_event_queue = NULL;
        s_initialized = false;
    }
}
