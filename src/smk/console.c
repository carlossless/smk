#include "console.h"

#if DEBUG == 1

#    include "report.h"
#    include "usb.h"     // usb_is_configured(), EP2 buffer/SFRs via sh68f90a.h
#    include "usbregs.h" // SET_EP2_CNT, SET_EP2_IN_RDY
#    include <stdint.h>

#    define CONSOLE_BUF_SIZE 128 // must stay a power of two
#    define CONSOLE_BUF_MASK (CONSOLE_BUF_SIZE - 1)

static __xdata unsigned char    console_buf[CONSOLE_BUF_SIZE];
static volatile __xdata uint8_t console_head; // producer (console_putc)
static volatile __xdata uint8_t console_tail; // consumer (console_task)

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

// Drains buffered bytes into the EP2 IN buffer as a REPORT_ID_CONSOLE HID report.
// Writes the hardware endpoint buffer directly (rather than via a helper taking
// a pointer/length) to avoid spending the SH68F90A's last bytes of internal RAM
// on parameter passing.
void console_task(void)
{
    uint8_t len;

    if (!usb_is_configured()) {
        return; // not enumerated over USB (e.g. wireless mode / unplugged)
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
