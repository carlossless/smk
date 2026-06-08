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
// effect frame outside the scan ISR (like stock) do the colour maths here; the
// scan ISR (indicators_update_step) only streams the framebuffer to the PWM.
void indicators_render();
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
