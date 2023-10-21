#include "clock.h"
#include "ldo.h"
#include "watchdog.h"
#include "delay.h"
#include "isp.h"
#include "uart.h"
#include "stdio.h"
#include "gpio.h"
#include "matrix.h"
#include "pwm.h"
#include "layout.h"

__xdata uint16_t pressed_keycodes[MATRIX_COLS * MATRIX_ROWS];
uint8_t pressed_keycode_count;

void init()
{
    ldo_init();
    clock_init();
    gpio_init();

    // MAPPING = 0x5a; // use M_TXD and M_RXD pins (C5 & C4 respectively)

    uart_init();

    matrix_init();
    pwm_init();

    // uart interupt highest priority
    IPH1 |= (1 << 6);
    IPL1 |= (1 << 6);

    EA = 1; // enable interrupts
}

void main()
{
    init();

    printf("HELLO, WORLD!\n\r");

    // enable pwm and interrupt (driving matrix scan)
    pwm_enable();

    for (uint16_t i = 0; i < 1000; i++) {
        delay_ms(10);
        if (key_state_changed) {
            printf("KEY STATE CHANGED\n\r");

            pressed_keycode_count = 0;
            for (int i = 0; i < MATRIX_COLS; i++) {
                uint8_t row = key_state[i];
                for (int j = 0; j < MATRIX_ROWS; j++) {
                    if ((row >> j) & 0x01) {
                        printf("%u %u\n\r", i, j);
                        pressed_keycodes[pressed_keycode_count] = keymaps[0][j][i];
                        pressed_keycode_count++;
                    }
                }
            }

            if (pressed_keycode_count > 0) {
                printf("PRESSED: ");
                for (int i = 0; i < pressed_keycode_count; i++) {
                    printf("0x%04hx ", pressed_keycodes[i]);
                }
                printf("\n\r");

                if (pressed_keycodes[0] == KC_A && pressed_keycodes[1] == KC_S && pressed_keycodes[2] == KC_D) {
                    printf("JUMPING INTO ISP...\n\r");
                    isp_jump();
                }
            }

            key_state_changed = 0;
        }
    }

    printf("JUMPING INTO ISP...\n\r");
    isp_jump();
}
