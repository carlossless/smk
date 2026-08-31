#include "indicators.h"
#include "sh68f881.h"
#include "kbdef.h"
#include "settings.h"
#include "led_effect.h"
#include "user_led.h"

#define LED_ROWS MATRIX_ROWS
#define LED_COLS MATRIX_COLS

#define LED_PHASE_SPEED 4

// The columns are plain outputs here, not PWM channels, so a subframe can only light an
// LED or not. Anything at or above this shows.
#define LED_ON_THRESHOLD 0x80u

#include LED_GEOMETRY_HEADER
_Static_assert(LED_GEOMETRY_ROWS == LED_ROWS && LED_GEOMETRY_COLS == LED_COLS, "generated LED geometry size does not match the key matrix");
static __xdata uint8_t led_fb[LED_ROWS][LED_COLS];

static uint8_t led_col;
static uint8_t led_phase;
static uint8_t regen_row;
static uint8_t regen_col;

// One column low with up to six anodes high, as the stock firmware multiplexes it. The
// other way round would put all 24 LEDs of a row on a single anode pin.
//
// Both pages are forced, never assumed: this runs in the tick interrupt, which can
// preempt main-loop code that is inside a page-1 window - sending a key report is one.
// On page 1 the P1 anodes are P6, so an assumed page writes the column drive instead and
// leaves the anodes high, which clamps every row line.
static void led_blank(void)
{
    uint8_t saved_page = INSCON;

    sfr_page_0();
    P1 &= (uint8_t)~KB_ANODE_P1_MASK;

    sfr_page_1();
    P5 &= (uint8_t)~KB_ANODE_P5_MASK;
    P6 = 0xFFu;
    P7 = 0xFFu;
    P8 = 0xFFu;

    INSCON = saved_page;
}

static void led_drive(uint8_t col, uint8_t anodes)
{
    uint8_t saved_page = INSCON;

    sfr_page_1();
    P6 = (col < 8) ? (uint8_t) ~(1u << col) : 0xFFu;
    P7 = (col >= 8 && col < 16) ? (uint8_t) ~(1u << (col - 8)) : 0xFFu;
    P8 = (col >= 16) ? (uint8_t) ~(1u << (col - 16)) : 0xFFu;
    P5 = (uint8_t)((P5 & ~KB_ANODE_P5_MASK) | ((anodes >> 3) & KB_ANODE_P5_MASK));

    sfr_page_0();
    P1 = (uint8_t)((P1 & ~KB_ANODE_P1_MASK) | (anodes & KB_ANODE_P1_MASK));

    INSCON = saved_page;
}

// Exactly one effect evaluation per subframe. Six of them, one per row, does not fit in
// a subframe on this core and starves the USB interrupt.
static void led_regen_one(void)
{
    uint8_t v;
    if (led_effect_mono((led_effect_t)user_settings.led_effect, regen_row, regen_col, led_phase, &v)) {
        led_fb[regen_row][regen_col] = v;
    }

    if (++regen_col >= LED_COLS) {
        regen_col = 0;
        if (++regen_row >= LED_ROWS) {
            regen_row = 0;
            led_phase = (uint8_t)(led_phase + LED_PHASE_SPEED);
        }
    }
}

void indicators_start(void)
{
    led_col   = 0;
    led_phase = 0;
    regen_row = 0;
    regen_col = 0;

    user_settings.led_effect = FX_RADIAL;
    settings_load();
    if (user_settings.led_effect > FX_OFF) {
        user_settings.led_effect = FX_RADIAL;
    }
}

void indicators_next_effect(void)
{
    if (++user_settings.led_effect > FX_OFF) {
        user_settings.led_effect = 0;
    }

    settings_save();
}

void indicators_factory_reset(void)
{
    user_settings.led_effect = FX_RADIAL;
    settings_save();
}

void indicators_pre_update(void)
{
    led_blank();
}

bool indicators_update_step(keyboard_state_t *keyboard, uint8_t current_step)
{
    (void)current_step;
    (void)keyboard;

    if (user_settings.led_effect >= FX_OFF) {
        return false;
    }

    led_regen_one();

    uint8_t anodes = 0;
    for (uint8_t row = 0; row < LED_ROWS; row++) {
        if (led_fb[row][led_col] >= LED_ON_THRESHOLD) {
            anodes |= (uint8_t)(1u << row);
        }
    }
    led_drive(led_col, anodes);

    bool wrapped = false;
    if (++led_col >= LED_COLS) {
        led_col = 0;
        wrapped = true;
    }

    return wrapped;
}

void indicators_post_update(void)
{
}

// An anode left high forward-biases the backlight against whichever column the scan
// pulls low and clamps that row line, so the matrix would read every key as pressed.
void indicators_pwm_disable(void)
{
    led_blank();
}

void indicators_pwm_enable(void)
{
}
