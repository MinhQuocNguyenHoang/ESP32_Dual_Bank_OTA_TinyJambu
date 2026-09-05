/**
 * @file uart_proto.h
 * @brief UART packet protocol interface for ESP32 dual-bank OTA update.
 * @details Defines the packet format, OTA command identifiers, ACK/NACK status
 *          codes, and UART protocol APIs. This layer is responsible only for
 *          UART framing, CRC16 packet validation, and ACK/NACK transmission.
 */

#ifndef UART_PROTO_H
#define UART_PROTO_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define UART_PROTO_MAGIC_0 0xA5U
#define UART_PROTO_MAGIC_1 0x5AU
#define UART_PROTO_MAX_PAYLOAD_SIZE 256U
#define UART_PROTO_MAX_FRAME_SIZE (2U + 1U + 2U + 2U + UART_PROTO_MAX_PAYLOAD_SIZE + 2U)

    /**
     * @brief UART OTA command identifiers carried in the CMD field.
     */
    typedef enum
    {
        UART_PROTO_CMD_START_OTA = 0x01,
        UART_PROTO_CMD_DATA = 0x02,
        UART_PROTO_CMD_END_OTA = 0x03,
        UART_PROTO_CMD_ABORT = 0x04
    } uart_proto_cmd_t;

    /**
     * @brief UART OTA response status codes for ACK/NACK frames.
     */
    typedef enum
    {
        UART_PROTO_STATUS_OK = 0x00,
        UART_PROTO_STATUS_CRC_ERROR = 0x01,
        UART_PROTO_STATUS_SEQ_ERROR = 0x02,
        UART_PROTO_STATUS_LENGTH_ERROR = 0x03,
        UART_PROTO_STATUS_STATE_ERROR = 0x04,
        UART_PROTO_STATUS_INTERNAL_ERROR = 0x05
    } uart_proto_status_t;

    /**
     * @brief UART hardware configuration for the OTA protocol layer.
     */
    typedef struct
    {
        uart_port_t uart_num;
        gpio_num_t tx_pin;
        gpio_num_t rx_pin;
        int baud_rate;
        uint32_t rx_buffer_size;
        uint32_t tx_buffer_size;
        uint32_t event_queue_size;
    } uart_proto_config_t;

    /**
     * @brief Decoded UART OTA protocol packet.
     * @details Packet wire format:
     *          [MAGIC0:1][MAGIC1:1][CMD:1][SEQ:2][LEN:2][PAYLOAD:0..256][CRC16:2].
     *          Multi-byte fields should be encoded little-endian by both PC host
     *          and ESP32 firmware.
     */
    typedef struct
    {
        uart_proto_cmd_t cmd;
        uint16_t seq;
        uint16_t len;
        uint8_t payload[UART_PROTO_MAX_PAYLOAD_SIZE];
        uint16_t received_crc16;
        uint16_t calculated_crc16;
        bool crc_valid;
    } uart_proto_packet_t;

    /* ========================================================================= */
    /*                          PUBLIC FUNCTION DECLARATIONS                     */
    /* ========================================================================= */

    /**
     * @brief Initializes the UART driver and protocol event queue.
     * @param[in] config Pointer to UART protocol hardware configuration.
     * @return esp_err_t ESP_OK on success, otherwise an ESP-IDF error code.
     */
    esp_err_t uart_proto_init(const uart_proto_config_t *config);

    /**
     * @brief Receives and decodes one validated UART OTA packet.
     * @param[out] packet Pointer to packet structure that receives decoded data.
     * @param[in] timeout_ticks FreeRTOS timeout while waiting for UART data.
     * @return esp_err_t ESP_OK on success, otherwise an ESP-IDF error code.
     */
    esp_err_t uart_proto_receive_packet(uart_proto_packet_t *packet, TickType_t timeout_ticks);

    /**
     * @brief Sends an ACK response for a successfully processed packet.
     * @param[in] seq Packet sequence number being acknowledged.
     * @param[in] status ACK status code.
     * @return esp_err_t ESP_OK on success, otherwise an ESP-IDF error code.
     */
    esp_err_t uart_proto_send_ack(uint16_t seq, uart_proto_status_t status);

    /**
     * @brief Sends a NACK response for a rejected packet.
     * @param[in] seq Packet sequence number being rejected.
     * @param[in] status NACK reason code.
     * @return esp_err_t ESP_OK on success, otherwise an ESP-IDF error code.
     */
    esp_err_t uart_proto_send_nack(uint16_t seq, uart_proto_status_t status);

    /**
     * @brief Deinitializes the UART protocol layer and releases UART driver resources.
     * @param None
     * @return None
     */
    void uart_proto_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // UART_PROTO_H
