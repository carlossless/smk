#include "clock.h"
#include "ldo.h"
#include "peripherals.h"
#include "usb.h"
#include "usbhw.h"
#include "watchdog.h"
#include <stdint.h>

void usb_interrupt_handler(void) __interrupt(USB_VECTOR);

#define EP0_PACKET 8u
#define EP1_PACKET 16u
#define EP2_PACKET 64u

#define REQ_GET_DESCRIPTOR    0x06u
#define REQ_SET_ADDRESS       0x05u
#define REQ_SET_CONFIGURATION 0x09u

#define DESC_DEVICE 0x01u
#define DESC_CONFIG 0x02u

// clang-format off
static const __code uint8_t device_desc[] = {
    18, DESC_DEVICE,
    0x00, 0x02,
    0xFF, 0x00, 0x00,
    EP0_PACKET,
    0x8A, 0x25,
    0xAD, 0xDE,
    0x00, 0x01,
    0, 0, 0,
    1,
};

static const __code uint8_t config_desc[] = {
    9, DESC_CONFIG,
    32, 0,
    1, 1,
    0,
    0x80,
    50,

    9, 0x04,
    0, 2,
    0xFF, 0x00, 0x00,
    0,

    7, 0x05,
    0x81, 0x03,
    EP1_PACKET, 0,
    10,

    7, 0x05,
    0x82, 0x03,
    EP2_PACKET, 0,
    10,
};
// clang-format on

static uint8_t configured;
static uint8_t ep1_report[4];
static uint8_t ep2_report[8];

static const __code uint8_t *ep0_in_src;
static uint8_t               ep0_in_left;
static uint8_t               pending_address;

static void ep0_in_step(void)
{
    uint8_t n = ep0_in_left > EP0_PACKET ? EP0_PACKET : ep0_in_left;

    for (uint8_t i = 0; i < n; i++) {
        EP0_IN_BUF[i] = ep0_in_src[i];
    }

    ep0_in_src += n;
    ep0_in_left -= n;

    SET_EP0_CNT(n);
}

static void ep0_in_start(const __code uint8_t *src, uint8_t len, uint8_t requested)
{
    ep0_in_src  = src;
    ep0_in_left = requested < len ? requested : len;

    ep0_in_step();
    SET_EP0_IN_RDY;
}

static void ep0_status_in(void)
{
    ep0_in_left = 0;
    CLEAR_EP0_CNT;
    SET_EP0_IN_RDY;
}

static void ep0_setup(void)
{
    uint8_t request   = EP0_OUT_BUF[1];
    uint8_t desc_type = EP0_OUT_BUF[3];
    uint8_t requested = EP0_OUT_BUF[6];

    switch (request) {
        case REQ_GET_DESCRIPTOR:
            if (desc_type == DESC_DEVICE) {
                ep0_in_start(device_desc, sizeof(device_desc), requested);
            } else if (desc_type == DESC_CONFIG) {
                ep0_in_start(config_desc, sizeof(config_desc), requested);
            } else {
                STALL_EP0();
            }
            break;

        case REQ_SET_ADDRESS:
            pending_address = EP0_OUT_BUF[2];
            ep0_status_in();
            break;

        case REQ_SET_CONFIGURATION:
            configured = EP0_OUT_BUF[2];
            ep0_status_in();
            break;

        default:
            STALL_EP0();
            break;
    }
}

static void ep0_in_complete(void)
{
    if (ep0_in_left != 0) {
        ep0_in_step();
        SET_EP0_IN_RDY;
        SET_EP0_OUT_RDY;
        return;
    }

    if (pending_address != 0) {
        USBADDR         = pending_address;
        pending_address = 0;
    }

    SET_EP0_IN_STALL;
    SET_EP0_OUT_RDY;
}

void usb_irq_dispatch(void)
{
    uint8_t if1 = USBIF1;
    uint8_t if2 = USBIF2;

    if (if1 & _SOFIF) {
        USBIF1 &= ~_SOFIF;
    }

    if (if1 & _SETUPIF) {
        USBIF1 &= ~_SETUPIF;
        USBIF1 &= ~(_OVERIF | _OW);
        ep0_setup();
    }

    if (if1 & _USBRSTIF) {
        USBIF1 &= ~_USBRSTIF;

        configured = 0;
        usb_hw_deinit();
        usb_hw_init();
    }

    if (if1 & (_SUSPIF | _RESMIF | _PUPIF)) {
        USBIF1 &= ~(_SUSPIF | _RESMIF | _PUPIF);
    }

    if (if2 & _IEP1IF) {
        USBIF2 &= ~_IEP1IF;
        usb_hw_ep1_in_complete();
    }

    if (if2 & _IEP2IF) {
        USBIF2 &= ~_IEP2IF;
        usb_hw_ep2_in_complete();
    }

    if (if2 & _IEP0IF) {
        USBIF2 &= ~_IEP0IF;
        ep0_in_complete();
    }

    if (if2 & _OEP0IF) {
        USBIF2 &= ~_OEP0IF;
        CLEAR_EP0_CNT;
        SET_EP0_IN_RDY;
    }
}

void main(void)
{
    ldo_init();
    clock_init();
    peripherals_init();

    usb_hw_init();
    EA = 1;

    for (;;) {
        watchdog_kick();

        if (configured != 0) {
            ep1_report[0]++;
            usb_hw_ep1_in_send(ep1_report, sizeof(ep1_report));

            ep2_report[0]++;
            usb_hw_ep2_in_send(ep2_report, sizeof(ep2_report));
        }
    }
}
