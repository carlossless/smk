#include "delay.h"
#include "watchdog.h"
#include <stdint.h>

#ifndef FREQ_SYS
#    error FREQ_SYS must be defined
#endif

// Every part in the family is a 1T core (1 machine cycle == 1 oscillator cycle), so FREQ_SYS
// alone turns the loop's cycle cost into time. One iteration is tuned to cost exactly
// DELAY_LOOP_CYCLES: body (19c, either a `MOV direct,#data` watchdog kick plus 16 NOPs or 19
// NOPs) + DJNZ Rn taken (5c). Entry/exit add a fixed ~18+14 cycles.
#define DELAY_LOOP_CYCLES   24u
#define DELAY_CYCLES_PER_US (FREQ_SYS / 1000000u)

#if DELAY_CYCLES_PER_US >= DELAY_LOOP_CYCLES
_Static_assert(DELAY_CYCLES_PER_US % DELAY_LOOP_CYCLES == 0, "FREQ_SYS does not scale the delay loop by a whole number");
#    define DELAY_ITERS(us) ((uint16_t)((us) * (DELAY_CYCLES_PER_US / DELAY_LOOP_CYCLES)))
#else
_Static_assert(DELAY_LOOP_CYCLES % DELAY_CYCLES_PER_US == 0, "FREQ_SYS does not scale the delay loop by a whole number");
// rounded up: a caller must never get a shorter delay than it asked for
#    define DELAY_DIV       ((uint16_t)(DELAY_LOOP_CYCLES / DELAY_CYCLES_PER_US))
#    define DELAY_ITERS(us) ((uint16_t)(((us) + (DELAY_DIV - 1u)) / DELAY_DIV))
#endif

static void delay_iters(uint16_t cnt) __naked
{
    (void)cnt;
    // clang-format off
    __asm
        ; cnt arrives in DPL (low) / DPH (high)
        mov     r6, dpl                 ; 3c
        mov     r7, dph                 ; 3c
        cjne    r6, #0x00, 00098$       ; 4c/6c  -- dpl != 0: bias r7 and run
        cjne    r7, #0x00, 00097$       ; 4c/6c  -- dpl == 0, dph != 0: no bias
        sjmp    00099$                  ; 4c     -- cnt == 0: exit
00098$:
        inc     r7                      ; 2c     -- bias so DJNZ on r7 walks dph+1 wraps
00097$:
        sjmp    00002$                  ; 4c
00001$:
#ifdef WATCHDOG_ENABLE
        mov     _RSTSTAT, #WATCHDOG_PERIOD ; 3c  -- watchdog kick
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop                             ; 16c    -- body total: 19c
#else
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop                             ; 19c    -- body total: 19c
#endif
00002$:
        djnz    r6, 00001$              ; 5c/3c
        djnz    r7, 00001$              ; 5c/3c
00099$:
        ret                             ; 8c
    __endasm;
    // clang-format on
}

void delay_us(uint16_t cnt)
{
    uint16_t iters = DELAY_ITERS(cnt);
    if (iters == 0) {
        return;
    }
    delay_iters(iters);
}

void delay_ms(uint16_t cnt)
{
    while (cnt--) {
        delay_us(1000);
    }
}
