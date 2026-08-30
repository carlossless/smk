#include "stack.h"

#if DEBUG == 1

#    include "sh68f90a.h"
#    include "debug.h"
#    include <stdint.h>

#    define STACK_SENTINEL 0xAA
extern uint8_t _start__stack;
#    define STACK_BASE ((uint8_t)((uint16_t)&_start__stack - 1u))
#    define STACK_TOP  0xFF // top of the SH68F90A's 256-byte internal RAM

// Fill the unused stack region (above the current SP, up to STACK_TOP) with
// a sentinel. Must run early in main(), while SP is still shallow, so we
// capture the largest possible window for later peak measurement.
void stack_paint(void)
{
    uint8_t addr = SP;
    do {
        addr++;
        *((__idata uint8_t *)addr) = STACK_SENTINEL;
    } while (addr != STACK_TOP);
}

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
    static uint8_t reported = 0;
    static uint8_t throttle = 0;

    if (++throttle != 0) {
        return; // ~once every 256 main-loop iterations
    }

    uint8_t peak = stack_peak();
    if (peak > reported) {
        reported = peak;
        dprintf("STACK peak: %02x\r\n", peak);
    }
}

#endif // DEBUG
