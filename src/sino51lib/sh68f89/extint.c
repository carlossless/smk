#include "extint.h"
#include "sh68f89.h"

// INT2, INT3 and INT40-47 can all wake the part, but no board here wires one, and which
// pin it would be is the board's to say. USB resume wakes it without any of this.
void extint_wake_arm(void) {}

void extint_wake_disable(void) {}

void extint_wake_clear(void) {}
