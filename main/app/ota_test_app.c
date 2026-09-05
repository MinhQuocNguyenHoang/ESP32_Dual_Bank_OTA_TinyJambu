/**
 * @file ota_test_app.c
 * @brief Minimal application used to validate UART OTA updates.
 */

#include "ota_test_app.h"

#include <inttypes.h>
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifndef APP_OTA_TEST_VERSION
#define APP_OTA_TEST_VERSION "ota-test"
#endif

static const char *TAG = "OTA_TEST_APP";
static uint32_t s_heartbeat_count = 0U;

void ota_test_app_init(void)
{
    const esp_partition_t *running_partition = NULL;

    running_partition = esp_ota_get_running_partition();

    ESP_LOGI(TAG, "Minimal OTA test firmware started");
    ESP_LOGI(TAG, "Version: %s", APP_OTA_TEST_VERSION);
    ESP_LOGI(TAG,
             "Running partition: %s at 0x%08" PRIx32,
             (running_partition != NULL) ? running_partition->label : "unknown",
             (running_partition != NULL) ? running_partition->address : 0U);
}

void ota_test_app_process(void)
{
    s_heartbeat_count++;

    if ((s_heartbeat_count % 200U) == 0U)
    {
        ESP_LOGI(TAG,
                 "OTA test heartbeat=%" PRIu32 ", free_heap=%u bytes, version=%s",
                 s_heartbeat_count / 200U,
                 (unsigned int)heap_caps_get_free_size(MALLOC_CAP_DEFAULT),
                 APP_OTA_TEST_VERSION);
    }
}
