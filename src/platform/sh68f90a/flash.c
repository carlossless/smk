#include "flash.h"
#include "sh68f90a.h"
#include <stdbool.h>

// On-chip flash is 128 x 512-byte sectors (64K). The bootloader occupies the top 4K
// (0xF000-0xFFFF), and the flasher stores the app's reset-vector redirect at 0xEFFC
// (sector 119) which the bootloader uses to enter the app - so sector 119 must NEVER
// be erased. We use the sector just below it, and cap the linker (--code-size 0xEC00)
// so no program code is ever placed here.
#define CFG_ADDR   0xEC00u // flash sector 118 (0xEC00..0xEDFF)
#define CFG_MAGIC0 0x5Au
#define CFG_MAGIC1 0xA5u
// record layout: [magic0][magic1][len][payload x len][checksum]
#define CFG_HDR 3u // magic0, magic1, len precede the payload

// SSP operation codes and the fixed unlock sequence (datasheet 7.4).
#define SSP_PROGRAM 0x6Eu
#define SSP_ERASE   0xE6u

static uint8_t flash_read(uint16_t addr)
{
    __code uint8_t *p = (__code uint8_t *)addr;
    return *p;
}

// SSP unlock + trigger. IB_CON2..5 must be written 0x05,0x0A,0x09,0x06 in order with
// no intervening writes; the 4 NOPs cover the auto-IDLE while the flash op runs.
static void ssp_run(uint16_t addr, uint8_t op, uint8_t data)
{
    EA        = 0;
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
    __endasm;
    // clang-format on
    XPAGE = 0;
    EA    = 1;
}

static void flash_erase_config(void)
{
    ssp_run(CFG_ADDR, SSP_ERASE, 0);
}

static void flash_program(uint16_t addr, uint8_t data)
{
    ssp_run(addr, SSP_PROGRAM, data);
}

bool flash_settings_load(__xdata uint8_t *dst, uint8_t len)
{
    if (flash_read(CFG_ADDR) != CFG_MAGIC0 || flash_read(CFG_ADDR + 1) != CFG_MAGIC1) {
        return false;
    }
    if (flash_read(CFG_ADDR + 2) != len) {
        return false; // a different-sized record (e.g. struct changed) - ignore it
    }

    uint8_t sum = 0;
    for (uint8_t i = 0; i < len; i++) {
        sum += flash_read((uint16_t)(CFG_ADDR + CFG_HDR + i));
    }
    if (flash_read((uint16_t)(CFG_ADDR + CFG_HDR + len)) != sum) {
        return false; // checksum mismatch
    }

    // valid: commit to dst only now, so a bad record never corrupts the caller's data
    for (uint8_t i = 0; i < len; i++) {
        dst[i] = flash_read((uint16_t)(CFG_ADDR + CFG_HDR + i));
    }
    return true;
}

void flash_settings_save(const __xdata uint8_t *src, uint8_t len)
{
    // skip the write entirely if the stored record already matches (avoids wear)
    bool same = flash_read(CFG_ADDR) == CFG_MAGIC0 && flash_read(CFG_ADDR + 1) == CFG_MAGIC1 && flash_read(CFG_ADDR + 2) == len;
    if (same) {
        for (uint8_t i = 0; i < len; i++) {
            if (flash_read((uint16_t)(CFG_ADDR + CFG_HDR + i)) != src[i]) {
                same = false;
                break;
            }
        }
    }
    if (same) {
        return;
    }

    // The SH68F90A only programs bytes right after a sector erase ("once programmed,
    // a sector can't be programmed again until erased"), so each change rewrites the
    // whole record: erase, then magic + length + payload + checksum.
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
