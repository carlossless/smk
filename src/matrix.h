#ifndef _MATRIX_H_
#define _MATRIX_H_

#include <stdint.h>

#define MATRIX_COLS 16
#define MATRIX_ROWS 5

extern volatile uint8_t key_state[MATRIX_COLS];
extern volatile __bit key_state_changed;
extern volatile uint16_t current_cycle;

void matrix_init();
inline void matrix_scan();

#endif
