#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include "esp_err.h"

esp_err_t motor_driver_init(void);

esp_err_t motor_driver_set_speed(int left_speed, int right_speed);

esp_err_t motor_driver_stop(void);

#endif