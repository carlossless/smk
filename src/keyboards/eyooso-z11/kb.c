#include "report.h"
#include "usb.h"

void kb_send_report(report_keyboard_t *report)
{
    usb_send_report(report);
}
