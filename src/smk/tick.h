#pragma once

// The keyboard's realtime work, driven by the platform's periodic tick.
//
// One hardware timer has to serve two jobs that can't overlap: sweeping the key
// matrix, and emitting LED PWM subframes (LED drive current couples into the row
// sense). So ticks alternate between them, and this is where that interleave is
// decided.

void tick_init(void);

// Run whichever slot is due. Called from the platform's tick interrupt.
void tick_dispatch(void);

// Hold the matrix sweep and the backlight still without stopping time. Needed
// around work they must not overlap — a blocking flash write, or sleep entry.
void tick_pause(void);
void tick_resume(void);
