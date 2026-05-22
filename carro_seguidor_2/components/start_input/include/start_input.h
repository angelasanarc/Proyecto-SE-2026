#ifndef START_INPUT_H
#define START_INPUT_H

#include <stdbool.h>

#include "esp_err.h"

esp_err_t start_input_init(void);

int start_input_read_raw(void);

bool start_input_is_active(void);

#endif