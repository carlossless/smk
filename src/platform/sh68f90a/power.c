#include "power.h"
#include "watchdog.h"
#include "delay.h"
#include "usb.h"
#include "clock.h"

// SH68F90A Power-Down entry/exit, with two variants selected by `usb_keep_alive`:
//
//  - RF / battery (usb_keep_alive = false): power the USB voltage regulator (and
//    PHY) all the way down for maximum battery saving. Wakes on INT4 only (the
//    board hook arms it); on wake the regulator comes back and the USB stack is
//    re-initialised, so the host re-enumerates.
//
//  - USB-suspend (usb_keep_alive = true): the host has parked the bus, so keep the
//    regulator ON and just put the USB block into suspend (GOSUSP). Wakes on INT4
//    (keypress) or a USB bus event, and resumes the link via remote-wakeup (WKUP)
//    WITHOUT re-enumerating — so the keys that wake it aren't dropped to a slow
//    re-enumeration.
//
// The work is split: the board hook user_sleep_prepare() does the per-port GPIO
// park + arms the INT4 wake (covering both the keypress pin P4.1 and the BK3632
// ACK line P4.2), and user_sleep_wake() does the board-specific pin restore. This
// routine owns the generic MCU teardown (clock/PLL/USB/regulator + the
// SUSLO/PCON.PD dance) and the wake rebuild.
//
// Datasheet 8.9.3 + 8.1: in Power-Down the HF oscillator stops and only INT2/3/4,
// LPD, a USB bus event, or reset wakes the core; the regulator "must be turned
// off by software before entering Power-Down" (when unused) and, switched back
// on, "outputs a stable 3.3V after 500us" — hence the wake delay in the RF path.

// Set by int4_isr when a keypress (P4.1) / BK3632 (P4.2) edge wakes the core,
// cleared right before each Power-Down. The USB resume (WKUP) is signalled only
// when an INT4 keypress woke us — not when the host resumed the bus on its own.
// (xdata, not the internal-RAM-overlaid default.)
static volatile __xdata uint8_t int4_woke;

void power_enter_powerdown(bool usb_keep_alive)
{
    if (usb_keep_alive) {
        // USB-suspend variant: tell the USB block to suspend; the regulator and
        // PHY stay powered so the device stays enumerated across the sleep.
        USBCON |= _GOSUSP;
    } else {
        // RF variant: the regulator/PHY are about to be powered down, so fully
        // de-init the USB device state (marks us un-configured + disables the
        // module). This makes the configured-gate in usb_send_* drop any send
        // until we re-enumerate on wake (no stale EP writes / no ~10 ms IEP-ready
        // stall during re-enum).
        usb_deinit();
    }

    // Kill the high-frequency clock tree (FS, PLLFS, PLLON, HFON).
    CLKCON &= ~_FS;
    PLLCON &= ~_PLLFS;
    PLLCON &= ~_PLLON;
    CLKCON &= ~_HFON;

    // Arm the USB bus-event wake bits + clear stale flags.
    USBIE1 |= (_BOOTS | _RESMIE | _PBRSTIE);
    USBIF1 &= 0x7A;

    if (usb_keep_alive) {
        // Keep the USB interrupt enabled so a host resume also wakes the core.
        IEN1 = _EUSB;
    } else {
        // RF: INT4 is the only wake; clear IEN1 (usb_deinit already dropped the
        // USB module/EUSB) and power the regulator OFF (the big battery saving,
        // and a datasheet precondition for PD).
        IEN1 = 0;
        REGCON &= ~_REGEN;
    }

    CLR_WDT();

    int4_woke = 0; // only a fresh INT4 wake should arm the remote-wakeup resume

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
    PCON |= 0x02; // PD — core halts here until a wake fires
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

    // ===== execution resumes here after the wake interrupt's ISR returns =====

    // Rebuild the clock tree. NOT clock_init() (boot path): on a Power-Down wake
    // the HF oscillator was fully stopped and clock_init's PLLSTA poll can return
    // before the cold-started oscillator is stable enough for USB-grade timing,
    // intermittently selecting a marginal clock that stalls both the LED scan and
    // USB enumeration. clock_wake_restart() masks IRQs, starts HF+PLL, does a fixed
    // warm-up, then commits the PLL as SYSCLK in one shot — no PLLSTA poll. The
    // datasheet (8.9.3) requires re-opening the PLL after a Power-Down wake.
    clock_wake_restart();

    // Clear the USB suspend state and, if an INT4 keypress woke us, signal a USB
    // remote-wakeup resume to the host on any keypress wake, regardless of whether
    // the host armed remote-wakeup, so the waking keystroke isn't lost.
    USBIF1 &= ~_SUSPIF;
    USBCON &= ~_GOSUSP;
    if (int4_woke) {
        USBCON |= _WKUP;
        int4_woke = 0;
    }

    if (usb_keep_alive) {
        // USB-suspend wake: the PHY stayed powered, so the device is still
        // enumerated — just re-arm the USB interrupts and resume. No re-init /
        // re-enumerate, so the keypress that woke us is reported the moment the
        // bus resumes.
        USBIE1 = 0x5F;
        IEN1 |= _EUSB;
        // We're resuming the bus (WKUP above, or the host did it). Drop the
        // suspended flag so the main loop doesn't immediately re-sleep before the
        // first SOF arrives; a genuine still-suspended host re-arms it on the
        // next SUSPIF (~3 ms).
        usb_suspended = 0;
    } else {
        // RF wake: regulator back on, let it restabilise (~500us per datasheet),
        // then fully re-init the USB module — the regulator-off sleep dropped the
        // PHY, so the host re-enumerates from address 0 and USBADDR must be reset,
        // which usb_init() does.
        REGCON |= _REGEN;
        delay_us(500);
        usb_init();
    }
}

void int4_isr(void) __interrupt(_INT_INT4)
{
    // Wake-from-Power-Down vector. The keypress (P4.1) or BK3632 ACK edge (P4.2)
    // that woke us latched a flag in EXF1 (the P4.x INT4 sources); clear them all
    // so the ISR does not immediately re-fire, and flag that an INT4 keypress wake
    // happened so power_enter_powerdown() raises the USB remote-wakeup resume. The
    // rest of the wake bookkeeping happens once this returns.
    EXF1      = 0;
    int4_woke = 1;
}
