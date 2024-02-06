#include "lib/sh68f90a/clock.h"
#include "lib/sh68f90a/ldo.h"
#include "lib/sh68f90a/watchdog.h"
#include "lib/sh68f90a/delay.h"
#include "lib/sh68f90a/isp.h"
#include "lib/sh68f90a/uart.h"
#include "lib/sh68f90a/gpio.h"
#include "matrix.h"
#include "lib/sh68f90a/pwm.h"
#include "lib/sh68f90a/usb.h"
#include "debug.h"
#include "utils.h"
#include "keyboard.h"

#include <stdio.h>

void init()
{
    ldo_init();
    clock_init();
    gpio_init();
    uart_init();
    pwm_init();
    usb_init();
    matrix_init();
    keyboard_init();

    // usb interupt priority - 3
    IPH1 |= (1 << 6);
    IPL1 |= (1 << 6);

    // uart interupt priority - 2
    IPH1 |= (1 << 6);
    IPL1 |= (0 << 6);

    EA = 1; // enable interrupts
}

void main()
{
    init();

    dprintf("SMK v" TOSTRING(SMK_VERSION) "\r\n");
    dprintf("DEVICE vId:" TOSTRING(USB_VID) " pId:" TOSTRING(USB_PID) "\n\r");
    dprintf("OS: 0x%x, CONN: 0x%x\n\r", keyboard_state.os_mode, keyboard_state.conn_mode);

    // enable pwm and interrupt (driving matrix scan)
    pwm_enable();

    while (1) {
        CLR_WDT();

        matrix_task();
    }
}
