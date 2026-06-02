#include "kbdef.h"
#include "user_init.h"
#include "pwm.h"

// Stock fw uses 0x0100 (256-tick period). At PWM_CLK = SYS_CLK / 4 = 6 MHz
// that gives a ~23 kHz PWM frequency, well above the flicker threshold.
// Our previous 0x0400 (1024-tick) ran at ~5.9 kHz which can be visible.
// Stock also writes 8-bit DUTY2 values (DUTY2H=0, DUTY2L=fb_byte) so the
// period must match the DUTY range — 8-bit DUTY against 0x100 PERD =
// usable 0..100% range; 8-bit DUTY against 0x400 PERD = only 0..25%.
#define PWM_PERD 0x0100

// Stock fw writes DUTY1=0 and DUTY2 = framebuffer byte. With DUTY1=0 the
// PWM hardware transitions at t=0 (start of period) and again at DUTY2.
// fb=0 → DUTY2=0 (both transitions at same point) → output stays in one
// state for full period (LED off). fb=PERD-1 → DUTY2=255 → output spans
// full period (LED on).
#define PWM_DUTY1 (uint16_t)0
#define PWM_DUTY2 (uint16_t)0

#define PWM_PERDH_INIT ((uint8_t)(PWM_PERD >> 8))
#define PWM_PERDL_INIT ((uint8_t)(PWM_PERD))

void user_gpio_init();
void user_pwm_init();

void user_init()
{
    user_gpio_init();
    user_pwm_init();

    // PWM0 IRQ is intentionally not enabled — stock fw runs all PWM banks
    // with their per-bank IE = 0 (the hardware drives the waveform; no ISR
    // needed). Timer 2 ISR handles the LED scan + animation; INT4 handles
    // matrix wake. Leaving IEN1 EPWM0 set would just queue a spurious ISR
    // if anything accidentally turned PWM00CON.IE on later.
}

void user_gpio_init()
{
    // configure driving capabilities
    DRVCON = 0x05; // allow P1 to be changed
    P1DRV  = 0x00; // 25mA

    DRVCON = 0x45; // allow P2 to be changed
    P2DRV  = 0x00; // 25mA

    DRVCON = 0x85; // allow P3 to be changed
    P3DRV  = 0x00; // 25mA

    DRVCON = 0xc5; // allow P5 to be changed
    P5DRV  = 0x00; // 25mA

    DRVCON = 0;

    P0CR = (uint8_t)(RGB_R2R_P0_2 | RGB_R0B_P0_3 | RGB_R0R_P0_4);
    P1CR = (uint8_t)(RGB_ULR_P1_1 | RGB_ULG_P1_2 | RGB_ULB_P1_3 | KB_C15_P1_5);
    P2CR = (uint8_t)(KB_C14_P2_0 | KB_C13_P2_1 | KB_C12_P2_2 | KB_C11_P2_3 | KB_C10_P2_4 | KB_C9_P2_5);
    P3CR = (uint8_t)(KB_C8_P3_0 | KB_C7_P3_1 | KB_C6_P3_2 | KB_C5_P3_3 | KB_C4_P3_4 | KB_C3_P3_5);
    P4CR = (uint8_t)(RGB_R4B_P4_3 | RGB_R4R_P4_4 | RGB_R3R_P4_5 | RGB_R3B_P4_6);
    P5CR = (uint8_t)(KB_C0_P5_0 | KB_C1_P5_1 | KB_C2_P5_2 | RGB_R2B_P5_7);
    P6CR = (uint8_t)(RGB_R0G_P6_1 | RGB_R1G_P6_2 | RGB_R2G_P6_3 | RGB_R3G_P6_4 | RGB_R4G_P6_5 | RGB_R1B_P6_6 | RGB_R1R_P6_7);

    P5PCR = (uint8_t)(KB_R3_P5_3 | KB_R4_P5_4 | CONN_MODE_SWITCH_P5_5 | OS_MODE_SWITCH_P5_6);
    P7PCR = (uint8_t)(KB_R0_P7_1 | KB_R1_P7_2 | KB_R2_P7_3);

    if (DEBUG) {
        // UART TXD conflicts with CONN_MODE_SWITCH
        // FIXME: make this configurable somehow
        // P5CR |= CONN_MODE_SWITCH_P5_5;
        // P5PCR &= ~CONN_MODE_SWITCH_P5_5;
    }

    // BB SPI pins for RF. Stock-faithful: pins idle as INPUT with pull-up
    // enabled (idle HIGH via passive pull-up). During each SPI bit the
    // bit-bang functions briefly switch the pin to OUTPUT to drive LOW,
    // then back to INPUT so the pull-up takes the line HIGH again. This
    // open-drain emulation matches the BK3632's preferred SCK/MOSI rise
    // characteristics — see stock fw bb_spi_byte_tx (CODE:0xB07D) and
    // bb_spi_byte_rx (CODE:0xB16D).
    //
    // Latch the high state up front so the FIRST direction-toggle to
    // output drives correctly (latch value matters only while output).
    P7 |= RF_BB_SPI_CS_P7_4;
    P4 |= RF_BB_SPI_SCK_P4_7;
    P0 |= (RF_BB_SPI_MOSI_P0_7 | RF_BB_SPI_MOT_P0_5);

    // DON'T set PxCR for these pins — they start as input. bb_spi will
    // toggle the direction bits per cycle.

    // Pull-ups: MISO + ACK (always input from BK3632), plus the SPI
    // output-side pins so they idle HIGH when in input mode.
    P0PCR |= (RF_BB_SPI_MISO_P0_6 | RF_BB_SPI_MOSI_P0_7 | RF_BB_SPI_MOT_P0_5);
    P4PCR |= (RF_BB_SPI_ACK_P4_2  | RF_BB_SPI_SCK_P4_7);
    P7PCR |= RF_BB_SPI_CS_P7_4;
}

