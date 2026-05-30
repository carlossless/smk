#pragma once

// Debug console over USB HID. Bytes written with console_putc() are queued in a
// ring buffer and drained to the host as REPORT_ID_CONSOLE HID input reports by
// console_task(), which must be called periodically from the main loop. Only
// active in DEBUG builds; see putchar() in uart.c for the stdio hookup.

void console_putc(unsigned char c);
void console_task(void);
