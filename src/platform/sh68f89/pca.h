#pragma once

#include "sh68f89.h"
#include <stdint.h>

// The four PCA units carry nine compare channels between them and are the part's only PWM.
// Every register here is on SFR page 1.

#define PCA_CHANNELS 9

#define PCA_CMD_RUN  0x02u // counter running, no overflow interrupt
#define PCA_CPM_PWM  0xD8u // PWM output, compare enabled
#define PCA_CPM_HOLD 0xD0u // the same with compare disabled, for a reload
#define PCA_CON_RUN  0x0Fu // all four counters enabled
#define PCA_TOP_8BIT 0xFFu // count maximum, so a duty is one byte

// brings all four units up as 8-bit PWM with every compare module driving its pin.
void pca_init(void);

// A compare value only reloads cleanly with the counters stopped and compare disabled;
// written live a channel can latch a half-updated duty. Write the PxCPLn registers between
// these two.
void pca_hold(void);
void pca_release(void);
