#include "indicators.h"

// Default no-op for boards without an RGB framebuffer to clear. RGB boards
// override this in their own indicators.c (see the nuphy-air60 layout).
void indicators_init() {}
