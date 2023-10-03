#include "delay.h"
#include <stdint.h>
#include "watchdog.h"

// very naive implementation
// likely not very accurate
// hardcoded for F_SYS = 12Mhz

#define delay3NOP() \
    __asm     \
        nop   \
        nop   \
        nop   \
    __endasm  \

#define delay4NOP() \
    __asm     \
        nop   \
        nop   \
        nop   \
        nop   \
    __endasm  \

void delay_ms(uint16_t cnt)
{
    for (uint16_t i = 0; i < cnt; i++) {
        delay_us(1000);
    }
}

void delay_us(uint16_t cnt)
{
    for (uint16_t i = 0; i < cnt; i++) {
        // 4*6 = 24 instructions
        // @ 12 Mhz one cycle should be 1us
        // unverified

#ifdef WATCHDOG_ENABLE
        CLR_WDT();
        delay3NOP();
#else
        delay4NOP();
#endif

        delay4NOP();
        delay4NOP();
        delay4NOP();
        delay4NOP();
        delay4NOP();
    }
}
