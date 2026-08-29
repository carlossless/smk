#include "kbdef.h"
#include "layout.h"
#include "user_layout.h"
#include "report.h"
#include <stdint.h>

// clang-format off

#define LAYOUT_60( \
                   K00_0, K01_0 \
                 ) { \
    { K00_0, K01_0 } \
}

#define _BL 0

const uint16_t keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /* Keymap _BL: (Base Layer) Default Layer
     * ,-------.
     * |Esc|Ent|
     * `-------'
     */
    [_BL] = LAYOUT_60(
        KC_ESC, KC_ENT
    )
};
