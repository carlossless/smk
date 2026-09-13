#include "clock.h"
#include "delay.h"
#include "ldo.h"
#include "peripherals.h"
#include "twi.h"
#include "watchdog.h"

#define FIRST_ADDR 0x50u

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
            if (twi_start() && twi_write_address(FIRST_ADDR, false) && twi_write(0x00)) {
                if (twi_start() && twi_write_address(FIRST_ADDR, true)) {
                    first_byte = twi_read(false);
                }
            }
            twi_stop();
        } else {
            twi_deinit();
            twi_init();
        }

        delay_ms(1000);
    }
}
