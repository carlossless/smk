#include "indicators.h"
#include "sh68f89.h"
#include "kbdef.h"
#include "settings.h"
#include "keyboard.h"
#include "led_effect.h"
#include "pca.h"

#define LED_ROWS MATRIX_ROWS
#define LED_COLS MATRIX_COLS

// rows are lit three at a time, so a column takes two subframes
#define LED_GROUP_ROWS 3
#define LED_GROUPS     2

// animation step per frame, indexed by user_settings.led_speed.
static const __code uint8_t led_speeds[] = {1, 2, 4, 8};
#define LED_SPEED_LEVELS (sizeof(led_speeds))

// duty is a real PWM level here, so brightness scales the colour rather than dithering frames.
static const __code uint8_t led_brightness_scale[LED_BRIGHTNESS_LEVELS] = {32, 80, 160, 255};

#include LED_GEOMETRY_HEADER
_Static_assert(LED_GEOMETRY_ROWS == LED_ROWS && LED_GEOMETRY_COLS == LED_COLS, "generated LED geometry size does not match the key matrix");

// one byte per colour per key, in the order the PCA channels want them: blue, green, red.
static __xdata uint8_t led_fb[LED_COLS][LED_ROWS][3];

static uint8_t led_col;
static uint8_t led_group;
static uint8_t led_phase;
static uint8_t regen_row;
static uint8_t regen_col;

// the nine compare channels, in the order the per-column table lists them: three rows of
// blue, green, red. Group 0 drives rows 0-2 and group 1 rows 3-5.
#define PCA_P3_PINS (uint8_t)(_P3_3 | _P3_4 | _P3_5 | _P3_7)
#define PCA_P4_PINS (uint8_t)(_P4_0 | _P4_2 | _P4_3)
#define PCA_P6_PINS (uint8_t)(_P6_0 | _P6_1)
#define LED_ENABLES (uint8_t)(_P4_4 | _P4_5)

// P7.0 gates the backlight supply and is active low; left high nothing lights at all.
#define LED_SUPPLY_P7_0 _P7_0

// the nine channels in the order the hardware lays them out: three consecutive rows, each
// blue, green then red.
static void pca_load(const __xdata uint8_t *bgr0, const __xdata uint8_t *bgr1, const __xdata uint8_t *bgr2)
{
    pca_hold();

    P1CPH0 = 0;
    P1CPL0 = bgr0[0];
    P1CPH1 = 0;
    P1CPL1 = bgr0[1];
    P1CPH2 = 0;
    P1CPL2 = bgr0[2];
    P2CPH0 = 0;
    P2CPL0 = bgr1[0];
    P2CPH1 = 0;
    P2CPL1 = bgr1[1];
    P3CPH0 = 0;
    P3CPL0 = bgr1[2];
    P3CPH1 = 0;
    P3CPL1 = bgr2[0];
    P0CPH1 = 0;
    P0CPL1 = bgr2[1];
    P0CPH0 = 0;
    P0CPL0 = bgr2[2];

    pca_release();
}

void indicators_init(void)
{
    sfr_page_0();
    P3CR |= PCA_P3_PINS;
    P3 = KB_P3_IDLE;
    P4CR |= (uint8_t)(PCA_P4_PINS | LED_ENABLES);
    kb_p4_shadow = KB_P4_IDLE;
    P4           = kb_p4_shadow;

    uint8_t saved_page = INSCON;
    sfr_page_1();

    P6CR |= PCA_P6_PINS;
    P6 &= (uint8_t)~PCA_P6_PINS;
    P7CR |= LED_SUPPLY_P7_0;
    P7 &= (uint8_t)~LED_SUPPLY_P7_0;

    pca_init();

    INSCON = saved_page;
}

void indicators_start(void)
{
    led_col   = 0;
    led_group = 0;
    led_phase = 0;
    regen_row = 0;
    regen_col = 0;

    user_settings.led_effect = FX_RADIAL;
    settings_load();
    if (user_settings.led_effect > FX_OFF) {
        user_settings.led_effect = FX_RADIAL;
    }
    if (user_settings.led_brightness >= LED_BRIGHTNESS_LEVELS) {
        user_settings.led_brightness = LED_BRIGHTNESS_LEVELS - 1;
    }
    if (user_settings.led_speed >= LED_SPEED_LEVELS) {
        user_settings.led_speed = 2;
    }
}

void indicators_next_effect(void)
{
    if (++user_settings.led_effect > FX_OFF) {
        user_settings.led_effect = 0;
    }
    settings_save();
}

void indicators_set_effect(uint8_t fx)
{
    if (fx > FX_OFF) {
        return;
    }
    user_settings.led_effect = fx;
    settings_save();
}

void indicators_step_brightness(bool up)
{
    if (up) {
        if (user_settings.led_brightness + 1u >= LED_BRIGHTNESS_LEVELS) {
            return;
        }
        user_settings.led_brightness++;
    } else {
        if (user_settings.led_brightness == 0) {
            return;
        }
        user_settings.led_brightness--;
    }
    settings_save();
}

