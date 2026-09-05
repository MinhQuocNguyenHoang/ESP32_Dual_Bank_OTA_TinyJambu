/**
 * @file max30102.c
 * @brief Implementation of Native ESP-IDF MAX30102 I2C Driver with Circular Buffer and FIFO Chunk Streaming.
 * @details Conforms to embedded MISRA-C standards with strict fixed-width integer types and Doxygen documentation.
 */

#include "max30102.h"
#include <string.h>
#include "esp_log.h"

#define MAX30102_BUFFER_SIZE 32

typedef struct
{
    uint32_t red[MAX30102_BUFFER_SIZE];
    uint32_t ir[MAX30102_BUFFER_SIZE];
    uint8_t head;
    uint8_t tail;
} max30102_circular_buffer_t;

static const char *TAG = "MAX30102";
static i2c_port_t g_i2c_port = I2C_NUM_0;
static max30102_circular_buffer_t s_fifo = {};

/**
 * @brief Writes an 8-bit value to a MAX30102 internal register.
 * @param[in] reg Register address byte.
 * @param[in] val Data byte to write.
 * @return esp_err_t ESP_OK on success, error code otherwise.
 */
static esp_err_t max30102_write_reg(uint8_t reg, uint8_t val)
{
    esp_err_t ret = ESP_OK;
    uint8_t write_buf[2] = {reg, val};
    const TickType_t timeout_ticks = pdMS_TO_TICKS(50);

    ret = i2c_master_write_to_device(g_i2c_port, MAX30102_I2C_ADDR, write_buf, sizeof(write_buf), timeout_ticks);

    return ret;
}

esp_err_t max30102_reset_fifo(void)
{
    esp_err_t ret = ESP_OK;

    max30102_write_reg(MAX30102_REG_FIFO_WR_PTR, 0x00);
    max30102_write_reg(MAX30102_REG_FIFO_OVF_CTR, 0x00);
    ret = max30102_write_reg(MAX30102_REG_FIFO_RD_PTR, 0x00);

    return ret;
}

esp_err_t max30102_init(i2c_port_t i2c_num, gpio_num_t sda_pin, gpio_num_t scl_pin)
{
    esp_err_t ret = ESP_OK;
    i2c_config_t conf = {};
    uint8_t reg = 0;
    uint8_t mode = 0;
    uint32_t wait_count = 0;
    const TickType_t timeout_ticks = pdMS_TO_TICKS(50);

    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = sda_pin;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = scl_pin;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 400000;

    g_i2c_port = i2c_num;
    memset(&s_fifo, 0, sizeof(s_fifo));

    ret = i2c_param_config(i2c_num, &conf);
    if (ret != ESP_OK)
    {
        return ret;
    }

    ret = i2c_driver_install(i2c_num, conf.mode, 0, 0, 0);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE)
    {
        return ret;
    }

    // Reset sensor
    max30102_write_reg(MAX30102_REG_MODE_CONFIG, 0x40);

    // Wait for reset bit to clear (POR completion)
    reg = MAX30102_REG_MODE_CONFIG;
    while (wait_count++ < 20)
    {
        vTaskDelay(pdMS_TO_TICKS(5));
        ret = i2c_master_write_read_device(g_i2c_port, MAX30102_I2C_ADDR, &reg, 1, &mode, 1, timeout_ticks);
        if ((ret == ESP_OK) && ((mode & 0x40) == 0))
        {
            break;
        }
    }

    // Configure FIFO: Sample avg 4 (0x40) | Rollover Enable (0x10) -> 0x50
    max30102_write_reg(MAX30102_REG_FIFO_CONFIG, 0x50);

    // Mode: Red + IR SpO2 mode (0x03)
    max30102_write_reg(MAX30102_REG_MODE_CONFIG, 0x03);

    // SPO2 Config: ADC Range 4096 (0x20) | Sample Rate 100Hz (0x04) | Pulse Width 411us (0x03) -> 0x27
    max30102_write_reg(MAX30102_REG_SPO2_CONFIG, 0x27);

    // LED Pulse Amplitude: 0x1F (6.4mA)
    max30102_write_reg(MAX30102_REG_LED1_PA, 0x1F); // Red
    max30102_write_reg(MAX30102_REG_LED2_PA, 0x1F); // IR

    // Multi-LED Slot Configuration (Slot 1 = Red, Slot 2 = IR)
    max30102_write_reg(0x11, 0x21);

    // Clear FIFO pointers
    max30102_reset_fifo();

    ESP_LOGI(TAG, "MAX30102 initialized successfully on SDA=%d, SCL=%d", (int32_t)sda_pin, (int32_t)scl_pin);
    ret = ESP_OK;

    return ret;
}

