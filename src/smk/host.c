#include "host.h"
#include "debug.h"
#include "usb.h"
#include "rf_controller.h"

/* send report */
void host_keyboard_send(report_keyboard_t *report)
{
    usb_send_report(report);
    rf_send_report(report);
}
