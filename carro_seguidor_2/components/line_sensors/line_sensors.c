#include "line_sensors.h"

#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"

static const gpio_num_t sensor_gpio[NUM_SENSORS] =
{
    GPIO_NUM_23,
    GPIO_NUM_19,
    GPIO_NUM_18,
    GPIO_NUM_27,
    GPIO_NUM_26,
    GPIO_NUM_25,
    GPIO_NUM_33,
    GPIO_NUM_32
};

esp_err_t line_sensors_init(void)
{
    /* Los pines se reconfiguran dinamicamente en cada lectura RC.
     * Aqui solo verificamos que sean validos configurandolos como entrada. */
    uint64_t sensor_mask = 0;

    for (int i = 0; i < NUM_SENSORS; i++)
    {
        sensor_mask |= (1ULL << sensor_gpio[i]);
    }

    gpio_config_t cfg = {
        .pin_bit_mask = sensor_mask,
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE
    };

    return gpio_config(&cfg);
}

esp_err_t line_sensors_read_raw(uint32_t raw[NUM_SENSORS])
{
    if (raw == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    /* Paso 1: cargar todos los capacitores simultaneamente */
    for (int i = 0; i < NUM_SENSORS; i++)
    {
        gpio_set_direction(sensor_gpio[i], GPIO_MODE_OUTPUT);
        gpio_set_level(sensor_gpio[i], 1);
    }

    esp_rom_delay_us(SENSOR_CHARGE_US);

    /* Paso 2: inicializar tiempos y pasar todos a entrada */
    for (int i = 0; i < NUM_SENSORS; i++)
    {
        raw[i] = SENSOR_MAX_READ_US;
        gpio_set_direction(sensor_gpio[i], GPIO_MODE_INPUT);
    }

    /* Paso 3: medir tiempo de descarga de cada sensor en paralelo */
    int64_t start     = esp_timer_get_time();
    int     remaining = NUM_SENSORS;
    bool    settled[NUM_SENSORS];

    for (int i = 0; i < NUM_SENSORS; i++) settled[i] = false;

    while (remaining > 0)
    {
        int64_t  now     = esp_timer_get_time();
        uint32_t elapsed = (uint32_t)(now - start);

        if (elapsed >= SENSOR_MAX_READ_US)
        {
            break;
        }

        for (int i = 0; i < NUM_SENSORS; i++)
        {
            if (!settled[i] && !gpio_get_level(sensor_gpio[i]))
            {
                raw[i]      = elapsed;
                settled[i]  = true;
                remaining--;
            }
        }
    }

    return ESP_OK;
}

esp_err_t line_sensors_compute(
    const int normalized[NUM_SENSORS],
    line_sensor_data_t *data
)
{
    if (normalized == NULL || data == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    long weighted_sum  = 0;
    long total_weight  = 0;
    bool any_detected  = false;

    for (int i = 0; i < NUM_SENSORS; i++)
    {
        data->detected[i] = (normalized[i] > CALIBRATION_THRESHOLD) ? 1 : 0;

        if (data->detected[i])
        {
            /* Media ponderada continua: mejor resolucion de posicion */
            weighted_sum += (long)(i * 1000) * normalized[i];
            total_weight += normalized[i];
            any_detected  = true;
        }
    }

    if (!any_detected)
    {
        data->line_detected = false;
        /* Conservar ultima posicion conocida para que el PID sepa hacia donde girar */
        return ESP_OK;
    }

    data->line_detected = true;
    data->position      = (int)(weighted_sum / total_weight);
    data->error         = data->position - LINE_CENTER_POSITION;

    return ESP_OK;
}
