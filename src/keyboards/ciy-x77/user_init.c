#include "kbdef.h"
#include "user_init.h"
#include "debug.h"

extern uint8_t reset_status;

void user_init(void)
{
    dprintf("RST %02x\r\n", reset_status);

    sfr_page_0();

    P0    = _P0_ALL;
    P0PCR = _P0_ALL;
    P0CR  = KB_C_P0_MASK;

    P1    = _P1_ALL;
    P1PCR = _P1_ALL;
    P1CR  = KB_C_P1_MASK;

    // the USB pair is driven by the transceiver, so it is left an input with no pull-up
    P2    = (uint8_t)~(USB_DM_P2_6 | USB_DP_P2_7);
    P2PCR = (uint8_t)~(USB_DM_P2_6 | USB_DP_P2_7);
    P2CR  = _P_NONE;

    P3    = _P3_ALL;
    P3PCR = _P3_ALL;
    P3CR  = KB_LOCK_LED_MASK;

    // write protect idles as an input held high by the EEPROM's own pull-up
    P4    = _P4_ALL;
    P4PCR = _P4_ALL;
    P4CR  = KB_C_P4_MASK;

    uint8_t saved_page = INSCON;
    sfr_page_1();

    // rows and the EEPROM bus are inputs with pull-ups; nothing on P5 is driven from here
    P5    = _P5_ALL;
    P5PCR = _P5_ALL;
    P5CR  = _P_NONE;

    P6    = _P6_ALL;
    P6PCR = _P6_ALL;
    P6CR  = _P_NONE;

    // P7 carries the columns of the full-size variant of this board; unfitted here, so it
    // stays an input rather than driving pins that may not be routed.
    P7    = _P7_ALL;
    P7PCR = _P7_ALL;
    P7CR  = _P_NONE;

    INSCON = saved_page;
}
