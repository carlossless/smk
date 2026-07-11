#include "settings.h"

// Default no-op post-settings-save hook. Boards with an LED scan override it to
// restore their drivers (re-enable columns + resume the scan) after a flash write.
void settings_save_post(void) {}
