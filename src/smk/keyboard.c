#include "keyboard.h"
#include "debug.h"

volatile __xdata keyboard_state_t keyboard_state;

__xdata keymap_config_t keymap_config;

void keyboard_init(void)
{
    keyboard_state.led_state     = 0x00;
    keyboard_state.rf_link       = 0x00;
    keyboard_state.battery_level = 0x00;
    keyboard_state.low_power     = 0x00;
    keyboard_state.connected     = 0x00;
    keyboard_state.paired        = 0x00;

#ifdef NKRO_ENABLE
    keymap_config.nkro = 1;
#endif
}

void keyboard_set_led_state(uint8_t led_state)
{
    keyboard_state.led_state = led_state;
}
