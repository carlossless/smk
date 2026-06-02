#include "pwm.h"
#include <stdint.h>

// Stock-match: PWM00CON.IE = 0 and IEN1 EPWM0 = 0, so this ISR vector is
// never reached in practice. The handler body stays as a safety net (in
// case anything accidentally flips an IE bit later) and to keep the
// SDCC linker honest about the vector slot.
void pwm_interrupt_handler() __interrupt(_INT_PWM0)
{
}
