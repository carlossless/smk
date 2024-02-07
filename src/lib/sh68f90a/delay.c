#include "delay.h"
#include "watchdog.h"
#include "../../utils.h"
#include <stdint.h>

// very naive implementation, not very accurate or optimized
// hardcoded for F_SYS = 24Mhz

void delay_ms(uint16_t cnt)
{
    for (uint16_t i = 0; i < cnt; i++) {
        delay_us(1000);
    }
}

void delay_us(uint16_t cnt) __naked
{
    for (uint16_t i = 0; i < cnt; i++) {
        // 4*6 = 24 instructions
        // @ 24 Mhz one cycle should be 1us
        // unverified

#ifdef WATCHDOG_ENABLE
        CLR_WDT(); // 2c
        _nop_3_(); // 3c
#else
        _nop_4_(); // 4c
#endif

        _nop_4_(); // 4c
        _nop_4_(); // 4c
        _nop_4_(); // 4c
        _nop_4_(); // 4c
        _nop_4_(); // 4c
        // = 
    }
}
