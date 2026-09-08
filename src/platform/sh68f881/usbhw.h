#pragma once

#include "sh68f881.h"
#include <stdint.h>
#include <stdbool.h>

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
