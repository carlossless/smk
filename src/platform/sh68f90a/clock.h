#pragma once

void clock_init();

// Restart the HF oscillator + PLL after a Power-Down wake: mask interrupts,
// start HF+PLL, spin a fixed warm-up, then commit the PLL as SYSCLK — no
// PLLSTA poll (see clock.c for why the poll is unsafe on wake).
void clock_wake_restart();
