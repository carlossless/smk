#include "nvm.h"
#include "sh68f89.h"
#include "kbdef.h"
#include "delay.h"
#include "watchdog.h"
#include <stdbool.h>

#define CFG_MAGIC0 0x5Au
#define CFG_MAGIC1 0xA5u
#define CFG_HDR    3u

#define EE_BASE     0u
#define EE_WRITE_MS 6u // 24Cxx self-timed write cycle, datasheet maximum is 5 ms

// The bus runs with SFR page 1 latched, where 0xB1 is USBCON rather than RSTSTAT, so the
// half-period cannot use delay_us(): its watchdog kick would write 0x02 into USBCON and
// drop the D+ pull-up. Spin on nops instead and kick the watchdog between transfers.
// ~60 cycles, so ~2.5 us at a 24 MHz FREQ_SYS and a bus around 200 kHz.
#define EE_HALF_PERIOD 10u

static void ee_delay(void)
{
    for (uint8_t i = 0; i < EE_HALF_PERIOD; i++) {
        // clang-format off
        __asm
            nop
            nop
        __endasm;
        // clang-format on
    }
}

// SDA and SCL are plain GPIO with no open-drain mode on this port, so a release is a
// switch to input and the line comes back up on its pull-up.
#define SDA_LOW()                   \
    do {                            \
        KB_EE_PORT &= ~KB_EE_SDA;   \
        KB_EE_PORT_CR |= KB_EE_SDA; \
    } while (0)
#define SDA_RELEASE()                \
    do {                             \
        KB_EE_PORT_CR &= ~KB_EE_SDA; \
    } while (0)
#define SCL_LOW()                   \
    do {                            \
        KB_EE_PORT &= ~KB_EE_SCL;   \
        KB_EE_PORT_CR |= KB_EE_SCL; \
    } while (0)
#define SCL_RELEASE()                \
    do {                             \
        KB_EE_PORT_CR &= ~KB_EE_SCL; \
    } while (0)
#define SDA_READ() ((KB_EE_PORT & KB_EE_SDA) != 0)

// SDA, SCL and their direction register all live on SFR page 1.
#define WITH_EE_PAGE(body)           \
    do {                             \
        uint8_t saved_page = INSCON; \
        sfr_page_1();                \
        body;                        \
        INSCON = saved_page;         \
    } while (0)

static void bus_idle(void)
{
    SDA_RELEASE();
    SCL_RELEASE();
    ee_delay();
}

static void bus_start(void)
{
    SDA_RELEASE();
    SCL_RELEASE();
    ee_delay();
    SDA_LOW();
    ee_delay();
    SCL_LOW();
    ee_delay();
}

static void bus_stop(void)
{
    SDA_LOW();
    ee_delay();
    SCL_RELEASE();
    ee_delay();
    SDA_RELEASE();
    ee_delay();
}

static bool write_byte(uint8_t value)
{
    for (uint8_t bit = 0; bit < 8; bit++) {
        if (value & 0x80u) {
            SDA_RELEASE();
        } else {
            SDA_LOW();
        }
        value = (uint8_t)(value << 1);
        ee_delay();
        SCL_RELEASE();
        ee_delay();
        SCL_LOW();
    }

    SDA_RELEASE();
    ee_delay();
    SCL_RELEASE();
    ee_delay();
    bool acked = !SDA_READ();
    SCL_LOW();
    ee_delay();
    return acked;
}

static uint8_t read_byte(bool ack)
{
    uint8_t value = 0;

    SDA_RELEASE();
    for (uint8_t bit = 0; bit < 8; bit++) {
        ee_delay();
        SCL_RELEASE();
        ee_delay();
        value = (uint8_t)((value << 1) | (SDA_READ() ? 1u : 0u));
        SCL_LOW();
    }

    if (ack) {
        SDA_LOW();
    } else {
        SDA_RELEASE();
    }
    ee_delay();
    SCL_RELEASE();
    ee_delay();
    SCL_LOW();
    SDA_RELEASE();
    ee_delay();
    return value;
}

// a 24Cxx random read: address the device for write, set the word address, then a
// repeated START to turn the bus around without releasing it.
static bool ee_read(uint8_t offset, __xdata uint8_t *dst, uint8_t len)
{
    bool ok = true;

    WITH_EE_PAGE({
        bus_start();
        ok = write_byte(KB_EE_DEV_ADDR) && write_byte(offset);
        if (ok) {
            bus_start();
            ok = write_byte((uint8_t)(KB_EE_DEV_ADDR | 1u));
        }
        if (ok) {
            for (uint8_t i = 0; i < len; i++) {
                dst[i] = read_byte(i + 1u < len);
            }
        }
        bus_stop();
        bus_idle();
    });

    watchdog_kick();
    return ok;
}

static bool ee_write_page(uint8_t offset, const __xdata uint8_t *src, uint8_t len)
{
    bool ok = true;

    WITH_EE_PAGE({
        bus_start();
        ok = write_byte(KB_EE_DEV_ADDR) && write_byte(offset);
        for (uint8_t i = 0; ok && i < len; i++) {
            ok = write_byte(src[i]);
        }
        bus_stop();
        bus_idle();
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

static __xdata uint8_t record[NVM_RECORD_SIZE];

static uint8_t record_length(uint8_t len)
{
    return (uint8_t)(CFG_HDR + len + 1u);
}

bool nvm_load(__xdata uint8_t *dst, uint8_t len)
{
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

static __xdata uint8_t stored[NVM_RECORD_SIZE];

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

    // write protect is on a page 0 pin, so pin the page around it rather than trusting the caller's.
    uint8_t saved_page = INSCON;
    sfr_page_0();
    KB_EE_WP_RELEASE();
    INSCON = saved_page;

    ee_write(record, record_length(len));

    sfr_page_0();
    KB_EE_WP_PROTECT();
    INSCON = saved_page;
}
