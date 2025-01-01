#pragma once

#include "sh68f90a.h"
#include "report.h"
#include <stdint.h>

enum {
    USB_PROTOCOL_BOOT   = 0,
    USB_PROTOCOL_REPORT = 1,
};

void    usb_init();
void    usb_send_report(__xdata report_keyboard_t *report);
void    usb_send_nkro(__xdata report_nkro_t *report);
void    usb_send_extra(__xdata report_extra_t *report);
uint8_t usb_device_state_get_protocol();

void usb_interrupt_handler() __interrupt(_INT_USB);
