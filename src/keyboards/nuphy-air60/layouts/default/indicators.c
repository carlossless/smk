#include "indicators.h"
#include "kbdef.h"
#include "pwm.h"
#include "settings.h"

// TODO: move these defines out
#define PWM_CLK_DIV         0b010 // PWM_CLK = SYS_CLK / 4
#define PWM_SS_BIT          (1 << 3)
#define PWM_MOD_BIT         (1 << 4)
#define PWM_INT_ENABLE_BIT  (1 << 6)
#define PWM_MODE_ENABLE_BIT (1 << 7)

// The LED matrix follows the key matrix dimensions, so a differently-sized RGB board
// only needs the right MATRIX_ROWS/MATRIX_COLS (kbdef.h), its geometry parameters in
// meson.build, plus its own sink/column wiring below.
#define LED_ROWS MATRIX_ROWS
#define LED_COLS MATRIX_COLS

// The underglow ("user") LEDs are wired as an extra sink group, scanned as one more
// row after the key matrix. They mirror the bottom key row so they animate together
// with whatever effect is active.
#define LED_UL_ROW    LED_ROWS
#define LED_SCAN_ROWS (LED_ROWS + 1)

// Hue increment applied once per full framebuffer regeneration sweep (~73 Hz).
// Larger = faster animation. A full 256-step cycle takes 256 / (73 * SPEED) seconds.
#define LED_PHASE_SPEED 4

// 8-bit framebuffer brightness -> 10-bit column PWM duty (0 = full on, 0x400 = off)
#define LED_DUTY(v) (uint16_t)(0x0400u - ((uint16_t)(v) << 2))

typedef enum {
    FX_RADIAL = 0, // rainbow rings radiating from the centre
    FX_HORIZONTAL, // rainbow sweeping across columns
    FX_VERTICAL,   // rainbow sweeping across rows
    FX_SOLID,      // static solid white (no animation)
    FX_COUNT
} led_effect_t;

// The cycle key steps OFF -> each effect -> OFF -> ...; FX_OFF is the dark state.
#define FX_OFF FX_COUNT

// Per-device geometry, baked into ROM at build time by utils/led_geometry_gen.c and
// included here as const __code tables (radial_index, col_hue, row_hue). Costs no RAM
// and no runtime computation. The generated header is selected per board via meson.
#include LED_GEOMETRY_HEADER
_Static_assert(LED_GEOMETRY_ROWS == LED_ROWS && LED_GEOMETRY_COLS == LED_COLS,
               "generated LED geometry size does not match the key matrix");

// Per-LED RGB framebuffer, indexed [row][color][col] (color: 0=R, 1=G, 2=B).
// Kept entirely in __xdata so the scarce internal RAM stays untouched.
static __xdata uint8_t led_fb[LED_ROWS][3][LED_COLS];

// LED scan cursor, advanced one (row,color) substep per PWM ISR. Decoupled from
// the key-matrix column scan (current_step). Also __xdata to avoid internal RAM.
static __xdata uint8_t led_row;
static __xdata uint8_t led_color;

// Animation state. The framebuffer is regenerated incrementally one LED per ISR
// (regen_row/regen_col); led_phase shifts the rainbow once per sweep. The active
// effect lives in persistent user_settings.led_effect.
static __xdata uint8_t led_phase;
static __xdata uint8_t regen_row;
static __xdata uint8_t regen_col;

void indicators_pwm_enable();
void indicators_pwm_disable();
static void led_regen_one();
static void led_enable_sink();
static void led_set_columns();

void indicators_start()
{
    led_row   = 0;
    led_color = 0;

    led_phase = 0;
    regen_row = 0;
    regen_col = 0;

    // restore the last saved effect (defaults to off on a fresh flash)
    user_settings.led_effect = FX_OFF;
    settings_load();
    if (user_settings.led_effect > FX_OFF) {
        user_settings.led_effect = FX_OFF;
    }

    indicators_pwm_enable();
}

void indicators_cycle_effect()
{
    // OFF -> FX_RADIAL -> ... -> FX_SOLID -> OFF -> ...
    if (++user_settings.led_effect > FX_OFF) {
        user_settings.led_effect = 0;
    }

    settings_save();
}

