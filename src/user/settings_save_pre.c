#include "settings.h"

// Default (no-op) pre-settings-save hook. settings.c calls this before every
// flash write; boards with an LED scan override it in their layout to quiesce
// their drivers (pause the scan + park the columns) for the duration, while
// LED-less boards fall back here.
void settings_save_pre(void) {}
