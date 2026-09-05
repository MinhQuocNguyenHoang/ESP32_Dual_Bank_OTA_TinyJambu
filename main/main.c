/**
 * @file main.c
 * @brief Main Entry Point for Native ESP-IDF Glucose Monitor Application.
 * @details Conforms to embedded MISRA-C standards with strict fixed-width integer types and Doxygen documentation.
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "nvs_flash.h"
#include "nvs_store.h"
#include "ota_controller.h"
#include "ota_writer.h"

#if APP_OTA_TEST_MODE
#include "ota_test_app.h"
#else
#include "glucose_monitor.h"
#endif

static const char *TAG = "MAIN_APP_IDF";

#define OTA_TASK_STACK_SIZE 8192U
#define OTA_TASK_PRIORITY 4U
#define OTA_UART_RX_BUFFER_SIZE 4096U
#define OTA_UART_TX_BUFFER_SIZE 512U
#define OTA_UART_EVENT_QUEUE_SIZE 20U
#define OTA_UART_BAUD_RATE 115200
#define OTA_PACKET_TIMEOUT_MS 200U
#define OTA_REBOOT_DELAY_MS 1000U

static esp_err_t app_nvs_init(void)
{
    esp_err_t ret = ESP_OK;

    ret = nvs_flash_init();
    if ((ret == ESP_ERR_NVS_NO_FREE_PAGES) || (ret == ESP_ERR_NVS_NEW_VERSION_FOUND))
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    return ret;
}

static esp_err_t ota_running_partition_is_pending(bool *pending)
{
    const esp_partition_t *running_partition = NULL;
    esp_ota_img_states_t ota_state = ESP_OTA_IMG_UNDEFINED;
    esp_err_t ret = ESP_OK;

    if (pending == NULL)
    {
        ret = ESP_ERR_INVALID_ARG;
    }
    else
    {
        *pending = false;
        running_partition = esp_ota_get_running_partition();
        if (running_partition == NULL)
        {
            ret = ESP_FAIL;
        }
        else
        {
            ret = esp_ota_get_state_partition(running_partition, &ota_state);
            if (ret == ESP_OK)
            {
                *pending = (ota_state == ESP_OTA_IMG_PENDING_VERIFY);
            }
            else if ((ret == ESP_ERR_NOT_FOUND) || (ret == ESP_ERR_NOT_SUPPORTED))
            {
                ret = ESP_OK;
            }
            else
            {
                /* Keep the original OTA state error for the caller. */
            }
        }
    }

    return ret;
}

static esp_err_t ota_boot_health_check(void)
{
    bool nvs_pending = false;
    bool ota_pending = false;
    uint32_t boot_count = 0U;
    esp_err_t ret = ESP_OK;

    ret = nvs_store_is_ota_pending(&nvs_pending);
    if (ret == ESP_OK)
    {
        ret = ota_running_partition_is_pending(&ota_pending);
    }

    if (ret == ESP_OK)
    {
        if (nvs_pending || ota_pending)
        {
            if (!nvs_pending)
            {
                ret = nvs_store_set_ota_pending();
            }

            if (ret == ESP_OK)
            {
                ret = nvs_store_increment_ota_boot_count(&boot_count);
            }

            if (ret == ESP_OK)
            {
                ESP_LOGW(TAG,
                         "OTA image pending validation, boot attempt %" PRIu32 "/%u",
                         boot_count,
                         (unsigned int)NVS_STORE_MAX_OTA_BOOT_ATTEMPTS);

                if (boot_count > NVS_STORE_MAX_OTA_BOOT_ATTEMPTS)
                {
                    ESP_LOGE(TAG, "OTA boot attempts exceeded, rolling back");
                    ret = ota_writer_mark_running_invalid_and_rollback();
                    if (ret != ESP_OK)
                    {
                        ESP_LOGE(TAG, "Rollback failed: %s", esp_err_to_name(ret));
                    }
                }
            }
        }
        else
        {
            ret = nvs_store_get_ota_boot_count(&boot_count);
            if ((ret == ESP_OK) && (boot_count > 0U))
            {
                ret = nvs_store_reset_ota_boot_count();
            }
        }
    }

    return ret;
}

