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

// fSCL = FREQ_SYS / (16 + 2 * prescaler * TWIBR). The prescaler ladder is 64, 16, 4, 1 for
// CR[1:0] = 0..3, and 1 with an 8-bit TWIBR reaches every rate a keyboard bus needs.
#define TWI_PRESCALER 1u
#define TWI_CR        3u
#define TWI_BR        (((FREQ_SYS / (KB_TWI_CLOCK_HZ)) - 16u) / (2u * TWI_PRESCALER))

_Static_assert(TWI_BR >= 1u && TWI_BR <= 255u, "KB_TWI_CLOCK_HZ is out of TWIBR's range at this FREQ_SYS");

// ETOT in TWISTA and EFREE in TWICON both read as disable-when-set. With the two timeouts
// off, TWINT is the only flag the driver has to look at, and a slave that stalls SCL spins in
// twi_wait() until the watchdog resets the part rather than raising TOUT behind its back.
#define TWISTA_INIT (uint8_t)((TWI_CR << 1) | _ETOT)

// TWINT is cleared by writing it back as zero, so each command spells out the whole register:
// keep ENTWI and EFREE, drop TWINT, set only the action wanted. That write is also what
// releases SCL, so it has to come after TWIDAT is loaded.
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

// Polls with the bus page latched but kicks with it released, so the kick always lands on
// RSTSTAT. Returns the status code the event left behind.
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

    // TWINT is left set: only loading TWIDAT before it is cleared puts the address byte on
    // the wire instead of whatever TWIDAT happened to hold.
    uint8_t status = twi_wait();
    return status == TWI_ST_START || status == TWI_ST_RSTART;
}

void twi_stop(void)
{
    // STO clears itself once the condition is on the wire, and raises no TWINT to wait for.
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
