/**
 * @file ota_writer.h
 * @brief OTA flash writer wrapper around ESP-IDF app_update APIs.
 */

#ifndef OTA_WRITER_H
#define OTA_WRITER_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const esp_partition_t *update_partition;
    esp_ota_handle_t handle;
    uint32_t image_size;
    uint32_t bytes_written;
    bool active;
} ota_writer_context_t;

/**
 * @brief Starts an OTA write session on the next inactive OTA app partition.
 *
 * @param[out] ctx OTA writer context owned by the caller.
 * @param[in] image_size Expected firmware image size in bytes.
 * @return ESP_OK on success, otherwise an ESP-IDF error code.
 */
esp_err_t ota_writer_begin(ota_writer_context_t *ctx, uint32_t image_size);

/**
 * @brief Writes the next sequential firmware chunk to the OTA partition.
 *
 * @param[in,out] ctx Active OTA writer context.
 * @param[in] data Firmware bytes to write.
 * @param[in] len Number of bytes in data.
 * @return ESP_OK on success, otherwise an ESP-IDF error code.
 */
esp_err_t ota_writer_write(ota_writer_context_t *ctx, const uint8_t *data, uint32_t len);

/**
 * @brief Finishes the OTA write, validates the image, and optionally sets it as next boot.
 *
 * @param[in,out] ctx Active OTA writer context.
 * @param[in] set_boot_partition True to boot the newly written image after restart.
 * @return ESP_OK on success, otherwise an ESP-IDF error code.
 */
esp_err_t ota_writer_finish(ota_writer_context_t *ctx, bool set_boot_partition);

/**
 * @brief Aborts an active OTA write session.
 *
 * @param[in,out] ctx Active OTA writer context.
 * @return ESP_OK on success, otherwise an ESP-IDF error code.
 */
esp_err_t ota_writer_abort(ota_writer_context_t *ctx);

/**
 * @brief Marks the currently running app as valid and cancels rollback.
 */
esp_err_t ota_writer_mark_running_valid(void);

/**
 * @brief Marks the currently running app as invalid and reboots into rollback.
 */
esp_err_t ota_writer_mark_running_invalid_and_rollback(void);

/**
 * @brief Restarts the ESP32 after a successful OTA update.
 */
void ota_writer_restart(void);

#ifdef __cplusplus
}
#endif

#endif // OTA_WRITER_H
