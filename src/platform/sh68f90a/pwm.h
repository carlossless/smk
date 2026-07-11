#pragma once

#include "sh68f90a.h"
#include <stdint.h>

#define PWM_DUTY_REG(pwm, duty, bit) pwm##DUTY##duty##bit
#define SET_PWM_DUTY_1(pwm, value)                       \
    do {                                                 \
        PWM_DUTY_REG(pwm, 1, H) = (uint8_t)(value >> 8); \
        PWM_DUTY_REG(pwm, 1, L) = (uint8_t)(value);      \
    } while (0)
#define SET_PWM_DUTY_2(pwm, value)                       \
    do {                                                 \
        PWM_DUTY_REG(pwm, 2, H) = (uint8_t)(value >> 8); \
        PWM_DUTY_REG(pwm, 2, L) = (uint8_t)(value);      \
    } while (0)
#define SET_PWM_DUTY(pwm, duty1, duty2) \
    do {                                \
        SET_PWM_DUTY_1(pwm, duty1);     \
        SET_PWM_DUTY_2(pwm, duty2);     \
    } while (0)

// __using(N) shelved. SDCC's rule: an ISR with __using(N) may only call
// functions also tagged __using(N) (or that touch no registers). This ISR
// calls into matrix_scan_step → indicators_update_step → … (all bank-0
// functions), which corrupted register state and produced stuck/chattering
// keys + a broken USB second-interface enumeration. Pushing R0–R7 on every
// fire is the safe path; we have stack headroom for it.
void pwm_interrupt_handler() __interrupt(_INT_PWM0);
