#include "pwm.h"
#include <stdint.h>

void pwm_interrupt_handler() __interrupt(_INT_PWM0) {}
