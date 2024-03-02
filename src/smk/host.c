#include "host.h"
#include "debug.h"
#include "../platform/sh68f90a/usb.h"

/* send report */
void host_keyboard_send(report_keyboard_t *report)
{
    usb_send_report(report);
}
