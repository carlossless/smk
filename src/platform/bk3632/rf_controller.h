#pragma once

#include "report.h"
#include "keyboard.h"

void rf_init();
void rf_send_report(report_keyboard_t *report);
void rf_update_keyboard_state(keyboard_state_t *keyboard);
