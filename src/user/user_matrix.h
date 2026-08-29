#pragma once

#include <stdint.h>

void    user_matrix_cols_deselect_all(void);
void    user_matrix_col_select(uint8_t col);
void    user_matrix_col_deselect(uint8_t col);
uint8_t user_matrix_read_rows(void);
// Per-sweep board hooks, for column-pin setup/teardown around a scan. A board
// that shares its column pins drives them for the sweep and releases them to
// high-Z at rest; boards with dedicated column outputs leave both as no-ops.
void user_matrix_scan_pre(void);  // before the column sweep
void user_matrix_scan_post(void); // after the column sweep
void user_matrix_sinks_off(void);
