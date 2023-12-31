#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

typedef enum {
    KEYBOARD_CONN_MODE_RF  = 0,
    KEYBOARD_CONN_MODE_USB = 1,
} keyboard_conn_mode_t;

typedef enum {
    KEYBOARD_OS_MODE_WIN = 0,
    KEYBOARD_OS_MODE_MAC = 1,
} keyboard_os_mode_t;

typedef struct {
    keyboard_conn_mode_t conn_mode;
    keyboard_os_mode_t   os_mode;
    uint8_t              led_state;
} keyboard_state_t;

extern __xdata keyboard_state_t keyboard_state;

#endif
