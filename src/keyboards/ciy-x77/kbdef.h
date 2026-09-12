#pragma once

#include "sh68f89.h"
#include "keycodes.h"
#include <stdint.h>

#define MATRIX_ROWS 6
#define MATRIX_COLS 22

// row pin bits, read on SFR page 1, active low. P5.6 and P5.7 are the EEPROM bus.
#define KB_R0_P5_0 _P5_0
#define KB_R1_P5_1 _P5_1
#define KB_R2_P5_2 _P5_2
#define KB_R3_P5_3 _P5_3
#define KB_R4_P5_4 _P5_4
#define KB_R5_P5_5 _P5_5

#define KB_R_P5_MASK (uint8_t)(KB_R0_P5_0 | KB_R1_P5_1 | KB_R2_P5_2 | KB_R3_P5_3 | KB_R4_P5_4 | KB_R5_P5_5)

// column pin bits, driven low one at a time. P0, P1 and P4 are on SFR page 0, P7 on page 1.
#define KB_C0_P0_0  _P0_0
#define KB_C1_P0_1  _P0_1
#define KB_C2_P0_2  _P0_2
#define KB_C3_P0_3  _P0_3
#define KB_C4_P0_4  _P0_4
#define KB_C5_P0_5  _P0_5
#define KB_C6_P0_6  _P0_6
#define KB_C7_P0_7  _P0_7
#define KB_C8_P1_0  _P1_0
#define KB_C9_P1_1  _P1_1
#define KB_C10_P1_2 _P1_2
#define KB_C11_P1_3 _P1_3
#define KB_C12_P1_4 _P1_4
#define KB_C13_P1_5 _P1_5
#define KB_C14_P1_6 _P1_6
#define KB_C15_P1_7 _P1_7
#define KB_C16_P4_6 _P4_6
#define KB_C17_P4_7 _P4_7
#define KB_C18_P7_1 _P7_1
#define KB_C19_P7_2 _P7_2
#define KB_C20_P7_3 _P7_3
#define KB_C21_P7_4 _P7_4

#define KB_C_P0_MASK _P0_ALL
#define KB_C_P1_MASK _P1_ALL
#define KB_C_P4_MASK (uint8_t)(KB_C16_P4_6 | KB_C17_P4_7)
#define KB_C_P7_MASK (uint8_t)(KB_C18_P7_1 | KB_C19_P7_2 | KB_C20_P7_3 | KB_C21_P7_4)

// grouping the columns by port is what lets a column index alone pick the port to drive.
#define KB_C_P1_FIRST 8
#define KB_C_P4_FIRST 16
#define KB_C_P7_FIRST 18

extern const __code uint8_t kb_col_masks[MATRIX_COLS];

// USB pin bits
#define USB_DM_P2_6 _P2_6
#define USB_DP_P2_7 _P2_7

// indicator pin bits, driven outputs
#define LED_NUM_P3_0    _P3_0
#define LED_CAPS_P3_1   _P3_1
#define LED_SCROLL_P3_2 _P3_2

#define KB_LOCK_LED_MASK (uint8_t)(LED_NUM_P3_0 | LED_CAPS_P3_1 | LED_SCROLL_P3_2)

// settings live in a 24Cxx on a bit-banged bus: SDA and SCL share P5 with the matrix rows,
// write protect is a page 0 pin the platform releases only for the duration of a write.
#define KB_EE_PORT      P5
#define KB_EE_PORT_CR   P5CR
#define KB_EE_SDA       _P5_6
#define KB_EE_SCL       _P5_7
#define KB_EE_DEV_ADDR  0xA0u
#define KB_EE_PAGE_SIZE 8u

#define KB_EE_WP_P4_1 _P4_1
#define KB_EE_WP_RELEASE()             \
    do {                               \
        P4 &= (uint8_t)~KB_EE_WP_P4_1; \
        P4CR |= KB_EE_WP_P4_1;         \
    } while (0)
#define KB_EE_WP_PROTECT()               \
    do {                                 \
        P4CR &= (uint8_t)~KB_EE_WP_P4_1; \
        P4 |= KB_EE_WP_P4_1;             \
    } while (0)

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
