#pragma once

#include "pca_hw.h"
#include <stdint.h>

// The PCA units, where the part has them, as an 8-bit PWM bank: every compare module drives
// its pin and a duty is one byte. Which units and channels exist is the part's pca_hw.h, and
// PCA_CHANNELS is how many there are in total.

// brings every unit up with all its compare modules driving.
void pca_init(void);

// A compare value only reloads cleanly with the counters stopped and compare disabled;
// written live a channel can latch a half-updated duty. Write the compare registers between
// these two.
void pca_hold(void);
void pca_release(void);
