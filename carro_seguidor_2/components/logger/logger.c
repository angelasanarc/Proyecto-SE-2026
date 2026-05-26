#include "logger.h"

#include <stdio.h>

esp_err_t logger_init(void)
{
    printf("LOGGER | iniciado\n");

    return ESP_OK;
}

void logger_system(const char *message)
{
    printf("SYS | %s\n", message);
}

void logger_control(
    const line_sensor_data_t *sensor_data,
    int left_speed,
    int right_speed,
    float correction,
    int start_active
)
{
    if (sensor_data == NULL)
    {
        return;
    }

    printf(
        "CTL | START=%d LINE=%d POS=%d ERR=%d CORR=%.2f L=%d R=%d S=%d%d%d%d%d%d%d%d\n",
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
    printf("RUN_START | run=%d\n", run);
}

void logger_run_end(int run)
{
    printf("RUN_END | run=%d\n", run);
}