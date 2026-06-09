#include "settings.h"

// Default (no-op) post-settings-save hook. settings.c calls this after every
// flash write; boards with an LED scan override it in their layout to restore
// their drivers (re-enable the columns + resume the scan).
void settings_save_post(void) {}