void indicators_pre_update()
{
    // set all rgb sinks to low (animation step will enable needed ones)
    P0 &= ~(RGB_R2R_P0_2 | RGB_R0B_P0_3 | RGB_R0R_P0_4);
    P1 &= ~(RGB_ULR_P1_1 | RGB_ULG_P1_2 | RGB_ULB_P1_3);
    P4 &= ~(RGB_R4B_P4_3 | RGB_R4R_P4_4 | RGB_R3R_P4_5 | RGB_R3B_P4_6);
    P5 &= ~(RGB_R2B_P5_7);
    P6 &= ~(RGB_R0G_P6_1 | RGB_R1G_P6_2 | RGB_R2G_P6_3 | RGB_R3G_P6_4 | RGB_R4G_P6_5 | RGB_R1B_P6_6 | RGB_R1R_P6_7);

    indicators_pwm_disable();
}

bool indicators_update_step(keyboard_state_t *keyboard, uint8_t current_step)
{
    keyboard;
    current_step;

    if (user_settings.led_effect >= FX_OFF) {
        // backlight off: pre_update already cleared every sink, so nothing lights
        return false;
    }

    // Regenerate one LED of the framebuffer per ISR (cheap), spreading the rainbow
    // computation over many interrupts instead of one expensive burst.
    led_regen_one();

    // Light exactly one (row, color) this substep, with per-column brightness from
    // the framebuffer. A full frame is LED_SCAN_ROWS * 3 substeps (key rows + underglow).
    led_enable_sink();
    led_set_columns();

    if (++led_color >= 3) {
        led_color = 0;
        if (++led_row >= LED_SCAN_ROWS) {
            led_row = 0;
        }
    }

    return false;
}

void indicators_post_update()
{
    // clear pwm isr flag
    PWM00CON &= ~(1 << 5);

    indicators_pwm_enable();
}

static void led_regen_one()
{
    if (user_settings.led_effect == FX_SOLID) {
        // static white, no animation
        led_fb[regen_row][0][regen_col] = 255;
        led_fb[regen_row][1][regen_col] = 255;
        led_fb[regen_row][2][regen_col] = 255;
    } else {
        uint8_t h;

        // hue source depends on the active effect; phase animates all of them
        switch (user_settings.led_effect) {
            case FX_HORIZONTAL:
                h = (uint8_t)(col_hue[regen_col] + led_phase);
                break;
            case FX_VERTICAL:
                h = (uint8_t)(row_hue[regen_row] + led_phase);
                break;
            case FX_RADIAL:
            default:
                h = (uint8_t)(radial_index[regen_row][regen_col] + led_phase);
                break;
        }

        // Full-saturation colour wheel (h: 0..255 -> R->B->G->R). S = V = max.
        if (h < 85) {
            led_fb[regen_row][0][regen_col] = (uint8_t)(255 - h * 3);
            led_fb[regen_row][1][regen_col] = 0;
            led_fb[regen_row][2][regen_col] = (uint8_t)(h * 3);
        } else if (h < 170) {
            h                               = (uint8_t)(h - 85);
            led_fb[regen_row][0][regen_col] = 0;
            led_fb[regen_row][1][regen_col] = (uint8_t)(h * 3);
            led_fb[regen_row][2][regen_col] = (uint8_t)(255 - h * 3);
        } else {
            h                               = (uint8_t)(h - 170);
            led_fb[regen_row][0][regen_col] = (uint8_t)(h * 3);
            led_fb[regen_row][1][regen_col] = (uint8_t)(255 - h * 3);
            led_fb[regen_row][2][regen_col] = 0;
        }
    }

    if (++regen_col >= LED_COLS) {
        regen_col = 0;
        if (++regen_row >= LED_ROWS) {
            regen_row = 0;
            led_phase = (uint8_t)(led_phase + LED_PHASE_SPEED);
        }
    }
}

static void led_enable_sink()
{
    switch (led_row) {
        case 0:
            if (led_color == 0) RGB_R0R = 1;
            else if (led_color == 1) RGB_R0G = 1;
            else RGB_R0B = 1;
            break;
        case 1:
            if (led_color == 0) RGB_R1R = 1;
            else if (led_color == 1) RGB_R1G = 1;
            else RGB_R1B = 1;
            break;
        case 2:
            if (led_color == 0) RGB_R2R = 1;
            else if (led_color == 1) RGB_R2G = 1;
            else RGB_R2B = 1;
            break;
        case 3:
            if (led_color == 0) RGB_R3R = 1;
            else if (led_color == 1) RGB_R3G = 1;
            else RGB_R3B = 1;
            break;
        case 4:
            if (led_color == 0) RGB_R4R = 1;
            else if (led_color == 1) RGB_R4G = 1;
            else RGB_R4B = 1;
            break;
        case LED_UL_ROW:
            if (led_color == 0) RGB_ULR = 1;
            else if (led_color == 1) RGB_ULG = 1;
            else RGB_ULB = 1;
            break;
    }
}

