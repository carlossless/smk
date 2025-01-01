#pragma once

#include "report.h"
#include "keyboard.h"

void rf_init();
void rf_send_report(__xdata report_keyboard_t *report);
void rf_send_nkro(__xdata report_nkro_t *report);
void rf_send_extra(__xdata report_extra_t *report);
void rf_update_keyboard_state(keyboard_state_t *keyboard);
