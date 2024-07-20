#include "host.h"
#include "debug.h"
#include "usb.h"

#if RF_ENABLED == 1
#    include "rf_controller.h"
#endif

/* send report */
void host_keyboard_send(report_keyboard_t *report)
{
    usb_send_report(report);
#if RF_ENABLED == 1
    rf_send_report(report);
#endif
}
