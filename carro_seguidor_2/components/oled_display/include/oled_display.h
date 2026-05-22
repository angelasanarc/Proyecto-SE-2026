#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include "esp_err.h"
#include "line_sensors.h"

esp_err_t oled_display_init(void);

void oled_display_clear(void);

void oled_display_show_startup(void);

void oled_display_show_status(
    int start_active,
    int motors_on,
    const line_sensor_data_t *sensor_data,
    int left_speed,
    int right_speed
);

#endif