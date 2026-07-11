#pragma once

#include <stdbool.h>
#include <stdint.h>

// Which sleep variant (if any) this board should use right now. The two variants
// differ in trigger and wake (see src/smk/sleep.c, power.c).
typedef enum {
    USER_SLEEP_NONE = 0, // don't sleep
    USER_SLEEP_RF   = 1, // RF/battery: regulator off, inactivity-triggered
    USER_SLEEP_USB  = 2, // USB: regulator on / GOSUSP, host-suspend-triggered
} user_sleep_mode_t;

// Board sleep hooks. Only referenced when SLEEP_ENABLE is set.
//
// supported(): which sleep mode applies *right now* — queried live (not cached)
//   so it can track the slider. Side-effect free.
//
// prepare(): park every port into its low-power sleep state and arm the wake
//   source(s) — INT4/EX4 on the board's wake pin, plus USB bus events. Called
//   with the LED scan already paused and the PWM parked.
//
// wake(): undo prepare() — restore the normal operating GPIO configuration and
//   disable the wake interrupt. Called immediately after the core wakes, before
//   the LED scan resumes.
user_sleep_mode_t user_sleep_supported(void);
void              user_sleep_prepare(void);
void              user_sleep_wake(void);
