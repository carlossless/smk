#pragma once

#include <stdint.h>

void console_putc(unsigned char c);

// Debug output sink, selected by the debug_sink build option.
void debug_putc(char c);

void console_task(void);

void console_notify_attached(void);

void console_printf(const __code char *fmt, ...) __reentrant;
