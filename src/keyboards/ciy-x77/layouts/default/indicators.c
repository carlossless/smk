#include "indicators.h"
#include "sh68f89.h"
#include "kbdef.h"
#include "keyboard.h"

// HID keyboard output report bits, in the order the boot protocol defines them.
#define USB_LED_NUM_LOCK    (1u << 0)
#define USB_LED_CAPS_LOCK   (1u << 1)
#define USB_LED_SCROLL_LOCK (1u << 2)

// the per-key backlight is on the PCA units and is not brought up yet, so the only
// indicators here are the three lock LEDs, which sink through P3 and so light on a low.
void indicators_render(void)
{
    uint8_t state = keyboard_state.led_state;
    uint8_t low   = 0;

    if (state & USB_LED_NUM_LOCK) {
        low |= LED_NUM_P3_0;
    }
    if (state & USB_LED_CAPS_LOCK) {
        low |= LED_CAPS_P3_1;
    }
    if (state & USB_LED_SCROLL_LOCK) {
        low |= LED_SCROLL_P3_2;
    }

    uint8_t saved_page = INSCON;
    sfr_page_0();
    P3     = (uint8_t)((P3 | KB_LOCK_LED_MASK) & ~low);
    INSCON = saved_page;
}
