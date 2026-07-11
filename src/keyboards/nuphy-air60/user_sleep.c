#include "kbdef.h" // sh68f90a.h (SFRs) + keycodes
#include "user_sleep.h"
#include "user_init.h"

#ifdef SLEEP_ENABLE

// Air60 INT4-wake Power-Down sleep. Before dropping into Power-Down we walk
// every port's control/pull registers to put the panel into a low-power state
// (LED column/row drivers released so they stop sourcing, the matrix arranged so
// a keypress drives the INT4 wake pin), then arm external interrupt 4.
//
// HARDWARE NOTE: this is the one part of the sleep feature that cannot be
// verified without the physical keyboard. If a pin here is wrong the board will
// not wake from sleep and will need a reflash to recover. The wake pin is P4.1;
// INT4 also sees the BK3632 ACK on P4.2, which is why sleep_task() puts the
// BK3632 to sleep (CMD_07) before we get here.

user_sleep_mode_t user_sleep_supported(void)
{
    // Pick the sleep variant by the conn slider (P5.5: 1 = USB, 0 = RF):
    //  - RF: regulator-off battery sleep, inactivity-triggered.
    //  - USB: USB-suspend sleep, triggered only when the host parks the bus
    //    (so we never self-suspend an active host).
    return (CONN_MODE_SWITCH == 0) ? USER_SLEEP_RF : USER_SLEEP_USB;
}

void user_sleep_prepare(void)
{
    // --- Per-port low-power parking -----------------------------------------
    // Transcribed from the stock sleep teardown, minus its provably-dead writes
    // (duplicate PxPCR clears and a redundant second P0CR mask).
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

    // --- Arm INT4 as the wake source ----------------------------------------
    EXF1 = 0;    // clear any latched P4.x edge flags
    IENC = 0xF3; // interrupt edge/control config
    EXF0 = 0x40; // INT4 trigger-mode select
    EX4  = 1;    // enable external interrupt 4
    // EA is already 1 (interrupts were running); power_enter_powerdown() now
    // handles the USB-wake arming + the SUSLO/PCON.PD entry.
}

void user_sleep_wake(void)
{
    // Disable INT4 so it stops firing on P4 activity during normal operation
    // (the BK3632 ACK on P4.2 toggles constantly once RF is back up) and clear
    // any edge it latched.
    EX4  = 0;
    EXF1 = 0;

    // Re-establish the RF MOT line (P0.5) before the BK3632 is re-synced:
    // drive P0.5 low, switch it to output, drive low again.
    RF_BB_SPI_MOT = 0;
    P0CR |= RF_BB_SPI_MOT_P0_5;
    RF_BB_SPI_MOT = 0;

    // Restore the normal operating GPIO configuration.
    user_gpio_init();
}

#endif // SLEEP_ENABLE
