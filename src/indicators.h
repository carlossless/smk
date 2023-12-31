#ifndef INDICATORS_H
#define INDICATORS_H

#include "keyboard.h"
#include <stdint.h>
#include <stdbool.h>

bool indicators_update(__xdata keyboard_state_t *keyboard);
bool indicators_update_step(__xdata keyboard_state_t *keyboard, uint8_t current_step);

#endif
