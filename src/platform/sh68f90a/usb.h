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

#if DEBUG == 1
bool usb_is_configured();
#endif // DEBUG

// __using would reserve a register bank from DSEG, which we can't afford
// right now — DSEG is already at max usage. Falls back to default (bank 0
// with push/pop of R0-R7 on entry/exit). If we ever free 8 bytes of DSEG,
// add __using(2) here (must differ from PWM/Timer 2 which use bank 1).
void usb_interrupt_handler() __interrupt(_INT_USB);
