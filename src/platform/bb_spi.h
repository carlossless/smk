#pragma once

#include <stdbool.h>
#include <stdint.h>

// Bit-bangs `len` bytes to the BK3632 (TX), watching for the ACK line to
// toggle for up to ~150 µs. Runs with interrupts enabled — TX tolerates
// ISR-induced SCK stretching. Returns true on ACK, false on timeout (callers
// retry on false). MISO bytes received during the bang overwrite `data` but
// should be ignored — the BK3632 isn't guaranteed to drive MISO on a command.
bool bb_spi_xfer(uint8_t *data, int len);

// Bit-bangs `len` bytes of MISO from the BK3632 (RX). The byte loop runs under
// __critical (EA=0) so SCK timing stays deterministic for the slave's MISO
// drive; the pre-CS wakeup delay runs EA-on so the LED ISR isn't starved.
// `data` is filled with received bytes — fill it with 0xFF before calling.
void bb_spi_recv(uint8_t *data, int len);
