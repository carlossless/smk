#include "extint.h"
#include "sh68f881.h"

// Wake sources pair with power_enter_powerdown, which is a no-op on this part.
void extint_wake_arm(void) {}

void extint_wake_disable(void) {}

void extint_wake_clear(void) {}
