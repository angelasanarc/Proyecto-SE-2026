#include "oled_display.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c_master.h"
#include "esp_log.h"

#include "robot_config.h"
static const char *OTAG = "OLED";

static bool                    oled_present = true;
static uint8_t                 oled_buffer[128 * 8];
static i2c_master_bus_handle_t s_bus;
static i2c_master_dev_handle_t s_dev;
static TickType_t              oled_failed_at = 0;

static const char font_chars[] = " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ:-_/.";

static const uint8_t font_5x7[][5] = {
    {0x00,0x00,0x00,0x00,0x00}, /* space */
    {0x3E,0x51,0x49,0x45,0x3E}, /* 0 */
    {0x00,0x42,0x7F,0x40,0x00}, /* 1 */
    {0x42,0x61,0x51,0x49,0x46}, /* 2 */
    {0x21,0x41,0x45,0x4B,0x31}, /* 3 */
    {0x18,0x14,0x12,0x7F,0x10}, /* 4 */
    {0x27,0x45,0x45,0x45,0x39}, /* 5 */
    {0x3C,0x4A,0x49,0x49,0x30}, /* 6 */
    {0x01,0x71,0x09,0x05,0x03}, /* 7 */
    {0x36,0x49,0x49,0x49,0x36}, /* 8 */
    {0x06,0x49,0x49,0x29,0x1E}, /* 9 */
    {0x7E,0x11,0x11,0x11,0x7E}, /* A */
    {0x7F,0x49,0x49,0x49,0x36}, /* B */
    {0x3E,0x41,0x41,0x41,0x22}, /* C */
    {0x7F,0x41,0x41,0x22,0x1C}, /* D */
    {0x7F,0x49,0x49,0x49,0x41}, /* E */
    {0x7F,0x09,0x09,0x09,0x01}, /* F */
    {0x3E,0x41,0x49,0x49,0x7A}, /* G */
    {0x7F,0x08,0x08,0x08,0x7F}, /* H */
    {0x00,0x41,0x7F,0x41,0x00}, /* I */
    {0x20,0x40,0x41,0x3F,0x01}, /* J */
    {0x7F,0x08,0x14,0x22,0x41}, /* K */
    {0x7F,0x40,0x40,0x40,0x40}, /* L */
    {0x7F,0x02,0x0C,0x02,0x7F}, /* M */
    {0x7F,0x04,0x08,0x10,0x7F}, /* N */
    {0x3E,0x41,0x41,0x41,0x3E}, /* O */
    {0x7F,0x09,0x09,0x09,0x06}, /* P */
    {0x3E,0x41,0x51,0x21,0x5E}, /* Q */
    {0x7F,0x09,0x19,0x29,0x46}, /* R */
    {0x46,0x49,0x49,0x49,0x31}, /* S */
    {0x01,0x01,0x7F,0x01,0x01}, /* T */
    {0x3F,0x40,0x40,0x40,0x3F}, /* U */
    {0x1F,0x20,0x40,0x20,0x1F}, /* V */
    {0x7F,0x20,0x18,0x20,0x7F}, /* W */
    {0x63,0x14,0x08,0x14,0x63}, /* X */
    {0x07,0x08,0x70,0x08,0x07}, /* Y */
    {0x61,0x51,0x49,0x45,0x43}, /* Z */
    {0x00,0x36,0x36,0x00,0x00}, /* : */
    {0x08,0x08,0x08,0x08,0x08}, /* - */
    {0x20,0x10,0x08,0x04,0x02}, /* _ */
    {0x20,0x10,0x08,0x04,0x02}, /* / */
    {0x00,0x60,0x60,0x00,0x00}  /* . */
};

/* ---------- primitivas ---------- */

static const uint8_t *get_glyph(char c)
{
    if (c >= 'a' && c <= 'z') c -= 32;

    for (int i = 0; i < (int)strlen(font_chars); i++)
    {
        if (font_chars[i] == c) return font_5x7[i];
    }
    return font_5x7[0];
}

static void oled_try_recover(void)
{
    if (oled_present) return;
    if ((xTaskGetTickCount() - oled_failed_at) < pdMS_TO_TICKS(2000)) return;
    uint8_t init_cmds[] = {
        0xAE, 0x20, 0x02, 0xB0, 0xC8, 0x00, 0x10, 0x40,
        0x81, 0x7F, 0xA1, 0xA6, 0xA8, 0x3F, 0xA4, 0xD3,
        0x00, 0xD5, 0x80, 0xD9, 0xF1, 0xDA, 0x12, 0xDB,
        0x40, 0x8D, 0x14, 0xAF
    };
    oled_present = true;
    for (int i = 0; i < (int)sizeof(init_cmds); i++) {
        uint8_t buf[2] = { 0x00, init_cmds[i] };
        if (i2c_master_transmit(s_dev, buf, 2, 100) != ESP_OK) {
            oled_present = false;
            oled_failed_at = xTaskGetTickCount();
            return;
        }
    }
}

