/**
 * @file ssd1306.c
 * @brief Native ESP-IDF Implementation of SSD1306 OLED Driver with Full Graphics & Adafruit-GFX Emulation.
 * @details Conforms to embedded MISRA-C standards with strict fixed-width integer types and Doxygen documentation.
 */

#include "ssd1306.h"
#include <string.h>
#include <stdio.h>
#include "esp_log.h"

static const char *TAG = "SSD1306";
static i2c_port_t g_i2c_port = I2C_NUM_0;

// 128x64 display framebuffer memory (1024 bytes)
static uint8_t s_buffer[SSD1306_WIDTH * (SSD1306_HEIGHT / 8)];

// Text rendering state variables
static int16_t s_cursor_x = 0;
static int16_t s_cursor_y = 0;
static uint8_t s_text_size = 1;
static uint8_t s_text_color = SSD1306_COLOR_WHITE;
static uint8_t s_text_bg = SSD1306_COLOR_BLACK;

// Complete standard 5x7 ASCII font table (0x20 to 0x7E)
static const uint8_t font5x7[95][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // 0x20 (Space)
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // 0x21 !
    {0x00, 0x07, 0x00, 0x07, 0x00}, // 0x22 "
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // 0x23 #
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // 0x24 $
    {0x23, 0x13, 0x08, 0x64, 0x62}, // 0x25 %
    {0x36, 0x49, 0x55, 0x22, 0x50}, // 0x26 &
    {0x00, 0x05, 0x03, 0x00, 0x00}, // 0x27 '
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // 0x28 (
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // 0x29 )
    {0x14, 0x08, 0x3E, 0x08, 0x14}, // 0x2A *
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // 0x2B +
    {0x00, 0x50, 0x30, 0x00, 0x00}, // 0x2C ,
    {0x08, 0x08, 0x08, 0x08, 0x08}, // 0x2D -
    {0x00, 0x60, 0x60, 0x00, 0x00}, // 0x2E .
    {0x20, 0x10, 0x08, 0x04, 0x02}, // 0x2F /
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0x30 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 0x31 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 0x32 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 0x33 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 0x34 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 0x35 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 0x36 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 0x37 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 0x38 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 0x39 9
    {0x00, 0x36, 0x36, 0x00, 0x00}, // 0x3A :
    {0x00, 0x56, 0x36, 0x00, 0x00}, // 0x3B ;
    {0x08, 0x14, 0x22, 0x41, 0x00}, // 0x3C <
    {0x14, 0x14, 0x14, 0x14, 0x14}, // 0x3D =
    {0x00, 0x41, 0x22, 0x14, 0x08}, // 0x3E >
    {0x02, 0x01, 0x51, 0x09, 0x06}, // 0x3F ?
    {0x32, 0x49, 0x79, 0x41, 0x3E}, // 0x40 @
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // 0x41 A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // 0x42 B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // 0x43 C
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // 0x44 D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // 0x45 E
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // 0x46 F
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // 0x47 G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // 0x48 H
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // 0x49 I
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // 0x4A J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // 0x4B K
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // 0x4C L
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // 0x4D M
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // 0x4E N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // 0x4F O
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // 0x50 P
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // 0x51 Q
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // 0x52 R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // 0x53 S
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // 0x54 T
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // 0x55 U
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // 0x56 V
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, // 0x57 W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // 0x58 X
    {0x07, 0x08, 0x70, 0x08, 0x07}, // 0x59 Y
    {0x61, 0x51, 0x49, 0x45, 0x43}, // 0x5A Z
    {0x00, 0x7F, 0x41, 0x41, 0x00}, // 0x5B [
    {0x02, 0x04, 0x08, 0x10, 0x20}, // 0x5C (Backslash)
    {0x00, 0x41, 0x41, 0x7F, 0x00}, // 0x5D ]
    {0x04, 0x02, 0x01, 0x02, 0x04}, // 0x5E ^
    {0x40, 0x40, 0x40, 0x40, 0x40}, // 0x5F _
    {0x00, 0x01, 0x02, 0x04, 0x00}, // 0x60 `
    {0x20, 0x54, 0x54, 0x54, 0x78}, // 0x61 a
    {0x7F, 0x48, 0x44, 0x44, 0x38}, // 0x62 b
    {0x38, 0x44, 0x44, 0x44, 0x20}, // 0x63 c
    {0x38, 0x44, 0x44, 0x48, 0x7F}, // 0x64 d
    {0x38, 0x54, 0x54, 0x54, 0x18}, // 0x65 e
    {0x08, 0x7E, 0x09, 0x01, 0x02}, // 0x66 f
    {0x08, 0x14, 0x54, 0x54, 0x3C}, // 0x67 g
    {0x7F, 0x08, 0x04, 0x04, 0x78}, // 0x68 h
    {0x00, 0x44, 0x7D, 0x40, 0x00}, // 0x69 i
    {0x20, 0x40, 0x44, 0x3D, 0x00}, // 0x6A j
    {0x7F, 0x10, 0x28, 0x44, 0x00}, // 0x6B k
    {0x00, 0x41, 0x7F, 0x40, 0x00}, // 0x6C l
    {0x7C, 0x04, 0x18, 0x04, 0x78}, // 0x6D m
    {0x7C, 0x08, 0x04, 0x04, 0x78}, // 0x6E n
    {0x38, 0x44, 0x44, 0x44, 0x38}, // 0x6F o
    {0x7C, 0x14, 0x14, 0x14, 0x08}, // 0x70 p
    {0x08, 0x14, 0x14, 0x18, 0x7C}, // 0x71 q
    {0x7C, 0x08, 0x04, 0x04, 0x08}, // 0x72 r
    {0x48, 0x54, 0x54, 0x54, 0x20}, // 0x73 s
    {0x04, 0x3F, 0x44, 0x40, 0x20}, // 0x74 t
    {0x3C, 0x40, 0x40, 0x20, 0x7C}, // 0x75 u
    {0x1C, 0x20, 0x40, 0x20, 0x1C}, // 0x76 v
    {0x3C, 0x40, 0x30, 0x40, 0x3C}, // 0x77 w
    {0x44, 0x28, 0x10, 0x28, 0x44}, // 0x78 x
    {0x0C, 0x50, 0x50, 0x50, 0x3C}, // 0x79 y
    {0x44, 0x64, 0x54, 0x4C, 0x44}, // 0x7A z
    {0x00, 0x08, 0x36, 0x41, 0x00}, // 0x7B {
    {0x00, 0x00, 0x7F, 0x00, 0x00}, // 0x7C |
    {0x00, 0x41, 0x36, 0x08, 0x00}, // 0x7D }
    {0x10, 0x08, 0x08, 0x10, 0x08}  // 0x7E ~
};

