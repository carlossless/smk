#include "systick.h"
#include "interrupts.h"
#include "tick.h"
#include <stdint.h>

// Timer 2 in 16-bit auto-reload mode, clocked at SYS_CLK/12 = 2 MHz.
//
// The LED slot gets the full subframe dwell. The matrix slot is armed much
// shorter than the sweep it precedes, so the timer has already overflowed by the
// time the sweep returns and the next LED subframe starts immediately instead of
// idling out a whole period in the dark.
#define RELOAD_LED_SUBFRAME 0xFCDF // 65536 - 801, 400 us
#define RELOAD_MATRIX_SCAN  0xFF37 // 65536 - 201, ~100 us against a ~320 us sweep

static void timer2_reload(uint16_t reload)
{
    TR2    = 0;
    RCAP2H = (uint8_t)(reload >> 8);
    RCAP2L = (uint8_t)(reload & 0xFF);
    TH2    = (uint8_t)(reload >> 8);
    TL2    = (uint8_t)(reload & 0xFF);
    TR2    = 1;
}

void systick_init(void)
{
    TR2   = 0;
    T2CON = 0;
    T2MOD = 0;
    timer2_reload(RELOAD_LED_SUBFRAME);
    TF2 = 0;
    ET2 = 1;
    TR2 = 1;
}

void systick_arm(systick_slot_t slot)
{
    timer2_reload(slot == SYSTICK_SLOT_MATRIX_SCAN ? RELOAD_MATRIX_SCAN : RELOAD_LED_SUBFRAME);
}

void systick_pause(void)
{
    ET2 = 0;
}

void systick_resume(void)
{
    ET2 = 1;
}

void systick_interrupt_handler(void) __interrupt(_INT_TIMER2)
{
    TF2 = 0;
    tick_dispatch();
}
