#ifndef LAYOUT_H
#define LAYOUT_H

#include "../smk/matrix.h"
#include "../smk/keycodes.h"
#include <stdbool.h>

extern const uint16_t keymaps[][MATRIX_ROWS][MATRIX_COLS];

bool process_record_user(uint16_t keycode, bool key_pressed);

#endif
