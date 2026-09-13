// The bit-banged I2C master, which is what a board uses when the pins it has are not the ones
// the TWI block sits on. Unlike twi.c this works on every part, because it is only GPIO.
//
// The pins arrive as macros from kbdef.h, so a board that has to select an SFR page or drive
// through an inverter can still describe them; the one beside this file is a placeholder.

#include "bb_i2c.h"
#include "clock.h"
#include "delay.h"
#include "ldo.h"
#include "peripherals.h"
#include "watchdog.h"

#define EEPROM_ADDR 0x50u

static uint8_t found;
static uint8_t first_byte;

static bool write_address(uint8_t addr7, bool read)
{
    return bb_i2c_write((uint8_t)((addr7 << 1) | (read ? 1u : 0u)));
}

void main(void)
{
    ldo_init();
    clock_init();
    peripherals_init();

    bb_i2c_idle();

    for (;;) {
        found = 0;

        for (uint8_t addr = 1; addr < 0x78; addr++) {
            watchdog_kick();

            bb_i2c_start();
            if (write_address(addr, false)) {
                found++;
            }
            bb_i2c_stop();
        }

        if (found != 0) {
            // a register read: address, register, repeated START, then read and NAK
            bb_i2c_start();
            if (write_address(EEPROM_ADDR, false) && bb_i2c_write(0x00)) {
                bb_i2c_start();
                if (write_address(EEPROM_ADDR, true)) {
                    first_byte = bb_i2c_read(false);
                }
            }
            bb_i2c_stop();
        }

        delay_ms(1000);
    }
}
