#pragma once

#include "keyboard.h"
#include <stdint.h>
#include <stdbool.h>

void indicators_init();
void indicators_start();
void indicators_render();
void indicators_pre_update();
bool indicators_update_step(keyboard_state_t *keyboard, uint8_t current_step);
void indicators_post_update();
void indicators_pwm_enable();
void indicators_pwm_disable();
void indicators_apply_defaults();
void indicators_validate_settings();
