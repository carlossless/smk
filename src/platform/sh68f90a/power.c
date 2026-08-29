#include "power.h"
#include "interrupts.h"
#include "watchdog.h"
#include "delay.h"
#include "usb.h"
#include "clock.h"

static volatile __xdata uint8_t int4_woke;

static void usb_park(powerdown_mode_t mode)
{
    if (mode == POWERDOWN_KEEP_USB_ALIVE) {
        USBCON |= _GOSUSP;
        return;
    }

    usb_deinit();
}

static void clock_tree_stop(void)
{
    CLKCON &= ~_FS;
    PLLCON &= ~_PLLFS;
    PLLCON &= ~_PLLON;
    CLKCON &= ~_HFON;
}

static void wake_sources_arm(powerdown_mode_t mode)
{
    USBIE1 |= (_BOOTS | _RESMIE | _PBRSTIE);
    USBIF1 &= 0x7A; // clear the stale bus-event flags, keep the rest

    if (mode == POWERDOWN_KEEP_USB_ALIVE) {
        IEN1 = _EUSB; // a host resume wakes us too
    } else {
        IEN1 = 0; // INT4 (armed by the board) is the only way back
        REGCON &= ~_REGEN;
    }
}

static void halt_until_wake(void)
{
    // Datasheet 8.9.3: SUSLO = 0x55 must be followed by PCON.PD on the very next
    // instruction, so nothing may come between these two writes.
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
}

static void usb_resume(powerdown_mode_t mode)
{
    USBIF1 &= ~_SUSPIF;
    USBCON &= ~_GOSUSP;

    if (int4_woke) {
        USBCON |= _WKUP;
        int4_woke = 0;
    }

    if (mode == POWERDOWN_KEEP_USB_ALIVE) {
        USBIE1 = 0x5F;
        IEN1 |= _EUSB;
        usb_suspended = 0;
        return;
    }

    REGCON |= _REGEN;
    delay_us(500); // regulator settle, per datasheet 8.1
    usb_init();
}

void power_enter_powerdown(powerdown_mode_t mode)
{
    usb_park(mode);
    clock_tree_stop();
    wake_sources_arm(mode);

    watchdog_kick();
    int4_woke = 0; // only a fresh wake should arm the remote-wakeup resume

    halt_until_wake();

    clock_wake_restart();
    usb_resume(mode);
}

void int4_interrupt_handler(void) __interrupt(_INT_INT4)
{
    EXF1      = 0;
    int4_woke = 1;
}
