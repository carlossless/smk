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
// At exit every column is HIGH. The LED PWM is expected to be disabled
// for the whole sweep — the column GPIOs and PWM channels share pins.
void    user_matrix_cols_high_all(void);
void    user_matrix_col_low(uint8_t col);
void    user_matrix_col_high(uint8_t col);
uint8_t user_matrix_read_rows(void);
// Drop any RGB row sink that's currently lit. Called before each matrix
// sweep so no LED current flows through the matrix during sampling
// (mirrors stock fw — sources of bias on the row pull-ups).
void    user_matrix_sinks_off(void);
