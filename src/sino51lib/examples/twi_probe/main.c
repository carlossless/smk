// Hardware TWI master, on the pins the part fixes (see its twi_hw.h). Walks the 7-bit address
// space and counts the devices that acknowledge, which is the smallest useful thing a bus
// master can do.

#include "clock.h"
#include "delay.h"
#include "ldo.h"
#include "peripherals.h"
#include "twi.h"
#include "watchdog.h"

#define FIRST_ADDR 0x50u // a 24Cxx, as good a placeholder as any

static uint8_t found;
static uint8_t first_byte;

static bool probe(uint8_t addr7)
{
    if (!twi_start()) {
        twi_stop();
        return false;
    }

    bool acked = twi_write_address(addr7, false);
    twi_stop();
    return acked;
}

void main(void)
{
    ldo_init();
    clock_init();
    peripherals_init();
    twi_init();

    for (;;) {
        found = 0;

        for (uint8_t addr = 1; addr < 0x78; addr++) {
            watchdog_kick();
            if (probe(addr)) {
                found++;
            }
        }

        if (found != 0) {
            // the shape of a register read: address, register, repeated START, read back
            if (twi_start() && twi_write_address(FIRST_ADDR, false) && twi_write(0x00)) {
                if (twi_start() && twi_write_address(FIRST_ADDR, true)) {
                    first_byte = twi_read(false); // NAK the last byte the master wants
                }
            }
            twi_stop();
        } else {
            // nothing answered: drop the block and bring it back up
            twi_deinit();
            twi_init();
        }

        delay_ms(1000);
    }
}