static void led_set_columns()
{
    // the underglow row has no framebuffer of its own; it mirrors the bottom key row
    uint8_t          src = (led_row == LED_UL_ROW) ? (LED_ROWS - 1) : led_row;
    __xdata uint8_t *fb  = led_fb[src][led_color];

    SET_PWM_DUTY_2(LED_PWM_C0, LED_DUTY(fb[0]));
    SET_PWM_DUTY_2(LED_PWM_C1, LED_DUTY(fb[1]));
    SET_PWM_DUTY_2(LED_PWM_C2, LED_DUTY(fb[2]));
    SET_PWM_DUTY_2(LED_PWM_C3, LED_DUTY(fb[3]));
    SET_PWM_DUTY_2(LED_PWM_C4, LED_DUTY(fb[4]));
    SET_PWM_DUTY_2(LED_PWM_C5, LED_DUTY(fb[5]));
    SET_PWM_DUTY_2(LED_PWM_C6, LED_DUTY(fb[6]));
    SET_PWM_DUTY_2(LED_PWM_C7, LED_DUTY(fb[7]));
    SET_PWM_DUTY_2(LED_PWM_C8, LED_DUTY(fb[8]));
    SET_PWM_DUTY_2(LED_PWM_C9, LED_DUTY(fb[9]));
    SET_PWM_DUTY_2(LED_PWM_C10, LED_DUTY(fb[10]));
    SET_PWM_DUTY_2(LED_PWM_C11, LED_DUTY(fb[11]));
    SET_PWM_DUTY_2(LED_PWM_C12, LED_DUTY(fb[12]));
    SET_PWM_DUTY_2(LED_PWM_C13, LED_DUTY(fb[13]));
    SET_PWM_DUTY_2(LED_PWM_C14, LED_DUTY(fb[14]));
    SET_PWM_DUTY_2(LED_PWM_C15, LED_DUTY(fb[15]));
}

void indicators_pwm_enable()
{
    // TODO: try abstracting individual banks away
    PWM00CON = (uint8_t)(PWM_MODE_ENABLE_BIT | PWM_INT_ENABLE_BIT | PWM_SS_BIT | PWM_CLK_DIV);
    PWM01CON = PWM_SS_BIT;
    PWM02CON = PWM_SS_BIT;
    PWM03CON = PWM_SS_BIT;
    PWM04CON = PWM_SS_BIT;
    PWM05CON = PWM_SS_BIT;

    PWM10CON = (uint8_t)(PWM_MODE_ENABLE_BIT | PWM_SS_BIT | PWM_CLK_DIV);
    PWM11CON = PWM_SS_BIT;
    PWM12CON = PWM_SS_BIT;
    PWM13CON = PWM_SS_BIT;
    PWM14CON = PWM_SS_BIT;
    PWM15CON = PWM_SS_BIT;

    PWM20CON = (uint8_t)(PWM_MODE_ENABLE_BIT | PWM_SS_BIT | PWM_CLK_DIV);
    // PWM24CON = PWM_SS_BIT;
    PWM25CON = PWM_SS_BIT;

    PWM40CON = (uint8_t)(PWM_MODE_ENABLE_BIT | PWM_SS_BIT | PWM_CLK_DIV);
    PWM41CON = PWM_SS_BIT;
    PWM42CON = PWM_SS_BIT;
}

void indicators_pwm_disable()
{
    // TODO: try abstracting individual banks away
    PWM00CON = (uint8_t)(PWM_CLK_DIV);
    PWM01CON = 0;
    PWM02CON = 0;
    PWM03CON = 0;
    PWM04CON = 0;
    PWM05CON = 0;

    PWM10CON = (uint8_t)(PWM_CLK_DIV);
    PWM11CON = 0;
    PWM12CON = 0;
    PWM13CON = 0;
    PWM14CON = 0;
    PWM15CON = 0;

    PWM20CON = (uint8_t)(PWM_CLK_DIV);
    // PWM24CON = 0;
    PWM25CON = 0;

    PWM40CON = (uint8_t)(PWM_CLK_DIV);
    PWM41CON = 0;
    PWM42CON = 0;
}
