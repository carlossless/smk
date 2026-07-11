#pragma once

#include "kbdef.h"
#include <stdbool.h>

extern const uint16_t keymaps[][MATRIX_ROWS][MATRIX_COLS];

// Retarget the base layer used for keys that aren't under a held MO() layer.
// Defaults to 0. For keyboards with runtime-selectable base layouts.
void set_default_layer(uint8_t layer);
