#include "clock.h"
#include "sh68f90a.h"
#include "delay.h"
#include "watchdog.h"

void clock_init()
{
    CLKCON = _HFON;
    PLLCON = _PLLON;

    while (!(PLLCON & _PLLSTA)) { // wait for PLL phase lock
        watchdog_kick();
    }

    PLLCON |= _PLLFS;
    CLKCON |= _FS; // HRCCLK as SYSCLK
}

void clock_wake_restart()
{
    // On a Power-Down wake the HF oscillator was fully stopped and needs a
    // cold-start settle. Do NOT poll PLLSTA - it can return the instant the lock
    // comparator trips, before the oscillator is stable enough for USB-grade
    // (12 MHz ±0.25%) timing, and selecting that marginal clock stalls both the
    // LED scan and USB enumeration. Start HF+PLL, spin a FIXED warm-up, then
    // commit the PLL as SYSCLK in one shot. Interrupts masked so no ISR runs on
    // a half-selected clock.
    __critical
    {
        CLKCON |= _HFON;
        PLLCON |= _PLLON;
        watchdog_kick();

        // Fixed settle. SYSCLK is still the slow pre-PLL clock, so these
        // iterations run far longer than the needed ~20 µs.
        for (uint8_t i = 0; i < 200; i++) {
            // clang-format off
            __asm
                nop
            __endasm;
            // clang-format on
        }

        PLLCON = _PLLFS | _PLLON;
        CLKCON = _FS | _HFON; // SYSCLK = HF/PLL
        watchdog_kick();
    }
}
