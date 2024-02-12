#include "delay.h"
#include "watchdog.h"
#include "../../smk/utils.h"
#include <stdint.h>

// very naive implementation, not very accurate or optimized
// hardcoded for FREQ_SYS = 24Mhz

void delay_ms(uint16_t cnt) // should be __naked, currently unaccounted for
{
    for (uint16_t i = 0; i < cnt; i++) {
        delay_us(1000);
    }
}

void delay_us(uint16_t cnt) // should be __naked, currently unaccounted for
{
    for (uint16_t i = 0; i < cnt; i++) { // unaccounted for
        // 4*6 = 24 instructions
        // @ 24 Mhz one cycle should be 1us
        // unverified

#ifdef WATCHDOG_ENABLE
        CLR_WDT(); // 3c - MOV direct, #data
        _nop_(); // 1c
#else
        _nop_4_(); // 4c
#endif

        _nop_4_(); // 4c
        _nop_4_(); // 4c
        _nop_4_(); // 4c
        _nop_4_(); // 4c
        _nop_4_(); // 4c
        // = 24c
    }
}
