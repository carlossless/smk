#include "stack.h"

#if DEBUG == 1

#    include "sh68f90a.h" // SP
#    include "debug.h"
#    include <stdint.h>

#    define STACK_SENTINEL 0xAA
#    define STACK_BASE     0x85 // SP reset value; the first push lands at 0x86
#    define STACK_TOP      0xFF // top of the SH68F90A's 256-byte internal RAM

// Fill everything above the current frame up to STACK_TOP with the sentinel.
// Must be the first call in main(), while the stack is shallow.
void stack_paint(void)
{
    uint8_t addr = SP;
    do {
        addr++;
        *((__idata uint8_t *)addr) = STACK_SENTINEL;
    } while (addr != STACK_TOP);
}

// Highest address the stack has ever written = first non-sentinel byte scanning
// down from the top. Returns bytes used at that peak.
static uint8_t stack_peak(void)
{
    uint8_t addr = STACK_TOP;
    while (addr > STACK_BASE && *((__idata uint8_t *)addr) == STACK_SENTINEL) {
        addr--;
    }
    return addr - STACK_BASE;
}

void stack_task(void)
{
    static __xdata uint8_t reported = 0; // __xdata: internal RAM is full
    static __xdata uint8_t throttle = 0;
    uint8_t                peak;

    if (++throttle != 0) {
        return; // scan ~once every 256 main-loop iterations
    }

    peak = stack_peak();
    if (peak > reported) {
        reported = peak;
        dprintf("STACK peak: %d/%d\r\n", peak, STACK_TOP - STACK_BASE);
    }
}

#endif // DEBUG
