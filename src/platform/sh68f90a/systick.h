#pragma once

#include "sh68f90a.h"

typedef enum {
    SYSTICK_SLOT_MATRIX_SCAN,
    SYSTICK_SLOT_LED_SUBFRAME,
} systick_slot_t;

void systick_init(void);
void systick_arm(systick_slot_t slot);

void systick_pause(void);
void systick_resume(void);