static esp_err_t oled_send(uint8_t control, const uint8_t *data, size_t len)
{
    if (!oled_present) { oled_try_recover(); return ESP_FAIL; }

    uint8_t buffer[17];
    size_t  offset = 0;

    while (offset < len)
    {
        size_t chunk = len - offset;
        if (chunk > 16) chunk = 16;

        buffer[0] = control;
        memcpy(&buffer[1], &data[offset], chunk);

        esp_err_t ret = i2c_master_transmit(s_dev, buffer, chunk + 1, 100);

        if (ret != ESP_OK) {
            ESP_LOGE(OTAG, "transmit err=0x%x ctrl=0x%02x", ret, buffer[0]);
            oled_present = false; oled_failed_at = xTaskGetTickCount(); return ret;
        }
        offset += chunk;
    }
    return ESP_OK;
}

static void oled_cmd(uint8_t cmd) { oled_send(0x00, &cmd, 1); }

static void oled_set_pos(uint8_t page, uint8_t col)
{
    oled_cmd(0xB0 + page);
    oled_cmd(0x00 + (col & 0x0F));
    oled_cmd(0x10 + ((col >> 4) & 0x0F));
}

static void oled_clear_buffer(void) { memset(oled_buffer, 0x00, sizeof(oled_buffer)); }

static void oled_draw_pixel(int x, int y, bool on)
{
    if (x < 0 || x >= 128 || y < 0 || y >= 64) return;

    int page  = y / 8;
    int bit   = y % 8;
    int index = page * 128 + x;

    if (on) oled_buffer[index] |=  (1 << bit);
    else    oled_buffer[index] &= ~(1 << bit);
}

static void oled_draw_rect(int x, int y, int w, int h, bool fill)
{
    for (int yy = y; yy < y + h; yy++)
    {
        for (int xx = x; xx < x + w; xx++)
        {
            if (fill || yy == y || yy == y + h - 1 || xx == x || xx == x + w - 1)
            {
                oled_draw_pixel(xx, yy, true);
            }
        }
    }
}

static void oled_hline(int x0, int x1, int y)
{
    for (int x = x0; x <= x1; x++) oled_draw_pixel(x, y, true);
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
                    for (int sy = 0; sy < scale; sy++)
                        oled_draw_pixel(x + col*scale + sx, y + row*scale + sy, true);
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
    if (!oled_present) return;

    for (int page = 0; page < 8; page++)
    {
        oled_set_pos(page, 0);
        oled_send(0x40, &oled_buffer[page * 128], 128);
    }
}

/* ---------- publicas ---------- */

esp_err_t oled_display_init(void)
{
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port            = I2C_PORT,
        .sda_io_num          = PIN_OLED_SDA,
        .scl_io_num          = PIN_OLED_SCL,
        .clk_source          = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt   = 7,
        .flags.enable_internal_pullup = true,
    };

    esp_err_t ret = i2c_new_master_bus(&bus_cfg, &s_bus);
    if (ret != ESP_OK) { oled_present = false; return ret; }

    /* Diagnostico: buscar dispositivo en 0x3C y 0x3D */
    bool found_3c = (i2c_master_probe(s_bus, 0x3C, 50) == ESP_OK);
    bool found_3d = (i2c_master_probe(s_bus, 0x3D, 50) == ESP_OK);
    ESP_LOGI(OTAG, "scan: 0x3C=%s 0x3D=%s",
             found_3c ? "ACK" : "---", found_3d ? "ACK" : "---");
    if (!found_3c && !found_3d)
        ESP_LOGE(OTAG, "OLED no responde — verificar VCC/GND/SDA/SCL");

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address  = OLED_ADDR,
        .scl_speed_hz    = 100000,  /* 100kHz: pull-ups internos insuficientes para 400kHz */
    };

    ret = i2c_master_bus_add_device(s_bus, &dev_cfg, &s_dev);
    if (ret != ESP_OK) { oled_present = false; return ret; }

    vTaskDelay(pdMS_TO_TICKS(200));

    uint8_t init_cmds[] = {
        0xAE, 0x20, 0x02, 0xB0, 0xC8, 0x00, 0x10, 0x40,
        0x81, 0x7F, 0xA1, 0xA6, 0xA8, 0x3F, 0xA4, 0xD3,
        0x00, 0xD5, 0x80, 0xD9, 0xF1, 0xDA, 0x12, 0xDB,
        0x40, 0x8D, 0x14, 0xAF
    };

    for (int i = 0; i < (int)sizeof(init_cmds); i++) oled_cmd(init_cmds[i]);

    oled_clear_buffer();
    oled_flush();
    return ESP_OK;
}

