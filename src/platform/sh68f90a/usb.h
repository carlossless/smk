#ifndef USB_H
#define USB_H

#include "sh68f90a.h"
#include "report.h"
#include <stdint.h>

void usb_init();
void usb_send_report(report_keyboard_t *report);

void usb_interrupt_handler() __interrupt(_INT_USB);

#endif
