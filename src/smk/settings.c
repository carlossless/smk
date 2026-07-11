#include "settings.h"
#include "flash.h"
#if DEBUG == 1
#    include "debug.h" // dprintf
#endif

// The settings record on flash is
//   [magic0][magic1][len][payload x len][checksum] = 4 + sizeof(payload) bytes
// and must fit in the 512-byte settings sector. If the struct grows past that,
// the build fails here instead of overrunning into the adjacent sectors at
// runtime.
_Static_assert(sizeof(user_settings_t) + 4u <= 512u, "user_settings_t too large for the 512-byte settings sector");

__xdata user_settings_t user_settings;

// Deferred-save dirty flag. Handlers set this and return; the main loop calls
// settings_task() which flushes and clears. Multiple mark_dirty() between
// flushes coalesce into a single sector erase + N programs, so a fast
// brightness ramp produces one flash op instead of one per step. Avoids
// per-press LED flicker (a sector erase stalls the CPU ~5 ms, freezing
// whatever row was active on the LED scan).
static __xdata bool settings_dirty;

bool settings_load(void)
{
    return flash_settings_load((__xdata uint8_t *)&user_settings, (uint8_t)sizeof(user_settings));
}

void settings_save(void)
{
    // Quiesce the board's LED drivers around the write (no-op on LED-less
    // boards) so the ~5 ms erase stall can't freeze a lit row bright.
    settings_save_pre();
    flash_settings_save((const __xdata uint8_t *)&user_settings, (uint8_t)sizeof(user_settings));
    settings_save_post();
    // Anyone calling settings_save() directly has just flushed, so the
    // dirty flag is no longer meaningful — clear it so the next
    // settings_task() doesn't re-do the same write.
    settings_dirty = false;
}

void settings_mark_dirty(void)
{
    settings_dirty = true;
}

#if DEBUG == 1
void settings_dump(void)
{
    dprintf("settings le=%02x lb=%02x ls=%02x ue=%02x ub=%02x us=%02x bat=%02x rf=%02x\r\n", user_settings.led_effect, user_settings.led_brightness, user_settings.led_speed, user_settings.ul_effect, user_settings.ul_brightness, user_settings.ul_speed, user_settings.battery_indicator_on, user_settings.rf_link);
}
#endif

void settings_task(void)
{
    if (!settings_dirty) {
        return;
    }
    settings_dirty = false;
#if DEBUG == 1
    settings_dump(); // report the coalesced change
#endif
    settings_save_pre();
    flash_settings_save((const __xdata uint8_t *)&user_settings, (uint8_t)sizeof(user_settings));
    settings_save_post();
}
