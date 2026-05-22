#include <stdio.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "robot_config.h"
#include "line_sensors.h"
#include "start_input.h"
#include "motor_driver.h"
#include "line_control.h"
#include "logger.h"
#include "oled_display.h"

static int clamp_forward_speed(int speed)
{
    if (speed > MAX_SPEED)
    {
        return MAX_SPEED;
    }

    if (speed < MIN_SPEED)
    {
        return MIN_SPEED;
    }

    return speed;
}

static void safe_stop(line_control_t *control)
{
    motor_driver_stop();
    line_control_reset(control);
}

void app_main(void)
{
    line_sensor_data_t sensor_data;
    line_control_t line_control;

    int left_speed = 0;
    int right_speed = 0;
    float correction = 0.0f;

    int start_active = 0;
    int motors_on = 0;

    logger_init();
    logger_system("Inicializando sistema");

    oled_display_init();
    oled_display_show_startup();

    line_sensors_init();
    start_input_init();
    motor_driver_init();

    line_control_init(
        &line_control,
        CONTROL_KP,
        CONTROL_KI,
        CONTROL_KD,
        CONTROL_OUTPUT_MIN,
        CONTROL_OUTPUT_MAX
    );

    motor_driver_stop();

    logger_system("Sistema listo");

    TickType_t last_wake_time = xTaskGetTickCount();
    TickType_t last_display_time = xTaskGetTickCount();
    TickType_t last_log_time = xTaskGetTickCount();

    while (1)
    {
        line_sensors_read(&sensor_data);

        start_active = start_input_is_active() ? 1 : 0;

        if (!start_active)
        {
            left_speed = 0;
            right_speed = 0;
            correction = 0.0f;
            motors_on = 0;

            safe_stop(&line_control);
        }
        else if (!sensor_data.line_detected)
        {
            left_speed = 0;
            right_speed = 0;
            correction = 0.0f;
            motors_on = 0;

            safe_stop(&line_control);
        }
        else
        {
            float dt = CONTROL_PERIOD_MS / 1000.0f;

            correction = line_control_compute(
                &line_control,
                (float)sensor_data.error,
                dt
            );

            left_speed = BASE_SPEED - (int)correction;
            right_speed = BASE_SPEED + (int)correction;

            left_speed = clamp_forward_speed(left_speed);
            right_speed = clamp_forward_speed(right_speed);

            motor_driver_set_speed(left_speed, right_speed);

            motors_on = 1;
        }

        TickType_t now = xTaskGetTickCount();

        if ((now - last_display_time) >= pdMS_TO_TICKS(80))
        {
            oled_display_show_status(
                start_active,
                motors_on,
                &sensor_data,
                left_speed,
                right_speed
            );

            last_display_time = now;
        }

        if ((now - last_log_time) >= pdMS_TO_TICKS(80))
        {
            logger_control(
                &sensor_data,
                left_speed,
                right_speed,
                correction,
                start_active
            );

            last_log_time = now;
        }

        vTaskDelayUntil(
            &last_wake_time,
            pdMS_TO_TICKS(CONTROL_PERIOD_MS)
        );
    }
}
