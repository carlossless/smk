#include "report.h"
#include "host.h"
#include "layout.h"
#include "keycodes.h"
#include <string.h>
#include "kb.h"
#include "debug.h"

static uint8_t real_mods = 0;
static uint8_t weak_mods = 0;

__xdata report_keyboard_t keyboard_report;
__xdata report_keyboard_t last_report;

__xdata report_nkro_t nkro_report;
__xdata report_nkro_t last_nkro_report;

uint8_t biton(uint8_t bits);

void send_6kro_report();
#ifdef NKRO_ENABLE
void send_nkro_report();
#endif

void send_keyboard_report()
{
#ifdef NKRO_ENABLE
    if (host_nkro_active()) {
        send_nkro_report();
    } else {
        send_6kro_report();
    }
#else
    send_6kro_report();
#endif
}

void send_6kro_report()
{
    keyboard_report.mods = real_mods;
    keyboard_report.mods |= weak_mods;

    if (memcmp(&keyboard_report, &last_report, sizeof(report_keyboard_t)) != 0) {
        for (uint8_t i = 0; i < KEYBOARD_REPORT_SIZE; i++) {
            last_report.raw[i] = keyboard_report.raw[i];
        }
        host_keyboard_send(&keyboard_report);
    }
}

#ifdef NKRO_ENABLE
void send_nkro_report()
{
    nkro_report.report_id = REPORT_ID_NKRO; // TODO: set this more permanently
    nkro_report.mods      = real_mods;
    nkro_report.mods |= weak_mods;

    if (memcmp(&nkro_report, &last_nkro_report, sizeof(report_nkro_t)) != 0) {
        for (uint8_t i = 0; i < NKRO_REPORT_SIZE; i++) {
            last_nkro_report.raw[i] = nkro_report.raw[i];
        }
        host_nkro_send(&nkro_report);
    }
}
#endif

uint8_t has_anykey(report_keyboard_t *keyboard_report)
{
    uint8_t  cnt = 0;
    uint8_t *p   = keyboard_report->keys;
    uint8_t  lp  = sizeof(keyboard_report->keys);

#ifdef NKRO_ENABLE
    if (host_nkro_active()) {
        p  = nkro_report.bits;
        lp = sizeof(nkro_report.bits);
    }
#endif

    while (lp--) {
        if (*p++) {
            cnt++;
        }
    }

    return cnt;
}

uint8_t get_first_key(report_keyboard_t *keyboard_report)
{
#ifdef NKRO_ENABLE
    if (host_nkro_active()) {
        uint8_t i = 0;
        for (; i < NKRO_REPORT_BITS && !nkro_report.bits[i]; i++)
            ;
        return i << 3 | biton(nkro_report.bits[i]);
    }
#endif
    return keyboard_report->keys[0];
}

bool is_key_pressed(report_keyboard_t *keyboard_report, uint8_t key)
{
    if (key == KC_NO) {
        return false;
    }

#ifdef NKRO_ENABLE
    if (host_nkro_active()) {
        if ((key >> 3) < NKRO_REPORT_BITS) {
            return nkro_report.bits[key >> 3] & 1 << (key & 7);
        } else {
            return false;
        }
    }
#endif

    for (int i = 0; i < KEYBOARD_REPORT_KEYS; i++) {
        if (keyboard_report->keys[i] == key) {
            return true;
        }
    }

    return false;
}

void add_key_byte(report_keyboard_t *keyboard_report, uint8_t code)
{
    int8_t i     = 0;
    int8_t empty = -1;

    for (; i < KEYBOARD_REPORT_KEYS; i++) {
        if (keyboard_report->keys[i] == code) {
            break;
        }

        if (empty == -1 && keyboard_report->keys[i] == 0) {
            empty = i;
        }
    }

    if (i == KEYBOARD_REPORT_KEYS) {
        if (empty != -1) {
            keyboard_report->keys[empty] = code;
        }
    }
}

void del_key_byte(report_keyboard_t *keyboard_report, uint8_t code)
{
    for (uint8_t i = 0; i < KEYBOARD_REPORT_KEYS; i++) {
        if (keyboard_report->keys[i] == code) {
            keyboard_report->keys[i] = 0;
        }
    }
}

#ifdef NKRO_ENABLE
void add_key_bit(report_nkro_t *nkro_report, uint8_t code)
{
    if ((code >> 3) < NKRO_REPORT_BITS) {
        nkro_report->bits[code >> 3] |= 1 << (code & 7);
    }
}

void del_key_bit(report_nkro_t *nkro_report, uint8_t code)
{
    if ((code >> 3) < NKRO_REPORT_BITS) {
        nkro_report->bits[code >> 3] &= ~(1 << (code & 7));
    }
}
#endif

void add_key_to_report(report_keyboard_t *keyboard_report, uint8_t key)
{
#ifdef NKRO_ENABLE
    if (host_nkro_active()) {
        add_key_bit(&nkro_report, key);
        return;
    }
#endif
    add_key_byte(keyboard_report, key);
}

void del_key_from_report(report_keyboard_t *keyboard_report, uint8_t key)
{
#ifdef NKRO_ENABLE
    if (host_nkro_active()) {
        del_key_bit(&nkro_report, key);
        return;
    }
#endif
    del_key_byte(keyboard_report, key);
}

void clear_keys_from_report(report_keyboard_t *keyboard_report)
{
#ifdef NKRO_ENABLE
    if (host_nkro_active()) {
        memset(nkro_report.bits, 0, sizeof(nkro_report.bits));
        return;
    }
#endif
    memset(keyboard_report->keys, 0, sizeof(keyboard_report->keys));
}

uint8_t get_mods(void)
{
    return real_mods;
}
void add_mods(uint8_t mods)
{
    real_mods |= mods;
}
void del_mods(uint8_t mods)
{
    real_mods &= ~mods;
}
void set_mods(uint8_t mods)
{
    real_mods = mods;
}
void clear_mods(void)
{
    real_mods = 0;
}

uint8_t get_weak_mods(void)
{
    return weak_mods;
}

// most significant on-bit - return highest location of on-bit
// NOTE: return 0 when bit0 is on or all bits are off
uint8_t biton(uint8_t bits)
{
    uint8_t n = 0;
    if (bits >> 4) {
        bits >>= 4;
        n += 4;
    }
    if (bits >> 2) {
        bits >>= 2;
        n += 2;
    }
    if (bits >> 1) {
        bits >>= 1;
        n += 1;
    }
    return n;
}

uint16_t keycode_to_system(uint8_t key)
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

uint16_t keycode_to_consumer(uint8_t key)
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
