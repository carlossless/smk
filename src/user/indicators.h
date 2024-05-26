#ifndef INDICATORS_H
#define INDICATORS_H

#include "keyboard.h"
#include <stdint.h>
#include <stdbool.h>

void indicators_start();
void indicators_pre_update();
bool indicators_update_step(keyboard_state_t *keyboard, uint8_t current_step);
void indicators_post_update();

#endif
