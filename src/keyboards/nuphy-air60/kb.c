#include <stdint.h>
#include <stdbool.h>
#include "keycodes.h"
#include "kbdef.h"
#include "keyboard.h"
#include "debug.h"
#include "report.h"
#include "usb.h"

#ifdef RF_ENABLED
#    include "rf_controller.h"
#endif

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

void kb_init()
{
    user_keyboard_state.conn_mode = CONN_MODE_SWITCH;
    user_keyboard_state.os_mode   = OS_MODE_SWITCH;
}

void kb_update_switches()
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

bool kb_process_record(uint16_t keycode, bool key_pressed)
{
    key_pressed;
    switch (keycode) {
        case LNK_BT1:
        case LNK_BT2:
        case LNK_BT3:
        case LNK_24G:
            return false;

        default:
            return true;
    }
}

void kb_send_report(__xdata report_keyboard_t *report)
{
    switch (user_keyboard_state.conn_mode) {
        case KEYBOARD_CONN_MODE_USB:
            usb_send_report(report);
            break;
#ifdef RF_ENABLED
        case KEYBOARD_CONN_MODE_RF:
            rf_send_report(report);
            break;
#endif
    }
}

void kb_send_nkro(__xdata report_nkro_t *report)
{
    switch (user_keyboard_state.conn_mode) {
        case KEYBOARD_CONN_MODE_USB:
            usb_send_nkro(report);
            break;
#ifdef RF_ENABLED
        case KEYBOARD_CONN_MODE_RF:
            rf_send_nkro(report);
            break;
#endif
    }
}

void kb_send_extra(__xdata report_extra_t *report)
{
    switch (user_keyboard_state.conn_mode) {
        case KEYBOARD_CONN_MODE_USB:
            usb_send_extra(report);
            break;
#ifdef RF_ENABLED
        case KEYBOARD_CONN_MODE_RF:
            rf_send_extra(report);
            break;
#endif
    }
}

uint16_t ticks = 0;

void kb_update()
{
#ifdef RF_ENABLED
    if (user_keyboard_state.conn_mode == KEYBOARD_CONN_MODE_RF) {
        if (ticks > 20000) {
            EA = 0;
            rf_update_keyboard_state(&keyboard_state);
            ticks = 0;
            EA    = 1;
        }
    }
#endif
    ticks++;
}
