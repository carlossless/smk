#include "clock.h"
#include "sh68f90a.h"
#include "delay.h"
#include "watchdog.h"

/** \brief sets up HRCCLK and uses it as SYSCLK */
void clock_init()
{
    CLKCON = _HFON;  // enable HRCCLK
    PLLCON = _PLLON; // init PLL

    while (!(PLLCON & _PLLSTA)) { // wait for PLL to lock phase
        CLR_WDT();
    }

    PLLCON |= _PLLFS; // switch OSCSCLK
    CLKCON |= _FS;    // use HRCCLK as SYSCLK
}

void clock_wake_restart()
{
    // Wake from Power-Down: the HF oscillator was fully stopped, so it needs a
    // cold-start settle before the PLL output is trustworthy. Do NOT poll PLLSTA
    // here — it can return the instant the lock comparator trips, before the
    // freshly restarted oscillator is stable enough for USB-grade (12 MHz ±0.25%)
    // timing. Selecting that marginal clock stalls both the Timer-2 LED/matrix scan
    // (frozen animation) and USB enumeration at once. Instead: start HF+PLL, spin a
    // FIXED warm-up, then commit the PLL as SYSCLK in one shot.
    //
    // Interrupts masked: an ISR firing mid clock-switch would run on a
    // half-selected/unstable clock.
    __critical
    {
        CLKCON |= _HFON;  // start HF oscillator
        PLLCON |= _PLLON; // start PLL
        CLR_WDT();

        // Fixed settle. SYSCLK is still the slow pre-PLL clock here, so these
        // iterations are far longer than the needed ~20 µs — comfortably generous.
        for (uint8_t i = 0; i < 200; i++) {
            // clang-format off
            __asm
                nop
            __endasm;
            // clang-format on
        }

        PLLCON = _PLLFS | _PLLON; // PLL drives the clock source
        CLKCON = _FS | _HFON;     // SYSCLK = HF/PLL
        CLR_WDT();
    }
}
