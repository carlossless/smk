#pragma once

#include "report.h"
#include <stdint.h>
#include <stdbool.h>

enum {
    USB_PROTOCOL_BOOT   = 0,
    USB_PROTOCOL_REPORT = 1,
};

void usb_init(void);
void usb_deinit(void);

void usb_send_report(__xdata report_keyboard_t *report);
void usb_send_nkro(__xdata report_nkro_t *report);
void usb_send_extra(__xdata report_extra_t *report);

bool    usb_is_configured(void);
uint8_t usb_device_state_get_protocol(void);

void usb_wait_for_enumeration(void);

// The part of the USB interrupt that is the same on every part. Each platform's
// interrupt vector calls it inside its own banking prologue, so it must run with the
// USB block already reachable.
void usb_irq_dispatch(void);

extern __bit usb_suspended;

#if DEBUG == 1
bool usb_console_ready(void);
void usb_console_send(const __xdata uint8_t *data, uint8_t len);
#endif