/**
 * @brief Sends a command byte to SSD1306 controller over I2C bus.
 * @param[in] cmd Command byte.
 * @return esp_err_t ESP_OK on success, error code otherwise.
 */
static esp_err_t ssd1306_send_cmd(uint8_t cmd)
{
    esp_err_t ret = ESP_OK;
    uint8_t buf[2] = {0x00, cmd};
    const TickType_t timeout_ticks = pdMS_TO_TICKS(100);

    ret = i2c_master_write_to_device(g_i2c_port, SSD1306_I2C_ADDR, buf, sizeof(buf), timeout_ticks);

    return ret;
}

esp_err_t ssd1306_init(i2c_port_t i2c_num)
{
    esp_err_t ret = ESP_OK;

    g_i2c_port = i2c_num;

    ssd1306_send_cmd(0xAE); // Display OFF
    ssd1306_send_cmd(0xD5);
    ssd1306_send_cmd(0x80); // Set display clock divide
    ssd1306_send_cmd(0xA8);
    ssd1306_send_cmd(0x3F); // Multiplex ratio 64
    ssd1306_send_cmd(0xD3);
    ssd1306_send_cmd(0x00); // Display offset 0
    ssd1306_send_cmd(0x40); // Start line 0
    ssd1306_send_cmd(0x8D);
    ssd1306_send_cmd(0x14); // Enable charge pump
    ssd1306_send_cmd(0x20);
    ssd1306_send_cmd(0x02); // Page Addressing Mode
    ssd1306_send_cmd(0xA1); // Segment remap
    ssd1306_send_cmd(0xC8); // COM scan direction dec
    ssd1306_send_cmd(0xDA);
    ssd1306_send_cmd(0x12); // COM pins hardware configuration
    ssd1306_send_cmd(0x81);
    ssd1306_send_cmd(0xCF); // Contrast control
    ssd1306_send_cmd(0xD9);
    ssd1306_send_cmd(0xF1); // Pre-charge period
    ssd1306_send_cmd(0xDB);
    ssd1306_send_cmd(0x40); // VCOMH deselect level
    ssd1306_send_cmd(0xA4); // Entire display ON resume
    ssd1306_send_cmd(0xA6); // Normal display
    ssd1306_send_cmd(0xAF); // Display ON

    ssd1306_clear();
    ssd1306_update();
    ESP_LOGI(TAG, "SSD1306 Framebuffer Graphics Engine Initialized");
    ret = ESP_OK;

    return ret;
}

