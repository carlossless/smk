#include "kbdef.h"
#include "keyboard.h"
#include "debug.h"

typedef enum {
    KEYBOARD_CONN_MODE_RF  = 0,
    KEYBOARD_CONN_MODE_USB = 1,
} user_keyboard_conn_mode_t;

typedef enum {
    KEYBOARD_OS_MODE_WIN = 0,
    KEYBOARD_OS_MODE_MAC = 1,
} user_keyboard_os_mode_t;

typedef struct {
    user_keyboard_conn_mode_t conn_mode;
    user_keyboard_os_mode_t   os_mode;
} user_keyboard_state_t;

volatile __xdata user_keyboard_state_t user_keyboard_state;

void user_keyboard_init()
{
    user_keyboard_state.conn_mode = CONN_MODE_SWITCH;
    user_keyboard_state.os_mode   = OS_MODE_SWITCH;
}

void user_keyboard_update_switches()
{
    if (user_keyboard_state.conn_mode != CONN_MODE_SWITCH) {
        user_keyboard_state.conn_mode = CONN_MODE_SWITCH;
        switch (user_keyboard_state.conn_mode) {
            case KEYBOARD_CONN_MODE_USB:
                dprintf("USB_MODE\r\n");
                break;
            case KEYBOARD_CONN_MODE_RF:
                dprintf("RF_MODE\r\n");
                break;
        }
    }

    if (user_keyboard_state.os_mode != OS_MODE_SWITCH) {
        user_keyboard_state.os_mode = OS_MODE_SWITCH;
        switch (user_keyboard_state.os_mode) {
            case KEYBOARD_OS_MODE_MAC:
                dprintf("MAC_MODE\r\n");
                break;
            case KEYBOARD_OS_MODE_WIN:
                dprintf("WIN_MODE\r\n");
                break;
        }
    }
}
