#pragma once

#include <stdbool.h>
#include <stdint.h>

// Sends `len` bytes over the bit-banged SPI link to the BK3632. Read bytes are
// written back in place over `data`.
//
// Interrupts are NOT disabled around the burst. This relies on every ISR that
// writes P0/P4/P7 using masked ANL/ORL or single-bit SETB/CLR so the SPI pin
// bits are preserved; an ISR firing mid-burst can stretch SCK but cannot
// corrupt the bus. Callers that need an atomic multi-byte receive must wrap
// with EA=0/EA=1 themselves (see rf_fetch_4).
//
// Returns true if the BK3632 toggled the ack line (RF_BB_SPI_ACK) within
// ~150 µs after CS rose, false on timeout. Command-style packets that change
// the BK3632's state typically ack; read-only fetches do not, so callers
// should only treat false as a problem for sends.
bool bb_spi_xfer(uint8_t *data, int len);
