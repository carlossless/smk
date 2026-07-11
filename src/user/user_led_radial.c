#include "user_led.h"

// Default no-op geometry for boards without a backlight.
uint8_t user_led_radial(uint8_t row, uint8_t col) __reentrant
{
    (void)row;
    (void)col;
    return 0;
}
