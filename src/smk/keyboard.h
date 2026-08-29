#pragma once

#include <stdint.h>

typedef struct {
    uint8_t led_state;
    uint8_t rf_link;
    uint8_t battery_level; // 0..7
    uint8_t low_power;     // 1 when the battery is critically low
    uint8_t connected;     // 1 when the active RF link has a host paired+connected
    uint8_t paired;        // 1 when the active RF link has a paired host
} keyboard_state_t;

typedef struct {
    uint8_t nkro;
} keymap_config_t;

extern volatile keyboard_state_t keyboard_state;
extern keymap_config_t           keymap_config;

void keyboard_init(void);

// Where a host link reports the lock-LED mask (caps / num / scroll) the host
// wants lit. Called from the USB control-transfer ISR, so it stays a single
// store and nothing more.
void keyboard_set_led_state(uint8_t led_state);
