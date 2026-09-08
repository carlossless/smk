#include "nvm.h"
#include "sh68f881.h"
#include <stdbool.h>

// the EEPROM-like block is 2 KB of 256-byte sectors from zero in the FLASHCON.FAC window; the record is in the first.
#define CFG_SECTOR 0u
#define CFG_BASE   (uint16_t)(CFG_SECTOR * NVM_SECTOR_SIZE)
#define CFG_MAGIC0 0x5Au
#define CFG_MAGIC1 0xA5u
#define CFG_HDR    3u

// IB_CON2..5 must receive this key, in order, to arm an SSP operation.
#define SSP_KEY_2 0x05u
#define SSP_KEY_3 0x0Au
#define SSP_KEY_4 0x09u
#define SSP_KEY_5 0x06u

// SSP operation codes (datasheet 7.4).
#define SSP_PROGRAM 0x6Eu
#define SSP_ERASE   0xE6u

static uint8_t eeprom_read(uint8_t offset)
{
    uint8_t value;
    // while FAC is set every MOVC hits the EEPROM block, so an ISR firing here would fetch its __code reads from the wrong place.
    __critical
    {
        FLASHCON          = _FAC;
        __code uint8_t *p = (__code uint8_t *)(CFG_BASE + offset);
        value             = *p;
        FLASHCON          = 0;
    }
    return value;
}

static uint8_t payload_offset(uint8_t i)
{
    return (uint8_t)(CFG_HDR + i);
}

static void ssp_run(uint8_t offset, uint8_t op, uint8_t data)
{
    // a sector erase auto-IDLEs the CPU for ~5 ms with interrupts off.
    __critical
    {
        FLASHCON  = _FAC;
        XPAGE     = CFG_SECTOR;
        IB_OFFSET = offset;
        IB_DATA   = data;
        IB_CON1   = op;
        IB_CON2   = SSP_KEY_2;
        IB_CON3   = SSP_KEY_3;
        IB_CON4   = SSP_KEY_4;
        IB_CON5   = SSP_KEY_5;
        // clang-format off
        __asm
            nop
            nop
            nop
            nop
            nop
        __endasm;
        // clang-format on
        XPAGE    = 0;
        FLASHCON = 0;
    }
}

static void eeprom_erase_config(void)
{
    ssp_run(0, SSP_ERASE, 0);
}

static void eeprom_program(uint8_t offset, uint8_t data)
{
    ssp_run(offset, SSP_PROGRAM, data);
}

static bool record_header_valid(uint8_t len)
{
    return eeprom_read(0) == CFG_MAGIC0 && eeprom_read(1) == CFG_MAGIC1 && eeprom_read(2) == len;
}

static bool stored_record_matches(const __xdata uint8_t *src, uint8_t len)
{
    if (!record_header_valid(len)) {
        return false;
    }

    for (uint8_t i = 0; i < len; i++) {
        if (eeprom_read(payload_offset(i)) != src[i]) {
            return false;
        }
    }
    return true;
}

static bool record_checksum_valid(uint8_t len)
{
    uint8_t sum = 0;
    for (uint8_t i = 0; i < len; i++) {
        sum += eeprom_read(payload_offset(i));
    }
    return eeprom_read(payload_offset(len)) == sum;
}

bool nvm_load(__xdata uint8_t *dst, uint8_t len)
{
    if (!record_header_valid(len) || !record_checksum_valid(len)) {
        return false;
    }

    for (uint8_t i = 0; i < len; i++) {
        dst[i] = eeprom_read(payload_offset(i));
    }
    return true;
}

void nvm_save(const __xdata uint8_t *src, uint8_t len)
{
    if (stored_record_matches(src, len)) {
        return;
    }

    // a sector only programs after an erase, so every change rewrites the record.
    eeprom_erase_config();
    eeprom_program(0, CFG_MAGIC0);
    eeprom_program(1, CFG_MAGIC1);
    eeprom_program(2, len);

    uint8_t sum = 0;
    for (uint8_t i = 0; i < len; i++) {
        eeprom_program(payload_offset(i), src[i]);
        sum += src[i];
    }
    eeprom_program(payload_offset(len), sum);
}
