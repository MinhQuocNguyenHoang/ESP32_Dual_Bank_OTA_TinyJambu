/**
 * @file ota_writer.cpp
 * @brief OTA flash writer wrapper around ESP-IDF app_update APIs.
 */

#include "ota_writer.h"

#include <inttypes.h>
#include <string.h>
#include "esp_log.h"
#include "esp_system.h"

static const char *TAG = "OTA_WRITER";

esp_err_t ota_writer_begin(ota_writer_context_t *ctx, uint32_t image_size)
{
    // 1. Variable Initialization
    const esp_partition_t *running_partition = NULL;
    const esp_partition_t *update_partition = NULL;
    esp_err_t ret = ESP_OK;

    // 2. Core Execution Logic
    if (ctx == NULL || image_size == 0) {
        ret = ESP_ERR_INVALID_ARG;
    } else {
        memset(ctx, 0, sizeof(*ctx));

        running_partition = esp_ota_get_running_partition();
        update_partition = esp_ota_get_next_update_partition(NULL);
        if (update_partition == NULL) {
            ESP_LOGE(TAG, "No inactive OTA app partition found");
            ret = ESP_ERR_NOT_FOUND;
        } else if (image_size > update_partition->size) {
            ESP_LOGE(TAG, "Image too large: %" PRIu32 " bytes > partition %s size %" PRIu32 " bytes",
                     image_size,
                     update_partition->label,
                     update_partition->size);
            ret = ESP_ERR_INVALID_SIZE;
        } else {
            ESP_LOGI(TAG, "Running partition: %s at 0x%08" PRIx32,
                     (running_partition != NULL) ? running_partition->label : "unknown",
                     (running_partition != NULL) ? running_partition->address : 0U);
            ESP_LOGI(TAG, "Writing OTA image to: %s at 0x%08" PRIx32 ", size %" PRIu32 " bytes",
                     update_partition->label,
                     update_partition->address,
                     image_size);

            ret = esp_ota_begin(update_partition, (size_t)image_size, &ctx->handle);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "esp_ota_begin failed: %s", esp_err_to_name(ret));
                memset(ctx, 0, sizeof(*ctx));
            } else {
                ctx->update_partition = update_partition;
                ctx->image_size = image_size;
                ctx->bytes_written = 0U;
                ctx->active = true;
            }
        }
    }

    // 3. Function Return
    return ret;
}

esp_err_t ota_writer_write(ota_writer_context_t *ctx, const uint8_t *data, uint32_t len)
{
    // 1. Variable Initialization
    esp_err_t ret = ESP_OK;

    // 2. Core Execution Logic
    if (ctx == NULL || !ctx->active || ctx->update_partition == NULL) {
        ret = ESP_ERR_INVALID_STATE;
    } else if (data == NULL && len > 0U) {
        ret = ESP_ERR_INVALID_ARG;
    } else if (len == 0U) {
        ret = ESP_OK;
    } else if ((ctx->image_size - ctx->bytes_written) < len) {
        ESP_LOGE(TAG, "OTA write would exceed expected image size");
        ret = ESP_ERR_INVALID_SIZE;
    } else {
        ret = esp_ota_write(ctx->handle, data, (size_t)len);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "esp_ota_write failed at offset %" PRIu32 ": %s",
                     ctx->bytes_written,
                     esp_err_to_name(ret));
        } else {
            ctx->bytes_written += len;
        }
    }

    // 3. Function Return
    return ret;
}

esp_err_t ota_writer_finish(ota_writer_context_t *ctx, bool set_boot_partition)
{
    // 1. Variable Initialization
    const esp_partition_t *update_partition = NULL;
    esp_err_t ret = ESP_OK;

    // 2. Core Execution Logic
    if (ctx == NULL || !ctx->active || ctx->update_partition == NULL) {
        ret = ESP_ERR_INVALID_STATE;
    } else if (ctx->bytes_written != ctx->image_size) {
        ESP_LOGE(TAG, "Incomplete OTA image: wrote %" PRIu32 " of %" PRIu32 " bytes",
                 ctx->bytes_written,
                 ctx->image_size);
        ret = ESP_ERR_INVALID_SIZE;
    } else {
        update_partition = ctx->update_partition;
        ret = esp_ota_end(ctx->handle);
        ctx->active = false;

        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "esp_ota_end failed: %s", esp_err_to_name(ret));
            memset(ctx, 0, sizeof(*ctx));
        } else if (set_boot_partition) {
            ret = esp_ota_set_boot_partition(update_partition);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "esp_ota_set_boot_partition failed: %s", esp_err_to_name(ret));
                memset(ctx, 0, sizeof(*ctx));
            } else {
                ESP_LOGI(TAG, "Next boot partition set to %s", update_partition->label);
                memset(ctx, 0, sizeof(*ctx));
            }
        } else {
            memset(ctx, 0, sizeof(*ctx));
        }
    }

    // 3. Function Return
    return ret;
}

esp_err_t ota_writer_abort(ota_writer_context_t *ctx)
{
    // 1. Variable Initialization
    esp_err_t ret = ESP_OK;

    // 2. Core Execution Logic
    if (ctx == NULL) {
        ret = ESP_ERR_INVALID_ARG;
    } else if (ctx->active) {
        ret = esp_ota_abort(ctx->handle);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "esp_ota_abort failed: %s", esp_err_to_name(ret));
        } else {
            memset(ctx, 0, sizeof(*ctx));
        }
    } else {
        memset(ctx, 0, sizeof(*ctx));
    }

    // 3. Function Return
    return ret;
}

esp_err_t ota_writer_mark_running_valid(void)
{
    // 1. Variable Initialization
    esp_err_t ret = esp_ota_mark_app_valid_cancel_rollback();

    // 2. Core Execution Logic
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Mark running app valid returned: %s", esp_err_to_name(ret));
    }

    // 3. Function Return
    return ret;
}

esp_err_t ota_writer_mark_running_invalid_and_rollback(void)
{
    // 1. Variable Initialization
    esp_err_t ret = ESP_OK;

    // 2. Core Execution Logic
    ret = esp_ota_mark_app_invalid_rollback_and_reboot();

    // 3. Function Return
    return ret;
}

void ota_writer_restart(void)
{
    // 1. Variable Initialization

    // 2. Core Execution Logic
    ESP_LOGI(TAG, "Restarting to boot selected OTA partition");
    esp_restart();
}
