#include "usbhw.h"
#include "usb.h"
#include "interrupts.h"
#include "usbregs.h"
#include "watchdog.h"
#include "report.h"
#include "delay.h"
#include <stdint.h>

// The IEPxRDY bits do not read back the endpoint's busy state on this part, so spinning
// on them burns the whole drain timeout on every send and starves the watchdog. Track the
// in-flight packet in software instead and let the completion interrupt clear it. EP2
// matters most: the console and the NKRO/consumer reports share it.
static __bit ep1_in_busy;
static __bit ep2_in_busy;

#define EP_IN_DRAIN_TRIES 40

static void set_ep1_in_buffer(uint8_t *src, uint8_t len)
{
    if (len > EP1_BUF_SIZE) {
        return;
    }
    for (uint8_t i = 0; i < len; i++) {
        EP1_IN_BUF[i] = src[i];
    }
}

static void set_ep2_in_buffer(uint8_t *src, uint8_t len)
{
    if (len > EP2_BUF_SIZE) {
        return;
    }
    for (uint8_t i = 0; i < len; i++) {
        EP2_IN_BUF[i] = src[i];
    }
}

// The drain stays on page 0: it kicks the watchdog, and RSTSTAT is page 0.
static void ep1_in_drain(void)
{
    uint8_t tries = 0;
    while (tries < EP_IN_DRAIN_TRIES && ep1_in_busy) {
        watchdog_kick();
        delay_us(40);
        tries++;
    }
    ep1_in_busy = 1;
}

static void ep2_in_drain(void)
{
    uint8_t tries = 0;
    while (tries < EP_IN_DRAIN_TRIES && ep2_in_busy) {
        watchdog_kick();
        delay_us(40);
        tries++;
    }
    ep2_in_busy = 1;
}

void usb_hw_init(void)
{
    ep1_in_busy = 0;
    ep2_in_busy = 0;

    // Callable from main (page 0) and from the handler (page 1), so restore either way.
    uint8_t saved_inscon = INSCON;
    sfr_page_1();

    USBADDR = 0;
    // The IN-ready bits come up set on this part, so usb_console_ready() would never
    // see the endpoint free until something forced a transfer.
    EP1CON = 0;
    EP2CON = 0;
    USBIE1 = (_OVERIE | _SETUPIE | _SOFIA | _RESMIE | _SUSPIE | _PBRSTIE);
    USBIE2 = (_OEP0IE | _IEP0IE | _IEP1IE | _IEP2IE);
    USBCON = (_ENUSB | _SW1CON);

    INSCON = saved_inscon;
    IEN1 |= _EUSB;
}

void usb_hw_deinit(void)
{
    USBADDR = 0;
    USBCON &= ~(_ENUSB | _SW1CON | _SW2CON); // drop the module-enable bits
    IEN1 &= ~_EUSB;                          // disable the USB interrupt
}

// The count and control registers live on SFR page 1, so every arming sequence has to
// borrow it.
void usb_hw_ep1_in_send(uint8_t *src, uint8_t len)
{
    ep1_in_drain();

    uint8_t saved_inscon = INSCON;
    sfr_page_1();

    set_ep1_in_buffer(src, len);
    SET_EP1_CNT(len);
    SET_EP1_IN_RDY;

    INSCON = saved_inscon;
}

void usb_hw_ep2_in_send(uint8_t *src, uint8_t len)
{
    ep2_in_drain();

    uint8_t saved_inscon = INSCON;
    sfr_page_1();

    set_ep2_in_buffer(src, len);
    SET_EP2_CNT(len);
    SET_EP2_IN_RDY;

    INSCON = saved_inscon;
}

void usb_hw_ep1_in_complete(void)
{
    ep1_in_busy = 0;
}

void usb_hw_ep2_in_complete(void)
{
    ep2_in_busy = 0;
}

#if DEBUG == 1
bool usb_hw_ep2_in_free(void)
{
    return !ep2_in_busy;
}

void usb_hw_console_send(const __xdata uint8_t *data, uint8_t len)
{
    uint8_t saved_inscon = INSCON;
    sfr_page_1();

    EP2_IN_BUF[0] = REPORT_ID_CONSOLE;
    for (uint8_t i = 0; i < CONSOLE_REPORT_SIZE; i++) {
        EP2_IN_BUF[1 + i] = (i < len) ? data[i] : 0;
    }

    SET_EP2_CNT(1 + CONSOLE_REPORT_SIZE);
    SET_EP2_IN_RDY;

    INSCON = saved_inscon;
}
#endif

void usb_interrupt_handler(void) __interrupt(_INT_USB)
{
    // The USB block is on SFR page 1 and the rest of the firmware runs on page 0, where
    // RSTSTAT is the watchdog kick. Borrow page 1 only for the length of the handler.
    uint8_t saved_inscon = INSCON;
    sfr_page_1();

    usb_irq_dispatch();

    INSCON = saved_inscon;
}
