/**
 * @file ssd1306.h
 * @brief Native ESP-IDF I2C OLED Driver Header with Adafruit-compatible Framebuffer & Graphics Engine.
 * @details Conforms to embedded MISRA-C standards with strict fixed-width integer types and Doxygen documentation.
 */

#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>
#include "esp_err.h"
#include "driver/i2c.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define SSD1306_I2C_ADDR 0x3C
#define SSD1306_WIDTH 128
#define SSD1306_HEIGHT 64

#define SSD1306_COLOR_BLACK 0
#define SSD1306_COLOR_WHITE 1
#define SSD1306_COLOR_INVERT 2

    /**
     * @brief Initializes SSD1306 OLED display over ESP-IDF I2C driver.
     * @param[in] i2c_num I2C port number (e.g. I2C_NUM_0).
     * @return esp_err_t ESP_OK on success, error code otherwise.
     */
    esp_err_t ssd1306_init(i2c_port_t i2c_num);

    /**
     * @brief Clears the internal display framebuffer memory.
     * @param None
     * @return None
     */
    void ssd1306_clear(void);

    /**
     * @brief Flushes internal display framebuffer to physical OLED screen via I2C.
     * @param None
     * @return None
     */
    void ssd1306_update(void);

    /**
     * @brief Sets a single pixel color at coordinate (x, y).
     * @param[in] x Horizontal pixel coordinate (0 - 127).
     * @param[in] y Vertical pixel coordinate (0 - 63).
     * @param[in] color Color value (SSD1306_COLOR_BLACK, SSD1306_COLOR_WHITE, SSD1306_COLOR_INVERT).
     * @return None
     */
    void ssd1306_draw_pixel(int16_t x, int16_t y, uint8_t color);

    /**
     * @brief Draws a fast horizontal line.
     * @param[in] x Starting X coordinate.
     * @param[in] y Starting Y coordinate.
     * @param[in] w Width in pixels.
     * @param[in] color Color value.
     * @return None
     */
    void ssd1306_draw_fast_h_line(int16_t x, int16_t y, int16_t w, uint8_t color);

    /**
     * @brief Draws a fast vertical line.
     * @param[in] x Starting X coordinate.
     * @param[in] y Starting Y coordinate.
     * @param[in] h Height in pixels.
     * @param[in] color Color value.
     * @return None
     */
    void ssd1306_draw_fast_v_line(int16_t x, int16_t y, int16_t h, uint8_t color);

    /**
     * @brief Draws an outline rectangle.
     * @param[in] x Starting X coordinate.
     * @param[in] y Starting Y coordinate.
     * @param[in] w Rectangle width.
     * @param[in] h Rectangle height.
     * @param[in] color Color value.
     * @return None
     */
    void ssd1306_draw_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color);

    /**
     * @brief Draws a filled rectangle.
     * @param[in] x Starting X coordinate.
     * @param[in] y Starting Y coordinate.
     * @param[in] w Rectangle width.
     * @param[in] h Rectangle height.
     * @param[in] color Color value.
     * @return None
     */
    void ssd1306_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color);

    /**
     * @brief Sets the text cursor coordinates in pixel units.
     * @param[in] x Horizontal cursor coordinate.
     * @param[in] y Vertical cursor coordinate.
     * @return None
     */
    void ssd1306_set_cursor(int16_t x, int16_t y);

    /**
     * @brief Sets the font scaling factor (1 = 5x7, 2 = 10x14).
     * @param[in] size Font multiplier size.
     * @return None
     */
    void ssd1306_set_text_size(uint8_t size);

    /**
     * @brief Sets text foreground and background colors.
     * @param[in] color Foreground text color.
     * @param[in] bg Background color.
     * @return None
     */
    void ssd1306_set_text_color(uint8_t color, uint8_t bg);

    /**
     * @brief Draws a single ASCII character at specified pixel coordinates with custom scaling.
     * @param[in] x Starting X coordinate.
     * @param[in] y Starting Y coordinate.
     * @param[in] c ASCII character byte.
     * @param[in] color Foreground color.
     * @param[in] bg Background color.
     * @param[in] size Scaling multiplier.
     * @return None
     */
    void ssd1306_draw_char(int16_t x, int16_t y, unsigned char c, uint8_t color, uint8_t bg, uint8_t size);

    /**
     * @brief Prints a single character at current cursor position.
     * @param[in] c Character byte to output.
     * @return None
     */
    void ssd1306_write(char c);

    /**
     * @brief Prints a null-terminated string at current cursor position.
     * @param[in] str Pointer to constant character string.
     * @return None
     */
    void ssd1306_print(const char *str);

    /**
     * @brief Prints a null-terminated string followed by a newline at current cursor position.
     * @param[in] str Pointer to constant character string.
     * @return None
     */
    void ssd1306_println(const char *str);

    /**
     * @brief Formatted printf output to OLED display.
     * @param[in] format Format string.
     * @param[in] ... Variable arguments list.
     * @return None
     */
    void ssd1306_printf(const char *format, ...);

    /**
     * @brief Renders a graphical progress bar matching the clinical UI.
     * @param[in] x Starting X coordinate.
     * @param[in] y Starting Y coordinate.
     * @param[in] width Total progress bar width.
     * @param[in] height Total progress bar height.
     * @param[in] progress Percentage progress value (0 - 100).
     * @return None
     */
    void ssd1306_draw_progress_bar(int32_t x, int32_t y, int32_t width, int32_t height, int32_t progress);

    /**
     * @brief Draws the standard project author footer ("By: M.Quoc").
     * @param None
     * @return None
     */
    void ssd1306_draw_footer(void);

#ifdef __cplusplus
}
#endif

#endif // SSD1306_H
