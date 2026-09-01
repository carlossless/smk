#include "clock.h"
#include "sh68f881.h"
#include "watchdog.h"
#include <stdint.h>

// Two stages, as the stock firmware does it: bring the PLL up on the slow setting, wait,
// then switch to the running clock. Stopping after the first stage leaves the core far
// slower than every delay and timer reload here assumes.
#define REGCON_INIT   0x03u
#define CLKCON_WARMUP 0x08u
#define PLLCON_WARMUP 0x02u
#define PLLCON_RUN    0x03u
#define CLKCON_RUN    0x0Cu

// The bootloader waits before handing the clock to the USB block; enumerating off a
// PLL that has not settled gives intermittent descriptor reads rather than silence.
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

// Stock clears these on page 0 immediately after the clock switch and before any port
// setup. Whatever they enable survives reset and holds the P2 matrix rows otherwise.
static void peripherals_reset(void)
{
    PERIPH_A2 = 0;
    PERIPH_93 = 0;
    PERIPH_92 = 0;
    PERIPH_95 = 0;
    PERIPH_AB = 0;
    PERIPH_AA = 0;
    PERIPH_9E = 0;
    PERIPH_9F = 0;
    PERIPH_AC = 0;
    PERIPH_AD = 0;
    PERIPH_AE = 0;
    SPCON = 0;
    SPSTA = 0;
    SPDAT = 0;
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
