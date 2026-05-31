#pragma once

#include <stdint.h>

// Persistent user settings, stored as one blob in flash (see settings.c / flash.h).
//
// To save more things across power cycles, add fields to this struct. Keep it small
// (it shares one 512-byte flash sector) and note that a firmware reflash resets it to
// the defaults set by the caller before settings_load().
typedef struct {
    // main backlight (key matrix rows)
    uint8_t led_effect;     // selected backlight effect (or "off")
    uint8_t led_brightness; // 0 (dark) .. 255 (full)
    uint8_t led_speed;      // phase increment per regen sweep (1 = slowest)
    // user / underglow lights (independent of main)
    uint8_t ul_effect;
    uint8_t ul_brightness;
    uint8_t ul_speed;
    // 1 = right-side underglow LEDs continuously show battery level color
    // (red <20%, yellow 20-80%, green >80%; red also when low_power is set).
    // 0 = right-side underglow follows the regular UL effect.
    uint8_t battery_indicator_on;
    // Last RF link mode selected by the user (RF_MODE_2_4G / BT1 / BT2 / BT3).
    // Re-applied on boot so the keyboard comes back on the host it was last on.
    uint8_t rf_link;
} user_settings_t;

extern __xdata user_settings_t user_settings;

// Overwrites `user_settings` with the persisted values if a valid record exists;
// otherwise leaves it untouched. Set defaults into `user_settings` before calling.
void settings_load(void);

// Persists the current `user_settings` (no-op if it already matches what's stored).
void settings_save(void);
