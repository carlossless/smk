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
    // underglow lights (independent of main)
    uint8_t ul_effect;
    uint8_t ul_brightness;
    uint8_t ul_speed;
    // 1 = underglow continuously shows battery level colour
    // (red <20%, yellow 20-80%, green >80%; red also when low_power is set).
    // 0 = underglow follows the regular effect.
    uint8_t battery_indicator_on;
    // Last RF link mode selected by the user; re-applied on boot so the keyboard
    // comes back on the host it was last on.
    uint8_t rf_link;
} user_settings_t;

extern __xdata user_settings_t user_settings;

// Overwrites `user_settings` with the persisted values if a valid record
// exists. Returns true on success; on false the caller is responsible for
// seeding sensible defaults (the in-RAM struct is left untouched on failure).
bool settings_load(void);

// Persists the current `user_settings` immediately (no-op if unchanged). The
// write blocks for several milliseconds, so prefer settings_mark_dirty() +
// settings_task() over calling this from hot paths like keycode handlers.
void settings_save(void);

// Mark user_settings as changed; the next settings_task() flushes it. Cheap
// (just sets a bit), so safe to call from every settings-mutating handler.
void settings_mark_dirty(void);

// Main-loop hook: if settings are dirty, persist them and clear the flag.
// Multiple marks between two calls coalesce into one write.
void settings_task(void);

// Board hooks fired immediately before / after a settings write. A board
// overrides these to quiesce anything the multi-millisecond write would
// otherwise disrupt; the pre hook must keep it quiet for the whole write.
// Default no-op.
void settings_save_pre(void);
void settings_save_post(void);

#if DEBUG == 1
// Print the current user_settings to the debug console.
void settings_dump(void);
#endif