void oled_display_clear(void)
{
    oled_clear_buffer();
    oled_flush();
}

/* =====================================================================
 * PANTALLA 0: ARRANQUE — verificacion de subsistemas
 *
 *  y= 2  "LINE BOT" scale 2 centrado
 *  y=19  separador
 *  y=23  "SENSORES  OK/ERR"
 *  y=31  "MOTORES   OK/ERR"
 *  y=39  "INICIO    OK/ERR"
 *  y=47  "WIFI      OK/ERR"
 * ===================================================================== */
void oled_display_show_boot(bool sensors_ok, bool motors_ok, bool start_ok, bool wifi_ok)
{
    oled_clear_buffer();

    oled_draw_text_scaled(16, 2, "LINE BOT", 2);
    oled_hline(0, 127, 19);

    const char *labels[4] = { "SENSORES", "MOTORES ", "INICIO  ", "WIFI    " };
    bool        ok[4]     = { sensors_ok, motors_ok, start_ok, wifi_ok };

    for (int i = 0; i < 4; i++)
    {
        int y = 23 + i * 8;
        oled_draw_text_scaled(0, y, labels[i], 1);
        oled_draw_text_scaled(60, y, ok[i] ? "OK" : "ERR", 1);
        if (!ok[i])
        {
            /* pequeño cuadro de alerta junto al ERR */
            oled_draw_rect(90, y, 7, 7, false);
            oled_draw_pixel(93, y + 2, true);
            oled_draw_pixel(93, y + 4, true);
        }
    }

    oled_flush();
}

/* =====================================================================
 * PANTALLA 1: CALIBRANDO
 *
 *  y= 0  "CALIBRANDO" centrado (scale 1)
 *  y= 9  separador
 *  y=11  8 barras verticales con marco (barras van de y=11 a y=40)
 *          altura proporcional a normalized[i] (0-1000)
 *  y=43  barra de progreso con marco
 *  y=54  contador "XXX/200" centrado
 * ===================================================================== */
void oled_display_show_calibrating(const int normalized[NUM_SENSORS], int samples)
{
    oled_clear_buffer();

    /* titulo centrado: "CALIBRANDO" = 10 chars × 6px = 60px → x=34 */
    oled_draw_text_scaled(34, 0, "CALIBRANDO", 1);

    oled_hline(0, 127, 9);

    /* 8 barras de sensores — cada slot 16px (14 barra + 2 gap) */
    for (int i = 0; i < NUM_SENSORS; i++)
    {
        int bx = i * 16;

        /* marco fijo */
        oled_draw_rect(bx, 11, 14, 30, false);  /* y=11 hasta y=40 */

        /* relleno desde abajo, maximo 28px interior */
        int fill_h = normalized[i] * 28 / 1000;
        if (fill_h > 0)
        {
            oled_draw_rect(bx + 1, 40 - fill_h, 12, fill_h, true);
        }
    }

    /* barra de progreso */
    oled_draw_rect(0, 43, 128, 10, false);           /* marco */
    int fill_w = samples * 124 / CALIBRATION_SAMPLES;
    if (fill_w > 0)
    {
        oled_draw_rect(2, 45, fill_w, 6, true);      /* relleno interior */
    }

    /* contador "XXX/200" centrado: 7 chars × 6px = 42px → x=43 */
    char buf[24];
    snprintf(buf, sizeof(buf), "%3d/%d", samples, CALIBRATION_SAMPLES);
    oled_draw_text_scaled(43, 55, buf, 1);

    oled_flush();
}

/* =====================================================================
 * PANTALLA 2: CALIBRACION LISTA — ESPERANDO START
 *
 *  y= 2  "LINE BOT" scale 2 centrado
 *  y=19  separador
 *  y=23  "CALIBRACION LISTA" scale 1 centrado
 *  y=33  separador
 *  y=38  "ESPERANDO" scale 1 centrado
 *  y=48  "START" scale 2 centrado
 * ===================================================================== */
