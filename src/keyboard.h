#ifndef KEYBOARD_H
#define KEYBOARD_H

typedef struct {
    char keyboard_led_state;
} keyboard_state_t;

extern __xdata keyboard_state_t keyboard_state;

#endif
