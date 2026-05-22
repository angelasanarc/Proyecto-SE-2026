#ifndef LOGGER_H
#define LOGGER_H

#include "esp_err.h"
#include "line_sensors.h"

esp_err_t logger_init(void);

void logger_system(const char *message);

void logger_control(
    const line_sensor_data_t *sensor_data,
    int left_speed,
    int right_speed,
    float correction,
    int start_active
);

#endif