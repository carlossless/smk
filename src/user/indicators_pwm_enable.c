#include "indicators.h"

// Default (no-op) LED column PWM enable. matrix.c calls this unconditionally
// to restore the LEDs after a matrix scan; boards with LEDs override it in
// their own layout indicators.c, while LED-less boards (e.g. example) fall
// back here.
void indicators_pwm_enable() {}
