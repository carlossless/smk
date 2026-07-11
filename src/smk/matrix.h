#pragma once

#include <stdint.h>

void    matrix_init();
uint8_t matrix_task();

// Called from the Timer 2 ISR. Sweeps all 16 columns in one shot with
// two-sample debounce, writes results to the matrix[] array, sets
// matrix_updated. Main loop polls matrix_updated from matrix_task() and
// dispatches key events.
void matrix_scan_full();
