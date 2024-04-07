#ifndef LAYOUT_H
#define LAYOUT_H

#include "../smk/matrix.h"
#include "../smk/keycodes.h"
#include <stdbool.h>

extern const uint16_t keymaps[][MATRIX_ROWS][MATRIX_COLS];

/*
  returns a boolean value to instruct whether further key process should continue or end here
*/
bool process_record_user(uint16_t keycode, bool key_pressed);

#endif
