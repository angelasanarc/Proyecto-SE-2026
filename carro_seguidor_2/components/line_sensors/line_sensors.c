#include "line_sensors.h"

#include "driver/gpio.h"

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

static int read_sensor_stable(gpio_num_t pin)
{
    int ones = 0;

    for (int i = 0; i < 5; i++)
    {
        if (gpio_get_level(pin))
        {
            ones++;
        }
    }

    return (ones >= 3) ? 1 : 0;
}

esp_err_t line_sensors_init(void)
{
    uint64_t sensor_mask = 0;

    for (int i = 0; i < NUM_SENSORS; i++)
    {
        sensor_mask |= (1ULL << sensor_gpio[i]);
    }

    gpio_config_t sensor_config =
    {
        .pin_bit_mask = sensor_mask,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    return gpio_config(&sensor_config);
}

esp_err_t line_sensors_read(line_sensor_data_t *data)
{
    if (data == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    long weighted_sum = 0;
    long total = 0;

    for (int i = 0; i < NUM_SENSORS; i++)
    {
        int level = read_sensor_stable(sensor_gpio[i]);

        if (level == DIGITAL_LINE_LEVEL)
        {
            data->detected[i] = 1;
        }
        else
        {
            data->detected[i] = 0;
        }

        if (data->detected[i])
        {
            weighted_sum += (long)(i * 1000);
            total++;
        }
    }

    if (total == 0)
    {
        data->line_detected = false;
        data->position = LINE_CENTER_POSITION;
        data->error = 0;

        return ESP_OK;
    }

    data->line_detected = true;
    data->position = weighted_sum / total;
    data->error = data->position - LINE_CENTER_POSITION;

    return ESP_OK;
}