void ssd1306_clear(void)
{
    memset(s_buffer, 0, sizeof(s_buffer));
    s_cursor_x = 0;
    s_cursor_y = 0;
}

void ssd1306_update(void)
{
    uint8_t data_pkt[SSD1306_WIDTH + 1] = {0};
    uint8_t page = 0;
    const TickType_t timeout_ticks = pdMS_TO_TICKS(100);

    data_pkt[0] = 0x40; // Co=0, D/C#=1 (Data stream byte)

    for (page = 0; page < 8; page++)
    {
        ssd1306_send_cmd(0xB0 + page);
        ssd1306_send_cmd(0x00); // Lower column 0
        ssd1306_send_cmd(0x10); // Higher column 0

        memcpy(&data_pkt[1], &s_buffer[page * SSD1306_WIDTH], SSD1306_WIDTH);
        i2c_master_write_to_device(g_i2c_port, SSD1306_I2C_ADDR, data_pkt, sizeof(data_pkt), timeout_ticks);
    }
}

void ssd1306_draw_pixel(int16_t x, int16_t y, uint8_t color)
{
    uint16_t idx = 0;
    uint8_t bit_mask = 0;

    if ((x >= 0) && (x < SSD1306_WIDTH) && (y >= 0) && (y < SSD1306_HEIGHT))
    {
        idx = (uint16_t)((y / 8) * SSD1306_WIDTH + x);
        bit_mask = (uint8_t)(1 << (y % 8));

        if (color == SSD1306_COLOR_WHITE)
        {
            s_buffer[idx] |= bit_mask;
        }
        else if (color == SSD1306_COLOR_BLACK)
        {
            s_buffer[idx] &= ~bit_mask;
        }
        else if (color == SSD1306_COLOR_INVERT)
        {
            s_buffer[idx] ^= bit_mask;
        }
    }
}

void ssd1306_draw_fast_h_line(int16_t x, int16_t y, int16_t w, uint8_t color)
{
    int16_t i = 0;

    if ((y >= 0) && (y < SSD1306_HEIGHT) && (w > 0))
    {
        for (i = 0; i < w; i++)
        {
            ssd1306_draw_pixel((int16_t)(x + i), y, color);
        }
    }
}

void ssd1306_draw_fast_v_line(int16_t x, int16_t y, int16_t h, uint8_t color)
{
    int16_t i = 0;

    if ((x >= 0) && (x < SSD1306_WIDTH) && (h > 0))
    {
        for (i = 0; i < h; i++)
        {
            ssd1306_draw_pixel(x, (int16_t)(y + i), color);
        }
    }
}

void ssd1306_draw_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color)
{
    if ((w > 0) && (h > 0))
    {
        ssd1306_draw_fast_h_line(x, y, w, color);
        ssd1306_draw_fast_h_line(x, (int16_t)(y + h - 1), w, color);
        ssd1306_draw_fast_v_line(x, y, h, color);
        ssd1306_draw_fast_v_line((int16_t)(x + w - 1), y, h, color);
    }
}

void ssd1306_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color)
{
    int16_t i = 0;

    if ((w > 0) && (h > 0))
    {
        for (i = x; i < (x + w); i++)
        {
            ssd1306_draw_fast_v_line(i, y, h, color);
        }
    }
}

void ssd1306_set_cursor(int16_t x, int16_t y)
{
    s_cursor_x = x;
    s_cursor_y = y;
}

void ssd1306_set_text_size(uint8_t size)
{
    s_text_size = (size > 0) ? size : 1;
}

void ssd1306_set_text_color(uint8_t color, uint8_t bg)
{
    s_text_color = color;
    s_text_bg = bg;
}

