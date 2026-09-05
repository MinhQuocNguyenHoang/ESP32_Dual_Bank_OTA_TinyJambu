/**
 * @file nvs_store.h
 * @brief NVS storage interface for OTA rollback bookkeeping.
 * @details Stores whether a newly installed OTA image is waiting for validation
 *          and how many times that pending image has booted.
 */

#ifndef NVS_STORE_H
#define NVS_STORE_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NVS_STORE_MAX_OTA_BOOT_ATTEMPTS 3U

/**
 * @brief Marks the next booted OTA image as pending validation.
 * @param None
 * @return esp_err_t ESP_OK on success, otherwise an ESP-IDF error code.
 */
esp_err_t nvs_store_set_ota_pending(void);

/**
 * @brief Clears pending OTA validation state and resets boot counter.
 * @param None
 * @return esp_err_t ESP_OK on success, otherwise an ESP-IDF error code.
 */
esp_err_t nvs_store_clear_ota_pending(void);

/**
 * @brief Reads whether an OTA image is pending validation.
 * @param[out] pending Pointer that receives true if pending, false otherwise.
 * @return esp_err_t ESP_OK on success, otherwise an ESP-IDF error code.
 */
esp_err_t nvs_store_is_ota_pending(bool *pending);

/**
 * @brief Reads pending OTA boot counter.
 * @param[out] count Pointer that receives current boot count.
 * @return esp_err_t ESP_OK on success, otherwise an ESP-IDF error code.
 */
esp_err_t nvs_store_get_ota_boot_count(uint32_t *count);

/**
 * @brief Increments pending OTA boot counter.
 * @param[out] count Pointer that receives updated boot count, or NULL if unused.
 * @return esp_err_t ESP_OK on success, otherwise an ESP-IDF error code.
 */
esp_err_t nvs_store_increment_ota_boot_count(uint32_t *count);

/**
 * @brief Resets pending OTA boot counter to zero.
 * @param None
 * @return esp_err_t ESP_OK on success, otherwise an ESP-IDF error code.
 */
esp_err_t nvs_store_reset_ota_boot_count(void);

#ifdef __cplusplus
}
#endif

#endif // NVS_STORE_H
