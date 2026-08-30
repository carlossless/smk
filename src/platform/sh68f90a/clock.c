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
    __critical
    {
        CLKCON |= _HFON;
        PLLCON |= _PLLON;
        CLR_WDT();

        for (uint8_t i = 0; i < 200; i++) {
            // clang-format off
            __asm
                nop
            __endasm;
            // clang-format on
        }

        PLLCON = _PLLFS | _PLLON;
        CLKCON = _FS | _HFON;
        CLR_WDT();
    }
}
