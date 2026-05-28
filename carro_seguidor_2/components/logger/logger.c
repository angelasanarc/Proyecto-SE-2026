#include "logger.h"

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static void fmt_ts(char *buf, size_t sz)
{
    uint32_t ms  = (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
    uint32_t s   = ms / 1000;
    uint32_t h   = s / 3600;
    uint32_t m   = (s % 3600) / 60;
    uint32_t sec = s % 60;
    snprintf(buf, sz, "%02lu:%02lu:%02lu", (unsigned long)h,
             (unsigned long)m, (unsigned long)sec);
}

esp_err_t logger_init(void)
{
    char ts[12];
    fmt_ts(ts, sizeof(ts));
    printf("%s LOGGER | iniciado\n", ts);
    return ESP_OK;
}

void logger_system(const char *message)
{
    char ts[12];
    fmt_ts(ts, sizeof(ts));
    printf("%s SYS | %s\n", ts, message);
}

void logger_control(
    const line_sensor_data_t *sensor_data,
    int left_speed,
    int right_speed,
    float correction,
    int start_active
)
{
    if (sensor_data == NULL) return;

    char ts[12];
    fmt_ts(ts, sizeof(ts));
    printf(
        "%s CTL | START=%d LINE=%d POS=%d ERR=%d CORR=%.2f L=%d R=%d S=%d%d%d%d%d%d%d%d\n",
        ts,
        start_active,
        sensor_data->line_detected,
        sensor_data->position,
        sensor_data->error,
        correction,
        left_speed,
        right_speed,
        sensor_data->detected[0],
        sensor_data->detected[1],
        sensor_data->detected[2],
        sensor_data->detected[3],
        sensor_data->detected[4],
        sensor_data->detected[5],
        sensor_data->detected[6],
        sensor_data->detected[7]
    );
}

void logger_run_start(int run)
{
    char ts[12];
    fmt_ts(ts, sizeof(ts));
    printf("%s RUN_START | run=%d\n", ts, run);
}

void logger_run_end(int run)
{
    char ts[12];
    fmt_ts(ts, sizeof(ts));
    printf("%s RUN_END | run=%d\n", ts, run);
}
