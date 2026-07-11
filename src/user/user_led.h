#pragma once

#include <stdint.h>

// Per-board LED geometry for the effect engine: a scalar field over the key
// matrix, read as hues by a colour board or as a brightness wave by a mono one.
// Boards with LEDs supply generated definitions; LED-less boards return 0.
// __reentrant: on some boards the effect render runs inside the Timer2 scan ISR
// (indicators_update_step -> led_regen_one -> these), so their params must live on
// the stack, not SDCC's static overlay, or the ISR corrupts a main-loop function
// sharing that slot. See project_sdcc_isr_overlay_collision; enforced by
// utils/check_isr_overlay.py.
uint8_t user_led_radial(uint8_t row, uint8_t col) __reentrant; // 0 (centre) .. 255 (corner)
uint8_t user_led_axis_x(uint8_t col) __reentrant;              // scalar per column (horizontal)
uint8_t user_led_axis_y(uint8_t row) __reentrant;              // scalar per row (vertical)
