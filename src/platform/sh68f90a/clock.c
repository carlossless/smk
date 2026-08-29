#include "clock.h"
#include "sh68f90a.h"
#include "delay.h"
#include "watchdog.h"

void clock_init()
{
    CLKCON = _HFON;
    PLLCON = _PLLON;

    while (!(PLLCON & _PLLSTA)) {
        watchdog_kick();
    }

    PLLCON |= _PLLFS;
    CLKCON |= _FS;
}

static void hf_oscillator_settle(void)
{
    for (uint8_t i = 0; i < 200; i++) {
        // clang-format off
        __asm
            nop
        __endasm;
        // clang-format on
    }
}

void clock_wake_restart()
{
    // Do NOT poll PLLSTA on a Power-Down wake: it trips the instant the lock
    // comparator does, before a cold-started oscillator is stable enough for
    // USB-grade timing, and committing that clock stalls the scan and enumeration.
    __critical
    {
        CLKCON |= _HFON;
        PLLCON |= _PLLON;
        watchdog_kick();

        hf_oscillator_settle();

        PLLCON = _PLLFS | _PLLON;
        CLKCON = _FS | _HFON;
        watchdog_kick();
    }
}
