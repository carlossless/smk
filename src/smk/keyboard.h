#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

typedef struct {
    uint8_t              led_state;
} keyboard_state_t;

extern volatile __xdata keyboard_state_t keyboard_state;

void keyboard_init();

#endif
