#pragma once

#include <stdint.h>

void console_putc(unsigned char c);
void console_task(void);

void console_notify_attached(void);

void console_printf(const __code char *fmt, ...) __reentrant;
