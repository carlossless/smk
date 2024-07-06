#ifndef KBDEF_H
#define KBDEF_H

#include "sh68f90a.h"

#define MATRIX_ROWS 5
#define MATRIX_COLS 14

// Row Pins
#define KB_R0 P7_1
#define KB_R1 P7_2
#define KB_R2 P7_3
#define KB_R3 P5_3
#define KB_R4 P5_4

// Column Pins
#define KB_C0  P6_0
#define KB_C1  P6_1
#define KB_C2  P6_2
#define KB_C3  P6_3
#define KB_C4  P6_4
#define KB_C5  P6_5
#define KB_C6  P6_6
#define KB_C7  P6_7
#define KB_C8  P5_0
#define KB_C9  P5_1
#define KB_C10 P5_2
#define KB_C11 P5_7
#define KB_C12 P4_0
#define KB_C13 P4_2

// ATTENTION: LED pins have not been traced.
// All code related to led handling doesn't do harm and serve as reference for future work on making the leds work.

// LED PWM Registers
#define LED_PWM_C0  PWM40
#define LED_PWM_C1  PWM41
#define LED_PWM_C2  PWM42
#define LED_PWM_C3  PWM05
#define LED_PWM_C4  PWM04
#define LED_PWM_C5  PWM03
#define LED_PWM_C6  PWM02
#define LED_PWM_C7  PWM01
#define LED_PWM_C8  PWM00
#define LED_PWM_C9  PWM15
#define LED_PWM_C10 PWM14
#define LED_PWM_C11 PWM13
#define LED_PWM_C12 PWM12
#define LED_PWM_C13 PWM11
#define LED_PWM_C14 PWM10
#define LED_PWM_C15 PWM25

// RGB Row Pins
#define RGB_R0R P0_4
#define RGB_R0G P6_1
#define RGB_R0B P0_3
#define RGB_R1R P6_7
#define RGB_R1G P6_2
#define RGB_R1B P6_6
#define RGB_R2R P0_2
#define RGB_R2G P6_3
#define RGB_R2B P5_7
#define RGB_R3R P4_5
#define RGB_R3G P6_4
#define RGB_R3B P4_6
#define RGB_R4R P4_4
#define RGB_R4G P6_5
#define RGB_R4B P4_3
#define RGB_ULR P1_1
#define RGB_ULG P1_2
#define RGB_ULB P1_3

// RGB Row Pin Bits
#define RGB_R0R_P0_4 _P0_4
#define RGB_R0G_P6_1 _P6_1
#define RGB_R0B_P0_3 _P0_3
#define RGB_R1R_P6_7 _P6_7
#define RGB_R1G_P6_2 _P6_2
#define RGB_R1B_P6_6 _P6_6
#define RGB_R2R_P0_2 _P0_2
#define RGB_R2G_P6_3 _P6_3
#define RGB_R2B_P5_7 _P5_7
#define RGB_R3R_P4_5 _P4_5
#define RGB_R3G_P6_4 _P6_4
#define RGB_R3B_P4_6 _P4_6
#define RGB_R4R_P4_4 _P4_4
#define RGB_R4G_P6_5 _P6_5
#define RGB_R4B_P4_3 _P4_3
#define RGB_ULR_P1_1 _P1_1
#define RGB_ULG_P1_2 _P1_2
#define RGB_ULB_P1_3 _P1_3

#endif
