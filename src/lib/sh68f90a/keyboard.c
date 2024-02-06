#include "../../keyboard.h"
#include "sh68f90a.h"
#include "../../debug.h"

__xdata keyboard_state_t keyboard_state;

void keyboard_init()
{
    keyboard_state.led_state = 0x00;
    keyboard_state.conn_mode = P5_5;
    keyboard_state.os_mode   = P5_6;
}

void keyboard_update_switches()
{
    if (keyboard_state.conn_mode != P5_5) {
        keyboard_state.conn_mode = P5_5;
        switch (keyboard_state.conn_mode) {
            case KEYBOARD_CONN_MODE_USB:
                dprintf("USB_MODE\r\n");
                break;
            case KEYBOARD_CONN_MODE_RF:
                dprintf("RF_MODE\r\n");
                break;
        }
    }

    if (keyboard_state.os_mode != P5_6) {
        keyboard_state.os_mode = P5_6;
        switch (keyboard_state.os_mode) {
            case KEYBOARD_OS_MODE_MAC:
                dprintf("MAC_MODE\r\n");
                break;
            case KEYBOARD_OS_MODE_WIN:
                dprintf("WIN_MODE\r\n");
                break;
        }
    }
}
