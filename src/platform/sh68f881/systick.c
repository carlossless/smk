#include "systick.h"
#include "interrupts.h"
#include "tick.h"
#include <stdint.h>

// This part has no LED engine wired up yet and its Timer2 clock is not characterised,
// so tick well below the SH68F90 rate: the scan runs inside this interrupt, and firing
// it faster than the sweep takes starves the main loop and its watchdog kick.
#define RELOAD_LED_SUBFRAME 0xC000
#define RELOAD_MATRIX_SCAN  0xC000

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
    TF2 = 0;
    systick_ticks++;
    tick_dispatch();
}
