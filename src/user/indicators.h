#pragma once

#include "keyboard.h"
#include <stdint.h>
#include <stdbool.h>

void indicators_start();
void indicators_pre_update();
bool indicators_update_step(keyboard_state_t *keyboard, uint8_t current_step);
void indicators_post_update();
// Exposed for the matrix scanner. Stock fw blanks the LED PWM around its
// matrix scan window — the column PWM hardware switching at MHz rates
// couples into the long row traces and biases the row-sample. Disabling
// PWM during the sweep silences that crosstalk; enabling it again after
// the sweep restores LEDs from the (still-valid) duty registers.
void indicators_pwm_enable();
void indicators_pwm_disable();
// Settings-side helpers, exposed so the boot flow in main.c can drive
// apply_defaults -> settings_load -> validate explicitly without that
// chain being hidden inside indicators_start.
void indicators_apply_defaults();
void indicators_validate_settings();
