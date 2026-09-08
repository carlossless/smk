#include "kbdef.h"
#include "user_init.h"
#include "debug.h"

extern uint8_t reset_status;

void user_init(void)
{
    dprintf("RST %02x\r\n", reset_status);

    P0    = (uint8_t)~(_P0_2 | _P0_3 | USB_DM_P0_4);
    P0PCR = (uint8_t)~(_P0_2 | _P0_3 | USB_DM_P0_4);
    P0CR  = _P_NONE;

    P1    = (uint8_t)~KB_ANODE_P1_MASK;
    P1PCR = _P1_ALL;
    P1CR  = (uint8_t)(KB_ANODE_P1_MASK | _P1_5 | _P1_6 | _P1_7);

    P2    = (uint8_t)~USB_DP_P2_0;
    P2PCR = (uint8_t)~USB_DP_P2_0;
    P2CR  = _P_NONE;

    P3    = _P3_ALL;
    P3PCR = _P3_ALL;
    P3CR  = (uint8_t)(_P3_4 | LED_CAPS_P3_5 | LED_SCROLL_P3_6 | _P3_7);

    P4    = _P4_ALL;
    P4PCR = _P4_ALL;
    P4CR  = _P_NONE;

    uint8_t saved_page = INSCON;
    sfr_page_1();

    P5    = _P5_ALL;
    P5PCR = _P5_ALL;
    P5CR  = KB_ANODE_P5_MASK;
    P5 &= (uint8_t)~KB_ANODE_P5_MASK; // anodes low; see above

    P6    = _P6_ALL;
    P6PCR = _P6_ALL;
    P6CR  = _P6_ALL;

    P7    = _P7_ALL;
    P7PCR = _P7_ALL;
    P7CR  = _P7_ALL;

    P8    = _P8_ALL;
    P8PCR = _P8_ALL;
    P8CR  = _P8_ALL;

    INSCON = saved_page;

    // the USB pair is floated last
    P0PCR &= (uint8_t)~USB_DM_P0_4;
    P0CR &= (uint8_t)~USB_DM_P0_4;
    P2PCR &= (uint8_t)~USB_DP_P2_0;
    P2CR &= (uint8_t)~USB_DP_P2_0;
}
