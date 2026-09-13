#pragma once

// dumps the factory information block to the debug console, one line per call so a long dump never outruns the ring buffer. DEBUG builds only.

void diag_task(void);

#if DEBUG == 1

#    include <stdbool.h>
#    include <stdint.h>

#    define DIAG_SCRATCH_SIZE 17u

extern __xdata uint8_t diag_scratch[DIAG_SCRATCH_SIZE];

void diag_info_read(uint16_t addr, __xdata uint8_t *dst, uint8_t len);
void diag_emit_hex(const __xdata uint8_t *src, uint8_t len);

// supplied per part: emits one line for step, or returns false once the steps run out.
bool diag_emit(uint8_t step);

#endif
