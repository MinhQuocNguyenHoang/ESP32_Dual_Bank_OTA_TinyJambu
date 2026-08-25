/**
 * @file glucose_monitor.h
 * @brief Native ESP-IDF Non-Invasive Glucose Monitor Core System Interface.
 * @details Manages FSM execution, PPG feature extraction, Edge AI inference, 
 *          TinyJAMBU-128 AEAD encryption, and native ESP-MQTT telemetry.
 *          Conforms to embedded MISRA-C standards with strict fixed-width integer types and Doxygen documentation.
 */

#ifndef GLUCOSE_MONITOR_H
#define GLUCOSE_MONITOR_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the native ESP-IDF Glucose Monitor System.
 * @details Initializes NVS storage, MAX30102 I2C sensor, SSD1306 OLED display,
 *          WiFi Station networking, and ESP-MQTT client.
 * @param None
 * @return None
 */
void glucose_monitor_init(void);

/**
 * @brief Main periodic task process for Glucose Monitor state machine execution.
 * @details Handles real-time finger detection, PPG buffer sampling, Edge AI prediction,
 *          TinyJAMBU encryption, OLED rendering, and MQTT telemetry upload with ACK verification.
 * @param None
 * @return None
 */
void glucose_monitor_process(void);

#ifdef __cplusplus
}
#endif

#endif // GLUCOSE_MONITOR_H
