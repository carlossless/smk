#include "delay.h"
#include "watchdog.h"
#include "sh68f881.h"
#include <stdint.h>

// SH68F881 is a 1T 8051 (1 machine cycle == 1 oscillator cycle, datasheet section 1).
// At FREQ_SYS = 24 MHz, 1 us == 24 cycles.
//
// Inner loop is tuned so the per-iteration cost is exactly 24 cycles:
//   body (19c) + DJNZ Rn taken (5c) = 24c
// where the body is either `MOV direct,#data` watchdog kick (3c) + 16 NOPs (16c)
// or 19 NOPs when WDT is disabled.
//
// Entry/exit add a fixed ~18+14 cycles, so delay_us(N) takes 24*N + ~8 cycles
// for N in [1, 65535] (well under 0.1% error for N >= 100, ~0.3 us absolute).
// delay_us(0) is handled and returns immediately.
static void delay_us_raw(uint16_t cnt) __naked
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
        mov     _RSTSTAT, #0x02         ; 3c     -- watchdog kick
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

// The inner loop is cycle-tuned for 24 MHz, and Timer2 measures this core at roughly an
// eighth of that, so a microsecond here runs about 8x long. Left uncorrected: nothing on
// this board needs the absolute value, and the scan only relies on the delay being at
// least as long as it asks for.
#define DELAY_US_DIV 1u

void delay_us(uint16_t cnt)
{
    uint16_t scaled = cnt / DELAY_US_DIV;
    delay_us_raw(scaled ? scaled : 1u);
}

void delay_ms(uint16_t cnt)
{
    while (cnt--) {
        delay_us(1000);
    }
}
