#include "oled_display.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c.h"
#include "driver/gpio.h"

#include "robot_config.h"

static bool oled_present = true;
static uint8_t oled_buffer[128 * 8];

static const char font_chars[] = " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ:-_/.";

static const uint8_t font_5x7[][5] = {
    {0x00,0x00,0x00,0x00,0x00},
    {0x3E,0x51,0x49,0x45,0x3E},
    {0x00,0x42,0x7F,0x40,0x00},
    {0x42,0x61,0x51,0x49,0x46},
    {0x21,0x41,0x45,0x4B,0x31},
    {0x18,0x14,0x12,0x7F,0x10},
    {0x27,0x45,0x45,0x45,0x39},
    {0x3C,0x4A,0x49,0x49,0x30},
    {0x01,0x71,0x09,0x05,0x03},
    {0x36,0x49,0x49,0x49,0x36},
    {0x06,0x49,0x49,0x29,0x1E},
    {0x7E,0x11,0x11,0x11,0x7E},
    {0x7F,0x49,0x49,0x49,0x36},
    {0x3E,0x41,0x41,0x41,0x22},
    {0x7F,0x41,0x41,0x22,0x1C},
    {0x7F,0x49,0x49,0x49,0x41},
    {0x7F,0x09,0x09,0x09,0x01},
    {0x3E,0x41,0x49,0x49,0x7A},
    {0x7F,0x08,0x08,0x08,0x7F},
    {0x00,0x41,0x7F,0x41,0x00},
    {0x20,0x40,0x41,0x3F,0x01},
    {0x7F,0x08,0x14,0x22,0x41},
    {0x7F,0x40,0x40,0x40,0x40},
    {0x7F,0x02,0x0C,0x02,0x7F},
    {0x7F,0x04,0x08,0x10,0x7F},
    {0x3E,0x41,0x41,0x41,0x3E},
    {0x7F,0x09,0x09,0x09,0x06},
    {0x3E,0x41,0x51,0x21,0x5E},
    {0x7F,0x09,0x19,0x29,0x46},
    {0x46,0x49,0x49,0x49,0x31},
    {0x01,0x01,0x7F,0x01,0x01},
    {0x3F,0x40,0x40,0x40,0x3F},
    {0x1F,0x20,0x40,0x20,0x1F},
    {0x7F,0x20,0x18,0x20,0x7F},
    {0x63,0x14,0x08,0x14,0x63},
    {0x07,0x08,0x70,0x08,0x07},
    {0x61,0x51,0x49,0x45,0x43},
    {0x00,0x36,0x36,0x00,0x00},
    {0x08,0x08,0x08,0x08,0x08},
    {0x20,0x10,0x08,0x04,0x02},
    {0x20,0x10,0x08,0x04,0x02},
    {0x00,0x60,0x60,0x00,0x00}
};

static const uint8_t *get_glyph(char c)
{
    if (c >= 'a' && c <= 'z')
    {
        c -= 32;
    }

    for (int i = 0; i < (int)strlen(font_chars); i++)
    {
        if (font_chars[i] == c)
        {
            return font_5x7[i];
        }
    }

    return font_5x7[0];
}

static esp_err_t oled_send(uint8_t control, const uint8_t *data, size_t len)
{
    if (!oled_present)
    {
        return ESP_FAIL;
    }

    uint8_t buffer[17];
    size_t offset = 0;

    while (offset < len)
    {
        size_t chunk = len - offset;

        if (chunk > 16)
        {
            chunk = 16;
        }

        buffer[0] = control;
        memcpy(&buffer[1], &data[offset], chunk);

        esp_err_t ret = i2c_master_write_to_device(
            I2C_PORT,
            OLED_ADDR,
            buffer,
            chunk + 1,
            pdMS_TO_TICKS(100)
        );

        if (ret != ESP_OK)
        {
            oled_present = false;
            return ret;
        }

        offset += chunk;
    }

    return ESP_OK;
}

static void oled_cmd(uint8_t cmd)
{
    oled_send(0x00, &cmd, 1);
}

static void oled_set_pos(uint8_t page, uint8_t col)
{
    oled_cmd(0xB0 + page);
    oled_cmd(0x00 + (col & 0x0F));
    oled_cmd(0x10 + ((col >> 4) & 0x0F));
}

static void oled_clear_buffer(void)
{
    memset(oled_buffer, 0x00, sizeof(oled_buffer));
}

static void oled_draw_pixel(int x, int y, bool on)
{
    if (x < 0 || x >= 128 || y < 0 || y >= 64)
    {
        return;
    }

    int page = y / 8;
    int bit = y % 8;
    int index = page * 128 + x;

    if (on)
    {
        oled_buffer[index] |= (1 << bit);
    }
    else
    {
        oled_buffer[index] &= ~(1 << bit);
    }
}

