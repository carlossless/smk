#include "peripherals.h"
#include "sh68f89.h"
#include <stdint.h>

// a pin mux'ed to an analog or display block ignores its port registers, and the LCD
// segment map alone covers P0-P6, so every selector has to be cleared before GPIO setup.
void peripherals_init(void)
{
    sfr_page_0();

    DISPCON  = 0;
    DISPCON1 = 0;
    P1SS     = 0;
    P2SS     = 0;
    P3SS     = 0;
    P4SS     = 0;
    P5SS     = 0;
    P6SS     = 0;
    P7SS     = 0;
    OPCON    = 0;
    OPIOS    = 0;
    DACCON0  = 0;
    DACCON1  = 0;
    SPCON    = 0;
    SPSTA    = 0;

    uint8_t saved_page = INSCON;
    sfr_page_1();
    ADCON1 = 0;
    ADCON2 = 0;
    PCACON = 0;
    TWICON = 0;
    INSCON = saved_page;
}
