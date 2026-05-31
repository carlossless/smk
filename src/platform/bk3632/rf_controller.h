#pragma once

#include "report.h"
#include "keyboard.h"

#include "utils/8051_inc_error_hide.h"

typedef enum {
    RF_MODE_2_4G = 0x00,
    RF_MODE_BT1  = 0x01,
    RF_MODE_BT2  = 0x02,
    RF_MODE_BT3  = 0x03
} rf_mode_t;

void rf_init();
void rf_send_report(__xdata report_keyboard_t *report);
void rf_send_nkro(__xdata report_nkro_t *report);
void rf_send_extra(__xdata report_extra_t *report);
void rf_update_keyboard_state(keyboard_state_t *keyboard);
void rf_set_link(rf_mode_t link);
// Switch to `link` and tell the BK3632 to start advertising / accepting new
// pairings on that slot. Triggered by a 3-second hold of the link key.
// After the pairing command, aggressively poll status so the BK3632's
// pairing-complete signal is consumed quickly and the chip doesn't sit in a
// half-finished state. `keyboard` is updated with whatever the poll observes.
void rf_set_link_pairing(rf_mode_t link, __xdata keyboard_state_t *keyboard);
// Called every main-loop iteration. Internally throttled; emits at most one
// "key release blanking" packet per ~RF_BLANKING_TICK_THROTTLE iterations and
// is a no-op when no blanking sequence is active.
void rf_blanking_tick(void);
