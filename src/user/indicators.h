#pragma once

#include "keyboard.h"
#include <stdint.h>
#include <stdbool.h>

// Clear LED framebuffers before the scan ISR starts (called from init(), before
// EA=1). Cold-boot xdata isn't guaranteed zero, so this stops the scan from
// streaming garbage to the LEDs before the first render. No-op on LED-less boards.
void indicators_init();
void indicators_start();
// Foreground LED render, called from the main loop. Boards that render their
// effect frame outside the scan ISR do the colour maths here; the scan ISR
// (indicators_update_step) only streams the framebuffer to the PWM.
void indicators_render();
void indicators_pre_update();
bool indicators_update_step(keyboard_state_t *keyboard, uint8_t current_step);
void indicators_post_update();
// Blank the LED PWM around a matrix scan: the column PWM switching at MHz rates
// couples into the long row traces and biases the row-sample. Disabling PWM
// during the sweep silences that crosstalk; enabling it again restores LEDs
// from the (still-valid) duty registers.
void indicators_pwm_enable();
void indicators_pwm_disable();
// Exposed so the boot flow in main.c can drive
// apply_defaults -> settings_load -> validate explicitly.
void indicators_apply_defaults();
void indicators_validate_settings();
