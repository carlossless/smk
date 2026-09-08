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

// runs what the USB interrupt defers to the main loop: currently the jump into the ISP bootloader.
void usb_task(void);

// the part of the USB interrupt that does not vary by part; the vector calls it inside its own banking prologue.
void usb_irq_dispatch(void);

extern __bit usb_suspended;

#if DEBUG == 1
bool usb_console_ready(void);
void usb_console_send(const __xdata uint8_t *data, uint8_t len);
#endif

/**
 * Indicate an error in response to a EP0 transfer.
 */
#define STALL_EP0()        \
    do {                   \
        SET_EP0_IN_STALL;  \
        SET_EP0_OUT_STALL; \
    } while (0)

#define SET_EP0_IN_RDY      \
    do {                    \
        EP0CON |= _IEP0RDY; \
    } while (0)
#define SET_EP0_IN_STALL    \
    do {                    \
        EP0CON |= _IEP0STL; \
    } while (0)
#define CANCEL_EP0_IN_STALL  \
    do {                     \
        EP0CON &= ~_IEP0STL; \
    } while (0)

#define SET_EP0_OUT_RDY     \
    do {                    \
        EP0CON |= _OEP0RDY; \
    } while (0)
#define SET_EP0_OUT_STALL   \
    do {                    \
        EP0CON |= _OEP0STL; \
    } while (0)
#define CANCEL_EP0_OUT_STALL \
    do {                     \
        EP0CON &= ~_OEP0STL; \
    } while (0)

#define SET_EP1_IN_RDY      \
    do {                    \
        EP1CON |= _IEP1RDY; \
    } while (0)
#define SET_EP1_IN_STALL    \
    do {                    \
        EP1CON |= _IEP1STL; \
    } while (0)
#define CANCEL_EP1_IN_STALL  \
    do {                     \
        EP1CON &= ~_IEP1STL; \
    } while (0)

#define SET_EP1_OUT_RDY     \
    do {                    \
        EP1CON |= _OEP1RDY; \
    } while (0)
#define SET_EP1_OUT_STALL   \
    do {                    \
        EP1CON |= _OEP1STL; \
    } while (0)
#define CANCEL_EP1_OUT_STALL \
    do {                     \
        EP1CON &= ~_OEP1STL; \
    } while (0)

#define SET_EP2_OUT_RDY     \
    do {                    \
        EP2CON |= _OEP2RDY; \
    } while (0)
#define SET_EP2_OUT_STALL   \
    do {                    \
        EP2CON |= _OEP2STL; \
    } while (0)
#define CANCEL_EP2_OUT_STALL \
    do {                     \
        EP2CON &= ~_OEP2STL; \
    } while (0)

#define SET_EP2_IN_RDY      \
    do {                    \
        EP2CON |= _IEP2RDY; \
    } while (0)
#define SET_EP2_IN_STALL    \
    do {                    \
        EP2CON |= _IEP2STL; \
    } while (0)
#define CANCEL_EP2_IN_STALL  \
    do {                     \
        EP2CON &= ~_IEP2STL; \
    } while (0)

#define CLEAR_EP0_CNT     \
    do {                  \
        IEP0CNT &= ~0x0f; \
    } while (0)
#define SET_EP0_CNT(COUNT) \
    do {                   \
        CLEAR_EP0_CNT;     \
        IEP0CNT |= COUNT;  \
    } while (0)

#define CLEAR_EP1_CNT     \
    do {                  \
        IEP1CNT &= ~0x1f; \
    } while (0)
#define SET_EP1_CNT(COUNT) \
    do {                   \
        CLEAR_EP1_CNT;     \
        IEP1CNT |= COUNT;  \
    } while (0)

#define CLEAR_EP2_CNT     \
    do {                  \
        IEP2CNT &= ~0x7f; \
    } while (0)
#define SET_EP2_CNT(COUNT) \
    do {                   \
        CLEAR_EP2_CNT;     \
        IEP2CNT |= COUNT;  \
    } while (0)
