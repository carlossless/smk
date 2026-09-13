#pragma once

#include "sfr.h"

#define PCA_HW_PRESENT 1

// Four units carrying nine compare channels between them, all on SFR page 1, and the part's
// only PWM. Every keyboard on this part drives its backlight from them, so the counter width
// and the all-four-running command are settled here rather than per board.
#define PCA_CHANNELS 9

#define PCA_CON_RUN  0x0Fu // all four counters enabled
#define PCA_TOP_8BIT 0xFFu // count maximum, so a duty is one byte

// X(config, command, top low, top high)
#define PCA_FOREACH_UNIT(X)        \
    X(P0CF, P0CMD, P0TOPL, P0TOPH) \
    X(P1CF, P1CMD, P1TOPL, P1TOPH) \
    X(P2CF, P2CMD, P2TOPL, P2TOPH) \
    X(P3CF, P3CMD, P3TOPL, P3TOPH)

// X(compare mode, compare low, compare high)
#define PCA_FOREACH_CHANNEL(X) \
    X(P0CPM0, P0CPL0, P0CPH0)  \
    X(P0CPM1, P0CPL1, P0CPH1)  \
    X(P1CPM0, P1CPL0, P1CPH0)  \
    X(P1CPM1, P1CPL1, P1CPH1)  \
    X(P1CPM2, P1CPL2, P1CPH2)  \
    X(P2CPM0, P2CPL0, P2CPH0)  \
    X(P2CPM1, P2CPL1, P2CPH1)  \
    X(P3CPM0, P3CPL0, P3CPH0)  \
    X(P3CPM1, P3CPL1, P3CPH1)
