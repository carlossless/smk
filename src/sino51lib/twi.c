#include "twi.h"
#include "twi_hw.h"
#include "watchdog.h"
#include "kbdef.h"
#include <stdint.h>

#if !TWI_HW_PRESENT
#    error "twi.c: this part has no TWI block, see its twi_hw.h"
#endif

#ifndef FREQ_SYS
#    error FREQ_SYS must be defined
#endif

#ifndef KB_TWI_CLOCK_HZ
#    define KB_TWI_CLOCK_HZ 100000u
#endif

#ifdef TWI_SFR_PAGE_1
#    define TWI_BUS_ENTER()        \
        uint8_t twi_page = INSCON; \
        sfr_page_1()
#    define TWI_BUS_LEAVE() INSCON = twi_page
#else
#    define TWI_BUS_ENTER() ((void)0)
#    define TWI_BUS_LEAVE() ((void)0)
#endif

#define TWI_PRESCALER 1u
#define TWI_CR        3u
#define TWI_BR        (((FREQ_SYS / (KB_TWI_CLOCK_HZ)) - 16u) / (2u * TWI_PRESCALER))

_Static_assert(TWI_BR >= 1u && TWI_BR <= 255u, "KB_TWI_CLOCK_HZ is out of TWIBR's range at this FREQ_SYS");

#define TWISTA_INIT (uint8_t)((TWI_CR << 1) | _ETOT)

#define TWICON_BASE  (uint8_t)(_ENTWI | _EFREE)
#define TWICON_START (uint8_t)(TWICON_BASE | _STA)
#define TWICON_STOP  (uint8_t)(TWICON_BASE | _STO)
#define TWICON_ACK   (uint8_t)(TWICON_BASE | _AA)

#define TWI_STATUS_MASK 0xF8u

#define TWI_ST_START      0x08u
#define TWI_ST_RSTART     0x10u
#define TWI_ST_SLAW_ACK   0x18u
#define TWI_ST_DATA_W_ACK 0x28u
#define TWI_ST_SLAR_ACK   0x40u

static uint8_t twi_wait(void)
{
    for (;;) {
        TWI_BUS_ENTER();
        bool    done   = (TWICON & _TWINT) != 0;
        uint8_t status = (uint8_t)(TWISTA & TWI_STATUS_MASK);
        TWI_BUS_LEAVE();

        watchdog_kick();

        if (done) {
            return status;
        }
    }
}

void twi_init(void)
{
    TWI_BUS_ENTER();
    TWICON = 0;
    TWISTA = TWISTA_INIT;
    TWIBR  = (uint8_t)TWI_BR;
    TWIADR = 0;
    TWIAMR = 0;
    TWICON = TWICON_BASE;
    TWI_BUS_LEAVE();
}

void twi_deinit(void)
{
    TWI_BUS_ENTER();
    TWICON = 0;
    TWI_BUS_LEAVE();
}

static void twi_command(uint8_t twicon)
{
    TWI_BUS_ENTER();
    TWICON = twicon;
    TWI_BUS_LEAVE();
}

bool twi_start(void)
{
    twi_command(TWICON_START);

    uint8_t status = twi_wait();
    return status == TWI_ST_START || status == TWI_ST_RSTART;
}

void twi_stop(void)
{
    twi_command(TWICON_STOP);
}

bool twi_write(uint8_t value)
{
    TWI_BUS_ENTER();
    TWIDAT = value;
    TWICON = TWICON_BASE;
    TWI_BUS_LEAVE();

    uint8_t status = twi_wait();
    return status == TWI_ST_DATA_W_ACK || status == TWI_ST_SLAW_ACK || status == TWI_ST_SLAR_ACK;
}

bool twi_write_address(uint8_t addr7, bool read)
{
    return twi_write((uint8_t)((addr7 << 1) | (read ? 1u : 0u)));
}

uint8_t twi_read(bool ack)
{
    twi_command(ack ? TWICON_ACK : TWICON_BASE);
    (void)twi_wait();

    TWI_BUS_ENTER();
    uint8_t value = TWIDAT;
    TWI_BUS_LEAVE();

    return value;
}
