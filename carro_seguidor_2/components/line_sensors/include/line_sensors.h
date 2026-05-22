#ifndef LINE_SENSORS_H
#define LINE_SENSORS_H

#include <stdbool.h>

#include "esp_err.h"
#include "robot_config.h"

typedef struct
{
    int detected[NUM_SENSORS];

    int position;

    int error;

    bool line_detected;

} line_sensor_data_t;

esp_err_t line_sensors_init(void);

esp_err_t line_sensors_read(line_sensor_data_t *data);

#endif