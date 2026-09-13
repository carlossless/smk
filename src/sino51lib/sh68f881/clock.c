#include "clock.h"
#include "sh68f881.h"
#include "watchdog.h"
#include <stdint.h>

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

void clock_init(void)
{
    CLKCON = CLKCON_WARMUP;
    PLLCON = PLLCON_WARMUP;
    pll_settle();
    PLLCON = PLLCON_RUN;
    CLKCON = CLKCON_RUN;
}
