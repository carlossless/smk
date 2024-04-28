#ifndef USER_MATRIX_H
#define USER_MATRIX_H

#include <stdint.h>

void    matrix_pre_scan(uint8_t col);
uint8_t matrix_scan_col(uint8_t col);
void    matrix_post_scan();

#endif
