#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "robot_config.h"
#include "qtr_calibration.h"
#include "line_sensors.h"
#include "start_input.h"
#include "motor_driver.h"
#include "line_control.h"
#include "logger.h"
#include "oled_display.h"
#include "wifi_monitor.h"

/* ---- Display task: pantalla en tarea separada para no bloquear el control ---- */
typedef enum { DISP_BOOT, DISP_CALIBRATING, DISP_WAITING, DISP_RUNNING } disp_mode_t;

typedef struct {
    disp_mode_t        mode;
    bool               boot_sensors;
    bool               boot_motors;
    bool               boot_start;
    bool               boot_wifi;
    int                cal_norm[NUM_SENSORS];
    int                cal_samples;
    int                start_active;
    int                motors_on;
    int                left_speed;
    int                right_speed;
    float              correction;
    int                run_count;
    line_sensor_data_t sensor_data;
} disp_state_t;

static SemaphoreHandle_t s_disp_mutex;
static disp_state_t      s_disp;

static void disp_set(const disp_state_t *st)
{
    if (xSemaphoreTake(s_disp_mutex, 0) == pdTRUE) {
        s_disp = *st;
        xSemaphoreGive(s_disp_mutex);
    }
}

static void display_task(void *arg)
{
    disp_state_t st = {0};
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(80));
        if (xSemaphoreTake(s_disp_mutex, 0) == pdTRUE) {
            st = s_disp;
            xSemaphoreGive(s_disp_mutex);
        }
        switch (st.mode) {
        case DISP_BOOT:
            oled_display_show_boot(st.boot_sensors, st.boot_motors,
                                   st.boot_start, st.boot_wifi);
            break;
        case DISP_CALIBRATING:
            oled_display_show_calibrating(st.cal_norm, st.cal_samples);
            break;
        case DISP_WAITING:
            oled_display_show_calibrated();
            break;
        case DISP_RUNNING:
            oled_display_show_status(st.start_active, st.motors_on,
                                     &st.sensor_data,
                                     st.left_speed, st.right_speed,
                                     st.correction, st.run_count);
            break;
        }
    }
}

/* ---- Helpers ---- */
static int clamp_speed(int v)
{
    if (v > MAX_SPEED) return MAX_SPEED;
    if (v < MIN_SPEED) return MIN_SPEED;
    return v;
}

static void safe_stop(line_control_t *control)
{
    motor_driver_stop();
    line_control_reset(control);
}

