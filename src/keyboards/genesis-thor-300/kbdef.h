#pragma once

#include "sh68f881.h"
#include "keycodes.h"

#define MATRIX_ROWS 6
#define MATRIX_COLS 19

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

// This board is the tenkeyless variant of what the stock firmware drives, so the four
// numpad columns are unpopulated and column 0 carries only vendor codes that never
// reach a report. What is left is not contiguous - the left GUI key is alone out on
// physical column 23 - so scan order maps through here rather than being the index.
extern const __code uint8_t kb_col_pins[MATRIX_COLS];

// The backlight multiplexes one column per subframe, so a whole frame is MATRIX_COLS
// subframes and they have to run far more often than the matrix scan.
#define LED_SUBFRAMES_PER_SCAN MATRIX_COLS

// Backlight anodes, driven high one column at a time: R0-R2 on P1.0-P1.2 (SFR page 0),
// R3-R5 on P5.0-P5.2 (page 1). The cathodes are the matrix columns.
#define KB_ANODE_P1_MASK 0x07u
#define KB_ANODE_P5_MASK 0x07u

// Indicators are driven outputs on P3.4-P3.7.
#define LED_NUM_P3_4    0x10u
#define LED_CAPS_P3_5   0x20u
#define LED_SCROLL_P3_6 0x40u
#define LED_WIN_P3_7    0x80u
