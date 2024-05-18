#ifndef LAYOUT_H
#define LAYOUT_H

#include "matrix.h"
#include "keycodes.h"
#include <stdbool.h>

#define MATRIX_ROWS 5
#define MATRIX_COLS 14

extern const uint16_t keymaps[][MATRIX_ROWS][MATRIX_COLS];

/*
  returns a boolean value to instruct whether further key process should continue or end here
*/
bool layout_process_record(uint16_t keycode, bool key_pressed);

#endif
