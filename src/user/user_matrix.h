#pragma once

#include <stdint.h>

// Stock-style matrix scan primitives. The sequencing is:
//
//   1. cols_high_all()     — raise every column HIGH (sweep-start state)
//   2. for each col 0..15:
//        col_low(col)      — drive this column LOW (others stay HIGH)
//        delay_us(2)       — settle the row pull-ups
//        read_rows()       — sample once
//        delay_us(2)
//        read_rows()       — sample twice (debounce confirm)
//        col_high(col)     — release this column back HIGH
//
// At exit every column is released to INPUT / high-Z (see cols_output /
// cols_highz). The LED PWM is expected to be disabled for the whole sweep —
// the column GPIOs and PWM channels share pins.
void    user_matrix_cols_high_all(void);
void    user_matrix_col_low(uint8_t col);
void    user_matrix_col_high(uint8_t col);
uint8_t user_matrix_read_rows(void);
// Column pin DIRECTION control (PxCR), mirroring stock fw: the muxed
// column/PWM pins are driven as outputs only while the matrix is being
// swept, and released to input (high-Z) at rest. That way, whenever the LED
// PWM is parked (per-subframe, or for the ~5 ms flash-erase stall), the
// column pins float instead of sourcing — so a frozen scan can't hold a row
// bright (stock relies on exactly this; the PWM peripheral still drives the
// pins when enabled, regardless of the input direction). The columns carry
// no pull-up, so input == true high-Z.
void    user_matrix_cols_output(void); // PxCR -> output (drive)
void    user_matrix_cols_highz(void);  // PxCR -> input (high-Z, no source)
// Drop any RGB row sink that's currently lit. Called before each matrix
// sweep so no LED current flows through the matrix during sampling
// (mirrors stock fw — sources of bias on the row pull-ups).
void    user_matrix_sinks_off(void);
