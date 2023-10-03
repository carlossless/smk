#include "clock.h"
#include "ldo.h"
#include "watchdog.h"
#include "delay.h"
#include "isp.h"
#include "uart.h"
#include "stdio.h"
#include "gpio.h"

void init()
{
    ldo_init();
    clock_init();

    gpio_init();

    MAPPING = 0x5a; // use M_TXD and M_RXD pins (COL 6 & COL 5 respectively)

    uart_init();

    EA = 1; // enable interrupts
}

void main()
{
    init();

    delay_ms(1000);

    printf("HELLO, WORLD!\n\r");

    printf("JUMPING INTO ISP...\n\r");
    isp_jump();
}
