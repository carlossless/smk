#include "flash.h"
#include "sh68f90a.h"
#include <stdbool.h>

#define CFG_ADDR   0xEC00u               // flash sector 118 (0xEC00..0xEDFF)
#define CFG_SIZE   512u                  // SH68F90A flash sector size
#define CFG_END    (CFG_ADDR + CFG_SIZE) // first address past the settings sector
#define CFG_MAGIC0 0x5Au
#define CFG_MAGIC1 0xA5u
#define CFG_HDR    3u // magic0, magic1, len precede the payload

_Static_assert((CFG_ADDR & (CFG_SIZE - 1)) == 0, "CFG_ADDR must be aligned to a 512-byte flash sector boundary");
_Static_assert(CFG_END <= 0xEE00u, "CFG_END must not reach sector 119 (holds reset-vector redirect at 0xEFFC)");

// SSP operation codes (datasheet 7.4).
#define SSP_PROGRAM 0x6Eu
#define SSP_ERASE   0xE6u

static uint8_t flash_read(uint16_t addr)
{
    __code uint8_t *p = (__code uint8_t *)addr;
    return *p;
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
        IB_CON2   = 0x05;
        IB_CON3   = 0x0A;
        IB_CON4   = 0x09;
        IB_CON5   = 0x06;
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

static bool stored_record_matches(const __xdata uint8_t *src, uint8_t len)
{
    if (flash_read(CFG_ADDR) != CFG_MAGIC0 || flash_read(CFG_ADDR + 1) != CFG_MAGIC1 || flash_read(CFG_ADDR + 2) != len) {
        return false;
    }

    for (uint8_t i = 0; i < len; i++) {
        if (flash_read((uint16_t)(CFG_ADDR + CFG_HDR + i)) != src[i]) {
            return false;
        }
    }
    return true;
}

bool flash_settings_load(__xdata uint8_t *dst, uint8_t len)
{
    if (flash_read(CFG_ADDR) != CFG_MAGIC0 || flash_read(CFG_ADDR + 1) != CFG_MAGIC1) {
        return false;
    }
    if (flash_read(CFG_ADDR + 2) != len) {
        return false; // different-sized record (struct changed) - ignore
    }

    uint8_t sum = 0;
    for (uint8_t i = 0; i < len; i++) {
        sum += flash_read((uint16_t)(CFG_ADDR + CFG_HDR + i));
    }
    if (flash_read((uint16_t)(CFG_ADDR + CFG_HDR + len)) != sum) {
        return false; // checksum mismatch
    }

    for (uint8_t i = 0; i < len; i++) {
        dst[i] = flash_read((uint16_t)(CFG_ADDR + CFG_HDR + i));
    }
    return true;
}

void flash_settings_save(const __xdata uint8_t *src, uint8_t len)
{
    if (stored_record_matches(src, len)) {
        return; // nothing changed; don't spend an erase cycle
    }

    flash_erase_config();
    flash_program(CFG_ADDR + 0, CFG_MAGIC0);
    flash_program(CFG_ADDR + 1, CFG_MAGIC1);
    flash_program(CFG_ADDR + 2, len);

    uint8_t sum = 0;
    for (uint8_t i = 0; i < len; i++) {
        flash_program((uint16_t)(CFG_ADDR + CFG_HDR + i), src[i]);
        sum += src[i];
    }
    flash_program((uint16_t)(CFG_ADDR + CFG_HDR + len), sum);
}
