#pragma once

#include <stdint.h>

// Matrix scan primitives. "Select" = put a column in the active state so its
// keys read on the rows; "deselect" = back to idle. The board defines the
// electrical polarity; the caller only ever selects/deselects. Sequencing:
//
//   1. cols_deselect_all() - every column idle (sweep-start state)
//   2. for each col:
//        col_select(col)    - make this column active (others stay idle)
//        settle, read_rows(), settle, read_rows() - sample twice to debounce
//        col_deselect(col)  - back to idle
void    user_matrix_cols_deselect_all(void);
void    user_matrix_col_select(uint8_t col);
void    user_matrix_col_deselect(uint8_t col);
uint8_t user_matrix_read_rows(void);
// Per-sweep board hooks, for column-pin setup/teardown around a scan. A board
// that shares its column pins drives them for the sweep and releases them to
// high-Z at rest; boards with dedicated column outputs leave both as no-ops.
void user_matrix_scan_pre(void);  // before the column sweep
void user_matrix_scan_post(void); // after the column sweep
// Drop any lit LED sink before each sweep, so LED current doesn't flow through
// the matrix and bias the row sense during sampling.
void user_matrix_sinks_off(void);
