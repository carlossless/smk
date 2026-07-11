#include "power.h"
#include "watchdog.h"
#include "delay.h"
#include "usb.h"
#include "clock.h"

#define SUSLO_POWERDOWN_KEY     0x55
#define USBIF1_BUS_EVENTS_CLEAR (uint8_t)(_SUSPIF | _SOFIF | _SETUPIF | _OW | _OVERIF)
#define USBIE1_RESUME_ARM       (uint8_t)(_PBRSTIE | _SUSPIE | _RESMIE | _SOFIA | _SETUPIE | _OVERIE)

static volatile __xdata uint8_t int4_woke;

void power_enter_powerdown(bool usb_keep_alive)
{
    if (usb_keep_alive) {
        USBCON |= _GOSUSP;
    } else {
        usb_deinit();
    }

    CLKCON &= ~_FS;
    PLLCON &= ~_PLLFS;
    PLLCON &= ~_PLLON;
    CLKCON &= ~_HFON;

    USBIE1 |= (_BOOTS | _RESMIE | _PBRSTIE);
    USBIF1 &= USBIF1_BUS_EVENTS_CLEAR;

    if (usb_keep_alive) {
        IEN1 = _EUSB;
    } else {
        IEN1 = 0;
        REGCON &= ~_REGEN;
    }

    CLR_WDT();

    int4_woke = 0;

    // The key write and PCON.PD must be consecutive instructions; nothing may
    // come between them.
    // clang-format off
    __asm
        nop
    __endasm;
    // clang-format on
    SUSLO = SUSLO_POWERDOWN_KEY;
    PCON |= _PD;
    // clang-format off
    __asm
        nop
        nop
        nop
        nop
        nop
        nop
    __endasm;
    // clang-format on

    clock_wake_restart();

    USBIF1 &= ~_SUSPIF;
    USBCON &= ~_GOSUSP;
    if (int4_woke) {
        USBCON |= _WKUP;
        int4_woke = 0;
    }

    if (usb_keep_alive) {
        USBIE1 = USBIE1_RESUME_ARM;
        IEN1 |= _EUSB;
        usb_suspended = 0;
    } else {
        REGCON |= _REGEN;
        delay_us(500);
        usb_init();
    }
}

void int4_isr(void) __interrupt(_INT_INT4)
{
    EXF1      = 0;
    int4_woke = 1;
}
