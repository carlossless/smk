#pragma once

#include "sh68f90.h"
#include <stdint.h>
#include <stdbool.h>

// The USB block's register interface. src/smk/usb.c owns the descriptors and the
// protocol state machine and reaches the hardware only through this; everything the
// two parts disagree about - SFR paging, which status bits read back - lives here.
//
// Every entry point below is called either from the main loop or from the USB
// interrupt; the ones marked ISR run with the handler's banking already established,
// so they must not switch it themselves.

void usb_hw_init(void);
void usb_hw_deinit(void);

void usb_hw_ep1_in_send(uint8_t *src, uint8_t len);
void usb_hw_ep2_in_send(uint8_t *src, uint8_t len);

void usb_hw_ep1_in_complete(void); // ISR
void usb_hw_ep2_in_complete(void); // ISR

#if DEBUG == 1
bool usb_hw_ep2_in_free(void);
void usb_hw_console_send(const __xdata uint8_t *data, uint8_t len);
#endif
