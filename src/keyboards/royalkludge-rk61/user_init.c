#include "kbdef.h"
#include "user_init.h"
#include "pwm.h"

// TODO: move these defines out
#define PWM_PERD 0x0400 // 1024 / PWM_CLK ~= 43 us

#define PWM_DUTY1 (uint16_t) PWM_PERD
#define PWM_DUTY2 (uint16_t)0

#define PWM_PERDH_INIT ((uint8_t)(PWM_PERD >> 8))
#define PWM_PERDL_INIT ((uint8_t)(PWM_PERD))

void user_gpio_init();
void user_pwm_init();

void user_init()
{
    user_gpio_init();
    user_pwm_init();

    IEN1 |= (1 << 1); // EPWM0
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

    // I set this up by following the same pattern done in the original setup (nuphy-air60) and it works, I don't really understand the meaning of this "registers" :D...
    P5    = (uint8_t)(_P5_0 | _P5_1 | _P5_2 | _P5_7);
    P5CR  = (uint8_t)(_P5_0 | _P5_1 | _P5_2 | _P5_7);
    P5PCR = (uint8_t)(_P5_0 | _P5_1 | _P5_2 | _P5_3 | _P5_4 | _P5_7);

    P7PCR = (uint8_t)(_P7_1 | _P7_2 | _P7_3);

    P6    = (uint8_t)(_P6_0 | _P6_1 | _P6_2 | _P6_3 | _P6_4 | _P6_5 | _P6_6 | _P6_7);
    P6CR  = (uint8_t)(_P6_0 | _P6_1 | _P6_2 | _P6_3 | _P6_4 | _P6_5 | _P6_6 | _P6_7);
    P6PCR = (uint8_t)(_P6_0 | _P6_1 | _P6_2 | _P6_3 | _P6_4 | _P6_5 | _P6_6 | _P6_7);

    P4    = (uint8_t)(_P4_0 | _P4_2);
    P4CR  = (uint8_t)(_P4_0 | _P4_2);
    P4PCR = (uint8_t)(_P4_0 | _P4_2);
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
