#include "sh68f881.h"
#include "watchdog.h"
#include "isp.h"
#include <stdint.h>

// The 0x7F00 entry does not touch the clock -- it assumes a running application left
// one the USB block can work off. Reset hands us CLKCON = 0, so set up the same
// regulator, clock and PLL the bootloader's own cold ISP path uses before jumping.
#define REGCON_ISP 0x03u
#define CLKCON_ISP 0x08u
#define PLLCON_ISP 0x02u

static void pll_settle(void)
{
    for (uint16_t i = 0; i < 2000; i++) {
        // clang-format off
        __asm
            nop
        __endasm;
        // clang-format on
    }
}

// Bring-up image: take the board and hand it straight back to the bootloader, so
// running it can never strand the keyboard away from ISP.
void main(void)
{
    EA = 0;
    watchdog_kick();

    REGCON = REGCON_ISP;
    CLKCON = CLKCON_ISP;
    PLLCON = PLLCON_ISP;
    pll_settle();

    watchdog_kick();
    isp_jump();
}
