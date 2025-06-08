#pragma once

#include "report.h"
#include "keyboard.h"

#include "../../utils/8051_inc_error_hide.h"

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
