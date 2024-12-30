#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "keycodes.h"

#define KEYBOARD_REPORT_SIZE 8
#define KEYBOARD_REPORT_KEYS 6

#define NKRO_REPORT_BITS 13
#define NKRO_REPORT_SIZE 2 + NKRO_REPORT_BITS

#define EXTRA_REPORT_SIZE 3

enum report_id {
    REPORT_ID_SYSTEM   = 1,
    REPORT_ID_CONSUMER = 2,
    REPORT_ID_ISP      = 5,
    REPORT_ID_NKRO     = 6,
};

/*
 * keyboard report is 8-byte array retains state of 8 modifiers and 6 keys.
 *
 * byte |0       |1       |2       |3       |4       |5       |6       |7
 * -----+--------+--------+--------+--------+--------+--------+--------+--------
 * desc |mods    |reserved|keys[0] |keys[1] |keys[2] |keys[3] |keys[4] |keys[5]
 *
 * mods retains state of 8 modifiers.
 *
 *  bit |0       |1       |2       |3       |4       |5       |6       |7
 * -----+--------+--------+--------+--------+--------+--------+--------+--------
 * desc |Lcontrol|Lshift  |Lalt    |Lgui    |Rcontrol|Rshift  |Ralt    |Rgui
 *
 */
typedef union {
    uint8_t raw[KEYBOARD_REPORT_SIZE];
    struct {
        uint8_t mods;
        uint8_t reserved;
        uint8_t keys[KEYBOARD_REPORT_KEYS];
    };
} report_keyboard_t;

typedef union {
    uint8_t raw[NKRO_REPORT_SIZE];
    struct {
        uint8_t report_id;
        uint8_t mods;
        uint8_t bits[NKRO_REPORT_BITS];
    };
} report_nkro_t;

typedef union {
    uint8_t raw[EXTRA_REPORT_SIZE];
    struct {
        uint8_t  report_id;
        uint16_t usage;
    };
} report_extra_t;

extern __xdata report_keyboard_t keyboard_report;
extern __xdata report_nkro_t     nkro_report;

void send_keyboard_report();

void add_key_to_report(report_keyboard_t *keyboard_report, uint8_t key);
void del_key_from_report(report_keyboard_t *keyboard_report, uint8_t key);
void clear_keys_from_report(report_keyboard_t *keyboard_report);

bool is_key_pressed(report_keyboard_t *keyboard_report, uint8_t key);

/* key */
inline void add_key(uint8_t key)
{
    add_key_to_report(&keyboard_report, key);
}

inline void del_key(uint8_t key)
{
    del_key_from_report(&keyboard_report, key);
}

inline void clear_keys(void)
{
    clear_keys_from_report(&keyboard_report);
}

/* modifiers */
uint8_t get_mods(void);
void    add_mods(uint8_t mods);
void    del_mods(uint8_t mods);
void    set_mods(uint8_t mods);
void    clear_mods(void);

/* Consumer Page (0x0C)
 *
 * See https://www.usb.org/sites/default/files/documents/hut1_12v2.pdf#page=75
 */
enum consumer_usages {
    // 15.5 Display Controls
    SNAPSHOT        = 0x065,
    BRIGHTNESS_UP   = 0x06F, // https://www.usb.org/sites/default/files/hutrr41_0.pdf
    BRIGHTNESS_DOWN = 0x070,
    // 15.7 Transport Controls
    TRANSPORT_RECORD       = 0x0B2,
    TRANSPORT_FAST_FORWARD = 0x0B3,
    TRANSPORT_REWIND       = 0x0B4,
    TRANSPORT_NEXT_TRACK   = 0x0B5,
    TRANSPORT_PREV_TRACK   = 0x0B6,
    TRANSPORT_STOP         = 0x0B7,
    TRANSPORT_EJECT        = 0x0B8,
    TRANSPORT_RANDOM_PLAY  = 0x0B9,
    TRANSPORT_STOP_EJECT   = 0x0CC,
    TRANSPORT_PLAY_PAUSE   = 0x0CD,
    // 15.9.1 Audio Controls - Volume
    AUDIO_MUTE     = 0x0E2,
    AUDIO_VOL_UP   = 0x0E9,
    AUDIO_VOL_DOWN = 0x0EA,
    // 15.15 Application Launch Buttons
    AL_CC_CONFIG       = 0x183,
    AL_EMAIL           = 0x18A,
    AL_CALCULATOR      = 0x192,
    AL_LOCAL_BROWSER   = 0x194,
    AL_LOCK            = 0x19E,
    AL_CONTROL_PANEL   = 0x19F,
    AL_ASSISTANT       = 0x1CB,
    AL_KEYBOARD_LAYOUT = 0x1AE,
    // 15.16 Generic GUI Application Controls
    AC_NEW                         = 0x201,
    AC_OPEN                        = 0x202,
    AC_CLOSE                       = 0x203,
    AC_EXIT                        = 0x204,
    AC_MAXIMIZE                    = 0x205,
    AC_MINIMIZE                    = 0x206,
    AC_SAVE                        = 0x207,
    AC_PRINT                       = 0x208,
    AC_PROPERTIES                  = 0x209,
    AC_UNDO                        = 0x21A,
    AC_COPY                        = 0x21B,
    AC_CUT                         = 0x21C,
    AC_PASTE                       = 0x21D,
    AC_SELECT_ALL                  = 0x21E,
    AC_FIND                        = 0x21F,
    AC_SEARCH                      = 0x221,
    AC_HOME                        = 0x223,
    AC_BACK                        = 0x224,
    AC_FORWARD                     = 0x225,
    AC_STOP                        = 0x226,
    AC_REFRESH                     = 0x227,
    AC_BOOKMARKS                   = 0x22A,
    AC_NEXT_KEYBOARD_LAYOUT_SELECT = 0x29D,
    AC_DESKTOP_SHOW_ALL_WINDOWS    = 0x29F,
    AC_SOFT_KEY_LEFT               = 0x2A0
};

