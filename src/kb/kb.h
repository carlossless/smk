#pragma once

#include "report.h"
#include <stdint.h>
#include <stdbool.h>

void kb_init();
void kb_send_report(report_keyboard_t *report);
bool kb_process_record(uint16_t keycode, bool key_pressed);
void kb_update_switches();
void kb_update();
