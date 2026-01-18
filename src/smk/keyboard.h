#pragma once

#include <stdint.h>

typedef struct {
    uint8_t led_state;
    uint8_t rf_link;
} keyboard_state_t;

typedef struct {
    uint8_t nkro;
} keymap_config_t;

extern volatile __xdata keyboard_state_t keyboard_state;
extern __xdata keymap_config_t           keymap_config;

void keyboard_init();
