#pragma once

#include <stdint.h>

uint8_t user_led_radial(uint8_t row, uint8_t col); // 0 (centre) .. 255 (corner)
uint8_t user_led_axis_x(uint8_t col);              // scalar per column (horizontal)
uint8_t user_led_axis_y(uint8_t row);              // scalar per row (vertical)
