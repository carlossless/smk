#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "keycodes.h"

#define KEYBOARD_REPORT_SIZE 8
#define KEYBOARD_REPORT_KEYS 6

#define NKRO_REPORT_BITS 20 // limited by wireless dongle hid descriptor
#define NKRO_REPORT_SIZE 2 + NKRO_REPORT_BITS

#define EXTRA_REPORT_SIZE 3

#define CONSOLE_REPORT_SIZE 32

enum report_id {
    REPORT_ID_SYSTEM   = 1,
    REPORT_ID_CONSUMER = 2,
    REPORT_ID_ISP      = 5,
    REPORT_ID_NKRO     = 6,
    REPORT_ID_CONSOLE  = 7,
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

extern report_keyboard_t keyboard_report;
extern report_nkro_t     nkro_report;

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
    SNAPSHOT                       = 0x065,
    BRIGHTNESS_UP                  = 0x06F, // https://www.usb.org/sites/default/files/hutrr41_0.pdf
    BRIGHTNESS_DOWN                = 0x070,
    TRANSPORT_RECORD               = 0x0B2,
    TRANSPORT_FAST_FORWARD         = 0x0B3,
    TRANSPORT_REWIND               = 0x0B4,
    TRANSPORT_NEXT_TRACK           = 0x0B5,
    TRANSPORT_PREV_TRACK           = 0x0B6,
    TRANSPORT_STOP                 = 0x0B7,
    TRANSPORT_EJECT                = 0x0B8,
    TRANSPORT_RANDOM_PLAY          = 0x0B9,
    TRANSPORT_STOP_EJECT           = 0x0CC,
    TRANSPORT_PLAY_PAUSE           = 0x0CD,
    AUDIO_MUTE                     = 0x0E2,
    AUDIO_VOL_UP                   = 0x0E9,
    AUDIO_VOL_DOWN                 = 0x0EA,
    AL_CC_CONFIG                   = 0x183,
    AL_EMAIL                       = 0x18A,
    AL_CALCULATOR                  = 0x192,
    AL_LOCAL_BROWSER               = 0x194,
    AL_LOCK                        = 0x19E,
    AL_CONTROL_PANEL               = 0x19F,
    AL_ASSISTANT                   = 0x1CB,
    AL_KEYBOARD_LAYOUT             = 0x1AE,
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
    SYSTEM_POWER_DOWN = 1 << 0,
    SYSTEM_SLEEP      = 1 << 1,
    SYSTEM_WAKE_UP    = 1 << 2,
};

/* keycode to system usage */
uint16_t keycode_to_system(uint8_t key);

/* keycode to consumer usage */
uint16_t keycode_to_consumer(uint8_t key);
