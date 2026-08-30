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
void rf_send_pending_flush(void);
void rf_blanking_tick(void);
void rf_kbd_lazy_state_init(void);

bool rf_update_keyboard_state(keyboard_state_t *keyboard);
void rf_link_supervisor(keyboard_state_t *keyboard);
void rf_set_link(rf_mode_t link);
void rf_set_link_pairing(rf_mode_t link, __xdata keyboard_state_t *keyboard);
void rf_reassert_link(rf_mode_t link);
void rf_apply_usb_mode(void);
void rf_factory_reset_bonds(void);

void rf_set_mac_mode_compat(bool is_mac);
void rf_byte9_set_disable(bool on);

void rf_sleep(uint8_t param);
void rf_wake(void);
void rf_wake_nudge(void);
