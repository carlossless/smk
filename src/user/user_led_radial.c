#include "user_led.h"

// Default no-op LED geometry for boards without a backlight; the effect engine is
// linked unconditionally, so these must resolve.
uint8_t user_led_radial(uint8_t row, uint8_t col)
{
    (void)row;
    (void)col;
    return 0;
}
