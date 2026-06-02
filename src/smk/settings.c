#include "settings.h"
#include "flash.h"

// Belt-and-suspenders: the settings record on flash is
//   [magic0][magic1][len][payload x len][checksum] = 4 + sizeof(payload) bytes
// and must fit in the 512-byte settings sector (sector 118, 0xEC00..0xEDFF).
// If the struct ever grows past that, the build fails here instead of at
// runtime — well before we ship a binary that would overrun into sector 119
// (reset-vector redirect) or the bootloader.
_Static_assert(sizeof(user_settings_t) + 4u <= 512u,
               "user_settings_t too large for the 512-byte settings sector");

__xdata user_settings_t user_settings;

// Stock-style deferred-save dirty flag. Handlers set this and return; the
// main loop calls settings_task() which flushes and clears. Multiple
// mark_dirty() between flushes coalesce into a single sector erase + N
// programs, so a fast brightness ramp produces one flash op instead of
// one per step. Avoids per-press LED flicker (sector erase stalls the
// CPU ~5 ms, freezing whatever row was active on the LED scan).
static __xdata bool settings_dirty;

bool settings_load(void)
{
    return flash_settings_load((__xdata uint8_t *)&user_settings, (uint8_t)sizeof(user_settings));
}

void settings_save(void)
{
    flash_settings_save((const __xdata uint8_t *)&user_settings, (uint8_t)sizeof(user_settings));
    // Anyone calling settings_save() directly has just flushed, so the
    // dirty flag is no longer meaningful — clear it so the next
    // settings_task() doesn't re-do the same write.
    settings_dirty = false;
}

void settings_mark_dirty(void)
{
    settings_dirty = true;
}

void settings_task(void)
{
    if (!settings_dirty) {
        return;
    }
    settings_dirty = false;
    flash_settings_save((const __xdata uint8_t *)&user_settings, (uint8_t)sizeof(user_settings));
}
