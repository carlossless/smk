#pragma once

#include <stdint.h>

void    user_matrix_pre_scan(uint8_t col);
uint8_t user_matrix_scan_col(uint8_t col);
void    user_matrix_post_scan();
