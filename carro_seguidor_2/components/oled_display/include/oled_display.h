#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include "esp_err.h"
#include "line_sensors.h"

esp_err_t oled_display_init(void);
void      oled_display_clear(void);

/*
 * Pantalla de calibracion activa.
 * normalized[i]: valor 0-1000 de cada sensor (0=blanco, 1000=negro).
 * samples: muestras recolectadas hasta ahora.
 * Muestra 8 barras verticales + barra de progreso + contador.
 */
void oled_display_show_calibrating(const int normalized[NUM_SENSORS], int samples);

/*
 * Pantalla que aparece cuando la calibracion termina, esperando START.
 */
void oled_display_show_calibrated(void);

/*
 * Pantalla de operacion normal.
 * correction: salida del PID (para mostrar en pantalla).
 */
void oled_display_show_status(
    int start_active,
    int motors_on,
    const line_sensor_data_t *sensor_data,
    int left_speed,
    int right_speed,
    float correction
);

#endif
