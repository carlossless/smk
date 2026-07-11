#include "pwm.h"
#include <stdint.h>

// PWM00CON.IE and IEN1 EPWM0 are both 0, so this vector is never reached. The
// empty handler stays as a safety net and to reserve the SDCC vector slot.
void pwm_interrupt_handler() __interrupt(_INT_PWM0) {}
