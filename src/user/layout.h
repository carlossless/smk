#ifndef LAYOUT_H
#define LAYOUT_H

#include "matrix.h"
#include "keycodes.h"
#include <stdbool.h>

extern const uint16_t keymaps[][MATRIX_ROWS][MATRIX_COLS];

/*
  returns a boolean value to instruct whether further key process should continue or end here
*/
bool layout_process_record(uint16_t keycode, bool key_pressed);

#endif
