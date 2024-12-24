#pragma once

#include <stdint.h>
#include "report.h"

void host_keyboard_send(report_keyboard_t *report);
void host_nkro_send(report_nkro_t *report);
void host_system_send(uint16_t usage);
void host_consumer_send(uint16_t usage);
