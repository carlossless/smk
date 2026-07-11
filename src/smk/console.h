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
void console_task(void);

// The host tool announces it is attached and ready to read; console_task() then
// starts draining the buffered output.
void console_notify_attached(void);

// Minimal printf for debug output (prefer the dprintf() macro in debug.h).
// Supports %%, %c, %s, and %d/%u/%x/%X with an optional '0' flag + single width
// digit. 16-bit range.
void console_printf(const __code char *fmt, ...) __reentrant;
