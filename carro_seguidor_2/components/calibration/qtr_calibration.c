#include "qtr_calibration.h"

#include <stdint.h>

void qtr_calibration_init(qtr_calibration_t *cal)
{
    for (int i = 0; i < NUM_SENSORS; i++)
    {
        cal->min_time[i] = UINT32_MAX;
        cal->max_time[i] = 0;
    }

    cal->sample_count = 0;
    cal->calibrated   = false;
}

void qtr_calibration_update(qtr_calibration_t *cal, const uint32_t raw[NUM_SENSORS])
{
    for (int i = 0; i < NUM_SENSORS; i++)
    {
        if (raw[i] < cal->min_time[i]) cal->min_time[i] = raw[i];
        if (raw[i] > cal->max_time[i]) cal->max_time[i] = raw[i];
    }

    cal->sample_count++;

    if (cal->sample_count >= CALIBRATION_SAMPLES)
    {
        cal->calibrated = true;
    }
}

void qtr_calibration_normalize(
    const qtr_calibration_t *cal,
    const uint32_t raw[NUM_SENSORS],
    int normalized[NUM_SENSORS]
)
{
    for (int i = 0; i < NUM_SENSORS; i++)
    {
        if (!cal->calibrated || cal->max_time[i] <= cal->min_time[i])
        {
            /* Sin calibracion: umbral fijo en la mitad del rango */
            normalized[i] = (raw[i] >= (SENSOR_MAX_READ_US / 2)) ? 1000 : 0;
            continue;
        }

        int32_t range = (int32_t)(cal->max_time[i] - cal->min_time[i]);
        int32_t val   = (int32_t)(raw[i] - cal->min_time[i]) * 1000 / range;

        if (val < 0)    val = 0;
        if (val > 1000) val = 1000;

        normalized[i] = (int)val;
    }
}

bool qtr_calibration_is_done(const qtr_calibration_t *cal)
{
    return cal->calibrated;
}
