#pragma once

#include "sh68f89.h"

void systick_interrupt_handler(void) __interrupt(_INT_TIMER2);
void usb_interrupt_handler(void) __interrupt(_INT_USB_TWI);

#define UNUSED_INTERRUPTS(X)                    \
    X(pca0, _INT_PCA0, IEN0, _EPCA0)            \
    X(pca1, _INT_PCA1, IEN0, _EPCA1)            \
    X(pca2, _INT_PCA2, IEN0, _EPCA2)            \
    X(pca3, _INT_PCA3, IEN0, _EPCA3)            \
    X(euart0, _INT_EUART0, IEN0, _ES0)          \
    X(adc, _INT_ADC, IEN0, _EADC)               \
    X(spi, _INT_SPI, IEN1, _ESPI)               \
    X(int2_dac, _INT_INT2_DAC, IEN1, _EX2_EDAC) \
    X(int3, _INT_INT3, IEN1, _EX3)              \
    X(int4, _INT_INT4, IEN1, _EX4)              \
    X(timer3, _INT_TIMER3, IEN1, _ET3)          \
    X(euart1, _INT_EUART1, IEN1, _ES1)          \
    X(scm_lpd, _INT_SCM_LPD, IEN1, _ESCM_ELPD)

#define UNUSED_INTERRUPT_DECL(name, vector, ien, bit) void name##_unused_interrupt_handler(void) __interrupt(vector);
UNUSED_INTERRUPTS(UNUSED_INTERRUPT_DECL)
#undef UNUSED_INTERRUPT_DECL

void interrupts_task(void);
