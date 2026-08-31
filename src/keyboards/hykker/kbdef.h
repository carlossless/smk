#pragma once

#include "sh68f881.h"
#include "keycodes.h"

#define MATRIX_ROWS 6
#define MATRIX_COLS 24

enum custom_keycodes {
    FX_NEXT = SAFE_RANGE, // cycle to the next backlight animation

    KB_SAFE_RANGE,
};

// Rows are read on SFR page 0, active low: R0-R2 on P2.1-P2.3, R3-R5 on P4.4-P4.6.
#define KB_R0_P2_1 0x02u
#define KB_R1_P2_2 0x04u
#define KB_R2_P2_3 0x08u
#define KB_R3_P4_4 0x10u
#define KB_R4_P4_5 0x20u
#define KB_R5_P4_6 0x40u

// Columns are driven low one at a time on SFR page 1, where P6/P7/P8 live.
#define KB_C_P6_MASK 0xFFu
#define KB_C_P7_MASK 0xFFu
#define KB_C_P8_MASK 0xFFu

// Indicators share the top four columns, active low.
#define LED_NUM_P8_7    0x80u
#define LED_CAPS_P8_6   0x40u
#define LED_SCROLL_P8_5 0x20u
#define LED_WIN_P8_4    0x10u
