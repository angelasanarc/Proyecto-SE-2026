#ifndef START_INPUT_H
#define START_INPUT_H

#include <stdbool.h>

#include "esp_err.h"

esp_err_t start_input_init(void);

/*
 * Devuelve el estado estable del boton.
 * Debe llamarse una vez por ciclo de control.
 * No bloquea — cambia de estado solo tras BUTTON_DEBOUNCE_CYCLES
 * lecturas consecutivas con el nuevo nivel.
 */
bool start_input_is_active(void);

#endif
