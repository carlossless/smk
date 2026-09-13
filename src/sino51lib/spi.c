#include "spi.h"
#include "spi_hw.h"
#include "watchdog.h"
#include "kbdef.h"
#include <stdint.h>

#if !SPI_HW_PRESENT
#    error "spi.c: this part has no SPI block, see its spi_hw.h"
#endif

#ifndef KB_SPI_CLOCK_DIV
#    define KB_SPI_CLOCK_DIV 16u
#endif
#ifndef KB_SPI_CPOL
#    define KB_SPI_CPOL 0u
#endif
#ifndef KB_SPI_CPHA
#    define KB_SPI_CPHA 0u
#endif
#ifndef KB_SPI_LSB_FIRST
#    define KB_SPI_LSB_FIRST 0u
#endif

// SPR counts doublings up from the part's fastest divider, and that divider is not the same
// on every part, so the board asks for a ratio and spi_hw.h says where the ladder starts.
#define SPI_RATIO ((KB_SPI_CLOCK_DIV) / (SPI_DIV_MIN))
#define SPI_SPR   (uint8_t)(((SPI_RATIO) >= 2u) + ((SPI_RATIO) >= 4u) + ((SPI_RATIO) >= 8u) + ((SPI_RATIO) >= 16u) + ((SPI_RATIO) >= 32u) + ((SPI_RATIO) >= 64u) + ((SPI_RATIO) >= 128u))

_Static_assert((SPI_RATIO) * (SPI_DIV_MIN) == (KB_SPI_CLOCK_DIV) && ((SPI_RATIO) & ((SPI_RATIO)-1u)) == 0u, "KB_SPI_CLOCK_DIV must be this part's fastest divider times a power of two");
_Static_assert((KB_SPI_CLOCK_DIV) >= (SPI_DIV_MIN), "KB_SPI_CLOCK_DIV is faster than this part's SPR ladder goes");
_Static_assert((KB_SPI_CLOCK_DIV) <= (SPI_DIV_MAX), "KB_SPI_CLOCK_DIV is slower than this part's SPR ladder goes");

// SSDIS: the SS pin is left to the board, which drives its own chip select, and with it tied
// off MODF can never trip.
#define SPCON_FORMAT (uint8_t)((KB_SPI_CPOL ? _CPOL : 0u) | (KB_SPI_CPHA ? _CPHA : 0u) | (KB_SPI_LSB_FIRST ? _DIR : 0u))
#define SPCON_INIT   (uint8_t)(_MSTR | _SSDIS | SPI_SPR | SPCON_FORMAT)

// SPIF and the error flags are cleared by software, so every write back to SPSTA has to keep
// SPEN set and the flags clear.
#define SPSTA_IDLE (uint8_t)(_SPEN)

void spi_init(void)
{
    SPCON = SPCON_INIT;
    SPSTA = SPSTA_IDLE;
}

void spi_deinit(void)
{
    SPSTA = 0;
    SPCON = 0;
}

uint8_t spi_xfer_byte(uint8_t out)
{
    SPDAT = out;

    while (!(SPSTA & _SPIF)) {
        watchdog_kick();
    }
    SPSTA = SPSTA_IDLE;

    return SPDAT;
}

void spi_xfer(uint8_t *data, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++) {
        data[i] = spi_xfer_byte(data[i]);
    }
}

void spi_send(const uint8_t *data, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++) {
        (void)spi_xfer_byte(data[i]);
    }
}

void spi_recv(uint8_t *data, uint8_t len, uint8_t idle)
{
    for (uint8_t i = 0; i < len; i++) {
        data[i] = spi_xfer_byte(idle);
    }
}
