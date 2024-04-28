#include "report.h"
#include "host.h"
#include "layout.h"
#include <string.h>

static uint8_t real_mods = 0;
static uint8_t weak_mods = 0;

__xdata report_keyboard_t keyboard_report;
__xdata report_keyboard_t last_report;

/** \brief Send keyboard report
 *
 */
void send_keyboard_report()
{
    keyboard_report.mods = real_mods;
    keyboard_report.mods |= weak_mods;

    /* Only send the report if there are changes to propagate to the host. */
    if (memcmp(&keyboard_report, &last_report, sizeof(report_keyboard_t)) != 0) {
        memcpy(&last_report, &keyboard_report, sizeof(report_keyboard_t));
        host_keyboard_send(&keyboard_report);
    }
}

/** \brief has_anykey
 *
 */
uint8_t has_anykey(report_keyboard_t *keyboard_report)
{
    uint8_t  cnt = 0;
    uint8_t *p   = keyboard_report->keys;
    uint8_t  lp  = sizeof(keyboard_report->keys);

    while (lp--) {
        if (*p++) {
            cnt++;
        }
    }

    return cnt;
}

/** \brief get_first_key
 *
 * FIXME: Needs doc
 */
uint8_t get_first_key(report_keyboard_t *keyboard_report)
{
    return keyboard_report->keys[0];
}

/** \brief Checks if a key is pressed in the report
 *
 * Returns true if the keyboard_report reports that the key is pressed, otherwise false
 * Note: The function doesn't support modifers currently, and it returns false for KC_NO
 */
bool is_key_pressed(report_keyboard_t *keyboard_report, uint8_t key)
{
    if (key == KC_NO) {
        return false;
    }

    for (int i = 0; i < KEYBOARD_REPORT_KEYS; i++) {
        if (keyboard_report->keys[i] == key) {
            return true;
        }
    }

    return false;
}

/** \brief add key byte
 *
 */
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

/** \brief del key byte
 *
 */
void del_key_byte(report_keyboard_t *keyboard_report, uint8_t code)
{
    for (uint8_t i = 0; i < KEYBOARD_REPORT_KEYS; i++) {
        if (keyboard_report->keys[i] == code) {
            keyboard_report->keys[i] = 0;
        }
    }
}

/** \brief add key to report
 *
 */
void add_key_to_report(report_keyboard_t *keyboard_report, uint8_t key)
{
    add_key_byte(keyboard_report, key);
}

/** \brief del key from report
 *
 */
void del_key_from_report(report_keyboard_t *keyboard_report, uint8_t key)
{
    del_key_byte(keyboard_report, key);
}

/** \brief clear key from report
 *
 */
void clear_keys_from_report(report_keyboard_t *keyboard_report)
{
    memset(keyboard_report->keys, 0, sizeof(keyboard_report->keys));
}

/** \brief Get mods
 *
 */
uint8_t get_mods(void)
{
    return real_mods;
}
/** \brief add mods
 *
 */
void add_mods(uint8_t mods)
{
    real_mods |= mods;
}
/** \brief del mods
 *
 */
void del_mods(uint8_t mods)
{
    real_mods &= ~mods;
}
/** \brief set mods
 *
 */
void set_mods(uint8_t mods)
{
    real_mods = mods;
}
/** \brief clear mods
 *
 */
void clear_mods(void)
{
    real_mods = 0;
}

/** \brief get weak mods
 *
 */
uint8_t get_weak_mods(void)
{
    return weak_mods;
}
