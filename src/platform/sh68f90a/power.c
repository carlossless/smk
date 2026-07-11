#include "power.h"
#include "watchdog.h"
#include "delay.h"
#include "usb.h"
#include "clock.h"

// Power-Down entry/exit, two variants selected by `usb_keep_alive`:
//
//  - RF / battery (false): power the USB regulator + PHY all the way down for
//    battery saving. Wakes on INT4 only; on wake the regulator returns and the
//    USB stack is re-initialised, so the host re-enumerates.
//
//  - USB-suspend (true): the host parked the bus, so keep the regulator ON and
//    just suspend the USB block (GOSUSP). Wakes on INT4 or a USB bus event and
//    resumes via remote-wakeup (WKUP) WITHOUT re-enumerating, so the waking keys
//    aren't dropped to a slow re-enumeration.
//
// The board hook user_sleep_prepare() parks GPIO and arms the INT4 wake (both
// the keypress pin P4.1 and the BK3632 ACK line P4.2); user_sleep_wake() does
// the pin restore. This routine owns the generic MCU teardown and wake rebuild.
//
// Per datasheet 8.9.3/8.1: in Power-Down the HF oscillator stops and only
// INT2/3/4, LPD, a USB bus event, or reset wakes the core; the regulator must be
// turned off before entering Power-Down when unused and outputs a stable 3.3V
// only 500us after being switched back on (hence the wake delay in the RF path).

// Set by int4_isr when a keypress (P4.1) / BK3632 (P4.2) edge wakes the core,
// cleared before each Power-Down. WKUP is signalled only when an INT4 keypress
// woke us, not when the host resumed the bus itself.
static volatile __xdata uint8_t int4_woke;

void power_enter_powerdown(bool usb_keep_alive)
{
    if (usb_keep_alive) {
        // Suspend the USB block; the regulator and PHY stay powered so the
        // device stays enumerated across the sleep.
        USBCON |= _GOSUSP;
    } else {
        // RF: the regulator/PHY are about to power down, so de-init USB (marks us
        // un-configured + disables the module). The configured-gate in
        // usb_send_* then drops any send until we re-enumerate on wake.
        usb_deinit();
    }

    // Kill the high-frequency clock tree.
    CLKCON &= ~_FS;
    PLLCON &= ~_PLLFS;
    PLLCON &= ~_PLLON;
    CLKCON &= ~_HFON;

    // Arm the USB bus-event wake bits + clear stale flags.
    USBIE1 |= (_BOOTS | _RESMIE | _PBRSTIE);
    USBIF1 &= 0x7A;

    if (usb_keep_alive) {
        // Keep the USB interrupt on so a host resume also wakes the core.
        IEN1 = _EUSB;
    } else {
        // RF: INT4 is the only wake. Regulator OFF (battery saving + a PD
        // precondition).
        IEN1 = 0;
        REGCON &= ~_REGEN;
    }

    CLR_WDT();

    int4_woke = 0; // only a fresh INT4 wake should arm the remote-wakeup resume

    // Datasheet 8.9.3 enter sequence: SUSLO = 0x55, then PCON.PD on the very next
    // instruction. EA is 1 and INT4 armed, so a keypress (P4.1), BK3632 event
    // (P4.2), or (USB variant) a USB resume wakes the core here.
    // clang-format off
    __asm
        nop
    __endasm;
    // clang-format on
    SUSLO = 0x55;
    PCON |= 0x02; // PD — core halts here until wake
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

    // ===== execution resumes here after the wake ISR returns =====

    // Rebuild the clock tree via clock_wake_restart() (not clock_init) — the
    // datasheet (8.9.3) requires re-opening the PLL after a Power-Down wake, and
    // clock_init's PLLSTA poll is unsafe on a cold-started oscillator (see
    // clock.c).
    clock_wake_restart();

    // Clear the USB suspend state; if an INT4 keypress woke us, raise remote-
    // wakeup regardless of whether the host armed it, so the keystroke isn't lost.
    USBIF1 &= ~_SUSPIF;
    USBCON &= ~_GOSUSP;
    if (int4_woke) {
        USBCON |= _WKUP;
        int4_woke = 0;
    }

    if (usb_keep_alive) {
        // PHY stayed powered, so the device is still enumerated — just re-arm
        // the USB interrupts and resume. No re-enumerate, so the waking keypress
        // is reported the moment the bus resumes.
        USBIE1 = 0x5F;
        IEN1 |= _EUSB;
        // Drop the suspended flag so the main loop doesn't immediately re-sleep
        // before the first SOF; a still-suspended host re-arms it on the next
        // SUSPIF (~3 ms).
        usb_suspended = 0;
    } else {
        // RF wake: regulator back on, let it restabilise (~500us), then re-init
        // USB — the regulator-off sleep dropped the PHY, so the host
        // re-enumerates from address 0 and usb_init() resets USBADDR.
        REGCON |= _REGEN;
        delay_us(500);
        usb_init();
    }
}

void int4_isr(void) __interrupt(_INT_INT4)
{
    // Wake-from-Power-Down vector. The waking P4.x edge latched a flag in EXF1;
    // clear them all so the ISR doesn't re-fire, and flag the INT4 wake so
    // power_enter_powerdown() raises remote-wakeup.
    EXF1      = 0;
    int4_woke = 1;
}