/* ---- app_main ---- */
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

    int   left_speed       = 0;
    int   right_speed      = 0;
    float correction       = 0.0f;
    int   start_active     = 0;
    int   prev_start       = 0;
    int   motors_on        = 0;
    int   lost_line_cycles = 0;
    int   run_count        = 0;

    /* ---- Inicializacion ---- */
    logger_init();
    logger_system("Inicializando sistema");

    s_disp_mutex = xSemaphoreCreateMutex();
    memset(&s_disp, 0, sizeof(s_disp));

    /* Tarea de pantalla arranca inmediatamente para que la OLED prenda */
    oled_display_init();
    xTaskCreatePinnedToCore(display_task, "display", 4096, NULL, 1, NULL, 1);

    bool sensors_ok = (line_sensors_init()  == ESP_OK);
    bool motors_ok  = (motor_driver_init()  == ESP_OK);
    bool start_ok   = (start_input_init()   == ESP_OK);

    if (!sensors_ok) logger_system("ERROR: sensores no inicializados");
    if (!motors_ok)  logger_system("ERROR: motores no inicializados");
    if (!start_ok)   logger_system("ERROR: boton START no inicializado");

    /* Mostrar estado de subsistemas criticos inmediatamente (WiFi aun pendiente) */
    {
        disp_state_t d = {
            .mode         = DISP_BOOT,
            .boot_sensors = sensors_ok,
            .boot_motors  = motors_ok,
            .boot_start   = start_ok,
            .boot_wifi    = false,
        };
        disp_set(&d);
    }

    bool wifi_ok = (wifi_monitor_init()  == ESP_OK);

    motor_driver_stop();

    if (!wifi_ok) logger_system("WARN: WiFi no disponible");

    /* Actualizar estado de WiFi en pantalla */
    {
        disp_state_t d = {
            .mode         = DISP_BOOT,
            .boot_sensors = sensors_ok,
            .boot_motors  = motors_ok,
            .boot_start   = start_ok,
            .boot_wifi    = wifi_ok,
        };
        disp_set(&d);
    }

    vTaskDelay(pdMS_TO_TICKS(2000));

    /* Bloquear si subsistemas criticos fallaron (pantalla sigue mostrando el error) */
    if (!sensors_ok || !motors_ok || !start_ok)
    {
        logger_system("FALLA CRITICA — sistema detenido");
        while (1) vTaskDelay(pdMS_TO_TICKS(1000));
    }

    /* Cargar parametros desde NVS */
    robot_params_t params;
    wifi_monitor_get_params(&params);
    int base_speed = params.base_speed;

    line_control_init(
        &line_control,
        params.kp, params.ki, params.kd,
        CONTROL_PERIOD_S,
        CONTROL_DERIVATIVE_ALPHA,
        CONTROL_OUTPUT_MIN,
        CONTROL_OUTPUT_MAX
    );

    /* ---- Fase de calibracion ---- */
    qtr_calibration_init(&calibration);
    logger_system("Calibrando sensores");

    for (int i = 0; i < NUM_SENSORS; i++) normalized[i] = 0;
    {
        disp_state_t d = {.mode = DISP_CALIBRATING, .cal_samples = 0};
        memcpy(d.cal_norm, normalized, sizeof(normalized));
        disp_set(&d);
    }

    while (!qtr_calibration_is_done(&calibration))
    {
        line_sensors_read_raw(raw_times);
        qtr_calibration_update(&calibration, raw_times);
        qtr_calibration_normalize(&calibration, raw_times, normalized);

        disp_state_t d = {.mode = DISP_CALIBRATING,
                          .cal_samples = calibration.sample_count};
        memcpy(d.cal_norm, normalized, sizeof(normalized));
        disp_set(&d);

        vTaskDelay(pdMS_TO_TICKS(CONTROL_PERIOD_MS));
    }

    if (qtr_calibration_is_valid(&calibration))
        logger_system("Calibracion valida (>=6 sensores con rango correcto)");
    else
        logger_system("WARN: calibracion debil (<6 sensores con rango suficiente)");

    logger_system("Calibracion completa - esperando START");
    {
        disp_state_t d = {.mode = DISP_WAITING};
        disp_set(&d);
    }

    /* ---- Esperar START explicito ---- */
    for (int i = 0; i < 10; i++)
    {
        start_input_is_active();
        vTaskDelay(pdMS_TO_TICKS(CONTROL_PERIOD_MS));
    }

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
    TickType_t last_wake_time = xTaskGetTickCount();
    TickType_t last_log_time  = xTaskGetTickCount();

    while (1)
    {
        line_sensors_read_raw(raw_times);
        qtr_calibration_normalize(&calibration, raw_times, normalized);
        line_sensors_compute(normalized, &sensor_data);

        /* Aplicar parametros actualizados desde la UI web */
        {
            robot_params_t np;
            if (wifi_monitor_params_changed(&np)) {
                params     = np;
                base_speed = params.base_speed;
                line_control_init(&line_control,
                                  params.kp, params.ki, params.kd,
                                  CONTROL_PERIOD_S, CONTROL_DERIVATIVE_ALPHA,
                                  CONTROL_OUTPUT_MIN, CONTROL_OUTPUT_MAX);
                logger_system("Parametros PID actualizados");
            }
        }

        start_active = start_input_is_active() ? 1 : 0;

        /* Detectar flanco start inactivo→activo para contar recorridos */
        if (!prev_start && start_active)
        {
            run_count++;
            logger_run_start(run_count);
        }
        else if (prev_start && !start_active)
        {
            logger_run_end(run_count);
        }
        prev_start = start_active;

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
            lost_line_cycles = 0;
            correction = line_control_compute(&line_control, (float)sensor_data.error);

            left_speed  = clamp_speed(base_speed - (int)correction);
            right_speed = clamp_speed(base_speed + (int)correction);

            motor_driver_set_speed(left_speed, right_speed);
            motors_on = 1;
        }
        else if (lost_line_cycles < LOST_LINE_MAX_CYCLES)
        {
            lost_line_cycles++;
            float recov = params.kp * (float)sensor_data.error;
            if (recov >  CONTROL_OUTPUT_MAX) recov =  CONTROL_OUTPUT_MAX;
            if (recov <  CONTROL_OUTPUT_MIN) recov =  CONTROL_OUTPUT_MIN;
            left_speed  = clamp_speed(base_speed - (int)recov);
            right_speed = clamp_speed(base_speed + (int)recov);
            motor_driver_set_speed(left_speed, right_speed);
            motors_on = 1;
        }
        else
        {
            left_speed  = 0;
            right_speed = 0;
            correction  = 0.0f;
            motors_on   = 0;
            safe_stop(&line_control);
        }

        /* Actualizar display (sin bloquear — la tarea display lee esto cada 80ms) */
        {
            disp_state_t d = {
                .mode        = DISP_RUNNING,
                .start_active = start_active,
                .motors_on   = motors_on,
                .left_speed  = left_speed,
                .right_speed = right_speed,
                .correction  = correction,
                .run_count   = run_count,
                .sensor_data = sensor_data,
            };
            disp_set(&d);
        }

        /* Telemetria WiFi */
        {
            robot_telemetry_t telem = {
                .error         = sensor_data.error,
                .position      = sensor_data.position,
                .left_speed    = left_speed,
                .right_speed   = right_speed,
                .correction    = correction,
                .line_detected = sensor_data.line_detected,
                .start_active  = start_active,
                .motors_on     = motors_on,
            };
            for (int i = 0; i < NUM_SENSORS; i++) telem.sensors[i] = normalized[i];
            wifi_monitor_update(&telem);
        }

        if ((xTaskGetTickCount() - last_log_time) >= pdMS_TO_TICKS(80))
        {
            logger_control(&sensor_data, left_speed, right_speed, correction, start_active);
            last_log_time = xTaskGetTickCount();
        }

        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(CONTROL_PERIOD_MS));
    }
}
