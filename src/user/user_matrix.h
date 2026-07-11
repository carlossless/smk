#pragma once

#include <stdint.h>

void    user_matrix_cols_deselect_all(void);
void    user_matrix_col_select(uint8_t col);
void    user_matrix_col_deselect(uint8_t col);
uint8_t user_matrix_read_rows(void);
void user_matrix_scan_pre(void);  // before the column sweep
void user_matrix_scan_post(void); // after the column sweep
void user_matrix_sinks_off(void);
