#include <stdio.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "robot_config.h"
#include "qtr_calibration.h"
#include "line_sensors.h"
#include "start_input.h"
#include "motor_driver.h"
#include "line_control.h"
#include "logger.h"
#include "oled_display.h"

static int clamp_forward_speed(int speed)
{
    if (speed > MAX_SPEED) return MAX_SPEED;
    if (speed < MIN_SPEED) return MIN_SPEED;
    return speed;
}

static void safe_stop(line_control_t *control)
{
    motor_driver_stop();
    line_control_reset(control);
}

void app_main(void)
{
    line_sensor_data_t sensor_data = {
        .line_detected = false,
        .position      = LINE_CENTER_POSITION,
        .error         = 0,
    };
    line_control_t    line_control;
    qtr_calibration_t calibration;

    uint32_t raw_times[NUM_SENSORS];
    int      normalized[NUM_SENSORS];

    int   left_speed      = 0;
    int   right_speed     = 0;
    float correction      = 0.0f;
    int   start_active    = 0;
    int   motors_on       = 0;
    int   lost_line_cycles = 0;

    /* ---- Inicializacion ---- */
    logger_init();
    logger_system("Inicializando sistema");

    oled_display_init();

    line_sensors_init();
    start_input_init();
    motor_driver_init();
    motor_driver_stop();

    line_control_init(
        &line_control,
        CONTROL_KP,
        CONTROL_KI,
        CONTROL_KD,
        CONTROL_PERIOD_S,
        CONTROL_DERIVATIVE_ALPHA,
        CONTROL_OUTPUT_MIN,
        CONTROL_OUTPUT_MAX
    );

    /* ---- Fase de calibracion ---- */
    qtr_calibration_init(&calibration);
    logger_system("Calibrando sensores - mueva el robot sobre la linea");

    /* Muestra la primera pantalla de calibracion antes de empezar */
    for (int i = 0; i < NUM_SENSORS; i++) normalized[i] = 0;
    oled_display_show_calibrating(normalized, 0);

    int display_tick = 0;

    while (!qtr_calibration_is_done(&calibration))
    {
        line_sensors_read_raw(raw_times);
        qtr_calibration_update(&calibration, raw_times);

        /* Actualizar OLED cada 80 ms (cada 8 ciclos de 10 ms) */
        display_tick++;
        if (display_tick >= 8)
        {
            qtr_calibration_normalize(&calibration, raw_times, normalized);
            oled_display_show_calibrating(normalized, calibration.sample_count);
            display_tick = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(CONTROL_PERIOD_MS));
    }

    logger_system("Calibracion completa - esperando START");
    oled_display_show_calibrated();

    /* ---- Esperar START explicito ---- */
    /* Estabilizar el debounce con el estado actual del boton */
    for (int i = 0; i < 10; i++)
    {
        start_input_is_active();
        vTaskDelay(pdMS_TO_TICKS(CONTROL_PERIOD_MS));
    }

    /*
     * Requiere ver el boton INACTIVO al menos una vez antes de aceptar
     * ACTIVO. Evita arranque si el modulo JA-Bots ya estaba en estado RUN.
     */
    {
        bool saw_inactive = !start_input_is_active();
        while (true)
        {
            vTaskDelay(pdMS_TO_TICKS(CONTROL_PERIOD_MS));
            bool active = start_input_is_active();
            if (!active) saw_inactive = true;
            if (saw_inactive && active) break;
        }
    }
    logger_system("START recibido - iniciando recorrido");

    /* ---- Loop de control ---- */
    TickType_t last_wake_time    = xTaskGetTickCount();
    TickType_t last_display_time = xTaskGetTickCount();
    TickType_t last_log_time     = xTaskGetTickCount();

    while (1)
    {
        line_sensors_read_raw(raw_times);
        qtr_calibration_normalize(&calibration, raw_times, normalized);
        line_sensors_compute(normalized, &sensor_data);

        start_active = start_input_is_active() ? 1 : 0;

        if (!start_active)
        {
            left_speed       = 0;
            right_speed      = 0;
            correction       = 0.0f;
            motors_on        = 0;
            lost_line_cycles = 0;
            safe_stop(&line_control);
        }
        else if (sensor_data.line_detected)
        {
            /* Linea visible: PID normal */
            lost_line_cycles = 0;
            correction = line_control_compute(&line_control, (float)sensor_data.error);

            left_speed  = clamp_forward_speed(BASE_SPEED - (int)correction);
            right_speed = clamp_forward_speed(BASE_SPEED + (int)correction);

            motor_driver_set_speed(left_speed, right_speed);
            motors_on = 1;
        }
        else if (lost_line_cycles < LOST_LINE_MAX_CYCLES)
        {
            /* Linea perdida: mantener ultima correccion para intentar recuperar */
            lost_line_cycles++;
            motor_driver_set_speed(left_speed, right_speed);
            motors_on = 1;
        }
        else
        {
            /* Linea perdida por demasiado tiempo: parar */
            left_speed  = 0;
            right_speed = 0;
            correction  = 0.0f;
            motors_on   = 0;
            safe_stop(&line_control);
        }

        TickType_t now = xTaskGetTickCount();

        if ((now - last_display_time) >= pdMS_TO_TICKS(80))
        {
            oled_display_show_status(
                start_active, motors_on, &sensor_data,
                left_speed, right_speed, correction
            );
            last_display_time = now;
        }

        if ((now - last_log_time) >= pdMS_TO_TICKS(80))
        {
            logger_control(&sensor_data, left_speed, right_speed, correction, start_active);
            last_log_time = now;
        }

        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(CONTROL_PERIOD_MS));
    }
}
