#pragma once

#include "sh68f881.h"
#include "keycodes.h"
#include <stdint.h>

#define MATRIX_ROWS 6
#define MATRIX_COLS 19

// row pin bits, read on SFR page 0, active low.
#define KB_R0_P2_1 _P2_1
#define KB_R1_P2_2 _P2_2
#define KB_R2_P2_3 _P2_3
#define KB_R3_P4_4 _P4_4
#define KB_R4_P4_5 _P4_5
#define KB_R5_P4_6 _P4_6

#define KB_R_P2_MASK (uint8_t)(KB_R0_P2_1 | KB_R1_P2_2 | KB_R2_P2_3)
#define KB_R_P4_MASK (uint8_t)(KB_R3_P4_4 | KB_R4_P4_5 | KB_R5_P4_6)

// column pin bits, driven low one at a time on SFR page 1, where P6/P7/P8 live.
#define KB_C0_P6_1  _P6_1
#define KB_C1_P6_2  _P6_2
#define KB_C2_P6_3  _P6_3
#define KB_C3_P6_4  _P6_4
#define KB_C4_P6_5  _P6_5
#define KB_C5_P6_6  _P6_6
#define KB_C6_P6_7  _P6_7
#define KB_C7_P7_0  _P7_0
#define KB_C8_P7_1  _P7_1
#define KB_C9_P7_2  _P7_2
#define KB_C10_P7_3 _P7_3
#define KB_C11_P7_4 _P7_4
#define KB_C12_P7_5 _P7_5
#define KB_C13_P7_6 _P7_6
#define KB_C14_P7_7 _P7_7
#define KB_C15_P8_0 _P8_0
#define KB_C16_P8_1 _P8_1
#define KB_C17_P8_2 _P8_2
#define KB_C18_P8_7 _P8_7

#define KB_C_P6_MASK (uint8_t)(KB_C0_P6_1 | KB_C1_P6_2 | KB_C2_P6_3 | KB_C3_P6_4 | KB_C4_P6_5 | KB_C5_P6_6 | KB_C6_P6_7)
#define KB_C_P7_MASK (uint8_t)(KB_C7_P7_0 | KB_C8_P7_1 | KB_C9_P7_2 | KB_C10_P7_3 | KB_C11_P7_4 | KB_C12_P7_5 | KB_C13_P7_6 | KB_C14_P7_7)
#define KB_C_P8_MASK (uint8_t)(KB_C15_P8_0 | KB_C16_P8_1 | KB_C17_P8_2 | KB_C18_P8_7)

// the populated columns are not contiguous, and grouping them by port is what lets a column index alone pick the port to drive.
#define KB_C_P7_FIRST 7
#define KB_C_P8_FIRST 15

extern const __code uint8_t kb_col_masks[MATRIX_COLS];

// one column lights per subframe, so a whole frame is MATRIX_COLS of them.
#define LED_SUBFRAMES_PER_SCAN MATRIX_COLS

// backlight anode pin bits, driven high one at a time, R0-R2 on page 0 and R3-R5 on page 1.
#define LED_R0_P1_0 _P1_0
#define LED_R1_P1_1 _P1_1
#define LED_R2_P1_2 _P1_2
#define LED_R3_P5_0 _P5_0
#define LED_R4_P5_1 _P5_1
#define LED_R5_P5_2 _P5_2

#define KB_ANODE_P1_MASK (uint8_t)(LED_R0_P1_0 | LED_R1_P1_1 | LED_R2_P1_2)
#define KB_ANODE_P5_MASK (uint8_t)(LED_R3_P5_0 | LED_R4_P5_1 | LED_R5_P5_2)

// USB pin bits
#define USB_DM_P0_4 _P0_4
#define USB_DP_P2_0 _P2_0

// indicator pin bits, driven outputs; only Caps and Scroll are fitted, P3.4 and P3.7 stay driven and unnamed.
#define LED_CAPS_P3_5   _P3_5
#define LED_SCROLL_P3_6 _P3_6

enum custom_keycodes {
    FX_NEXT = SAFE_RANGE,
    KB_LOCK,  // drops every key until pressed again
    GUI_LOCK, // drops Gui and App presses
    NKRO_TG,  // switches the report between 6KRO and NKRO
    WASD_TG,  // swaps WASD with the arrow cluster, both ways
    BRI_UP,
    BRI_DN,
    SPD_UP,
    SPD_DN,

    // direct effect select, one keycode per animation plus off.
    FX_SET_0,
    FX_SET_1,
    FX_SET_2,
    FX_SET_3,
    FX_SET_OFF,

    KB_SAFE_RANGE,
};

#define LED_BRIGHTNESS_LEVELS 4