uint16_t max30102_check(void)
{
    uint8_t reg = 0;
    uint8_t read_ptr = 0;
    uint8_t write_ptr = 0;
    int32_t num_samples = 0;
    int32_t bytes_to_read = 0;
    int32_t to_get = 0;
    uint8_t raw_buf[30] = {0}; // 5 samples * 6 bytes = 30 bytes per chunk
    esp_err_t ret = ESP_OK;
    int32_t i = 0;
    uint32_t red = 0;
    uint32_t ir = 0;
    uint16_t total_samples_read = 0;
    const TickType_t timeout_ticks = pdMS_TO_TICKS(50);

    // Read Write Pointer
    reg = MAX30102_REG_FIFO_WR_PTR;
    ret = i2c_master_write_read_device(g_i2c_port, MAX30102_I2C_ADDR, &reg, 1, &write_ptr, 1, timeout_ticks);
    if (ret != ESP_OK)
    {
        return 0;
    }

    // Read Read Pointer
    reg = MAX30102_REG_FIFO_RD_PTR;
    ret = i2c_master_write_read_device(g_i2c_port, MAX30102_I2C_ADDR, &reg, 1, &read_ptr, 1, timeout_ticks);
    if (ret != ESP_OK)
    {
        return 0;
    }

    if (read_ptr == write_ptr)
    {
        return 0;
    }

    num_samples = (int32_t)(write_ptr - read_ptr);
    if (num_samples < 0)
    {
        num_samples += 32;
    }

    bytes_to_read = num_samples * 6;

    // Read available samples in safe I2C chunks
    while (bytes_to_read > 0)
    {
        to_get = (bytes_to_read > (int32_t)sizeof(raw_buf)) ? (int32_t)sizeof(raw_buf) : bytes_to_read;
        to_get = to_get - (to_get % 6);
        if (to_get == 0)
        {
            break;
        }

        reg = MAX30102_REG_FIFO_DATA;
        ret = i2c_master_write_read_device(g_i2c_port, MAX30102_I2C_ADDR, &reg, 1, raw_buf, to_get, timeout_ticks);
        if (ret != ESP_OK)
        {
            break;
        }

        bytes_to_read -= to_get;
        num_samples = to_get / 6;

        for (i = 0; i < num_samples; i++)
        {
            s_fifo.head = (uint8_t)((s_fifo.head + 1) % MAX30102_BUFFER_SIZE);

            red = ((uint32_t)raw_buf[i * 6 + 0] << 16) |
                  ((uint32_t)raw_buf[i * 6 + 1] << 8) |
                  ((uint32_t)raw_buf[i * 6 + 2]);
            red &= 0x03FFFF;

            ir = ((uint32_t)raw_buf[i * 6 + 3] << 16) |
                 ((uint32_t)raw_buf[i * 6 + 4] << 8) |
                 ((uint32_t)raw_buf[i * 6 + 5]);
            ir &= 0x03FFFF;

            s_fifo.red[s_fifo.head] = red;
            s_fifo.ir[s_fifo.head] = ir;
            total_samples_read++;
        }
    }

    return total_samples_read;
}

esp_err_t max30102_read_fifo(max30102_sample_t *sample)
{

    if (sample == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    max30102_check();
    sample->red = s_fifo.red[s_fifo.head];
    sample->ir = s_fifo.ir[s_fifo.head];

    return ESP_OK;
}

esp_err_t max30102_get_ir(uint32_t *ir_val)
{

    if (ir_val == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    max30102_check();
    *ir_val = s_fifo.ir[s_fifo.head];

    return ESP_OK;
}
