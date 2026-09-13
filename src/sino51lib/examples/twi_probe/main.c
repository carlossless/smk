// Hardware TWI master, on the pins the part fixes (see its twi_hw.h). Walks the 7-bit address
// space and counts the devices that acknowledge, which is the smallest useful thing a bus
// master can do.

#include "clock.h"
#include "delay.h"
#include "ldo.h"
#include "peripherals.h"
#include "twi.h"
#include "watchdog.h"

static uint8_t found;

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

        delay_ms(1000);
    }
}
