#include "report.h"
#include "usb.h"

void kb_send_report(report_keyboard_t *report)
{
    usb_send_report(report);
}

void kb_send_nkro(report_nkro_t *report)
{
    usb_send_nkro(report);
}

void kb_send_extra(report_extra_t *report)
{
    usb_send_extra(report);
}
