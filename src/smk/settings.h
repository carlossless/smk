#pragma once

#include <stdint.h>
#include <stdbool.h>

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
    uint8_t battery_indicator_on;
    uint8_t rf_link;
} user_settings_t;

extern __xdata user_settings_t user_settings;

bool settings_load(void);

void settings_save(void);

void settings_mark_dirty(void);

void settings_task(void);

void settings_save_pre(void);
void settings_save_post(void);

#if DEBUG == 1
void settings_dump(void);
#endif
