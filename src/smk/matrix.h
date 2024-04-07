#ifndef MATRIX_H
#define MATRIX_H

#include <stdint.h>

#define MATRIX_COLS 16
#define MATRIX_ROWS 5

inline void        matrix_init();
inline uint8_t     matrix_task();
inline void matrix_scan_step();

#endif
