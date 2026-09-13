#include "nvm.h"
#include "flash.h"
#include <stdbool.h>
#include <stdint.h>

#define CFG_MAGIC0 0x5Au
#define CFG_MAGIC1 0xA5u
#define CFG_HDR    3u

#define RECORD_AT(offset) (uint16_t)(NVM_BASE + (offset))
#define PAYLOAD_AT        RECORD_AT(CFG_HDR)

_Static_assert(NVM_CAPACITY >= 1u, "the sector is too small to hold a record");

// A uint8_t length cannot exceed a capacity of 255 or more, and SDCC rejects the comparison
// that says so, so the guards below only exist on a part where the length can overrun.
#define NVM_LEN_CAN_OVERRUN (NVM_CAPACITY < 255u)

static uint8_t checksum(const __xdata uint8_t *src, uint8_t len)
{
    uint8_t sum = 0;
    for (uint8_t i = 0; i < len; i++) {
        sum += src[i];
    }
    return sum;
}

// Header and checksum go through the same run-length calls as the payload, so the store is
// reached from exactly three places however long the record is.
static __xdata uint8_t frame[CFG_HDR];

static bool header_valid(uint8_t len)
{
    flash_read_into(NVM_WINDOW, RECORD_AT(0), frame, CFG_HDR);
    return frame[0] == CFG_MAGIC0 && frame[1] == CFG_MAGIC1 && frame[2] == len;
}

bool nvm_load(__xdata uint8_t *dst, uint8_t len)
{
#if NVM_LEN_CAN_OVERRUN
    if (len > NVM_CAPACITY) {
        return false;
    }
#endif
    if (!header_valid(len)) {
        return false;
    }

    flash_read_into(NVM_WINDOW, PAYLOAD_AT, dst, len);

    flash_read_into(NVM_WINDOW, RECORD_AT(CFG_HDR + len), frame, 1);
    return frame[0] == checksum(dst, len);
}

void nvm_save(const __xdata uint8_t *src, uint8_t len)
{
#if NVM_LEN_CAN_OVERRUN
    if (len > NVM_CAPACITY) {
        return;
    }
#endif
    if (header_valid(len) && flash_matches(NVM_WINDOW, PAYLOAD_AT, src, len)) {
        return;
    }

    // a sector only programs after an erase, so every change rewrites the record.
    flash_erase(NVM_WINDOW, RECORD_AT(0));

    frame[0] = CFG_MAGIC0;
    frame[1] = CFG_MAGIC1;
    frame[2] = len;
    flash_program_from(NVM_WINDOW, RECORD_AT(0), frame, CFG_HDR);
    flash_program_from(NVM_WINDOW, PAYLOAD_AT, src, len);

    frame[0] = checksum(src, len);
    flash_program_from(NVM_WINDOW, RECORD_AT(CFG_HDR + len), frame, 1);
}
