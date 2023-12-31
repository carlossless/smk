#include "pwm.h"
#include "matrix.h"
#include <stdint.h>
#include <stdio.h>

/*
# Columns
P5_0 - PWM40 - C0
P5_1 - PWM41 - C1
P5_2 - PWM42 - C2
P3_5 - PWM05 - C3
P3_4 - PWM04 - C4
P3_3 - PWM03 - C5
P3_2 - PWM02 - C6
P3_1 - PWM01 - C7
P3_0 - PWM00 - C8
P2_5 - PWM15 - C9
P2_4 - PWM14 - C10
P2_3 - PWM13 - C11
P2_2 - PMM12 - C12
P2_1 - PWM11 - C13
P2_0 - PWM10 - C14
P1_5 - PWM25 - C15
*/

#define PWM_CLK_DIV             0b010 // PWM_CLK = SYS_CLK / 4
#define PWM_SS_BIT              (1 << 3)
#define PWM_MOD_BIT             (1 << 4)
#define PWM_INT_ENABLE_BIT      (1 << 6)
#define PWM_MODE_ENABLE_BIT     (1 << 7)

#define PWM_PERD                0x0400 // 1024 / PWM_CLK ~= 43 us

#define PWM_DUTY1               PWM_PERD
#define PWM_DUTY2               0

#define PWM_PERDH_INIT          ((uint8_t)(PWM_PERD>>8))
#define PWM_PERDL_INIT          ((uint8_t)(PWM_PERD))

#define PWM_DUTY1H_INIT         ((uint8_t)(PWM_DUTY1>>8))
#define PWM_DUTY1L_INIT         ((uint8_t)(PWM_DUTY1))
#define PWM_DUTY2H_INIT         ((uint8_t)(PWM_DUTY2>>8))
#define PWM_DUTY2L_INIT         ((uint8_t)(PWM_DUTY2))

void pwm_init()
{
    PWM0PERDH = PWM_PERDH_INIT;
    PWM0PERDL = PWM_PERDL_INIT;

    PWM1PERDH = PWM_PERDH_INIT;
    PWM1PERDL = PWM_PERDL_INIT;

    PWM2PERDH = PWM_PERDH_INIT;
    PWM2PERDL = PWM_PERDL_INIT;

    PWM4PERDH = PWM_PERDH_INIT;
    PWM4PERDL = PWM_PERDL_INIT;

    PWM00DUTY1H = PWM_DUTY1H_INIT;
    PWM00DUTY1L = PWM_DUTY1L_INIT;
    PWM00DUTY2H = PWM_DUTY2H_INIT;
    PWM00DUTY2L = PWM_DUTY2L_INIT;

    PWM01DUTY1H = PWM_DUTY1H_INIT;
    PWM01DUTY1L = PWM_DUTY1L_INIT;
    PWM01DUTY2H = PWM_DUTY2H_INIT;
    PWM01DUTY2L = PWM_DUTY2L_INIT;

    PWM02DUTY1H = PWM_DUTY1H_INIT;
    PWM02DUTY1L = PWM_DUTY1L_INIT;
    PWM02DUTY2H = PWM_DUTY2H_INIT;
    PWM02DUTY2L = PWM_DUTY2L_INIT;

    PWM03DUTY1H = PWM_DUTY1H_INIT;
    PWM03DUTY1L = PWM_DUTY1L_INIT;
    PWM03DUTY2H = PWM_DUTY2H_INIT;
    PWM03DUTY2L = PWM_DUTY2L_INIT;

    PWM04DUTY1H = PWM_DUTY1H_INIT;
    PWM04DUTY1L = PWM_DUTY1L_INIT;
    PWM04DUTY2H = PWM_DUTY2H_INIT;
    PWM04DUTY2L = PWM_DUTY2L_INIT;

    PWM05DUTY1H = PWM_DUTY1H_INIT;
    PWM05DUTY1L = PWM_DUTY1L_INIT;
    PWM05DUTY2H = PWM_DUTY2H_INIT;
    PWM05DUTY2L = PWM_DUTY2L_INIT;

    PWM10DUTY1H = PWM_DUTY1H_INIT;
    PWM10DUTY1L = PWM_DUTY1L_INIT;
    PWM10DUTY2H = PWM_DUTY2H_INIT;
    PWM10DUTY2L = PWM_DUTY2L_INIT;

    PWM11DUTY1H = PWM_DUTY1H_INIT;
    PWM11DUTY1L = PWM_DUTY1L_INIT;
    PWM11DUTY2H = PWM_DUTY2H_INIT;
    PWM11DUTY2L = PWM_DUTY2L_INIT;

    PWM12DUTY1H = PWM_DUTY1H_INIT;
    PWM12DUTY1L = PWM_DUTY1L_INIT;
    PWM12DUTY2H = PWM_DUTY2H_INIT;
    PWM12DUTY2L = PWM_DUTY2L_INIT;

    PWM13DUTY1H = PWM_DUTY1H_INIT;
    PWM13DUTY1L = PWM_DUTY1L_INIT;
    PWM13DUTY2H = PWM_DUTY2H_INIT;
    PWM13DUTY2L = PWM_DUTY2L_INIT;

    PWM14DUTY1H = PWM_DUTY1H_INIT;
    PWM14DUTY1L = PWM_DUTY1L_INIT;
    PWM14DUTY2H = PWM_DUTY2H_INIT;
    PWM14DUTY2L = PWM_DUTY2L_INIT;

    PWM15DUTY1H = PWM_DUTY1H_INIT;
    PWM15DUTY1L = PWM_DUTY1L_INIT;
    PWM15DUTY2H = PWM_DUTY2H_INIT;
    PWM15DUTY2L = PWM_DUTY2L_INIT;

    PWM25DUTY1H = PWM_DUTY1H_INIT;
    PWM25DUTY1L = PWM_DUTY1L_INIT;
    PWM25DUTY2H = PWM_DUTY2H_INIT;
    PWM25DUTY2L = PWM_DUTY2L_INIT;

    PWM40DUTY1H = PWM_DUTY1H_INIT;
    PWM40DUTY1L = PWM_DUTY1L_INIT;
    PWM40DUTY2H = PWM_DUTY2H_INIT;
    PWM40DUTY2L = PWM_DUTY2L_INIT;

    PWM41DUTY1H = PWM_DUTY1H_INIT;
    PWM41DUTY1L = PWM_DUTY1L_INIT;
    PWM41DUTY2H = PWM_DUTY2H_INIT;
    PWM41DUTY2L = PWM_DUTY2L_INIT;

    PWM42DUTY1H = PWM_DUTY1H_INIT;
    PWM42DUTY1L = PWM_DUTY1L_INIT;
    PWM42DUTY2H = PWM_DUTY2H_INIT;
    PWM42DUTY2L = PWM_DUTY2L_INIT;

    IEN1 |= (1 << 1); // EPWM0
}

