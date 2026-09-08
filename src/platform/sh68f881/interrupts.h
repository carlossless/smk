#pragma once

#include "sh68f881.h"

void systick_interrupt_handler(void) __interrupt(_INT_TIMER2);
void usb_interrupt_handler(void) __interrupt(_INT_USB);

#define UNUSED_INTERRUPTS(X)                   \
    X(int0, _INT_INT0, IEN0, _EX0)             \
    X(timer4, _INT_TIMER4, IEN0, _ET4)         \
    X(int1, _INT_INT1, IEN0, _EX1)             \
    X(timer3, _INT_TIMER3, IEN0, _ET3)         \
    X(euart, _INT_EUART, IEN0, _ES)            \
    X(adc, _INT_ADC, IEN0, _EADC)              \
    X(spi, _INT_SPI, IEN1, _ESPI)              \
    X(int2, _INT_INT2, IEN1, _EX2)             \
    X(int3, _INT_INT3, IEN1, _EX3)             \
    X(int4, _INT_INT4, IEN1, _EX4)             \
    X(pwm, _INT_PWM, IEN1, _EPWM)              \
    X(base_timer, _INT_BASE_TIMER, IEN1, _EBT) \
    X(scm, _INT_SCM, IEN1, _ESCM)

#define UNUSED_INTERRUPT_DECL(name, vector, ien, bit) void name##_unused_interrupt_handler(void) __interrupt(vector);
UNUSED_INTERRUPTS(UNUSED_INTERRUPT_DECL)
#undef UNUSED_INTERRUPT_DECL

void interrupts_task(void);
