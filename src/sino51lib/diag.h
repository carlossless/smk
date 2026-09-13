#pragma once

void diag_task(void);

#if DEBUG == 1

#    include <stdbool.h>
#    include <stdint.h>

#    define DIAG_SCRATCH_SIZE 17u

extern __xdata uint8_t diag_scratch[DIAG_SCRATCH_SIZE];

void diag_info_read(uint16_t addr, __xdata uint8_t *dst, uint8_t len);
void diag_emit_hex(const __xdata uint8_t *src, uint8_t len);

bool diag_emit(uint8_t step);

#endif
