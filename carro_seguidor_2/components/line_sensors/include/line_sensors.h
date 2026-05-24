#ifndef LINE_SENSORS_H
#define LINE_SENSORS_H

#include <stdint.h>
#include <stdbool.h>

#include "esp_err.h"
#include "robot_config.h"

typedef struct
{
    int  detected[NUM_SENSORS]; /* 0 o 1 por sensor (para visualizacion)  */
    int  position;              /* posicion ponderada 0-7000              */
    int  error;                 /* error respecto a LINE_CENTER_POSITION  */
    bool line_detected;

} line_sensor_data_t;

/* Configura los GPIO como salida/entrada segun el ciclo RC. */
esp_err_t line_sensors_init(void);

/*
 * Lee los 8 sensores QTR-8RC en paralelo midiendo el tiempo de descarga.
 * raw[i]: tiempo en µs hasta que el sensor i baja a LOW.
 *   Valor bajo  -> alta reflectancia (blanco).
 *   Valor alto  -> baja reflectancia (negro / linea).
 *   SENSOR_MAX_READ_US -> timeout (definitivamente negro).
 */
esp_err_t line_sensors_read_raw(uint32_t raw[NUM_SENSORS]);

/*
 * Calcula posicion y error a partir de valores normalizados (0-1000 c/u).
 * detected[i] = 1 si normalized[i] > CALIBRATION_THRESHOLD.
 * La posicion se calcula como media ponderada por el valor normalizado.
 */
esp_err_t line_sensors_compute(
    const int normalized[NUM_SENSORS],
    line_sensor_data_t *data
);

#endif
