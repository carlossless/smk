#include "bb_i2c.h"
#include "kbdef.h"

// A transfer runs with the board's bus page latched, so the half period cannot go through
// delay_us(): that kicks the watchdog from inside its loop, and the watchdog register is
// not the watchdog register on every SFR page. Spin on nops; the caller kicks in between.
#ifndef KB_I2C_HALF_PERIOD
#    define KB_I2C_HALF_PERIOD 10u
#endif

static void half_period(void)
{
    for (uint8_t i = 0; i < KB_I2C_HALF_PERIOD; i++) {
        // clang-format off
        __asm
            nop
            nop
        __endasm;
        // clang-format on
    }
}

void bb_i2c_idle(void)
{
    KB_I2C_SDA_RELEASE();
    KB_I2C_SCL_RELEASE();
    half_period();
}

void bb_i2c_start(void)
{
    KB_I2C_SDA_RELEASE();
    KB_I2C_SCL_RELEASE();
    half_period();
    KB_I2C_SDA_LOW();
    half_period();
    KB_I2C_SCL_LOW();
    half_period();
}

void bb_i2c_stop(void)
{
    KB_I2C_SDA_LOW();
    half_period();
    KB_I2C_SCL_RELEASE();
    half_period();
    KB_I2C_SDA_RELEASE();
    half_period();
}

bool bb_i2c_write(uint8_t value)
{
    for (uint8_t bit = 0; bit < 8; bit++) {
        if (value & 0x80u) {
            KB_I2C_SDA_RELEASE();
        } else {
            KB_I2C_SDA_LOW();
        }
        value = (uint8_t)(value << 1);
        half_period();
        KB_I2C_SCL_RELEASE();
        half_period();
        KB_I2C_SCL_LOW();
    }

    KB_I2C_SDA_RELEASE();
    half_period();
    KB_I2C_SCL_RELEASE();
    half_period();
    bool acked = !KB_I2C_SDA_READ();
    KB_I2C_SCL_LOW();
    half_period();
    return acked;
}

uint8_t bb_i2c_read(bool ack)
{
    uint8_t value = 0;

    KB_I2C_SDA_RELEASE();
    for (uint8_t bit = 0; bit < 8; bit++) {
        half_period();
        KB_I2C_SCL_RELEASE();
        half_period();
        value = (uint8_t)((value << 1) | (KB_I2C_SDA_READ() ? 1u : 0u));
        KB_I2C_SCL_LOW();
    }

    if (ack) {
        KB_I2C_SDA_LOW();
    } else {
        KB_I2C_SDA_RELEASE();
    }
    half_period();
    KB_I2C_SCL_RELEASE();
    half_period();
    KB_I2C_SCL_LOW();
    KB_I2C_SDA_RELEASE();
    half_period();
    return value;
}
