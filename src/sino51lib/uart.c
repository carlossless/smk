#include "uart.h"
#include "uart_hw.h"
#include "watchdog.h"
#include "console.h"
#include <stdint.h>

#if defined(DEBUG_SINK_UART) && !UART_HW_PRESENT
#    error "DEBUG_SINK_UART: this part has no EUART mapping, see its uart_hw.h"
#endif

#ifdef DEBUG_SINK_UART

#    include "interrupts.h"

#    define UART_BPS   57600
#    define UART_MODE  1
#    define UART_RX_EN 0

#    if (UART_MODE == 1 || UART_MODE == 3)
#        ifndef FREQ_SYS
#            error FREQ_SYS must be defined
#        endif

// Mode 1/3 baud rate:
//   BaudRate = Fsys / (16 * (32768 - SBRT) + BFINE)
#        define SBRT_INT (FREQ_SYS / 16 / UART_BPS)
#        define SBRT_S   (32768 - SBRT_INT)
#        define SFINE_S  ((FREQ_SYS / UART_BPS) - (16 * SBRT_INT))

#        define UART_RESULT_BPS (FREQ_SYS / (16 * SBRT_INT + SFINE_S))
#        define UART_BPS_ERROR  ((UART_RESULT_BPS - UART_BPS) * 100 / UART_BPS)
#        if UART_BPS_ERROR >= 5 || UART_BPS_ERROR <= -5
#            error "UART baudrate error >= 5%, try a different UART_BPS"
#        endif
#    else
#        define SBRT_S  0
#        define SFINE_S 0
#    endif

#    define SCON_INIT  ((UART_MODE << 6) | (UART_RX_EN << 4))
#    define SFINE_INIT (SFINE_S)

#    define UART_ISR_ENABLE()  (UART_IEN |= UART_IEN_BIT)
#    define UART_ISR_DISABLE() (UART_IEN &= ~UART_IEN_BIT)
_Static_assert(SBRT_S >= 0 && SBRT_S < 32768, "SBRT out of 15-bit range");
_Static_assert(SFINE_S >= 0 && SFINE_S < 16, "SFINE must be in [0, 15]");
#    define _DS_SBRT(fsys, baud)  (32768 - ((fsys) / 16 / (baud)))
#    define _DS_SFINE(fsys, baud) (((fsys) / (baud)) - 16 * ((fsys) / 16 / (baud)))
_Static_assert(_DS_SBRT(8000000, 115200) == 32764, "SBRT formula diverges from datasheet example");
_Static_assert(_DS_SFINE(8000000, 115200) == 5, "SFINE formula diverges from datasheet example");
#    undef _DS_SBRT
#    undef _DS_SFINE

volatile static __bit uart_tx_busy;

void uart_init()
{
    SCON = SCON_INIT;
    UART_SBRT_WRITE(SBRT_S);
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
        watchdog_kick();
    }
}

void uart_interrupt_handler() __interrupt(UART_VECTOR)
{
    UART_ISR_DISABLE();

    if (TI) {
        TI = 0;

        uart_tx_busy = 0;
    }

    UART_ISR_ENABLE();
}

#endif // DEBUG_SINK_UART

void debug_putc(char c)
{
#ifdef DEBUG_SINK_UART
    uart_putc((unsigned char)c);
#endif
#ifdef DEBUG_SINK_CONSOLE
    console_putc((unsigned char)c);
#endif
    (void)c;
}

void putchar(int c)
{
    debug_putc((char)c);
}
