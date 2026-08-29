#pragma once

#include <stdint.h>
#include <stdbool.h>

// Persistent user settings, stored as one blob (see settings.c).
//
// To save more things across power cycles, add fields to this struct. Keep it
// small (it shares one storage sector) and note that a firmware reflash resets
// it to the defaults the caller seeds before settings_load().
typedef struct {
    uint8_t led_effect;     // selected backlight effect (or "off")
    uint8_t led_brightness; // 0 (dark) .. 255 (full)
    uint8_t led_speed;      // animation phase increment (1 = slowest)
    uint8_t ul_effect;
    uint8_t ul_brightness;
    uint8_t ul_speed;
    uint8_t battery_indicator_on;
    uint8_t rf_link;
} user_settings_t;

extern user_settings_t user_settings;

bool settings_load(void);

void settings_save(void);

void settings_mark_dirty(void);

void settings_task(void);

void settings_save_pre(void);
void settings_save_post(void);

#if DEBUG == 1
void settings_dump(void);
#endif
