#include "stack.h"

#if DEBUG == 1

#    include "sh68f90a.h" // SP
#    include "console.h"  // dprint_str / dprint_hex / dprint_nl
#    include <stdint.h>

#    define STACK_SENTINEL 0xAA
// SP reset value; first push lands at STACK_BASE+1. After the printf/DSEG
// shrink (printf_large → tiny dprint_* helpers, rf_* functions __reentrant,
// rf_bt names → __code, ticks → __xdata, PWM ISR __using(1)), SDCC's
// linker places SSEG at 0x43, giving 189 bytes of stack — up from 122
// before the cleanup. STACK_BASE matches that so stack_peak()'s scan
// returns sensible numbers.
#    define STACK_BASE     0x42
#    define STACK_TOP      0xFF // top of the SH68F90A's 256-byte internal RAM

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

// Highest address the stack has ever written = first non-sentinel byte
// scanning down from STACK_TOP. Returns bytes used at that peak.
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
    static __xdata uint8_t reported = 0;
    static __xdata uint8_t throttle = 0;

    if (++throttle != 0) {
        return; // ~once every 256 main-loop iterations
    }

    uint8_t peak = stack_peak();
    if (peak > reported) {
        reported = peak;
        dprint_str("SPpk ");
        dprint_hex(peak);
        dprint_nl();
    }
}

#endif // DEBUG
