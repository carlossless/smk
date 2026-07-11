#pragma once

#include <stdint.h>

// Matrix scan primitives. "Select" = put a column in the active state so its
// keys read on the rows; "deselect" = back to idle. Each board defines the
// electrical polarity (the Air60 columns are active-LOW: select drives low,
// deselect drives high). matrix.c only deals in select/deselect, so it works
// for active-high boards too. The sequencing is:
//
//   1. cols_deselect_all() — every column idle (sweep-start state)
//   2. for each col 0..N-1:
//        col_select(col)    — make this column active (others stay idle)
//        delay_us(2)        — settle the row pull-ups
//        read_rows()        — sample once
//        delay_us(2)
//        read_rows()        — sample twice (debounce confirm)
//        col_deselect(col)  — back to idle
//
// What the columns look like before/after the sweep is up to the board's
// scan_pre / scan_post hooks (below). The LED PWM is expected to be disabled
// for the whole sweep — the column GPIOs and PWM channels share pins.
void    user_matrix_cols_deselect_all(void);
void    user_matrix_col_select(uint8_t col);
void    user_matrix_col_deselect(uint8_t col);
uint8_t user_matrix_read_rows(void);
// Per-sweep board hooks, called right before/after the column sweep so a board
// can do whatever column-pin setup/teardown it needs around a scan. A board with
// muxed column pins switches them to output for the sweep and releases them to
// input/high-Z at rest, so that when the LED PWM is parked a column floats
// instead of sourcing. Boards whose columns are permanently outputs leave both
// as no-ops.
void user_matrix_scan_pre(void);  // before the column sweep
void user_matrix_scan_post(void); // after the column sweep
// Drop any RGB row sink that's currently lit, before each sweep, so no LED
// current flows through the matrix and biases the row pull-ups during sampling.
void user_matrix_sinks_off(void);
