#pragma once

#include "keyboard.h"
#include <stdint.h>
#include <stdbool.h>

// Clear LED framebuffers before the scan starts, so an uninitialised buffer
// isn't streamed to the LEDs before the first render. No-op on LED-less boards.
void indicators_init();
void indicators_start();
// Foreground LED render, from the main loop: boards that render outside the scan
// do their colour maths here; the scan only streams the framebuffer to the LEDs.
void indicators_render();
void indicators_pre_update();
bool indicators_update_step(keyboard_state_t *keyboard, uint8_t current_step);
void indicators_post_update();
// Blank the LEDs around a matrix scan: the LED drive couples into the row sense
// and biases the sample, so it's silenced for the sweep and restored afterward.
void indicators_pwm_enable();
void indicators_pwm_disable();
// Split out so the boot flow can order apply_defaults -> load -> validate.
void indicators_apply_defaults();
void indicators_validate_settings();