void ssd1306_draw_char(int16_t x, int16_t y, unsigned char c, uint8_t color, uint8_t bg, uint8_t size)
{
    uint8_t safe_c = c;
    uint8_t char_idx = 0;
    int8_t i = 0;
    int8_t j = 0;
    uint8_t line = 0;

    if ((x >= SSD1306_WIDTH) || (y >= SSD1306_HEIGHT) || ((x + 6 * size - 1) < 0) || ((y + 8 * size - 1) < 0))
    {
        return;
    }

    if (safe_c < 0x20 || safe_c > 0x7E)
    {
        safe_c = ' ';
    }

    char_idx = (uint8_t)(safe_c - 0x20);

    for (i = 0; i < 5; i++)
    {
        line = font5x7[char_idx][i];
        for (j = 0; j < 8; j++, line >>= 1)
        {
            if (line & 1)
            {
                if (size == 1)
                {
                    ssd1306_draw_pixel((int16_t)(x + i), (int16_t)(y + j), color);
                }
                else
                {
                    ssd1306_fill_rect((int16_t)(x + i * size), (int16_t)(y + j * size), size, size, color);
                }
            }
            else if (bg != color)
            {
                if (size == 1)
                {
                    ssd1306_draw_pixel((int16_t)(x + i), (int16_t)(y + j), bg);
                }
                else
                {
                    ssd1306_fill_rect((int16_t)(x + i * size), (int16_t)(y + j * size), size, size, bg);
                }
            }
        }
    }

    // 6th column character spacing
    if (bg != color)
    {
        if (size == 1)
        {
            ssd1306_draw_fast_v_line((int16_t)(x + 5), y, 8, bg);
        }
        else
        {
            ssd1306_fill_rect((int16_t)(x + 5 * size), y, size, (int16_t)(8 * size), bg);
        }
    }
}

void ssd1306_write(char c)
{
    if (c == '\n')
    {
        s_cursor_x = 0;
        s_cursor_y = (int16_t)(s_cursor_y + s_text_size * 8);
    }
    else if (c == '\r')
    {
        // Ignore carriage return
    }
    else
    {
        if ((s_cursor_x + s_text_size * 6) > SSD1306_WIDTH)
        {
            s_cursor_x = 0;
            s_cursor_y = (int16_t)(s_cursor_y + s_text_size * 8);
        }
        ssd1306_draw_char(s_cursor_x, s_cursor_y, (unsigned char)c, s_text_color, s_text_bg, s_text_size);
        s_cursor_x = (int16_t)(s_cursor_x + s_text_size * 6);
    }
}

void ssd1306_print(const char *str)
{
    if (str != NULL)
    {
        while (*str != '\0')
        {
            ssd1306_write(*str);
            str++;
        }
    }
}

void ssd1306_println(const char *str)
{
    ssd1306_print(str);
    ssd1306_write('\n');
}

void ssd1306_printf(const char *format, ...)
{
    char buf[128] = {0};
    va_list args;

    if (format != NULL)
    {
        va_start(args, format);
        vsnprintf(buf, sizeof(buf), format, args);
        va_end(args);

        ssd1306_print(buf);
    }
}

void ssd1306_draw_progress_bar(int32_t x, int32_t y, int32_t width, int32_t height, int32_t progress)
{
    int32_t clamped_progress = progress;
    int32_t fill_width = 0;

    if (clamped_progress < 0)
    {
        clamped_progress = 0;
    }
    if (clamped_progress > 100)
    {
        clamped_progress = 100;
    }

    fill_width = (width - 4) * clamped_progress / 100;

    ssd1306_draw_rect((int16_t)x, (int16_t)y, (int16_t)width, (int16_t)height, SSD1306_COLOR_WHITE);
    if (fill_width > 0)
    {
        ssd1306_fill_rect((int16_t)(x + 2), (int16_t)(y + 2), (int16_t)fill_width, (int16_t)(height - 4), SSD1306_COLOR_WHITE);
    }
}

void ssd1306_draw_footer(void)
{
    ssd1306_set_cursor(0, 56);
    ssd1306_set_text_size(1);
    ssd1306_set_text_color(SSD1306_COLOR_WHITE, SSD1306_COLOR_BLACK);
    ssd1306_print("By: M.Quoc");
}