static esp_err_t ota_confirm_running_app_if_pending(void)
{
    bool nvs_pending = false;
    bool ota_pending = false;
    esp_err_t ret = ESP_OK;

    ret = nvs_store_is_ota_pending(&nvs_pending);
    if (ret == ESP_OK)
    {
        ret = ota_running_partition_is_pending(&ota_pending);
    }

    if ((ret == ESP_OK) && (nvs_pending || ota_pending))
    {
        ret = ota_writer_mark_running_valid();
        if (ret == ESP_OK)
        {
            ret = nvs_store_clear_ota_pending();
        }

        if (ret == ESP_OK)
        {
            ESP_LOGI(TAG, "OTA image confirmed valid");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to confirm OTA image: %s", esp_err_to_name(ret));
        }
    }

    return ret;
}

static void ota_task(void *arg)
{
    ota_controller_config_t ota_config = {0};
    esp_err_t ret = ESP_OK;

    (void)arg;

    ota_config.uart_config.uart_num = UART_NUM_2;
    ota_config.uart_config.tx_pin = GPIO_NUM_27;
    ota_config.uart_config.rx_pin = GPIO_NUM_26;
    ota_config.uart_config.baud_rate = OTA_UART_BAUD_RATE;
    ota_config.uart_config.rx_buffer_size = OTA_UART_RX_BUFFER_SIZE;
    ota_config.uart_config.tx_buffer_size = OTA_UART_TX_BUFFER_SIZE;
    ota_config.uart_config.event_queue_size = OTA_UART_EVENT_QUEUE_SIZE;
    ota_config.packet_timeout_ticks = pdMS_TO_TICKS(OTA_PACKET_TIMEOUT_MS);
    ota_config.reboot_delay_ticks = pdMS_TO_TICKS(OTA_REBOOT_DELAY_MS);
    ota_config.reboot_on_success = true;

    ret = ota_controller_init(&ota_config);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "OTA task init failed: %s", esp_err_to_name(ret));
    }
    else
    {
        ESP_LOGI(TAG, "OTA task started on UART2 RX=%d TX=%d",
                 (int)ota_config.uart_config.rx_pin,
                 (int)ota_config.uart_config.tx_pin);
    }

    while (ret == ESP_OK)
    {
        ret = ota_controller_process_once();
        if (ret == ESP_ERR_TIMEOUT)
        {
            ret = ESP_OK;
        }
        else if (ret != ESP_OK)
        {
            ESP_LOGW(TAG, "OTA packet handling returned: %s", esp_err_to_name(ret));
            ret = ESP_OK;
        }
        else
        {
            ESP_LOGI(TAG, "OTA packet handled successfully");
        }
    }

    ota_controller_deinit();
    vTaskDelete(NULL);
}

void app_main(void)
{
    const TickType_t delay_ticks = pdMS_TO_TICKS(10);
    BaseType_t task_created = pdFALSE;
    esp_err_t ret = ESP_OK;

    ESP_LOGI(TAG, "Starting ESP-IDF Non-Invasive Glucose Monitoring Platform...");
    ret = app_nvs_init();
    ESP_ERROR_CHECK(ret);

    ret = ota_boot_health_check();
    if (ret != ESP_OK)
    {
        ESP_LOGW(TAG, "OTA boot health check returned: %s", esp_err_to_name(ret));
    }

#if APP_OTA_TEST_MODE
    ota_test_app_init();
#else
    glucose_monitor_init();
#endif

    ret = ota_confirm_running_app_if_pending();
    if (ret != ESP_OK)
    {
        ESP_LOGW(TAG, "OTA validation confirm returned: %s", esp_err_to_name(ret));
    }

    task_created = xTaskCreate(ota_task,
                               "ota_task",
                               OTA_TASK_STACK_SIZE,
                               NULL,
                               OTA_TASK_PRIORITY,
                               NULL);
    if (task_created != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create OTA task");
    }

    while (1)
    {
#if APP_OTA_TEST_MODE
        ota_test_app_process();
#else
        glucose_monitor_process();
#endif
        vTaskDelay(delay_ticks);
    }
}
