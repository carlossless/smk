#pragma once

#include <stdint.h>

void    matrix_init();
uint8_t matrix_task();

// Sweep every column once with two-sample debounce, write results to matrix[],
// and set matrix_updated. matrix_task() polls that flag and dispatches events.
void matrix_scan_full();
