#pragma once

#include <stdint.h>

// Debug console over the host link. Bytes written with console_putc() are queued
// in a ring buffer and drained to the host by console_task(), which must be
// called periodically from the main loop. DEBUG builds only.
//
// Output is held in the ring buffer until the host announces it is listening
// (console_notify_attached), so nothing queued (the boot banner included) is
// lost to a race where the host opens its node after the link comes up.

void console_putc(unsigned char c);

// Debug output sink, selected by the debug_sink build option.
void debug_putc(char c);

void console_task(void);

void console_notify_attached(void);

void console_printf(const __code char *fmt, ...) __reentrant;
