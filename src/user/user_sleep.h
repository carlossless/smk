#pragma once

#include <stdbool.h>
#include <stdint.h>

// Which sleep variant (if any) this board should use right now. The variants
// differ in trigger and wake source.
typedef enum {
    USER_SLEEP_NONE = 0, // don't sleep
    USER_SLEEP_RF   = 1, // RF/battery: inactivity-triggered
    USER_SLEEP_USB  = 2, // USB: host-suspend-triggered
} user_sleep_mode_t;

// Board sleep hooks. Only referenced when SLEEP_ENABLE is set.
//
// supported(): which sleep mode applies right now - queried live, side-effect free.
//
// prepare(): park every port into its low-power state and arm the wake source(s).
//   Called with the LED scan already paused.
//
// wake(): undo prepare() - restore normal operating GPIO and disable the wake
//   interrupt. Called immediately after wake, before the LED scan resumes.
user_sleep_mode_t user_sleep_supported(void);
void              user_sleep_prepare(void);
void              user_sleep_wake(void);
