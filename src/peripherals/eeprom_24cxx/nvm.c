#include "nvm.h"
#include "bb_i2c.h"
#include "kbdef.h"
#include "delay.h"
#include "watchdog.h"
#if DEBUG == 1
#    include "debug.h"
#endif

#define CFG_MAGIC0 0x5Au
#define CFG_MAGIC1 0xA5u
#define CFG_HDR    3u

#define EE_BASE     0u
#define EE_WRITE_MS 6u // 24Cxx self-timed write cycle, 5 ms worst case

static __xdata uint8_t record[NVM_RECORD_SIZE];
static __xdata uint8_t stored[NVM_RECORD_SIZE];

static uint8_t record_length(uint8_t len)
{
    return (uint8_t)(CFG_HDR + len + 1u);
}

// an empty record and an absent chip both read as all-ones, so the address probe is the
// only thing that separates "nothing saved yet" from "the bus is not working".
bool nvm_present(void)
{
    bool acked;

    KB_I2C_WITH_BUS({
        bb_i2c_start();
        acked = bb_i2c_write(KB_EE_DEV_ADDR);
        bb_i2c_stop();
        bb_i2c_idle();
    });

    watchdog_kick();
    return acked;
}

// a 24Cxx random read: address the device for write, set the word address, then a
// repeated START to turn the bus around without releasing it.
static bool ee_read(uint8_t offset, __xdata uint8_t *dst, uint8_t len)
{
    bool ok;

    KB_I2C_WITH_BUS({
        bb_i2c_start();
        ok = bb_i2c_write(KB_EE_DEV_ADDR) && bb_i2c_write(offset);
        if (ok) {
            bb_i2c_start();
            ok = bb_i2c_write((uint8_t)(KB_EE_DEV_ADDR | 1u));
        }
        if (ok) {
            for (uint8_t i = 0; i < len; i++) {
                dst[i] = bb_i2c_read(i + 1u < len);
            }
        }
        bb_i2c_stop();
        bb_i2c_idle();
    });

    watchdog_kick();
    return ok;
}

static bool ee_write_page(uint8_t offset, const __xdata uint8_t *src, uint8_t len)
{
    bool ok;

    KB_I2C_WITH_BUS({
        bb_i2c_start();
        ok = bb_i2c_write(KB_EE_DEV_ADDR) && bb_i2c_write(offset);
        for (uint8_t i = 0; ok && i < len; i++) {
            ok = bb_i2c_write(src[i]);
        }
        bb_i2c_stop();
        bb_i2c_idle();
    });

    watchdog_kick();
    delay_ms(EE_WRITE_MS);
    return ok;
}

static bool ee_write(const __xdata uint8_t *src, uint8_t len)
{
    // a page write wraps inside the device's page rather than carrying, so each burst
    // has to stop at the next page boundary.
    uint8_t offset = EE_BASE;
    while (len) {
        uint8_t room  = (uint8_t)(KB_EE_PAGE_SIZE - (offset % KB_EE_PAGE_SIZE));
        uint8_t chunk = len < room ? len : room;
        if (!ee_write_page(offset, src, chunk)) {
            return false;
        }
        src += chunk;
        offset = (uint8_t)(offset + chunk);
        len    = (uint8_t)(len - chunk);
    }
    return true;
}

bool nvm_load(__xdata uint8_t *dst, uint8_t len)
{
#if DEBUG == 1
    static __bit probed;
    if (!probed) {
        probed = 1;
        dprintf("EE %s\r\n", nvm_present() ? "present" : "absent");
    }
#endif

    if (len > NVM_CAPACITY) {
        return false;
    }
    if (!ee_read(EE_BASE, record, record_length(len))) {
        return false;
    }
    if (record[0] != CFG_MAGIC0 || record[1] != CFG_MAGIC1 || record[2] != len) {
        return false;
    }

    uint8_t sum = 0;
    for (uint8_t i = 0; i < len; i++) {
        sum += record[CFG_HDR + i];
    }
    if (record[CFG_HDR + len] != sum) {
        return false;
    }

    for (uint8_t i = 0; i < len; i++) {
        dst[i] = record[CFG_HDR + i];
    }
    return true;
}

static bool stored_record_matches(uint8_t len)
{
    uint8_t total = record_length(len);

    if (!ee_read(EE_BASE, stored, total)) {
        return false;
    }
    for (uint8_t i = 0; i < total; i++) {
        if (stored[i] != record[i]) {
            return false;
        }
    }
    return true;
}

void nvm_save(const __xdata uint8_t *src, uint8_t len)
{
    if (len > NVM_CAPACITY) {
        return;
    }

    record[0] = CFG_MAGIC0;
    record[1] = CFG_MAGIC1;
    record[2] = len;

    uint8_t sum = 0;
    for (uint8_t i = 0; i < len; i++) {
        record[CFG_HDR + i] = src[i];
        sum += src[i];
    }
    record[CFG_HDR + len] = sum;

    if (stored_record_matches(len)) {
        return;
    }

    KB_EE_WP_RELEASE();
    ee_write(record, record_length(len));
    KB_EE_WP_PROTECT();
}
