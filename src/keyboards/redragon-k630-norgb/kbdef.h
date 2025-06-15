#ifndef KBDEF_H
#define KBDEF_H

#include "sh68f90a.h"

#define MATRIX_ROWS 5
#define MATRIX_COLS 14

// Row Pin Bits
#define KB_R0_P7_1 _P7_1
#define KB_R1_P7_2 _P7_2
#define KB_R2_P7_3 _P7_3
#define KB_R3_P5_3 _P5_3
#define KB_R4_P5_4 _P5_4

// Row Pins
#define KB_R0 P7_1
#define KB_R1 P7_2
#define KB_R2 P7_3
#define KB_R3 P5_3
#define KB_R4 P5_4

// Column Pin Bits
#define KB_C0_P1_4  _P1_4
#define KB_C1_P1_5  _P1_5
#define KB_C2_P2_0  _P2_0
#define KB_C3_P2_1  _P2_1
#define KB_C4_P2_2  _P2_2
#define KB_C5_P2_3  _P2_3
#define KB_C6_P2_4  _P2_4
#define KB_C7_P2_5  _P2_5
#define KB_C8_P3_0  _P3_0
#define KB_C9_P3_1  _P3_1
#define KB_C10_P3_2 _P3_2
#define KB_C11_P3_3 _P3_3
#define KB_C12_P3_4 _P3_4
#define KB_C13_P3_5 _P3_5

// Column Pins
#define KB_C0  P1_4
#define KB_C1  P1_5
#define KB_C2  P2_0
#define KB_C3  P2_1
#define KB_C4  P2_2
#define KB_C5  P2_3
#define KB_C6  P2_4
#define KB_C7  P2_5
#define KB_C8  P3_0
#define KB_C9  P3_1
#define KB_C10 P3_2
#define KB_C11 P3_3
#define KB_C12 P3_4
#define KB_C13 P3_5

// LED PWM Registers
#define LED_PWM_C0  PWM24
#define LED_PWM_C1  PWM25
#define LED_PWM_C2  PWM10
#define LED_PWM_C3  PWM11
#define LED_PWM_C4  PWM12
#define LED_PWM_C5  PWM13
#define LED_PWM_C6  PWM14
#define LED_PWM_C7  PWM15
#define LED_PWM_C8  PWM00
#define LED_PWM_C9  PWM01
#define LED_PWM_C10 PWM02
#define LED_PWM_C11 PWM03
#define LED_PWM_C12 PWM04
#define LED_PWM_C13 PWM05

// LED Row Pin Bits
#define LED_R0_P6_1 _P6_1
#define LED_R1_P6_2 _P6_2
#define LED_R2_P6_3 _P6_3
#define LED_R3_P6_4 _P6_4
#define LED_R4_P6_5 _P6_5

// LED Row Pins
#define LED_R0 P6_1
#define LED_R1 P6_2
#define LED_R2 P6_3
#define LED_R3 P6_4
#define LED_R4 P6_5

// KC_CAPS LED Pin
#define LED_CAPS P0_3

// KC_CAPS LED Pin Bit
#define LED_CAPS_P0_3 _P0_3

#endif
