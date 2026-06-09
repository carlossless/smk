#include "flash.h"
#include "sh68f90a.h"
#include <stdbool.h>

// On-chip flash is 128 x 512-byte sectors (64K). The bootloader occupies the top 4K
// (0xF000-0xFFFF), and the flasher stores the app's reset-vector redirect at 0xEFFC
// (sector 119) which the bootloader uses to enter the app - so sector 119 must NEVER
// be erased. We use the sector just below it, and cap the linker (--code-size 0xEC00)
// so no program code is ever placed here.
#define CFG_ADDR   0xEC00u // flash sector 118 (0xEC00..0xEDFF)
#define CFG_SIZE   512u    // SH68F90A sector size — see SH68F90A.gpt [SectorSize]
#define CFG_END    (CFG_ADDR + CFG_SIZE) // first address past the settings sector
#define CFG_MAGIC0 0x5Au
#define CFG_MAGIC1 0xA5u
// record layout: [magic0][magic1][len][payload x len][checksum]
#define CFG_HDR 3u // magic0, magic1, len precede the payload

// Belt-and-suspenders compile-time checks. If any of these fail, the build
// stops before we can flash a binary that might wipe the bootloader.
_Static_assert((CFG_ADDR & (CFG_SIZE - 1)) == 0,
               "CFG_ADDR must be aligned to a 512-byte flash sector boundary");
_Static_assert(CFG_END <= 0xEE00u,
               "CFG_END must not reach sector 119 (holds reset-vector redirect at 0xEFFC)");

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
// __critical (save+restore EA) instead of CLR/SETB so callers that already run
// with EA=0 don't get interrupts silently re-enabled on return.
static void ssp_run(uint16_t addr, uint8_t op, uint8_t data)
{
    // Address gate — refuses any SSP op outside the settings sector. Covers
    // a stack-corrupted addr, a future caller bug, or anything else that
    // would put SSP_ERASE over sector 119 (boot redirect) or the bootloader
    // at 0xF000+. Inlined (not a separate function) so SDCC doesn't have to
    // allocate an OSEG slot for the helper's return path.
    if (addr < CFG_ADDR || addr >= CFG_END) {
        return;
    }
    // A sector erase auto-IDLEs the CPU for ~5 ms with interrupts off. Any LED
    // blanking needed to hide that stall is handled one layer up, around the
    // whole write (settings_save_pre/post), so this platform driver stays LED-
    // agnostic.
    __critical {
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
    // No runtime length check here: `len` is uint8_t, so max payload is 255
    // and the total record (CFG_HDR + len + 1) caps at 259 bytes — already
    // well inside the 512-byte sector. The compile-time assertion in
    // settings.c (paired with sizeof(user_settings_t)) is what actually
    // enforces the bound; if the struct ever grows past the limit, the
    // build fails before we can flash a binary that would overrun.

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
