#pragma once

// External interrupt 4. Alongside a USB bus event it is the only thing that can
// bring the core back out of Power-Down (datasheet 8.9.3), so it is the wake
// source the sleep path arms.
//
// INT4 is shared by every P4.x pin: a board arms it for its own wake pin and
// gets edges from everything else on that port too, which is why the wake path
// quiets other P4 users first.

// Arm INT4 and clear any edge already latched.
void extint_wake_arm(void);

// Stop INT4 firing during normal operation and drop whatever it latched.
void extint_wake_disable(void);

// Clear the latched P4.x edge flags without changing the enable.
void extint_wake_clear(void);