static void oled_draw_rect(int x, int y, int w, int h, bool fill)
{
    for (int yy = y; yy < y + h; yy++)
    {
        for (int xx = x; xx < x + w; xx++)
        {
            if (fill)
            {
                oled_draw_pixel(xx, yy, true);
            }
            else
            {
                if (yy == y || yy == y + h - 1 || xx == x || xx == x + w - 1)
                {
                    oled_draw_pixel(xx, yy, true);
                }
            }
        }
    }
}

static void oled_draw_char_scaled(int x, int y, char c, int scale)
{
    const uint8_t *glyph = get_glyph(c);

    for (int col = 0; col < 5; col++)
    {
        for (int row = 0; row < 7; row++)
        {
            if (glyph[col] & (1 << row))
            {
                for (int sx = 0; sx < scale; sx++)
                {
                    for (int sy = 0; sy < scale; sy++)
                    {
                        oled_draw_pixel(
                            x + col * scale + sx,
                            y + row * scale + sy,
                            true
                        );
                    }
                }
            }
        }
    }
}

static void oled_draw_text_scaled(int x, int y, const char *text, int scale)
{
    while (*text)
    {
        oled_draw_char_scaled(x, y, *text, scale);
        x += 6 * scale;
        text++;
    }
}

static void oled_flush(void)
{
    if (!oled_present)
    {
        return;
    }

    for (int page = 0; page < 8; page++)
    {
        oled_set_pos(page, 0);
        oled_send(0x40, &oled_buffer[page * 128], 128);
    }
}

esp_err_t oled_display_init(void)
{
    i2c_config_t config =
    {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = PIN_OLED_SDA,
        .scl_io_num = PIN_OLED_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000
    };

    esp_err_t ret = i2c_param_config(I2C_PORT, &config);

    if (ret != ESP_OK)
    {
        oled_present = false;
        return ret;
    }

    ret = i2c_driver_install(
        I2C_PORT,
        config.mode,
        0,
        0,
        0
    );

    if (ret != ESP_OK)
    {
        oled_present = false;
        return ret;
    }

    vTaskDelay(pdMS_TO_TICKS(100));

    uint8_t init_cmds[] =
    {
        0xAE, 0x20, 0x02, 0xB0, 0xC8, 0x00, 0x10, 0x40,
        0x81, 0x7F, 0xA1, 0xA6, 0xA8, 0x3F, 0xA4, 0xD3,
        0x00, 0xD5, 0x80, 0xD9, 0xF1, 0xDA, 0x12, 0xDB,
        0x40, 0x8D, 0x14, 0xAF
    };

    for (int i = 0; i < (int)sizeof(init_cmds); i++)
    {
        oled_cmd(init_cmds[i]);
    }

    oled_clear_buffer();
    oled_flush();

    return ESP_OK;
}

void oled_display_clear(void)
{
    oled_clear_buffer();
    oled_flush();
}

void oled_display_show_startup(void)
{
    oled_clear_buffer();

    oled_draw_text_scaled(10, 0, "LINE BOT", 2);
    oled_draw_text_scaled(0, 25, "SISTEMA OK", 1);
    oled_draw_text_scaled(0, 38, "ESPERANDO START", 1);

    oled_flush();
}

void oled_display_show_status(
    int start_active,
    int motors_on,
    const line_sensor_data_t *sensor_data,
    int left_speed,
    int right_speed
)
{
    if (sensor_data == NULL)
    {
        return;
    }

    oled_clear_buffer();

    oled_draw_text_scaled(10, 0, "LINE BOT", 2);

    char status[24];

    snprintf(
        status,
        sizeof(status),
        "ST:%d MOT:%s",
        start_active,
        motors_on ? "ON" : "OFF"
    );

    oled_draw_text_scaled(0, 18, status, 1);

    char speeds[24];

    snprintf(
        speeds,
        sizeof(speeds),
        "L:%03d R:%03d",
        left_speed,
        right_speed
    );

    oled_draw_text_scaled(0, 27, speeds, 1);

    oled_draw_rect(7, 38, 114, 14, false);

    for (int i = 0; i < NUM_SENSORS; i++)
    {
        int x = 10 + i * 14;
        int h = sensor_data->detected[i] ? 10 : 2;

        oled_draw_rect(x, 49 - h, 10, h, true);
    }

    int marker_x = 8 + ((sensor_data->position * 112) / LINE_MAX_POSITION);

    if (marker_x < 8)
    {
        marker_x = 8;
    }

    if (marker_x > 120)
    {
        marker_x = 120;
    }

    oled_draw_rect(marker_x - 1, 35, 3, 20, true);

    char line[24];

    snprintf(
        line,
        sizeof(line),
        "POS:%04d E:%04d",
        sensor_data->position,
        sensor_data->error
    );

    oled_draw_text_scaled(0, 56, line, 1);

    oled_flush();
}