void indicators_step_speed(bool up)
{
    if (up) {
        if (user_settings.led_speed + 1u >= LED_SPEED_LEVELS) {
            return;
        }
        user_settings.led_speed++;
    } else {
        if (user_settings.led_speed == 0) {
            return;
        }
        user_settings.led_speed--;
    }
    settings_save();
}

void indicators_factory_reset(void)
{
    user_settings.led_effect     = FX_RADIAL;
    user_settings.led_brightness = LED_BRIGHTNESS_LEVELS - 1;
    user_settings.led_speed      = 2;
    settings_save();
}

// Re-evaluating the effect is by far the most expensive thing here, so it runs from the
// main loop rather than the subframe interrupt: at a 0.25 ms subframe it does not fit in
// the interrupt, and overrunning it starves USB until the device stops enumerating.
static void led_regen_one(void)
{
    uint8_t rgb[3];
    if (led_effect_rgb((led_effect_t)user_settings.led_effect, regen_row, regen_col, led_phase, led_brightness_scale[user_settings.led_brightness], rgb)) {
        led_fb[regen_col][regen_row][0] = rgb[2]; // blue
        led_fb[regen_col][regen_row][1] = rgb[1]; // green
        led_fb[regen_col][regen_row][2] = rgb[0]; // red
    }

    if (++regen_col >= LED_COLS) {
        regen_col = 0;
        if (++regen_row >= LED_ROWS) {
            regen_row = 0;
            led_phase = (uint8_t)(led_phase + led_speeds[user_settings.led_speed]);
        }
    }
}

void indicators_pwm_disable(void)
{
    sfr_page_0();
    kb_p4_shadow &= (uint8_t)~LED_ENABLES;
    P4 = kb_p4_shadow;
}

static void led_drive(uint8_t col, uint8_t group)
{
    uint8_t mask = kb_col_masks[col];
    uint8_t p4   = (uint8_t)(KB_P4_IDLE | (group ? _P4_5 : _P4_4));

    sfr_page_0();
    P0 = (col < KB_C_P1_FIRST) ? (uint8_t)~mask : 0xFF;
    P1 = (col >= KB_C_P1_FIRST && col < KB_C_P4_FIRST) ? (uint8_t)~mask : 0xFF;
    if (col >= KB_C_P4_FIRST) {
        p4 &= (uint8_t)~mask;
    }

    kb_p4_shadow = p4;
    P4           = p4;
}

// deliberately not restoring the drive here: the next subframe tick is a quarter of a
// millisecond away and lights the *next* cell, whereas relighting the last one would give
// that single cell the whole scan slot on top of its own subframe and leave it visibly
// brighter than its neighbours.
void indicators_pwm_enable(void) {}

void indicators_pre_update(void)
{
    indicators_pwm_disable();
}

bool indicators_update_step(keyboard_state_t *keyboard, uint8_t current_step)
{
    (void)current_step;
    (void)keyboard;

    if (user_settings.led_effect >= FX_OFF) {
        return false;
    }

    const uint8_t base = (uint8_t)(led_group * LED_GROUP_ROWS);

    uint8_t saved_page = INSCON;
    sfr_page_1();
    pca_load(led_fb[led_col][base], led_fb[led_col][base + 1], led_fb[led_col][base + 2]);
    INSCON = saved_page;

    // one column low, then the group enable, so the drive settles before anything lights
    led_drive(led_col, led_group);

    bool wrapped = false;
    if (++led_group >= LED_GROUPS) {
        led_group = 0;
        if (++led_col >= LED_COLS) {
            led_col = 0;
            wrapped = true;
        }
    }

    return wrapped;
}

void indicators_post_update(void) {}

// HID keyboard output report bits, in the order the boot protocol defines them.
#define USB_LED_NUM_LOCK    (1u << 0)
#define USB_LED_CAPS_LOCK   (1u << 1)
#define USB_LED_SCROLL_LOCK (1u << 2)

// the lock LEDs are plain GPIO on P3 and sink, so they light on a low. P3 also carries four
// PCA channels, so it is never read back: a read samples live PWM on those pins and writing
// the value back latches the channel on.
void indicators_render(void)
{
    if (user_settings.led_effect < FX_OFF) {
        led_regen_one();
    }

    uint8_t state = keyboard_state.led_state;
    uint8_t low   = 0;

    if (state & USB_LED_NUM_LOCK) {
        low |= LED_NUM_P3_0;
    }
    if (state & USB_LED_CAPS_LOCK) {
        low |= LED_CAPS_P3_1;
    }
    if (state & USB_LED_SCROLL_LOCK) {
        low |= LED_SCROLL_P3_2;
    }

    uint8_t saved_page = INSCON;
    sfr_page_0();
    P3     = (uint8_t)(KB_P3_IDLE & ~low);
    INSCON = saved_page;
}
