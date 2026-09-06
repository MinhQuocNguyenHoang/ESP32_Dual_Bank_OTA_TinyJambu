/**
 * @file nvs_store.c
 * @brief NVS storage implementation for OTA rollback bookkeeping.
 */

#include "nvs_store.h"

#include "nvs.h"

#define NVS_STORE_NAMESPACE "ota_store"
#define NVS_STORE_KEY_PENDING "pending"
#define NVS_STORE_KEY_BOOT_COUNT "boot_cnt"
#define NVS_STORE_PENDING_TRUE 1U
#define NVS_STORE_PENDING_FALSE 0U

static esp_err_t nvs_store_open(nvs_open_mode_t mode, nvs_handle_t *handle)
{
    esp_err_t ret = ESP_OK;

    if (handle == NULL)
    {
        ret = ESP_ERR_INVALID_ARG;
    }
    else
    {
        ret = nvs_open(NVS_STORE_NAMESPACE, mode, handle);
    }

    return ret;
}

static esp_err_t nvs_store_read_u32(const char *key, uint32_t default_value, uint32_t *value)
{
    nvs_handle_t handle = 0U;
    esp_err_t ret = ESP_OK;

    if ((key == NULL) || (value == NULL))
    {
        ret = ESP_ERR_INVALID_ARG;
    }
    else
    {
        ret = nvs_store_open(NVS_READONLY, &handle);
        if (ret == ESP_ERR_NVS_NOT_FOUND)
        {
            *value = default_value;
            ret = ESP_OK;
        }
        if (ret == ESP_OK)
        {
            ret = nvs_get_u32(handle, key, value);
            if (ret == ESP_ERR_NVS_NOT_FOUND)
            {
                *value = default_value;
                ret = ESP_OK;
            }
        }

        if (handle != 0U)
        {
            nvs_close(handle);
        }
    }

    return ret;
}

static esp_err_t nvs_store_write_u32(const char *key, uint32_t value)
{
    nvs_handle_t handle = 0U;
    esp_err_t ret = ESP_OK;

    if (key == NULL)
    {
        ret = ESP_ERR_INVALID_ARG;
    }
    else
    {
        ret = nvs_store_open(NVS_READWRITE, &handle);
        if (ret == ESP_OK)
        {
            ret = nvs_set_u32(handle, key, value);
            if (ret == ESP_OK)
            {
                ret = nvs_commit(handle);
            }
        }

        if (handle != 0U)
        {
            nvs_close(handle);
        }
    }

    return ret;
}

esp_err_t nvs_store_set_ota_pending(void)
{
    esp_err_t ret = ESP_OK;

    ret = nvs_store_write_u32(NVS_STORE_KEY_PENDING, NVS_STORE_PENDING_TRUE);
    if (ret == ESP_OK)
    {
        ret = nvs_store_reset_ota_boot_count();
    }

    return ret;
}

esp_err_t nvs_store_clear_ota_pending(void)
{
    esp_err_t ret = ESP_OK;

    ret = nvs_store_write_u32(NVS_STORE_KEY_PENDING, NVS_STORE_PENDING_FALSE);
    if (ret == ESP_OK)
    {
        ret = nvs_store_reset_ota_boot_count();
    }

    return ret;
}

esp_err_t nvs_store_is_ota_pending(bool *pending)
{
    uint32_t raw_pending = NVS_STORE_PENDING_FALSE;
    esp_err_t ret = ESP_OK;

    if (pending == NULL)
    {
        ret = ESP_ERR_INVALID_ARG;
    }
    else
    {
        ret = nvs_store_read_u32(NVS_STORE_KEY_PENDING, NVS_STORE_PENDING_FALSE, &raw_pending);
        if (ret == ESP_OK)
        {
            *pending = (raw_pending == NVS_STORE_PENDING_TRUE);
        }
    }

    return ret;
}

esp_err_t nvs_store_get_ota_boot_count(uint32_t *count)
{
    esp_err_t ret = ESP_OK;

    ret = nvs_store_read_u32(NVS_STORE_KEY_BOOT_COUNT, 0U, count);

    return ret;
}

esp_err_t nvs_store_increment_ota_boot_count(uint32_t *count)
{
    uint32_t current_count = 0U;
    esp_err_t ret = ESP_OK;

    ret = nvs_store_get_ota_boot_count(&current_count);
    if (ret == ESP_OK)
    {
        current_count++;
        ret = nvs_store_write_u32(NVS_STORE_KEY_BOOT_COUNT, current_count);
    }

    if ((ret == ESP_OK) && (count != NULL))
    {
        *count = current_count;
    }

    return ret;
}

esp_err_t nvs_store_reset_ota_boot_count(void)
{
    esp_err_t ret = ESP_OK;

    ret = nvs_store_write_u32(NVS_STORE_KEY_BOOT_COUNT, 0U);

    return ret;
}
