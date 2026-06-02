#pragma once

#include <stdbool.h>
#include <stdint.h>

// Bit-bangs `len` bytes of `data` to the BK3632 (TX direction), then
// polls the ACK line for up to ~150 µs. Runs with interrupts enabled
// throughout — TX is tolerant of ISR-induced SCK edge stretching.
//
// Matches stock fw's bb_spi_xfer at CODE:0x9cd4 — it sends a command
// buffer via bb_spi_byte_tx and watches `spi_ack_success` for the
// BK3632 to toggle its ACK line. Returns true on ACK, false on
// timeout. Callers retry on `false` (rf_send_or_retry).
//
// Receive bytes that arrive on MISO during the TX bit-bang are written
// back in place over `data` but should be ignored — the BK3632 isn't
// guaranteed to drive MISO during a command.
bool bb_spi_xfer(uint8_t *data, int len);

// Bit-bangs `len` bytes of MISO from the BK3632 (RX direction). The
// byte-bang loop runs under __critical (EA=0) so SCK timing is
// deterministic for the slave's MISO drive. The wakeup delay before
// CS goes low runs EA-on, so the LED ISR isn't starved across the full
// transfer. No ACK polling — stock fw's RX path
// (rf_get_status at CODE:0x9108) reads four bytes via bb_spi_byte_rx
// wrapped in `EA = 0` / `EA = 1` and never checks ACK.
//
// `data` is filled with the received bytes. The bytes on MOSI during
// the bit-bang are whatever was in `data` on entry — stock fills the
// buffer with 0xFF before calling, callers should do the same.
void bb_spi_recv(uint8_t *data, int len);
