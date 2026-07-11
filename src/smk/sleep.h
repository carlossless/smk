#pragma once

// Inactivity sleep feature: an inactivity counter ticks up once per matrix
// frame, is reset to zero on key activity, and on reaching a threshold the
// keyboard drops the MCU into Power-Down mode (LEDs off, RF asleep) until a
// keypress (INT4) or USB bus event wakes it.
//
// Gated by SLEEP_ENABLE (meson `sleep` feature, enabled by default). When off,
// every entry point compiles to nothing so call sites stay clean.

#ifdef SLEEP_ENABLE

// One-time setup; call once after the keyboard subsystems are up. Zeroes the
// inactivity counter / request state.
void sleep_init(void);

// Advance the inactivity counter by one. Called from the Timer 2 ISR, once per
// matrix-scan frame. Cheap: a couple of xdata ops. No-op until the threshold,
// then latches a sleep request.
void sleep_tick(void);

// Mark that the user did something (a key changed state). Resets the inactivity
// counter on the next tick. Single-byte flag write — safe to call from either
// the main loop or an ISR without guarding.
void sleep_note_activity(void);

// Main-loop hook: if a sleep request is pending, quiesce LEDs/RF, enter
// Power-Down via the platform + board hooks, and on wake restore everything.
// Blocks (the core is halted) for the whole sleep. No-op when nothing pending.
void sleep_task(void);

#else

#    define sleep_init()          ((void)0)
#    define sleep_tick()          ((void)0)
#    define sleep_note_activity() ((void)0)
#    define sleep_task()          ((void)0)

#endif
