#pragma once

// systick.c calls out to exactly one function, and this is it: the periodic work the timer
// interrupt should do. What that work is belongs to whoever is using the library, which is why
// this header lives beside the example.
//
// The firmware in src/smk has its own, much larger, version of this file.

void tick_dispatch(void);
