#include "keyboard.h"
#include "debug.h"

volatile __xdata keyboard_state_t keyboard_state;

__xdata keymap_config_t keymap_config;

void keyboard_init()
{
    keyboard_state.led_state = 0x00;
    keyboard_state.rf_link   = 0x00;

#ifdef NKRO_ENABLE
    keymap_config.nkro = 1;
#endif
}