void user_pwm_init()
{
    PWM0PERDH = PWM_PERDH_INIT;
    PWM0PERDL = PWM_PERDL_INIT;

    PWM1PERDH = PWM_PERDH_INIT;
    PWM1PERDL = PWM_PERDL_INIT;

    PWM2PERDH = PWM_PERDH_INIT;
    PWM2PERDL = PWM_PERDL_INIT;

    PWM4PERDH = PWM_PERDH_INIT;
    PWM4PERDL = PWM_PERDL_INIT;

    SET_PWM_DUTY(LED_PWM_C0, PWM_DUTY1, PWM_DUTY2);
    SET_PWM_DUTY(LED_PWM_C1, PWM_DUTY1, PWM_DUTY2);
    SET_PWM_DUTY(LED_PWM_C2, PWM_DUTY1, PWM_DUTY2);
    SET_PWM_DUTY(LED_PWM_C3, PWM_DUTY1, PWM_DUTY2);
    SET_PWM_DUTY(LED_PWM_C4, PWM_DUTY1, PWM_DUTY2);
    SET_PWM_DUTY(LED_PWM_C5, PWM_DUTY1, PWM_DUTY2);
    SET_PWM_DUTY(LED_PWM_C6, PWM_DUTY1, PWM_DUTY2);
    SET_PWM_DUTY(LED_PWM_C7, PWM_DUTY1, PWM_DUTY2);
    SET_PWM_DUTY(LED_PWM_C8, PWM_DUTY1, PWM_DUTY2);
    SET_PWM_DUTY(LED_PWM_C9, PWM_DUTY1, PWM_DUTY2);
    SET_PWM_DUTY(LED_PWM_C10, PWM_DUTY1, PWM_DUTY2);
    SET_PWM_DUTY(LED_PWM_C11, PWM_DUTY1, PWM_DUTY2);
    SET_PWM_DUTY(LED_PWM_C12, PWM_DUTY1, PWM_DUTY2);
    SET_PWM_DUTY(LED_PWM_C13, PWM_DUTY1, PWM_DUTY2);
    SET_PWM_DUTY(LED_PWM_C14, PWM_DUTY1, PWM_DUTY2);
    SET_PWM_DUTY(LED_PWM_C15, PWM_DUTY1, PWM_DUTY2);
}
