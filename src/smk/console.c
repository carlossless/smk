#include "console.h"

#if DEBUG == 1

#    include "report.h"
#    include "usb.h"
#    include "usbregs.h"
#    include <stdint.h>
#    include <stdarg.h>

static void console_emit_num(uint16_t val, uint8_t base, uint8_t width, char pad, uint8_t upper) __reentrant
{
    char    buf[5]; // 16-bit: 5 decimal digits / 4 hex digits max
    uint8_t n = 0;
    do {
        uint8_t d = (uint8_t)(val % base);
        buf[n++]  = (char)(d < 10 ? '0' + d : (upper ? 'A' : 'a') + d - 10);
        val /= base;
    } while (val);
    for (uint8_t w = n; w < width; w++) {
        debug_putc((char)pad);
    }
    while (n) {
        debug_putc((char)buf[--n]);
    }
}

void console_printf(const __code char *fmt, ...) __reentrant
{
    va_list ap;
    va_start(ap, fmt);

    for (char c = *fmt; c; c = *++fmt) {
        if (c != '%') {
            debug_putc((char)c);
            continue;
        }

        c             = *++fmt;
        char    pad   = ' ';
        uint8_t width = 0;
        if (c == '0') {
            pad = '0';
            c   = *++fmt;
        }
        if (c >= '1' && c <= '9') {
            width = (uint8_t)(c - '0');
            c     = *++fmt;
        }

        switch (c) {
            case 'c':
                debug_putc((char)va_arg(ap, int));
                break;
            case 's': {
                const char *s = va_arg(ap, const char *);
                while (*s)
                    debug_putc((char)*s++);
                break;
            }
            case 'd': {
                int v = va_arg(ap, int);
                if (v < 0) {
                    debug_putc('-');
                    v = -v;
                }
                console_emit_num((uint16_t)v, 10, width, pad, 0);
                break;
            }
            case 'u':
                console_emit_num((uint16_t)va_arg(ap, unsigned int), 10, width, pad, 0);
                break;
            case 'x':
                console_emit_num((uint16_t)va_arg(ap, unsigned int), 16, width, pad, 0);
                break;
            case 'X':
                console_emit_num((uint16_t)va_arg(ap, unsigned int), 16, width, pad, 1);
                break;
            case '%':
                debug_putc('%');
                break;
            case 0:
                va_end(ap);
                return;
            default:
                debug_putc('%');
                debug_putc((char)c);
                break;
        }
    }

    va_end(ap);
}

#    define CONSOLE_BUF_SIZE 128 // must stay a power of two
#    define CONSOLE_BUF_MASK (CONSOLE_BUF_SIZE - 1)

static __xdata unsigned char    console_buf[CONSOLE_BUF_SIZE];
static volatile __xdata uint8_t console_head; // producer (console_putc)
static volatile __xdata uint8_t console_tail; // consumer (console_task)

static __xdata uint8_t console_attached;

void console_notify_attached(void)
{
    console_attached = 1;
}

void console_putc(unsigned char c)
{
    uint8_t next;
    // __critical so a dprintf() reached from an interrupt can't tear the head
    // update against a main-loop writer. Drops the byte when the buffer is full.
    __critical
    {
        next = (console_head + 1) & CONSOLE_BUF_MASK;
        if (next != console_tail) {
            console_buf[console_head] = c;
            console_head              = next;
        }
    }
}

void console_task(void)
{
    uint8_t len;

    if (!usb_is_configured()) {
        console_attached = 0; // require a fresh handshake after re-enumeration
        return;               // not enumerated over USB (e.g. wireless / unplugged)
    }
    if (!console_attached) {
        return; // host tool hasn't announced itself yet; keep buffering
    }
    if (console_head == console_tail) {
        return; // nothing buffered
    }
    if (EP2CON & _IEP2RDY) {
        return; // previous EP2 report still pending; retry next iteration
    }

    EP2_IN_BUF[0] = REPORT_ID_CONSOLE;
    for (len = 0; len < CONSOLE_REPORT_SIZE && console_tail != console_head; len++) {
        EP2_IN_BUF[1 + len] = console_buf[console_tail];
        console_tail        = (console_tail + 1) & CONSOLE_BUF_MASK;
    }
    for (; len < CONSOLE_REPORT_SIZE; len++) {
        EP2_IN_BUF[1 + len] = 0; // zero-pad to the fixed HID report size
    }

    SET_EP2_CNT(1 + CONSOLE_REPORT_SIZE);
    SET_EP2_IN_RDY;
}

#endif // DEBUG
