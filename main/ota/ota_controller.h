/**
 * @file ota_controller.h
 * @brief OTA update controller interface over the UART packet protocol.
 * @details Coordinates UART_PROTO packet handling, OTA_WRITER flash writes,
 *          and CRC32 image verification. This layer owns the OTA update state
 *          machine and does not access UART registers or flash directly.
 */

#ifndef OTA_CONTROLLER_H
#define OTA_CONTROLLER_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "uart_proto.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OTA_CONTROLLER_START_PAYLOAD_SIZE 8U

/**
 * @brief High-level OTA controller state.
 */
typedef enum {
    OTA_CONTROLLER_STATE_IDLE = 0,
    OTA_CONTROLLER_STATE_RECEIVING,
    OTA_CONTROLLER_STATE_VERIFYING,
    OTA_CONTROLLER_STATE_SUCCESS,
    OTA_CONTROLLER_STATE_FAILED
} ota_controller_state_t;

/**
 * @brief OTA controller runtime configuration.
 */
typedef struct {
    uart_proto_config_t uart_config;
    TickType_t packet_timeout_ticks;
    TickType_t reboot_delay_ticks;
    bool reboot_on_success;
} ota_controller_config_t;

/**
 * @brief Initializes UART protocol layer and resets OTA controller state.
 * @param[in] config Pointer to OTA controller configuration.
 * @return esp_err_t ESP_OK on success, otherwise an ESP-IDF error code.
 */
esp_err_t ota_controller_init(const ota_controller_config_t *config);

/**
 * @brief Processes one OTA protocol packet.
 * @details This function waits up to the configured packet timeout, receives
 *          one UART packet, updates the OTA state machine, and sends ACK/NACK.
 * @param None
 * @return esp_err_t ESP_OK on handled packet, otherwise an ESP-IDF error code.
 */
esp_err_t ota_controller_process_once(void);

/**
 * @brief Gets current OTA controller state.
 * @param None
 * @return ota_controller_state_t Current high-level OTA state.
 */
ota_controller_state_t ota_controller_get_state(void);

/**
 * @brief Deinitializes OTA controller and releases UART protocol resources.
 * @param None
 * @return None
 */
void ota_controller_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // OTA_CONTROLLER_H
