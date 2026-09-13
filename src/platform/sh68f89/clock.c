#include "clock.h"
#include "sh68f89.h"
#include "watchdog.h"
#include <stdint.h>

#define CLKCON_OSC2ON (uint8_t)(_OSC2ON)
#define CLKCON_RUN    (uint8_t)(_OSC2ON | _FS)
#define PLLCON_WARMUP (uint8_t)(_PLLON)
#define PLLCON_RUN    (uint8_t)(_PLLON | _PLLFS)

#define PLL_LOCK_TRIES 2000u

static void spin(uint16_t count)
{
    while (count--) {
        watchdog_kick(); // OP_WDT is enabled on this part, so never spin here unkicked
        // clang-format off
        __asm
            nop
        __endasm;
        // clang-format on
    }
}

// a PLL that has not settled enumerates with intermittent descriptor reads, so wait for
// the lock flag and then let it sit a while longer.
static void pll_settle(void)
{
    for (uint16_t tries = 0; tries < PLL_LOCK_TRIES; tries++) {
        watchdog_kick();
        if (PLLCON & _PLLSTA) {
            break;
        }
    }

    for (uint8_t outer = 0; outer < 20; outer++) {
        spin(500);
    }
}

void clock_init(void)
{
    sfr_page_0();

    // OP_OSC selects the 128kHz RC as OSC1CLK, so until FS is set the core runs there and
    // this spin is the 12MHz RC's warm-up, some milliseconds long at that clock.
    CLKCON = CLKCON_OSC2ON;
    spin(200);
    CLKCON = CLKCON_RUN;

    // from here the PLL settles at 12MHz, and selecting it doubles SYSCLK to 24MHz.
    PLLCON = PLLCON_WARMUP;
    pll_settle();
    PLLCON = PLLCON_RUN;
}
