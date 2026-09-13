#include "systick.h"
#include "interrupts.h"
#include "tick.h"
#include <stdint.h>

// Timer2 takes T2MOD.TCLKP2's default 1/12 prescale, so 2 MHz at a 24 MHz FREQ_SYS.
// a board with N LED subframes per scan spends every scan slot dark, so the scan slot is
// kept close to a subframe: at 36 subframes a 8 ms slot alone is a 30% dark duty.
#define RELOAD_LED_SUBFRAME 0xFE0C // ~0.25 ms
#define RELOAD_MATRIX_SCAN  0xF830 // ~1 ms

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

volatile uint16_t systick_ticks;

void systick_interrupt_handler(void) __interrupt(_INT_TIMER2)
{
    // TF2 is T2CON.7 on page 0 and PCA1's CF1 on page 1, so pin the page before touching it.
    uint8_t saved_page = INSCON;
    sfr_page_0();

    TF2 = 0;
    systick_ticks++;
    tick_dispatch();

    INSCON = saved_page;
}
