#include "power.h"
#include "interrupts.h"
#include "watchdog.h"
#include "delay.h"
#include "usb.h"
#include "clock.h"

#define SUSLO_POWERDOWN_KEY 0x55
#define REGULATOR_SETTLE_US 500
#define USBIE1_RESUME_ARM   (uint8_t)(_OVERIE | _SETUPIE | _SOFIE | _RESMIE | _SUSPIE | _PBRSTIE)

// The USB block answers on SFR page 1, where the clock and regulator registers do not.
#define USB_PAGE_ENTER()             \
    uint8_t usb_saved_page = INSCON; \
    sfr_page_1()
#define USB_PAGE_LEAVE() INSCON = usb_saved_page

static void usb_park(powerdown_mode_t mode)
{
    if (mode == POWERDOWN_KEEP_USB_ALIVE) {
        USB_PAGE_ENTER();
        USBCON |= _GOSUSP;
        USB_PAGE_LEAVE();
        return;
    }
    usb_deinit();
}

static void clock_tree_stop(void)
{
    CLKCON &= ~_FS;
    PLLCON &= ~_PLLFS;
    PLLCON &= ~_PLLON;
    CLKCON &= ~_OSC2ON;
}

static void halt_until_wake(void)
{
    // The key write and PCON.PD must be consecutive instructions; nothing may come between
    // them, and the datasheet wants three NOPs after.
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
}

static void usb_resume(powerdown_mode_t mode)
{
    if (mode == POWERDOWN_KEEP_USB_ALIVE) {
        USB_PAGE_ENTER();
        USBIF1 &= ~_SUSPIF;
        USBCON &= ~_GOSUSP;
        USBIE1 = USBIE1_RESUME_ARM;
        USB_PAGE_LEAVE();

        IEN1 |= _EUSB;
        usb_suspended = 0;
        return;
    }

    REGCON |= _REGEN;
    delay_us(REGULATOR_SETTLE_US);
    usb_init();
}

// The only documented ways out of power-down here are an external interrupt, Timer3, the base
// timer and reset. USB is not one of them, unlike on the SH68F89, so a board that enters this
// without arming a wake pin of its own in user_sleep_prepare() will not come back until it is
// unplugged. Keeping USB alive across the sleep does not change that.
void power_enter_powerdown(powerdown_mode_t mode)
{
    usb_park(mode);
    clock_tree_stop();

    if (mode != POWERDOWN_KEEP_USB_ALIVE) {
        REGCON &= ~_REGEN;
    }

    watchdog_kick();

    halt_until_wake();

    // the oscillator comes back cold, so bring the tree up the way a reset would.
    clock_init();
    usb_resume(mode);
}
