#pragma once

// Puts the blocks that can steal port pins into a known state, so GPIO setup afterwards
// actually reaches the pins. Runs once, after the clock is up and before any board init.
void peripherals_init(void);
