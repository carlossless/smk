#include "settings.h"
#include "flash.h"

__xdata user_settings_t user_settings;

void settings_load(void)
{
    flash_settings_load((__xdata uint8_t *)&user_settings, (uint8_t)sizeof(user_settings));
}

void settings_save(void)
{
    flash_settings_save((const __xdata uint8_t *)&user_settings, (uint8_t)sizeof(user_settings));
}
