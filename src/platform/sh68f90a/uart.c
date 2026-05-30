#include "uart.h"
#include "delay.h"
#include "watchdog.h"
#include "console.h"
#include <stdint.h>

#define UART_BPS   57600
#define UART_MODE  1
#define UART_RX_EN 0

#if (UART_MODE == 1 || UART_MODE == 3)
#    ifndef FREQ_SYS
#        error FREQ_SYS must be defined
#    endif

// Mode 1/3 baud rate (datasheet §8.5.3):
//   BaudRate = Fsys / (16 * (32768 - SBRT) + BFINE)
// SBRT is a 15-bit value stored in SBRTH[6:0]:SBRTL[7:0]; SBRTH[7] is SBRTEN.
#    define SBRT_INT (FREQ_SYS / 16 / UART_BPS)
#    define SBRT_S   (32768 - SBRT_INT)
#    define SFINE_S  ((FREQ_SYS / UART_BPS) - (16 * SBRT_INT))

#    define UART_RESULT_BPS (FREQ_SYS / (16 * SBRT_INT + SFINE_S))
#    define UART_BPS_ERROR  ((UART_RESULT_BPS - UART_BPS) * 100 / UART_BPS)
#    if UART_BPS_ERROR >= 5 || UART_BPS_ERROR <= -5
#        error "UART baudrate error >= 5%, try a different UART_BPS"
#    endif
#else
#    define SBRT_S  0
#    define SFINE_S 0
#endif

#define SCON_INIT  ((UART_MODE << 6) | (UART_RX_EN << 4))
#define SBRT_INIT  (uint16_t)((_SBRTEN << 8) | SBRT_S)
#define SFINE_INIT (SFINE_S)

#define UART_ISR_ENABLE()  (IEN1 |= _ES0)
#define UART_ISR_DISABLE() (IEN1 &= ~_ES0)

// Compile-time verification against datasheet §8.5.3.
// Mode 1, SBRT (15-bit) is in valid range [0, 32767].
_Static_assert(SBRT_S >= 0 && SBRT_S < 32768, "SBRT out of 15-bit range");
_Static_assert(SFINE_S >= 0 && SFINE_S < 16, "SFINE must be in [0, 15]");
// Datasheet's worked example: Fsys=8MHz, baud=115200 -> SBRT=32764, BFINE=5,
// effective baud 115942 (+0.64%). Verify our formulas reproduce it.
#define _DS_SBRT(fsys, baud)  (32768 - ((fsys) / 16 / (baud)))
#define _DS_SFINE(fsys, baud) (((fsys) / (baud)) - 16 * ((fsys) / 16 / (baud)))
_Static_assert(_DS_SBRT(8000000, 115200) == 32764, "SBRT formula diverges from datasheet example");
_Static_assert(_DS_SFINE(8000000, 115200) == 5, "SFINE formula diverges from datasheet example");
#undef _DS_SBRT
#undef _DS_SFINE

volatile static __bit uart_tx_busy;

void uart_init()
{
    SCON  = SCON_INIT;
    SBRTH = (uint8_t)(SBRT_INIT >> 8);
    SBRTL = (uint8_t)(SBRT_INIT);
    SFINE = SFINE_INIT;

    PCON  = 0x00;
    SADDR = 0x00;
    SADEN = 0x00;

    UART_ISR_DISABLE();

    uart_tx_busy = 0;

    if (TI) {
        TI = 0;
    }

    if (RI) {
        RI = 0;
    }

    UART_ISR_ENABLE();
}

void uart_putc(unsigned char c)
{
    UART_ISR_DISABLE();

    uart_tx_busy = 1;

    SBUF = c;

    UART_ISR_ENABLE();

    while (uart_tx_busy) {
        CLR_WDT();
    }
}

void uart_interrupt_handler() __interrupt(_INT_EUART0)
{
    UART_ISR_DISABLE();

    if (TI) {
        TI = 0;

        uart_tx_busy = 0;
    }

    UART_ISR_ENABLE();
}

void putchar(int c)
{
    uart_putc(c);
#if DEBUG == 1
    console_putc(c);
#endif
}
