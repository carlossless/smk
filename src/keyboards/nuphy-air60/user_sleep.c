#include "kbdef.h"
#include "user_sleep.h"
#include "user_init.h"
#include "gpio.h"
#include "extint.h"

#ifdef SLEEP_ENABLE

// Air60 INT4-wake Power-Down sleep. Which pins go where is board knowledge and
// lives here; how a port is driven (gpio.h) and how the wake source is armed
// (extint.h) belong to the platform.
//
// HARDWARE NOTE: this parking sequence cannot be verified without the physical
// keyboard, and it is transcribed from the stock firmware's teardown, so the
// order of the writes is preserved exactly. If a pin here is wrong the board
// will not wake and will need a reflash to recover. The wake pin is P4.1; INT4
// also sees the BK3632 ACK on P4.2, which is why sleep_task() puts the BK3632
// to sleep (CMD_07) before we get here.
//
// _Px_y appears where a pin has no name in kbdef.h — those six are touched by
// the stock teardown but configured nowhere else in SMK.

user_sleep_mode_t user_sleep_supported(void)
{
    // The conn slider picks the variant: RF is regulator-off battery sleep on an
    // inactivity timeout, USB sleeps only once the host parks the bus, so we
    // never self-suspend an active host.
    return (CONN_MODE_SWITCH == 0) ? USER_SLEEP_RF : USER_SLEEP_USB;
}

