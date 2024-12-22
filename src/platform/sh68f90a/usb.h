#pragma once

#include "sh68f90a.h"
#include "report.h"
#include <stdint.h>

enum usb_report_id {
    REPORT_ID_SYSTEM   = 1,
    REPORT_ID_CONSUMER = 2,
    REPORT_ID_ISP      = 5,
    REPORT_ID_NKRO     = 6,
};

void usb_init();
void usb_send_report(report_keyboard_t *report);
void usb_send_extra(report_extra_t *report);

void usb_interrupt_handler() __interrupt(_INT_USB);
