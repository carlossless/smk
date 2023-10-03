#include "clock.h"
#include "sh68f90a.h"
#include "delay.h"

#define _SYSCLK_DIV 0 // no SYSCLK division (CLKS)
#define _HRCCLK_EN (1 << 3) // enable HRCCLK (HFON)
#define _CLK_SRC (1 << 2) // use OSCSCLK as SYSCLK (FS)

// PLL also necessary for USB peripheral
#define _PLL_EN (1 << 1) // enables PLL, necessary for HRCCLK (PLLON)
#define _OSCS_SRC (1 << 0) // PLL divided by two acts as OSCSCLK (PLLFS)

#define CLKCON_INIT (_SYSCLK_DIV|_HRCCLK_EN|_CLK_SRC)
#define PLLCON_INIT (_PLL_EN|_OSCS_SRC)

void clock_init()
{
    CLKCON = (CLKCON_INIT & _HRCCLK_EN);  // init HRCCLK
    delay_us(350); // unsure about span and if necessary
    PLLCON = (PLLCON_INIT & _PLL_EN); // init PLL
    delay_us(533); // unsure about span and if necessary
    PLLCON = PLLCON_INIT; // switch OSCSCLK
    CLKCON = CLKCON_INIT; // use HRCCLK as SYSCLK
}
