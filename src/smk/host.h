#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "report.h"

void host_keyboard_send(__xdata report_keyboard_t *report);
void host_nkro_send(__xdata report_nkro_t *report);
void host_system_send(uint16_t usage);
void host_consumer_send(uint16_t usage);

// True when the active host link is actually carrying NKRO reports: the keymap
// asks for it and the host hasn't put us back into boot protocol (BIOS setup and
// some KVMs do, and boot protocol only understands the 6KRO report).
bool host_nkro_active(void);
