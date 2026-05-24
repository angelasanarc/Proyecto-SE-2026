#include "start_input.h"

#include "driver/gpio.h"
#include "robot_config.h"

/*
 * Debounce por histéresis de estado.
 *
 * Se llama una vez por ciclo de control (cada CONTROL_PERIOD_MS).
 * El estado solo cambia despues de BUTTON_DEBOUNCE_CYCLES lecturas
 * consecutivas con el nuevo nivel, eliminando rebotes y flancos
 * inestables sin bloquear el loop de control.
 */

static bool s_state         = false;
static int  s_counter       = 0;

esp_err_t start_input_init(void)
{
    gpio_config_t cfg = {
        .pin_bit_mask = (1ULL << PIN_START),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE
    };

    s_state   = false;
    s_counter = 0;

    return gpio_config(&cfg);
}

bool start_input_is_active(void)
{
    bool reading = (gpio_get_level(PIN_START) == START_ACTIVE_LEVEL);

    if (reading == s_state)
    {
        s_counter = 0;
    }
    else
    {
        s_counter++;

        if (s_counter >= BUTTON_DEBOUNCE_CYCLES)
        {
            s_state   = reading;
            s_counter = 0;
        }
    }

    return s_state;
}
