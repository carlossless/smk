#pragma once

#include <stdint.h>

// Per-board LED geometry, consumed by the shared effect engine (src/smk/
// led_effect.c). These are a scalar field over the key matrix; a colour board
// reads them as hues, a single-colour board as a brightness wave. Boards with
// LEDs supply generated definitions; LED-less boards fall back to no-op defaults
// that return 0.
uint8_t user_led_radial(uint8_t row, uint8_t col); // 0 (centre) .. 255 (corner)
uint8_t user_led_axis_x(uint8_t col);              // scalar per column (horizontal)
uint8_t user_led_axis_y(uint8_t row);              // scalar per row (vertical)
