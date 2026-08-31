#include "kbdef.h"
#include "user_init.h"
#include "debug.h"

extern uint8_t reset_status;

// Port setup transcribed from the stock firmware's init at CODE:0x43B7, in its order.
// P2 and P4 are entirely inputs and carry the matrix rows, the six backlight anodes are
// P1.0-P1.2 plus P5.0-P5.2, and the indicators live on P3.4-P3.7.
//
// The anodes are the one departure from those values: stock leaves them high here and
// its blanking interrupts drive them low again before the main loop samples the rows.
// Nothing here runs that interrupt, and an anode left high clamps the row lines through
// the backlight, so every column reads as pressed.
void user_init(void)
{
    dprintf("RST %02x\r\n", reset_status);

    P0    = 0xE3u;
    P0PCR = 0xE3u;
    P0CR  = 0x00u;

    P1    = 0xF8u;
    P1PCR = 0xFFu;
    P1CR  = 0xE7u;

    P2    = 0xFEu;
    P2PCR = 0xFEu;
    P2CR  = 0x00u;

    P3    = 0xFFu;
    P3PCR = 0xFFu;
    P3CR  = 0xF0u;

    P4    = 0xFFu;
    P4PCR = 0xFFu;
    P4CR  = 0x00u;

    uint8_t saved_page = INSCON;
    sfr_page_1();

    P5    = 0xFFu;
    P5PCR = 0xFFu;
    P5CR  = 0x07u;
    P5 &= (uint8_t)~0x07u; // anodes low; see above

    P6    = 0xFFu;
    P6PCR = 0xFFu;
    P6CR  = 0xFFu;

    P7    = 0xFFu;
    P7PCR = 0xFFu;
    P7CR  = 0xFFu;

    P8    = 0xFFu;
    P8PCR = 0xFFu;
    P8CR  = 0xFFu;

    INSCON = saved_page;

    // The USB pair is released last, as the stock firmware does after its port setup:
    // P0.4 is D- and P2.0 is D+, both floated with their pull-ups off.
    P0PCR &= (uint8_t)~0x10u;
    P0CR &= (uint8_t)~0x10u;
    P2PCR &= (uint8_t)~0x01u;
    P2CR &= (uint8_t)~0x01u;
}
