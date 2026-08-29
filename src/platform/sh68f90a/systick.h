#pragma once

#include "sh68f90a.h"

// Periodic interrupt driving the keyboard's realtime work. One timer has to
// serve both the key matrix and the backlight, so the core alternates between
// them (see smk/tick.h) and each slot gets its own interval, armed by the core
// just before it runs that slot's work.
typedef enum {
    SYSTICK_SLOT_MATRIX_SCAN,
    SYSTICK_SLOT_LED_SUBFRAME,
} systick_slot_t;

void systick_init(void);
void systick_arm(systick_slot_t slot);

// Mask / unmask the tick. The timer keeps counting, so this is a way to hold the
// realtime work still, not a way to stop time.
void systick_pause(void);
void systick_resume(void);
