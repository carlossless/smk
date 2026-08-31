#include "kbdef.h"
#include "layout.h"
#include "user_layout.h"
#include "report.h"
#include <stdint.h>

// Transcribed from the stock firmware's 144-entry table, indexed by column*6 + row.
// Rows are physical keyboard rows: R0 function, R1 number, R2 QWERTY, R3 ASDF,
// R4 ZXCV, R5 bottom. The Lang/Intl entries belong to a JIS variant of the same PCB
// and are unpopulated here.

// clang-format off

#define _BL 0
#define _FL 1

const uint16_t keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BL] = {
        { KC_NO, KC_NO,   KC_ESC,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_NO,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  KC_PSCR, KC_SCRL, KC_PAUS, KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO   },
        { KC_NO, KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSPC, KC_NO,   KC_INS,  KC_HOME, KC_PGUP, KC_NUM,  KC_PSLS, KC_PAST, KC_PMNS, KC_NO   },
        { KC_NO, KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSLS, KC_NO,   KC_DEL,  KC_END,  KC_PGDN, KC_P7,   KC_P8,   KC_P9,   KC_PPLS, KC_NO   },
        { KC_NO, KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT, KC_NUHS, KC_ENT,  KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_P4,   KC_P5,   KC_P6,   KC_NO,   KC_NO   },
        { KC_NO, KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_INT1, KC_RSFT, KC_NO,   KC_NO,   KC_NO,   KC_UP,   KC_NO,   KC_P1,   KC_P2,   KC_P3,   KC_PENT, KC_NO   },
        { KC_NO, KC_LCTL, KC_LALT, KC_LNG2, KC_INT5, KC_NO,   KC_SPC,  KC_INT4, KC_LNG1, KC_RALT, MO(_FL), KC_APP,  KC_INT2, KC_RCTL, KC_NO,   KC_NO,   KC_LEFT, KC_DOWN, KC_RGHT, KC_P0,   KC_NO,   KC_PDOT, KC_NO,   KC_LGUI }
    },
    [_FL] = {
        { KC_NO, KC_NO,   _______, _______, _______, _______, _______, KC_NO,   _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO   },
        { KC_NO, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, KC_NO,   _______, _______, _______, _______, _______, _______, _______, KC_NO   },
        { KC_NO, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, KC_NO,   _______, _______, _______, _______, _______, _______, _______, KC_NO   },
        { KC_NO, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, KC_NO,   KC_NO,   KC_NO,   KC_NO,   _______, _______, _______, KC_NO,   KC_NO   },
        { KC_NO, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, KC_NO,   KC_NO,   KC_NO,   _______, KC_NO,   _______, _______, _______, _______, KC_NO   },
        { KC_NO, _______, _______, _______, _______, KC_NO,   _______, _______, _______, _______, _______, _______, _______, _______, KC_NO,   KC_NO,   _______, _______, _______, _______, KC_NO,   _______, KC_NO,   _______ }
    }
};
