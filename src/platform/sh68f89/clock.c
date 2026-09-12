#include "clock.h"
#include "sh68f89.h"
#include "watchdog.h"
#include <stdint.h>

#define REGCON_INIT   (uint8_t)(_REGEN)
#define CLKCON_WARMUP (uint8_t)(_OSC2ON)
#define PLLCON_WARMUP (uint8_t)(_PLLON)
#define PLLCON_RUN    (uint8_t)(_PLLON | _PLLFS)
#define CLKCON_RUN    (uint8_t)(_OSC2ON | _FS)

// the core still runs off OSC1CLK here, so bound the wait by tries rather than by time.
#define PLL_LOCK_TRIES 2000u

// a PLL that has not settled enumerates with intermittent descriptor reads, so wait for
// the lock flag and then let it sit a while longer.
static void pll_settle(void)
{
    for (uint16_t tries = 0; tries < PLL_LOCK_TRIES; tries++) {
        watchdog_kick(); // OP_WDT is enabled on this part, so never spin here unkicked
        if (PLLCON & _PLLSTA) {
            break;
        }
    }

    for (uint16_t i = 0; i < 2000; i++) {
        watchdog_kick();
        // clang-format off
        __asm
            nop
        __endasm;
        // clang-format on
    }
}

// a pin mux'ed to an analog or display block ignores its port registers, and the LCD
// segment map alone covers P0-P6, so every selector has to be cleared before GPIO setup.
static void peripherals_reset(void)
{
    DISPCON  = 0;
    DISPCON1 = 0;
    P1SS     = 0;
    P2SS     = 0;
    P3SS     = 0;
    P4SS     = 0;
    P5SS     = 0;
    P6SS     = 0;
    P7SS     = 0;
    OPCON    = 0;
    OPIOS    = 0;
    DACCON0  = 0;
    DACCON1  = 0;
    SPCON    = 0;
    SPSTA    = 0;
}

void clock_init(void)
{
    sfr_page_0();

    REGCON = REGCON_INIT;
    CLKCON = CLKCON_WARMUP;
    PLLCON = PLLCON_WARMUP;
    pll_settle();
    PLLCON = PLLCON_RUN;
    CLKCON = CLKCON_RUN;

    peripherals_reset();

    uint8_t saved_page = INSCON;
    sfr_page_1();
    ADCON1 = 0;
    ADCON2 = 0;
    PCACON = 0;
    TWICON = 0;
    INSCON = saved_page;
}
