/**
 * @file max30102.h
 * @brief Native ESP-IDF I2C Driver Header for MAX30102 Pulse Oximeter Sensor.
 * @details Conforms to embedded MISRA-C standards with strict fixed-width integer types and Doxygen documentation.
 */

#ifndef MAX30102_H
#define MAX30102_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "driver/i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX30102_I2C_ADDR         0x57
#define MAX30102_REG_INTR_STATUS1 0x00
#define MAX30102_REG_INTR_ENABLE1 0x02
#define MAX30102_REG_FIFO_WR_PTR  0x04
#define MAX30102_REG_FIFO_OVF_CTR 0x05
#define MAX30102_REG_FIFO_RD_PTR  0x06
#define MAX30102_REG_FIFO_DATA    0x07
#define MAX30102_REG_FIFO_CONFIG  0x08
#define MAX30102_REG_MODE_CONFIG  0x09
#define MAX30102_REG_SPO2_CONFIG  0x0A
#define MAX30102_REG_LED1_PA      0x0C // Red LED
#define MAX30102_REG_LED2_PA      0x0D // IR LED

/**
 * @brief Structure containing raw MAX30102 Red and IR optical samples.
 */
typedef struct {
    uint32_t red;
    uint32_t ir;
} max30102_sample_t;

/**
 * @brief Initializes MAX30102 sensor over ESP-IDF I2C driver.
 * @param[in] i2c_num I2C port number (e.g. I2C_NUM_0).
 * @param[in] sda_pin GPIO pin number for I2C SDA (e.g. GPIO_NUM_32).
 * @param[in] scl_pin GPIO pin number for I2C SCL (e.g. GPIO_NUM_33).
 * @return esp_err_t ESP_OK on success, error code otherwise.
 */
esp_err_t max30102_init(i2c_port_t i2c_num, gpio_num_t sda_pin, gpio_num_t scl_pin);

/**
 * @brief Polls the MAX30102 FIFO buffer and stores new samples in circular buffer.
 * @param None
 * @return uint16_t Number of newly acquired samples.
 */
uint16_t max30102_check(void);

/**
 * @brief Reads the latest Red and IR FIFO samples from MAX30102.
 * @param[out] sample Pointer to max30102_sample_t structure to receive optical data.
 * @return esp_err_t ESP_OK on success, ESP_ERR_INVALID_ARG if NULL.
 */
esp_err_t max30102_read_fifo(max30102_sample_t *sample);

/**
 * @brief Gets current IR raw reading for real-time finger placement detection.
 * @param[out] ir_val Pointer to store uint32_t IR raw intensity.
 * @return esp_err_t ESP_OK on success, ESP_ERR_INVALID_ARG if NULL.
 */
esp_err_t max30102_get_ir(uint32_t *ir_val);

/**
 * @brief Resets MAX30102 FIFO read and write pointers to zero.
 * @param None
 * @return esp_err_t ESP_OK on success, error code otherwise.
 */
esp_err_t max30102_reset_fifo(void);

#ifdef __cplusplus
}
#endif

#endif // MAX30102_H
