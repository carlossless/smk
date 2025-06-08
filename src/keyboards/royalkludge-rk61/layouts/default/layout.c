#include "kbdef.h"
#include "layout.h"
#include "user_layout.h"
#include "report.h"
#include "delay.h"
#include <stdint.h>

// clang-format off

#define LAYOUT_60(                                                      \
                  K00_0, K01_0, K02_0, K03_0, K04_0, K05_0, K06_0, K07_0, K08_0, K09_0, K10_0, K11_0, K12_0, K13_0, \
                  K00_1, K01_1, K02_1, K03_1, K04_1, K05_1, K06_1, K07_1, K08_1, K09_1, K10_1, K11_1, K12_1, K13_1, \
                  K00_2, K01_2, K02_2, K03_2, K04_2, K05_2, K06_2, K07_2, K08_2, K09_2, K10_2, K11_2,        K13_2, \
                  K00_3, K01_3, K02_3, K03_3, K04_3, K05_3, K06_3, K07_3, K08_3, K09_3, K10_3, K13_3, K14_3, K15_3, \
                  K00_4, K01_4, K02_4,                      K05_4,               K08_4, K09_4, K10_4, K13_4, K14_4 \
                  ) {                                                   \
        { K00_0, K01_0, K02_0, K03_0, K04_0, K05_0, K06_0, K07_0, K08_0, K09_0, K10_0, K11_0, K12_0, K13_0 }, \
        { K00_1, K01_1, K02_1, K03_1, K04_1, K05_1, K06_1, K07_1, K08_1, K09_1, K10_1, K11_1, K12_1, K13_1 }, \
        { K00_2, K01_2, K02_2, K03_2, K04_2, K05_2, K06_2, K07_2, K08_2, K09_2, K10_2, K11_2, KC_NO, K13_2 }, \
        { K00_3, K01_3, K02_3, K03_3, K04_3, K05_3, K06_3, K07_3, K08_3, K09_3, K10_3, KC_NO, KC_NO, K13_3 }, \
        { K00_4, K01_4, K02_4, KC_NO, KC_NO, K05_4, KC_NO, KC_NO, K08_4, K09_4, K10_4, KC_NO, KC_NO, K13_4 } \
    }

// Each layer gets a name for readability, which is then used in the keymap matrix below.
// The underscores don't mean anything - you can have a layer called STUFF or any other name.
// Layer names don't all need to be of the same length, obviously, and you can also skip them
// entirely and just use numbers.
#define _BL 0
#define _FL 1

enum custom_keycodes {
    // Acts as KB_RGUI modifier when used with other keys, but if no other keys are pressed during the modifier's active state, sends an extra KB_ESC (add/del_key) on release. See layout_process_record for details.
    RGUI_ESC = SAFE_RANGE
};

const uint16_t keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /* Keymap _BL: (Base Layer) Default Layer
     * ,------------------------------------------------------------.
     * |Fn |  1|  2|  3|  4|  5|  6|  7|  8|  9|  0|  -|  =|     Gui|
     * |------------------------------------------------------------|
     * |Tab  |  Q|  W|  E|  R|  T|  Y|  U|  I|  O|  P|  [|  ]|Backsp|
     * |------------------------------------------------------------|
     * |Gui   |  A|  S|  D|  F|  G|  H|  J|  K|  L|  ;|  '|  Return|
     * |------------------------------------------------------------|
     * |Shift   |  Z|  X|  C|  V|  B|  N|  M|  ,|  .|  /|Shi| NO| NO|
     * |------------------------------------------------------------|
     * |App |Alt |Ctl |          Space          |Ctl|Alt |App|Fn| NO|
     * `------------------------------------------------------------'
     */
    [_BL] = LAYOUT_60(
                      MO(_FL), KC_1,    KC_2,    KC_3,   KC_4,    KC_5,    KC_6,    KC_7,     KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  RGUI_ESC,
                      KC_TAB,  KC_Q,    KC_W,    KC_E,   KC_R,    KC_T,    KC_Y,    KC_U,     KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSPC,
                      KC_LGUI, KC_A,    KC_S,    KC_D,   KC_F,    KC_G,    KC_H,    KC_J,     KC_K,    KC_L,    KC_SCLN, KC_QUOT,          KC_ENT,
                      KC_LSFT, KC_Z,    KC_X,    KC_C,   KC_V,    KC_B,    KC_N,    KC_M,     KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT, KC_NO,   KC_NO,
                      KC_APP,  KC_LALT, KC_LCTL,                           KC_SPC,                     KC_RCTL, KC_RALT, KC_APP,  MO(_FL), KC_NO
                      ),

    /* Keymap _FL: (Base Layer) Function Layer
     * ,------------------------------------------------------------.
     * |   | F1| F2| F3| F4| F5| F6| F7| F8| F9|F10|F11|F12|    Esc |
     * |------------------------------------------------------------|
     * | \   |  `|   |   |   |   |  | Hm| Up|End| F17|  |   |       |
     * |------------------------------------------------------------|
     * |Caps   |F13|F14|F15|F16|   |PgU|Lft|Dwn|Rgt|F18|F19|        |
     * |------------------------------------------------------------|
     * |        |F21|   |   |   |   |PgD|   |Del|   |F20|   |   |   |
     * |------------------------------------------------------------|
     * |    |    |    |                         |   |   |   |   |   |
     * `------------------------------------------------------------'
     */
    [_FL] = LAYOUT_60(
                      _______,  KC_F1,   KC_F2,   KC_F3,   KC_F4,  KC_F5,   KC_F6,   KC_F7,     KC_F8,   KC_F9,   KC_F10,   KC_F11,  KC_F12, _______,
                      KC_BSLS, KC_GRV, _______, _______, _______, _______, _______,  KC_HOME,   KC_UP,   KC_END,  KC_F17,  _______, _______, _______,
                      KC_CAPS, KC_F13,  KC_F14,  KC_F15,  KC_F16, _______, KC_PGUP,  KC_LEFT, KC_DOWN, KC_RIGHT,  KC_F18,   KC_F19,          _______,
                      _______, KC_F21, _______,_______, _______, _______,  KC_PGDN,  KC_DEL,  KC_PAUSE,  _______,  KC_F20, _______, _______, _______,
                      _______, _______, _______,                           KC_ESC,                       _______, _______, _______, _______, _______
                      ),
};

// clang-format on

void key_action(uint16_t keycode)
{
    delay_ms(5);
    add_key(keycode);
    send_keyboard_report();

    delay_ms(5);
    del_key(keycode);
    send_keyboard_report();
}

bool layout_process_record(uint16_t keycode, bool key_pressed)
{
    static bool RGUI_ESC_SINGLE_ACTION = false;
    switch (keycode) {
        case RGUI_ESC:
            if (key_pressed) {
                add_mods((uint8_t)MOD_BIT(KC_RGUI));
                RGUI_ESC_SINGLE_ACTION = true;
                send_keyboard_report();
            } else {
                del_mods((uint8_t)MOD_BIT(KC_RGUI));
                send_keyboard_report();
                if (RGUI_ESC_SINGLE_ACTION) {
                    key_action(KC_ESC);
                    RGUI_ESC_SINGLE_ACTION = false;
                }
            }
            return false;
        default:
            RGUI_ESC_SINGLE_ACTION = false;
            return true;
    }
}
