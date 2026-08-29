#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    USER_SLEEP_NONE = 0, // don't sleep
    USER_SLEEP_RF   = 1, // RF/battery: inactivity-triggered
    USER_SLEEP_USB  = 2, // USB: host-suspend-triggered
} user_sleep_mode_t;

user_sleep_mode_t user_sleep_supported(void);
void              user_sleep_prepare(void);
void              user_sleep_wake(void);
