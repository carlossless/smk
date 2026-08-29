#include "power.h"
#include "interrupts.h"
#include "watchdog.h"
#include "delay.h"
#include "usb.h"
#include "clock.h"
#include "extint.h"

// Datasheet 8.9.3 / 8.1: in Power-Down the HF oscillator stops and only
// INT2/3/4, LPD, a USB bus event, or reset wakes the core. The regulator must be
// off before entering Power-Down when it isn't needed, and takes 500 us to put
// out a stable 3.3 V once switched back on — hence the wake delay below.
//
// This file owns the generic MCU teardown and wake rebuild only. Parking the
// GPIO and arming the INT4 wake (the keypress pin P4.1 and the BK3632 ACK line
// P4.2) belongs to the board hooks around the call.

// Set when an INT4 edge woke the core, cleared before each Power-Down. Remote
// wakeup is only signalled for an INT4 wake, never when the host resumed the bus
// itself.
static volatile uint8_t int4_woke;

static void usb_park(powerdown_mode_t mode)
{
    if (mode == POWERDOWN_KEEP_USB_ALIVE) {
        USBCON |= _GOSUSP;
        return;
    }

    // The regulator and PHY are about to go, so tear the stack down. Being
    // un-configured is what makes usb_send_* drop reports until the host has
    // re-enumerated us on wake.
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

// Halts the core. Everything after the PCON write runs on the far side of the
// wake interrupt.
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

    // A keypress woke us, so raise remote-wakeup whether or not the host armed
    // it — otherwise that keystroke is lost to a bus that never resumes.
    if (int4_woke) {
        USBCON |= _WKUP;
        int4_woke = 0;
    }

    if (mode == POWERDOWN_KEEP_USB_ALIVE) {
        // Still enumerated, so re-arming the interrupts is the whole of it: the
        // waking keypress is reported the moment the bus resumes.
        USBIE1 = 0x5F;
        IEN1 |= _EUSB;
        // Drop the flag so the main loop doesn't re-sleep before the first SOF.
        // A host that is still suspended re-arms it on the next SUSPIF (~3 ms).
        usb_suspended = 0;
        return;
    }

    // The PHY lost power, so the host will re-enumerate us from address 0.
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

    // clock_init() is not usable here: its PLLSTA poll can return before a
    // cold-started oscillator is stable enough for USB-grade timing.
    clock_wake_restart();
    usb_resume(mode);
}

void int4_interrupt_handler(void) __interrupt(_INT_INT4)
{
    // The waking P4.x edge latched a flag; clear it so the ISR doesn't re-fire.
    extint_wake_clear();
    int4_woke = 1;
}
