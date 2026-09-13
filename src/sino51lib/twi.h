#pragma once

#include <stdbool.h>
#include <stdint.h>

// Hardware TWI master, polled. The pins are fixed by the part, not chosen by the board: see
// its twi_hw.h. For a bus on arbitrary pins use bb_i2c instead.
//
// The transfer is composed from these the same way bb_i2c's is, so a device driver can be
// written against either. Every call returns once the bus event has completed; a call that
// reports false has left the bus for twi_stop() to release.
//
// The board may set KB_TWI_CLOCK_HZ in kbdef.h; it defaults to 100 kHz.

void twi_init(void);
void twi_deinit(void);

bool twi_start(void); // also the repeated START
void twi_stop(void);

bool    twi_write_address(uint8_t addr7, bool read);
bool    twi_write(uint8_t value); // true when the slave acknowledged
uint8_t twi_read(bool ack);