// Release the LED drivers and park the matrix so nothing sources current, then
// leave the panel arranged so a keypress reaches the INT4 wake pin.
static void park_panel(void)
{
    GPIO_PULLUP_OFF(1, _P1_4 | KB_C15_P1_5);
    GPIO_INPUT(1, _P1_4 | KB_C15_P1_5);

    GPIO_PULLUP_OFF(2, KB_C14_P2_0 | KB_C13_P2_1 | KB_C12_P2_2 | KB_C11_P2_3 | KB_C10_P2_4 | KB_C9_P2_5);
    GPIO_INPUT(2, KB_C14_P2_0 | KB_C13_P2_1 | KB_C12_P2_2 | KB_C11_P2_3 | KB_C10_P2_4 | KB_C9_P2_5);

    GPIO_PULLUP_OFF(3, KB_C8_P3_0 | KB_C7_P3_1 | KB_C6_P3_2 | KB_C5_P3_3 | KB_C4_P3_4 | KB_C3_P3_5);
    GPIO_INPUT(3, KB_C8_P3_0 | KB_C7_P3_1 | KB_C6_P3_2 | KB_C5_P3_3 | KB_C4_P3_4 | KB_C3_P3_5);

    GPIO_PULLUP_OFF(5, KB_C0_P5_0 | KB_C1_P5_1 | KB_C2_P5_2);
    GPIO_INPUT(5, KB_C0_P5_0 | KB_C1_P5_1 | KB_C2_P5_2);

    GPIO_OUTPUT(5, KB_C0_P5_0 | KB_C1_P5_1 | KB_C2_P5_2 | OS_MODE_SWITCH_P5_6);
    GPIO_LOW(5, KB_C0_P5_0 | KB_C1_P5_1 | KB_C2_P5_2 | OS_MODE_SWITCH_P5_6);

    GPIO_OUTPUT(3, KB_C8_P3_0 | KB_C7_P3_1 | KB_C6_P3_2 | KB_C5_P3_3 | KB_C4_P3_4 | KB_C3_P3_5);
    GPIO_LOW(3, KB_C8_P3_0 | KB_C7_P3_1 | KB_C6_P3_2 | KB_C5_P3_3 | KB_C4_P3_4 | KB_C3_P3_5);

    GPIO_OUTPUT(2, KB_C14_P2_0 | KB_C13_P2_1 | KB_C12_P2_2 | KB_C11_P2_3 | KB_C10_P2_4 | KB_C9_P2_5);
    GPIO_LOW(2, KB_C14_P2_0 | KB_C13_P2_1 | KB_C12_P2_2 | KB_C11_P2_3 | KB_C10_P2_4 | KB_C9_P2_5);

    GPIO_OUTPUT(1, RGB_ULB_P1_3 | _P1_4 | KB_C15_P1_5);
    GPIO_LOW(1, _P1_0 | RGB_ULR_P1_1 | RGB_ULG_P1_2 | RGB_ULB_P1_3 | _P1_4 | KB_C15_P1_5);

    GPIO_PULLUP_OFF(0, RGB_R2R_P0_2 | RGB_R0B_P0_3 | RGB_R0R_P0_4);
    GPIO_OUTPUT(0, RGB_R2R_P0_2 | RGB_R0B_P0_3 | RGB_R0R_P0_4);
    GPIO_LOW(0, RGB_R2R_P0_2 | RGB_R0B_P0_3 | RGB_R0R_P0_4);

    GPIO_PULLUP_OFF(5, RGB_R2B_P5_7);
    GPIO_OUTPUT(5, OS_MODE_SWITCH_P5_6 | RGB_R2B_P5_7);
    GPIO_LOW(5, OS_MODE_SWITCH_P5_6 | RGB_R2B_P5_7);

    GPIO_PULLUP_WRITE(6, 0);
    GPIO_DIR_WRITE(6, _P6_0 | RGB_R0G_P6_1 | RGB_R1G_P6_2 | RGB_R2G_P6_3 | RGB_R3G_P6_4 | RGB_R4G_P6_5 | RGB_R1B_P6_6 | RGB_R1R_P6_7);
    GPIO_WRITE(6, 0);

    GPIO_PULLUP_OFF(4, _P4_0 | _P4_1 | RGB_R4B_P4_3 | RGB_R4R_P4_4 | RGB_R3R_P4_5 | RGB_R3B_P4_6);
    GPIO_OUTPUT(4, _P4_0 | _P4_1 | RGB_R4B_P4_3 | RGB_R4R_P4_4 | RGB_R3R_P4_5 | RGB_R3B_P4_6);
    GPIO_LOW(4, _P4_0 | _P4_1 | RGB_R4B_P4_3 | RGB_R4R_P4_4 | RGB_R3R_P4_5 | RGB_R3B_P4_6);

    GPIO_PULLUP_OFF(5, CONN_MODE_SWITCH_P5_5 | OS_MODE_SWITCH_P5_6);
    GPIO_INPUT(5, CONN_MODE_SWITCH_P5_5 | OS_MODE_SWITCH_P5_6);
    GPIO_LOW(5, CONN_MODE_SWITCH_P5_5 | OS_MODE_SWITCH_P5_6);

    GPIO_PULLUP_ON(7, _P7_7);
    GPIO_OUTPUT(7, _P7_7);
    GPIO_LOW(7, _P7_7);

    // Hand the bit-banged SPI lines back to input so their pull-ups hold them
    // high rather than the MCU driving them.
    GPIO_INPUT(4, RF_BB_SPI_SCK_P4_7);
    GPIO_INPUT(0, RF_BB_SPI_MOT_P0_5 | RF_BB_SPI_MOSI_P0_7);
    GPIO_INPUT(7, RF_BB_SPI_CS_P7_4);

    P5_6 = 1;
    P1_3 = 1;
    P7_7 = 1;
    P7_6 = 0;
}

void user_sleep_prepare(void)
{
    park_panel();
    extint_wake_arm();
    // EA is already 1 (interrupts were running); power_enter_powerdown() handles
    // the USB-wake arming and the SUSLO/PCON.PD entry.
}

void user_sleep_wake(void)
{
    // The BK3632 ACK on P4.2 toggles constantly once RF is back up, so INT4 has
    // to go before normal operation resumes.
    extint_wake_disable();

    // Re-establish the RF MOT line before the BK3632 is re-synced: drive low,
    // switch to output, drive low again.
    RF_BB_SPI_MOT = 0;
    GPIO_OUTPUT(0, RF_BB_SPI_MOT_P0_5);
    RF_BB_SPI_MOT = 0;

    user_gpio_init();
}

#endif // SLEEP_ENABLE
