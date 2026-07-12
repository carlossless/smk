#pragma once

#include <stdint.h>

// Per-board LED geometry for the effect engine: a scalar field over the key
// matrix, read as hues by a colour board or as a brightness wave by a mono one.
// Boards with LEDs supply generated definitions; LED-less boards return 0.
uint8_t user_led_radial(uint8_t row, uint8_t col); // 0 (centre) .. 255 (corner)
uint8_t user_led_axis_x(uint8_t col);              // scalar per column (horizontal)
uint8_t user_led_axis_y(uint8_t row);              // scalar per row (vertical)
