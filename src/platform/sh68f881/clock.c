#include "clock.h"
#include "sh68f881.h"
#include "watchdog.h"
#include <stdint.h>

#define REGCON_INIT   (uint8_t)(_REGS | _REGEN)
#define CLKCON_WARMUP (uint8_t)(_OSC2ON)
#define PLLCON_WARMUP (uint8_t)(_PLLON)
#define PLLCON_RUN    (uint8_t)(_PLLON | _PLLFS)
#define CLKCON_RUN    (uint8_t)(_OSC2ON | _FS)

// a PLL that has not settled enumerates with intermittent descriptor reads, so settle by delay.
static void pll_settle(void)
{
    for (uint16_t outer = 0; outer < 20; outer++) {
        watchdog_kick(); // OP_WDT is enabled on this board, so never spin here unkicked
        for (uint16_t i = 0; i < 500; i++) {
            // clang-format off
            __asm
                nop
            __endasm;
            // clang-format on
        }
    }
}

// a pin mux'ed to an analog block ignores its port registers: OPCON.OPOS takes rows P2.1-P2.3, ADCH and P5SS the anodes.
static void peripherals_reset(void)
{
    OPCON   = 0;
    ADCON   = 0;
    ADCDS   = 0;
    ADCH    = 0;
    LCDCON  = 0;
    LCDCON1 = 0;
    P5SS    = 0;
    P6SS    = 0;
    P7SS    = 0;
    P8SS    = 0;
    PXSS    = 0;
    SPCON   = 0;
    SPSTA   = 0;
    SPDAT   = 0;
}

void clock_init(void)
{
    REGCON = REGCON_INIT;
    CLKCON = CLKCON_WARMUP;
    PLLCON = PLLCON_WARMUP;
    pll_settle();
    PLLCON = PLLCON_RUN;
    CLKCON = CLKCON_RUN;

    sfr_page_0();
    peripherals_reset();
}
