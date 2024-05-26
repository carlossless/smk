#include "keyboard.h"
#include "debug.h"

volatile __xdata keyboard_state_t keyboard_state;

void keyboard_init()
{
    keyboard_state.led_state = 0x00;
}
