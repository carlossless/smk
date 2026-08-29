#pragma once

#include <stdint.h>

// Generic backlight animation, shared by every keyboard. led_effect.c turns
// (effect, cell, phase) into an animated 0..255 index using the board's geometry
// (the user_led_* primitives in user_led.h). The keyboard then maps that index
// onto its pixels however it likes: an RGB board runs it through led_color_wheel()
// for a rainbow; a single-colour board uses the index directly as brightness.
// This keeps the effect maths in smk and leaves only the geometry + the pixel
// write-out to the board.
typedef enum {
    FX_RADIAL = 0, // rings radiating from the centre
    FX_HORIZONTAL, // wave across columns
    FX_VERTICAL,   // wave across rows
    FX_SOLID,      // static (no animation)
    FX_COUNT
} led_effect_t;

// The cycle key steps OFF -> each effect -> OFF -> ...; FX_OFF is the dark state.
#define FX_OFF FX_COUNT

// Animated per-cell index for `fx` at animation `phase`: HORIZONTAL uses the
// column hue, VERTICAL the row hue, RADIAL the radial distance - each + phase.
// Geometry comes from the user_led_* primitives. Only meaningful for the three
// animated effects; the caller handles FX_SOLID / FX_OFF itself.
uint8_t led_effect_index(led_effect_t fx, uint8_t row, uint8_t col, uint8_t phase);

// Full-saturation colour wheel: index (0..255, wrapping R -> B -> G -> R) to raw
// (un-dimmed) RGB in out[0..2]. For RGB boards; the caller applies brightness.
void led_color_wheel(uint8_t index, uint8_t out[3]);

// Final per-cell value for one backlight LED - the whole animation for the cell,
// so a board's render is just "call this, write the framebuffer". Both return
// false for FX_OFF (leave the cell unchanged), true otherwise.
//
//   led_effect_rgb:  RGB boards. Animated effects -> colour wheel; FX_SOLID ->
//                    white; all scaled by `brightness`. Writes out[0..2].
//   led_effect_mono: single-colour boards. Animated effects -> triangle wave of
//                    the index; FX_SOLID -> full. Writes *out (brightness applied
//                    by the board, e.g. in its PWM duty).
bool led_effect_rgb(led_effect_t fx, uint8_t row, uint8_t col, uint8_t phase, uint8_t brightness, uint8_t out[3]);
bool led_effect_mono(led_effect_t fx, uint8_t row, uint8_t col, uint8_t phase, uint8_t *out);