/* Generic Desktop Page (0x01)
 *
 * See https://www.usb.org/sites/default/files/documents/hut1_12v2.pdf#page=26
 */
enum desktop_usages {
    // 4.5.1 System Controls - Power Controls
    SYSTEM_POWER_DOWN = 1 << 0,
    SYSTEM_SLEEP      = 1 << 1,
    SYSTEM_WAKE_UP    = 1 << 2,
};

/* keycode to system usage */
static inline uint16_t KEYCODE2SYSTEM(uint8_t key)
{
    switch (key) {
        case KC_SYSTEM_POWER:
            return SYSTEM_POWER_DOWN;
        case KC_SYSTEM_SLEEP:
            return SYSTEM_SLEEP;
        case KC_SYSTEM_WAKE:
            return SYSTEM_WAKE_UP;
        default:
            return 0;
    }
}

/* keycode to consumer usage */
static inline uint16_t KEYCODE2CONSUMER(uint8_t key)
{
    switch (key) {
        case KC_AUDIO_MUTE:
            return AUDIO_MUTE;
        case KC_AUDIO_VOL_UP:
            return AUDIO_VOL_UP;
        case KC_AUDIO_VOL_DOWN:
            return AUDIO_VOL_DOWN;
        case KC_MEDIA_NEXT_TRACK:
            return TRANSPORT_NEXT_TRACK;
        case KC_MEDIA_PREV_TRACK:
            return TRANSPORT_PREV_TRACK;
        case KC_MEDIA_FAST_FORWARD:
            return TRANSPORT_FAST_FORWARD;
        case KC_MEDIA_REWIND:
            return TRANSPORT_REWIND;
        case KC_MEDIA_STOP:
            return TRANSPORT_STOP;
        case KC_MEDIA_EJECT:
            return TRANSPORT_STOP_EJECT;
        case KC_MEDIA_PLAY_PAUSE:
            return TRANSPORT_PLAY_PAUSE;
        case KC_MEDIA_SELECT:
            return AL_CC_CONFIG;
        case KC_MAIL:
            return AL_EMAIL;
        case KC_CALCULATOR:
            return AL_CALCULATOR;
        case KC_MY_COMPUTER:
            return AL_LOCAL_BROWSER;
        case KC_CONTROL_PANEL:
            return AL_CONTROL_PANEL;
        case KC_ASSISTANT:
            return AL_ASSISTANT;
        case KC_WWW_SEARCH:
            return AC_SEARCH;
        case KC_WWW_HOME:
            return AC_HOME;
        case KC_WWW_BACK:
            return AC_BACK;
        case KC_WWW_FORWARD:
            return AC_FORWARD;
        case KC_WWW_STOP:
            return AC_STOP;
        case KC_WWW_REFRESH:
            return AC_REFRESH;
        case KC_BRIGHTNESS_UP:
            return BRIGHTNESS_UP;
        case KC_BRIGHTNESS_DOWN:
            return BRIGHTNESS_DOWN;
        case KC_WWW_FAVORITES:
            return AC_BOOKMARKS;
        case KC_MISSION_CONTROL:
            return AC_DESKTOP_SHOW_ALL_WINDOWS;
        case KC_LAUNCHPAD:
            return AC_SOFT_KEY_LEFT;
        default:
            return 0;
    }
}
