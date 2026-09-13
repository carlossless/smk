#pragma once

#include <stdint.h>
#include <stdbool.h>

// Bit-banged I2C master. The board supplies the two pins, and where they sit on a
// non-default SFR page it also supplies the wrapper that selects it; see kbdef.h.
// A transfer is composed from these by the device driver, inside KB_I2C_WITH_BUS.

void bb_i2c_start(void); // also the repeated START
void bb_i2c_stop(void);
void bb_i2c_idle(void);

bool    bb_i2c_write(uint8_t value); // true when the device acknowledged
uint8_t bb_i2c_read(bool ack);
