#ifndef USER_MATRIX_H
#define USER_MATRIX_H

#include <stdint.h>

void    user_matrix_pre_scan(uint8_t col);
uint8_t user_matrix_scan_col(uint8_t col);
void    user_matrix_post_scan();

#endif
