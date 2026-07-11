#include "settings.h"

// Default no-op pre-settings-save hook. Boards with an LED scan override it to
// quiesce their drivers (pause the scan + park the columns) during a flash write.
void settings_save_pre(void) {}
