#pragma once

#include "sh68f90a.h"
#include <stdint.h>

#define PWM_CLK_DIV_4   0b010 // PWM_CLK = SYS_CLK / 4
#define PWM_SS          (1 << 3)
#define PWM_MOD         (1 << 4)
#define PWM_INT_ENABLE  (1 << 6)
#define PWM_MODE_ENABLE (1 << 7)
#define PWM_CON_PARKED  PWM_CLK_DIV_4

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
