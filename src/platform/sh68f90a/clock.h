#pragma once

void clock_init();

// Restart the HF oscillator + PLL after a Power-Down wake. Unlike clock_init()
// (boot path, where the oscillator is already in a known post-reset state): with
// interrupts masked, start HF+PLL, spin a fixed warm-up, then commit the PLL as
// SYSCLK — no PLLSTA poll. See clock.c for why the poll is unsafe on wake.
void clock_wake_restart();
