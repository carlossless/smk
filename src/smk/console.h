#pragma once

#include <stdint.h>

// Debug console over USB HID. Bytes written with console_putc() are queued in a
// ring buffer and drained to the host as REPORT_ID_CONSOLE HID input reports by
// console_task(), which must be called periodically from the main loop. Only
// active in DEBUG builds; see putchar() in uart.c for the stdio hookup.
//
// Output is held in the ring buffer until the host announces it is listening
// (console_notify_attached(), driven by a SET_REPORT handshake from the host
// tool). This avoids the hidraw attach race: a one-shot drain on USB-configured
// would be consumed by the kernel before the host tool opens the node, losing
// the boot banner. Holding until the explicit handshake means whatever is
// queued (banner included) flushes the moment the tool actually attaches.

void console_putc(unsigned char c);
void console_task(void);

// The host tool announces it is attached and ready to read; console_task() then
// starts draining the buffered output. Called from the USB SET_REPORT handler.
void console_notify_attached(void);

// Minimal printf for debug output (prefer the dprintf() macro in debug.h). Tiny
// so it fits where SDCC's printf_large does not: bulk storage is console_buf
// (__xdata) and the formatter keeps only transient state on the stack. Supports
// %%, %c, %s, and %d/%u/%x/%X with an optional '0' flag + single width digit.
// 16-bit range. __reentrant, so ISR-safe.
void console_printf(const __code char *fmt, ...) __reentrant;
