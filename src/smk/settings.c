#include "settings.h"
#include "flash.h"
#if DEBUG == 1
#    include "debug.h"
#endif

_Static_assert(sizeof(user_settings_t) + 4u <= 512u, "user_settings_t too large for the 512-byte settings sector");

__xdata user_settings_t user_settings;

static __xdata bool settings_dirty;

bool settings_load(void)
{
    return flash_settings_load((__xdata uint8_t *)&user_settings, (uint8_t)sizeof(user_settings));
}

void settings_save(void)
{
    settings_save_pre();
    flash_settings_save((const __xdata uint8_t *)&user_settings, (uint8_t)sizeof(user_settings));
    settings_save_post();
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
    settings_dump();
#endif
    settings_save_pre();
    flash_settings_save((const __xdata uint8_t *)&user_settings, (uint8_t)sizeof(user_settings));
    settings_save_post();
}
