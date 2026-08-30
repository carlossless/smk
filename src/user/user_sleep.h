#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    USER_SLEEP_NONE = 0,
    USER_SLEEP_RF   = 1,
    USER_SLEEP_USB  = 2,
} user_sleep_mode_t;

user_sleep_mode_t user_sleep_supported(void);
void              user_sleep_prepare(void);
void              user_sleep_wake(void);
