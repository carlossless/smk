#include "clock.h"
#include "sh68f881.h"
#include "watchdog.h"
#include <stdint.h>

// The clock the bootloader brings up for its own ISP loop. Reset hands the application
// CLKCON = 0, which the USB block cannot run off.
#define REGCON_USB 0x03u
#define CLKCON_USB 0x08u
#define PLLCON_USB 0x02u

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

void clock_init(void)
{
    REGCON = REGCON_USB;
    CLKCON = CLKCON_USB;
    PLLCON = PLLCON_USB;
    pll_settle();
}
