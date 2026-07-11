#include "kbdef.h" // sh68f90a.h (SFRs) + keycodes
#include "user_sleep.h"
#include "user_init.h"

#ifdef SLEEP_ENABLE

user_sleep_mode_t user_sleep_supported(void)
{
    return (CONN_MODE_SWITCH == 0) ? USER_SLEEP_RF : USER_SLEEP_USB;
}

void user_sleep_prepare(void)
{
    P1PCR &= 0xCF;
    P1CR &= 0xCF;
    P2PCR &= 0xC0;
    P2CR &= 0xC0;
    P3PCR &= 0xC0;
    P3CR &= 0xC0;
    P5PCR &= 0xF8;
    P5CR &= 0xF8;
    P5CR |= 0x47;
    P5 &= 0xB8;
    P3CR |= 0x3F;
    P3 &= 0xC0;
    P2CR |= 0x3F;
    P2 &= 0xC0;
    P1CR |= 0x38;
    P1 &= 0xC0;
    P0PCR &= 0xE3;
    P0CR |= 0x1C;
    P0 &= 0xE3;
    P5PCR &= 0x7F;
    P5CR |= 0xC0;
    P5 &= 0x3F;
    P6PCR = 0x00;
    P6CR  = 0xFF;
    P6    = 0x00;
    P4PCR &= 0x84;
    P4CR |= 0x7B;
    P4 &= 0x84;
    P5PCR &= 0x9F;
    P5CR &= 0x9F;
    P5 &= 0x9F;
    P7PCR |= 0x80;
    P7CR |= 0x80;
    P7 &= 0x7F;
    P4CR &= 0x7F;
    P0CR &= 0x5F;
    P7CR &= 0xEF;
    P5_6 = 1;
    P1_3 = 1;
    P7_7 = 1;
    P7_6 = 0;

    EXF1 = 0;    // clear any latched P4.x edge flags
    IENC = 0xF3; // interrupt edge/control config
    EXF0 = 0x40; // INT4 trigger-mode select
    EX4  = 1;    // enable external interrupt 4
}

void user_sleep_wake(void)
{
    EX4  = 0;
    EXF1 = 0;

    RF_BB_SPI_MOT = 0;
    P0CR |= RF_BB_SPI_MOT_P0_5;
    RF_BB_SPI_MOT = 0;

    user_gpio_init();
}

#endif // SLEEP_ENABLE
