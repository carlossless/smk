#include "power.h"
#include "watchdog.h"
#include "delay.h"
#include "usb.h"
#include "clock.h"

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
    USBIF1 &= 0x7A;

    if (usb_keep_alive) {
        IEN1 = _EUSB;
    } else {
        IEN1 = 0;
        REGCON &= ~_REGEN;
    }

    CLR_WDT();

    int4_woke = 0;

    // Datasheet 8.9.3 enter sequence: SUSLO = 0x55 then set PCON.PD on the very
    // next instruction, NOPs padding after. EA is 1 and INT4 is armed, so a
    // keypress (P4.1) or BK3632 event (P4.2) — or a USB resume, in the USB
    // variant — wakes the core here.
    // clang-format off
    __asm
        nop
    __endasm;
    // clang-format on
    SUSLO = 0x55;
    PCON |= 0x02;
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
        USBIE1 = 0x5F;
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
