#ifndef QTR_CALIBRATION_H
#define QTR_CALIBRATION_H

#include <stdint.h>
#include <stdbool.h>

#include "robot_config.h"

/*
 * Calibracion del QTR-8RC por sensor.
 *
 * Durante la fase de calibracion el robot (o el usuario) pasa los sensores
 * sobre la superficie blanca y la linea negra.  Para cada sensor se registra
 * el tiempo minimo de descarga (blanco) y el maximo (negro).
 *
 * Normalizacion: valor = (raw - min) * 1000 / (max - min)
 *   0   -> blanco (alta reflectancia)
 *   1000 -> negro (baja reflectancia, linea)
 */
typedef struct
{
    uint32_t min_time[NUM_SENSORS];
    uint32_t max_time[NUM_SENSORS];
    int      sample_count;
    bool     calibrated;
} qtr_calibration_t;

/* Inicializa la estructura antes de comenzar la calibracion. */
void qtr_calibration_init(qtr_calibration_t *cal);

/*
 * Actualiza min/max con una nueva lectura de tiempos crudos.
 * Llamar repetidamente durante la fase de calibracion.
 */
void qtr_calibration_update(qtr_calibration_t *cal, const uint32_t raw[NUM_SENSORS]);

/*
 * Convierte tiempos crudos a valores normalizados 0-1000 por sensor.
 * Si la calibracion no esta completa usa un umbral fijo de SENSOR_MAX_READ_US/2.
 */
void qtr_calibration_normalize(
    const qtr_calibration_t *cal,
    const uint32_t raw[NUM_SENSORS],
    int normalized[NUM_SENSORS]
);

/* Devuelve true cuando se han recopilado CALIBRATION_SAMPLES muestras. */
bool qtr_calibration_is_done(const qtr_calibration_t *cal);

/*
 * Devuelve true si la calibracion es valida: al menos 6 de 8 sensores
 * tienen un rango min/max suficiente para distinguir blanco de negro.
 */
bool qtr_calibration_is_valid(const qtr_calibration_t *cal);

#endif
