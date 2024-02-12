#ifndef INDICATORS_H
#define INDICATORS_H

#include "../smk/keyboard.h"
#include <stdint.h>
#include <stdbool.h>

bool indicators_update_step(keyboard_state_t *keyboard, uint8_t current_step);

#endif
