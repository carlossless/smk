#pragma once

#include <stdbool.h>

// Inactivity sleep: a counter climbs once per LED frame, resets on key activity,
// and on reaching a threshold drops the MCU into its lowest-power state (LEDs
// off, RF asleep) until a keypress or host event wakes it.
//
// Gated by SLEEP_ENABLE. When off, every entry point compiles to nothing so call
// sites stay clean.

#ifdef SLEEP_ENABLE

// One-time setup; call once after the keyboard subsystems are up. Zeroes the
// inactivity counter / request state.
void sleep_init(void);

// Called on every realtime tick with whether that tick completed an LED frame.
// The counter advances per frame, not per matrix sweep, so the timeout doesn't
// move when the scan/LED interleave ratio does. Latches a sleep request once the
// threshold is reached.
void sleep_note_frame(bool frame_completed);

// Mark that the user did something (a key changed state). Resets the inactivity
// counter on the next tick. Single-byte flag write — safe to call from either
// the main loop or an ISR without guarding.
void sleep_note_activity(void);

// Main-loop hook: if a sleep request is pending, quiesce everything, sleep via
// the platform + board hooks, and restore on wake. Blocks for the whole sleep.
// No-op when nothing is pending.
void sleep_task(void);

#else

#    define sleep_init()           ((void)0)
#    define sleep_note_frame(done) ((void)(done))
#    define sleep_note_activity()  ((void)0)
#    define sleep_task()           ((void)0)

#endif
