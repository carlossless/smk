#include "kbdef.h"
#include "user_init.h"
#include "debug.h"

extern uint8_t reset_status;

// Port direction bits are 1 for output. These are the values the stock firmware's
// gpio_port_init writes, which leave exactly the six row pins and the USB pair as
// inputs.
#define P0CR_INIT 0x00u // USB D- floats
#define P2CR_INIT 0xF0u // P2.0 is USB D+, P2.1-P2.3 are rows
#define P4CR_INIT 0x8Fu // P4.4-P4.6 are rows

#define KB_ROWS_P2 (KB_R0_P2_1 | KB_R1_P2_2 | KB_R2_P2_3)
#define KB_ROWS_P4 (KB_R3_P4_4 | KB_R4_P4_5 | KB_R5_P4_6)

void user_init(void)
{
    dprintf("RST %02x\r\n", reset_status);

    P0CR = P0CR_INIT;
    P2CR = P2CR_INIT;
    P4CR = P4CR_INIT;

    P2PCR |= KB_ROWS_P2;
    P4PCR |= KB_ROWS_P4;

    uint8_t saved_page = INSCON;
    sfr_page_1();

    P6CR = 0xFFu;
    P7CR = 0xFFu;
    P8CR = 0xFFu;

    P6 = 0xFFu;
    P7 = 0xFFu;
    P8 = 0xFFu;

    INSCON = saved_page;
}
