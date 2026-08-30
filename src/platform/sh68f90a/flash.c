#include "flash.h"
#include "sh68f90a.h"
#include <stdbool.h>

#define CFG_ADDR   0xEC00u // sector 118
#define CFG_SIZE   512u
#define CFG_END    (CFG_ADDR + CFG_SIZE)
#define CFG_MAGIC0 0x5Au
#define CFG_MAGIC1 0xA5u
#define CFG_HDR    3u

_Static_assert((CFG_ADDR & (CFG_SIZE - 1)) == 0, "CFG_ADDR must be aligned to a 512-byte flash sector boundary");
_Static_assert(CFG_END <= 0xEE00u, "CFG_END must not reach sector 119 (holds reset-vector redirect at 0xEFFC)");

// SSP operation codes (datasheet 7.4).
// IB_CON2..5 must receive this key, in order, to arm an SSP operation.
#define SSP_KEY_2 0x05u
#define SSP_KEY_3 0x0Au
#define SSP_KEY_4 0x09u
#define SSP_KEY_5 0x06u

#define SSP_PROGRAM 0x6Eu
#define SSP_ERASE   0xE6u

static uint8_t flash_read(uint16_t addr)
{
    __code uint8_t *p = (__code uint8_t *)addr;
    return *p;
}

static uint16_t payload_addr(uint8_t i)
{
    return (uint16_t)(CFG_ADDR + CFG_HDR + i);
}

static void ssp_run(uint16_t addr, uint8_t op, uint8_t data)
{
    if (addr < CFG_ADDR || addr >= CFG_END) {
        return;
    }
    // A sector erase auto-IDLEs the CPU for ~5 ms with interrupts off.
    __critical
    {
        XPAGE     = (uint8_t)(addr >> 8);
        IB_OFFSET = (uint8_t)(addr & 0xFFu);
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
        XPAGE = 0;
    }
}

static void flash_erase_config(void)
{
    ssp_run(CFG_ADDR, SSP_ERASE, 0);
}

static void flash_program(uint16_t addr, uint8_t data)
{
    ssp_run(addr, SSP_PROGRAM, data);
}

static bool record_header_valid(uint8_t len)
{
    return flash_read(CFG_ADDR) == CFG_MAGIC0 && flash_read(CFG_ADDR + 1) == CFG_MAGIC1 && flash_read(CFG_ADDR + 2) == len;
}

static bool stored_record_matches(const __xdata uint8_t *src, uint8_t len)
{
    if (!record_header_valid(len)) {
        return false;
    }

    for (uint8_t i = 0; i < len; i++) {
        if (flash_read(payload_addr(i)) != src[i]) {
            return false;
        }
    }
    return true;
}

static bool record_checksum_valid(uint8_t len)
{
    uint8_t sum = 0;
    for (uint8_t i = 0; i < len; i++) {
        sum += flash_read(payload_addr(i));
    }
    return flash_read(payload_addr(len)) == sum;
}

bool flash_settings_load(__xdata uint8_t *dst, uint8_t len)
{
    if (!record_header_valid(len) || !record_checksum_valid(len)) {
        return false;
    }

    for (uint8_t i = 0; i < len; i++) {
        dst[i] = flash_read(payload_addr(i));
    }
    return true;
}

void flash_settings_save(const __xdata uint8_t *src, uint8_t len)
{
    if (stored_record_matches(src, len)) {
        return;
    }

    // A sector can only be programmed after an erase, so every change rewrites
    // the whole record.
    flash_erase_config();
    flash_program(CFG_ADDR + 0, CFG_MAGIC0);
    flash_program(CFG_ADDR + 1, CFG_MAGIC1);
    flash_program(CFG_ADDR + 2, len);

    uint8_t sum = 0;
    for (uint8_t i = 0; i < len; i++) {
        flash_program(payload_addr(i), src[i]);
        sum += src[i];
    }
    flash_program(payload_addr(len), sum);
}
