#pragma once

#include <stdbool.h>
#include <stdint.h>

// The SSP engine, which is the same on every part in the family: IB_CON2..5 take a fixed key
// in order to arm one byte-program or one sector-erase.
//
// The run-length calls are not a convenience: SDCC passes all but the first parameter on the
// stack, so a caller looping over single-byte calls pays the marshalling per byte. Keep loops
// on this side of the boundary.
//
// These are raw accesses with no bounds checking of any kind. A program or erase aimed at the
// wrong address will take out running code; nvm.c is the layer that keeps its writes inside
// one sector.

typedef enum {
    FLASH_CODE, // the program flash, as MOVC sees it with FLASHCON clear
    FLASH_DATA, // what FLASHCON.FAC swaps in: the EEPROM-like block, and the information block
} flash_window_t;

void flash_read_into(flash_window_t window, uint16_t addr, __xdata uint8_t *dst, uint8_t len);
bool flash_matches(flash_window_t window, uint16_t addr, const __xdata uint8_t *src, uint8_t len);

// Programming only clears bits, so a sector has to be erased before it will take new content.
void flash_program_from(flash_window_t window, uint16_t addr, const __xdata uint8_t *src, uint8_t len);

// Erases the sector holding addr. Sector size is the part's, see its nvm_hw.h.
void flash_erase(flash_window_t window, uint16_t addr);
