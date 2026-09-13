#include "delay.h"
#include "watchdog.h"
#include "sh68f89.h"
#include <stdint.h>

// 1T core: at FREQ_SYS 24 MHz 1 us is 24 cycles, which is what one iteration below costs.
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

void delay_us(uint16_t cnt)
{
    delay_us_raw(cnt ? cnt : 1u);
}

void delay_ms(uint16_t cnt)
{
    while (cnt--) {
        delay_us(1000);
    }
}
