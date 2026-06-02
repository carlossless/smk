#pragma once

#include <stdint.h>

// Debug console over USB HID. Bytes written with console_putc() are queued in a
// ring buffer and drained to the host as REPORT_ID_CONSOLE HID input reports by
// console_task(), which must be called periodically from the main loop. Only
// active in DEBUG builds; see putchar() in uart.c for the stdio hookup.

void console_putc(unsigned char c);
void console_task(void);

// Minimal printf-bypass helpers. dprintf goes through SDCC's printf_large
// which keeps a ~24 B PARM state in DSEG. For hot-path diagnostics we want
// to avoid that — call these directly.
void dputc(unsigned char c) __reentrant;
void dprint_hex(uint8_t v) __reentrant;
void dprint_str(const __code char *s) __reentrant;
void dprint_nl(void) __reentrant;
