#pragma once

#include "report.h"
#include <stdint.h>
#include <stdbool.h>

enum {
    USB_PROTOCOL_BOOT   = 0,
    USB_PROTOCOL_REPORT = 1,
};

void usb_init(void);
// Reset the device-state machine and disable the module. The un-configured state
// is what makes usb_send_* drop reports while the PHY is powered down.
void usb_deinit(void);

void usb_send_report(__xdata report_keyboard_t *report);
void usb_send_nkro(__xdata report_nkro_t *report);
void usb_send_extra(__xdata report_extra_t *report);

bool    usb_is_configured(void);
uint8_t usb_device_state_get_protocol(void);

// Block until the host has enumerated us and the bus has gone quiet again, or
// until it's clear no host is attached. Bounded either way. Callers use it to
// hold work that competes with enumeration (the LED scan) until it's over.
// Busy-waits with interrupts on, so the scan and USB ISRs keep running.
void usb_wait_for_enumeration(void);

// True while the host has the bus suspended (set on SUSPIF when configured,
// cleared on the next SOF / bus reset). The USB-mode sleep trigger.
extern __bit usb_suspended;

#if DEBUG == 1
// Debug console pipe. Ready means the previous report has been collected, so a
// caller can commit to draining its buffer before handing bytes over. Anything
// short of CONSOLE_REPORT_SIZE is zero-padded.
bool usb_console_ready(void);
void usb_console_send(const __xdata uint8_t *data, uint8_t len);
#endif
