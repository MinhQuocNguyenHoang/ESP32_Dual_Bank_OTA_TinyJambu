/**
 * @file main.cpp
 * @brief Main Entry Point for Native ESP-IDF Glucose Monitor Application.
 * @details Conforms to embedded MISRA-C standards with strict fixed-width integer types and Doxygen documentation.
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "glucose_monitor.h"

static const char *TAG = "MAIN_APP_IDF";

/**
 * @brief Application Main Entry Point for ESP-IDF Framework.
 * @param None
 * @return None
 */
extern "C" void app_main(void)
{
    // 1. Variable Initialization
    const TickType_t delay_ticks = pdMS_TO_TICKS(10);

    // 2. Core Execution Logic
    ESP_LOGI(TAG, "Starting ESP-IDF Non-Invasive Glucose Monitoring Platform...");
    glucose_monitor_init();

    while (1) {
        glucose_monitor_process();
        vTaskDelay(delay_ticks);
    }
}
