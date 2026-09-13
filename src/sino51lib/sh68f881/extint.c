#include "extint.h"
#include "sh68f881.h"

// INT0-INT3 and INT4x can wake the part and are the only thing that can, but no board here
// wires one, and which pin it would be is the board's to say.
void extint_wake_arm(void) {}

void extint_wake_disable(void) {}

void extint_wake_clear(void) {}