void pwm_enable()
{
    PWM00CON = (uint8_t)(PWM_MODE_ENABLE_BIT | PWM_INT_ENABLE_BIT | PWM_SS_BIT | PWM_CLK_DIV);
    PWM01CON = PWM_SS_BIT;
    PWM02CON = PWM_SS_BIT;
    PWM03CON = PWM_SS_BIT;
    PWM04CON = PWM_SS_BIT;
    PWM05CON = PWM_SS_BIT;

    PWM10CON = (uint8_t)(PWM_MODE_ENABLE_BIT | PWM_SS_BIT | PWM_CLK_DIV);
    PWM11CON = PWM_SS_BIT;
    PWM12CON = PWM_SS_BIT;
    PWM13CON = PWM_SS_BIT;
    PWM14CON = PWM_SS_BIT;
    PWM15CON = PWM_SS_BIT;

    PWM20CON = (uint8_t)(PWM_MODE_ENABLE_BIT | PWM_SS_BIT | PWM_CLK_DIV);
    PWM25CON = PWM_SS_BIT;

    PWM40CON = (uint8_t)(PWM_MODE_ENABLE_BIT | PWM_SS_BIT | PWM_CLK_DIV);

    PWM41CON = PWM_SS_BIT;
    PWM42CON = PWM_SS_BIT;
}

void pwm_disable()
{
    PWM00CON = (uint8_t)(PWM_CLK_DIV);
    PWM01CON = 0;
    PWM02CON = 0;
    PWM03CON = 0;
    PWM04CON = 0;
    PWM05CON = 0;

    PWM10CON = (uint8_t)(PWM_CLK_DIV);
    PWM11CON = 0;
    PWM12CON = 0;
    PWM13CON = 0;
    PWM14CON = 0;
    PWM15CON = 0;

    PWM20CON = (uint8_t)(PWM_CLK_DIV);
    PWM25CON = 0;

    PWM40CON = (uint8_t)(PWM_CLK_DIV);
    PWM41CON = 0;
    PWM42CON = 0;
}

void pwm_set_all_columns(uint16_t intensity)
{
    uint16_t adjusted = 0x0400 - intensity;

    PWM00DUTY2H = adjusted >> 8;
    PWM00DUTY2L = adjusted;
    PWM01DUTY2H = adjusted >> 8;
    PWM01DUTY2L = adjusted;
    PWM02DUTY2H = adjusted >> 8;
    PWM02DUTY2L = adjusted;
    PWM03DUTY2H = adjusted >> 8;
    PWM03DUTY2L = adjusted;
    PWM04DUTY2H = adjusted >> 8;
    PWM04DUTY2L = adjusted;
    PWM05DUTY2H = adjusted >> 8;
    PWM05DUTY2L = adjusted;
    PWM10DUTY2H = adjusted >> 8;
    PWM10DUTY2L = adjusted;
    PWM11DUTY2H = adjusted >> 8;
    PWM11DUTY2L = adjusted;
    PWM12DUTY2H = adjusted >> 8;
    PWM12DUTY2L = adjusted;
    PWM13DUTY2H = adjusted >> 8;
    PWM13DUTY2L = adjusted;
    PWM14DUTY2H = adjusted >> 8;
    PWM14DUTY2L = adjusted;
    PWM15DUTY2H = adjusted >> 8;
    PWM15DUTY2L = adjusted;
    PWM25DUTY2H = adjusted >> 8;
    PWM25DUTY2L = adjusted;
    PWM40DUTY2H = adjusted >> 8;
    PWM40DUTY2L = adjusted;
    PWM41DUTY2H = adjusted >> 8;
    PWM41DUTY2L = adjusted;
    PWM42DUTY2H = adjusted >> 8;
    PWM42DUTY2L = adjusted;
}

void pwm_interrupt_handler() __interrupt (_INT_PWM0)
{
    matrix_scan_step();
}
