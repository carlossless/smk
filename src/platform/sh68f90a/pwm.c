#include "pwm.h"
#include <stdint.h>
#include <stdio.h>
#include "matrix.h"

void pwm_interrupt_handler() __interrupt(_INT_PWM0)
{
    matrix_scan_step();
}
