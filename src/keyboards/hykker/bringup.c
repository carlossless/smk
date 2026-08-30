#include "sh68f881.h"
#include "watchdog.h"
#include "isp.h"
#include <stdint.h>

// Roughly a second at any clock the bootloader might leave us on; the exact figure
// only has to be long enough to see the application PID before the board flips back.
#define SETTLE_OUTER 30000u
#define SETTLE_INNER 250u

static void settle(void)
{
    for (uint16_t outer = 0; outer < SETTLE_OUTER; outer++) {
        watchdog_kick();
        for (uint16_t inner = 0; inner < SETTLE_INNER; inner++) {
            // clang-format off
            __asm
                nop
            __endasm;
            // clang-format on
        }
    }
}

// Bring-up image: take the board and hand it straight back to the bootloader, so
// running it can never strand the keyboard away from ISP.
void main(void)
{
    EA = 0;
    settle();
    isp_jump();
}
