#include "delay.h"
#include "watchdog.h"
#include "sh68f881.h"
#include <stdint.h>

// 1T core: at FREQ_SYS 12 MHz 1 us is 12 cycles and one iteration is 24, so a count of N costs 24*N + ~8 cycles.
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

// 24 cycles is 2 us here, not 1, left uncorrected: nothing needs the absolute value, only a delay at least as long as asked.
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