void oled_display_show_calibrated(void)
{
    oled_clear_buffer();

    /* "LINE BOT": 8 chars × 12px = 96px → x = (128-96)/2 = 16 */
    oled_draw_text_scaled(16, 2, "LINE BOT", 2);

    oled_hline(0, 127, 19);

    /* "CALIBRACION LISTA": 17 chars × 6px = 102px → x = (128-102)/2 = 13 */
    oled_draw_text_scaled(13, 23, "CALIBRACION LISTA", 1);

    oled_hline(0, 127, 33);

    /* "ESPERANDO": 9 chars × 6px = 54px → x = (128-54)/2 = 37 */
    oled_draw_text_scaled(37, 38, "ESPERANDO", 1);

    /* "START" scale 2: 5 chars × 12px = 60px → x = (128-60)/2 = 34 */
    oled_draw_text_scaled(34, 48, "START", 2);

    oled_flush();
}

/* =====================================================================
 * PANTALLA 3: OPERACION NORMAL
 *
 *  y= 0  barras compactas de 8 sensores (alto max=10px)
 *  y=11  marcador de posicion (bloque 2×3px)
 *  y=14  separador
 *  y=16  "E:XXXXX L:XXX R:XXX"
 *  y=25  barra motor izquierdo con velocidad
 *  y=34  barra motor derecho con velocidad
 *  y=43  separador
 *  y=45  "CORRIENDO" o "DETENIDO" centrado
 *  y=55  "P:XXXX C:XXX.X"
 * ===================================================================== */
void oled_display_show_status(
    int start_active,
    int motors_on,
    const line_sensor_data_t *sensor_data,
    int left_speed,
    int right_speed,
    float correction,
    int run_count
)
{
    if (sensor_data == NULL) return;

    oled_clear_buffer();

    /* ---- Zona A: sensores y posicion ---- */

    /* 8 barras compactas: slot 16px, barra 14px, gap 2px */
    for (int i = 0; i < NUM_SENSORS; i++)
    {
        int bx = i * 16;
        int h  = sensor_data->detected[i] ? 10 : 2;
        oled_draw_rect(bx, 10 - h, 14, h, true);
    }

    /* marcador de posicion (pequeno bloque bajo las barras) */
    {
        int mx = (int)((long)sensor_data->position * 124 / LINE_MAX_POSITION);
        if (mx < 0)   mx = 0;
        if (mx > 124) mx = 124;
        oled_draw_rect(mx, 11, 4, 3, true);
    }

    oled_hline(0, 127, 14);

    /* ---- Zona B: datos numericos ---- */

    {
        char line1[24];
        snprintf(line1, sizeof(line1), "E:%d L:%d R:%d",
                 sensor_data->error, left_speed, right_speed);
        oled_draw_text_scaled(0, 16, line1, 1);
    }

    /* barra motor izquierdo: "L:" (12px) + frame (92px) + " XXX" (24px) = 128px */
    {
        oled_draw_text_scaled(0, 25, "L:", 1);
        oled_draw_rect(12, 25, 92, 7, false);
        int fill = left_speed * 90 / MAX_SPEED;
        if (fill > 0) oled_draw_rect(13, 26, fill, 5, true);
        char spd_l[12];
        snprintf(spd_l, sizeof(spd_l), " %3d", left_speed);
        oled_draw_text_scaled(104, 25, spd_l, 1);
    }

    /* barra motor derecho */
    {
        oled_draw_text_scaled(0, 34, "R:", 1);
        oled_draw_rect(12, 34, 92, 7, false);
        int fill = right_speed * 90 / MAX_SPEED;
        if (fill > 0) oled_draw_rect(13, 35, fill, 5, true);
        char spd_r[12];
        snprintf(spd_r, sizeof(spd_r), " %3d", right_speed);
        oled_draw_text_scaled(104, 34, spd_r, 1);
    }

    oled_hline(0, 127, 43);

    /* ---- Zona C: estado ---- */

    {
        char state[22];
        const char *label = motors_on ? "CORRIENDO" :
                            start_active ? "SIN LINEA" : "DETENIDO ";
        snprintf(state, sizeof(state), "%s R:%d", label, run_count);
        oled_draw_text_scaled(0, 45, state, 1);
    }

    {
        char line2[24];
        snprintf(line2, sizeof(line2), "P:%d C:%.0f",
                 sensor_data->position, (double)correction);
        oled_draw_text_scaled(0, 55, line2, 1);
    }

    oled_flush();
}
