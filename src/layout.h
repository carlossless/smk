#ifndef LAYOUT_H
#define LAYOUT_H

#include "matrix.h"
#include "keycodes.h"
#include <stdbool.h>

extern const uint16_t keymaps[][MATRIX_ROWS][MATRIX_COLS];

bool process_record_user(uint16_t keycode, bool key_pressed);

#endif
