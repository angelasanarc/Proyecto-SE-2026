#include "start_input.h"

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "robot_config.h"

esp_err_t start_input_init(void)
{
    gpio_config_t start_config =
    {
        .pin_bit_mask = (1ULL << PIN_START),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    return gpio_config(&start_config);
}

int start_input_read_raw(void)
{
    int ones = 0;

    for (int i = 0; i < 20; i++)
    {
        if (gpio_get_level(PIN_START))
        {
            ones++;
        }

        vTaskDelay(pdMS_TO_TICKS(1));
    }

    return (ones >= 10) ? 1 : 0;
}

bool start_input_is_active(void)
{
    int level = start_input_read_raw();

    return level == START_ACTIVE_LEVEL;
}