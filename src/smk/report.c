#include "report.h"
#include "host.h"
#include "layout.h"
#include "keycodes.h"
#include <string.h>
#include "kb.h"
#include "usb.h"
#include "keyboard.h"
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

/** \brief Send keyboard report
 *
 */
void send_keyboard_report()
{
#ifdef NKRO_ENABLE
    if (usb_device_state_get_protocol() == USB_PROTOCOL_REPORT && keymap_config.nkro) {
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

    /* Only send the report if there are changes to propagate to the host. */
    if (memcmp(&keyboard_report, &last_report, sizeof(report_keyboard_t)) != 0) {
        memcpy(&last_report, &keyboard_report, sizeof(report_keyboard_t));
        host_keyboard_send(&keyboard_report);
    }
}

#ifdef NKRO_ENABLE
void send_nkro_report()
{
    nkro_report.report_id = REPORT_ID_NKRO; // TODO: set this once
    nkro_report.mods      = real_mods;
    nkro_report.mods |= weak_mods;

    /* Only send the report if there are changes to propagate to the host. */
    if (memcmp(&nkro_report, &last_nkro_report, sizeof(report_nkro_t)) != 0) {
        memcpy(&last_nkro_report, &nkro_report, sizeof(report_nkro_t));
        host_nkro_send(&nkro_report);
    }
}
#endif

/** \brief has_anykey
 *
 */
uint8_t has_anykey(report_keyboard_t *keyboard_report)
{
    uint8_t  cnt = 0;
    uint8_t *p   = keyboard_report->keys;
    uint8_t  lp  = sizeof(keyboard_report->keys);

#ifdef NKRO_ENABLE
    if (usb_device_state_get_protocol() == USB_PROTOCOL_REPORT && keymap_config.nkro) {
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

/** \brief get_first_key
 *
 * FIXME: Needs doc
 */
uint8_t get_first_key(report_keyboard_t *keyboard_report)
{
#ifdef NKRO_ENABLE
    if (usb_device_state_get_protocol() == USB_PROTOCOL_REPORT && keymap_config.nkro) {
        uint8_t i = 0;
        for (; i < NKRO_REPORT_BITS && !nkro_report.bits[i]; i++)
            ;
        return i << 3 | biton(nkro_report.bits[i]);
    }
#endif
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

#ifdef NKRO_ENABLE
    if (usb_device_state_get_protocol() == USB_PROTOCOL_REPORT && keymap_config.nkro) {
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

#ifdef NKRO_ENABLE
/** \brief add key bit
 *
 * FIXME: Needs doc
 */
void add_key_bit(report_nkro_t *nkro_report, uint8_t code)
{
    if ((code >> 3) < NKRO_REPORT_BITS) {
        nkro_report->bits[code >> 3] |= 1 << (code & 7);
    } else {
        dprintf("add_key_bit: can't add: %02X\n", code);
    }
}

/** \brief del key bit
 *
 * FIXME: Needs doc
 */
void del_key_bit(report_nkro_t *nkro_report, uint8_t code)
{
    if ((code >> 3) < NKRO_REPORT_BITS) {
        nkro_report->bits[code >> 3] &= ~(1 << (code & 7));
    } else {
        dprintf("del_key_bit: can't del: %02X\n", code);
    }
}
#endif

/** \brief add key to report
 *
 */
void add_key_to_report(report_keyboard_t *keyboard_report, uint8_t key)
{
#ifdef NKRO_ENABLE
    if (usb_device_state_get_protocol() == USB_PROTOCOL_REPORT && keymap_config.nkro) {
        add_key_bit(&nkro_report, key);
        return;
    }
#endif
    add_key_byte(keyboard_report, key);
}

/** \brief del key from report
 *
 */
void del_key_from_report(report_keyboard_t *keyboard_report, uint8_t key)
{
#ifdef NKRO_ENABLE
    if (usb_device_state_get_protocol() == USB_PROTOCOL_REPORT && keymap_config.nkro) {
        del_key_bit(&nkro_report, key);
        return;
    }
#endif
    del_key_byte(keyboard_report, key);
}

/** \brief clear key from report
 *
 */
void clear_keys_from_report(report_keyboard_t *keyboard_report)
{
#ifdef NKRO_ENABLE
    if (usb_device_state_get_protocol() == USB_PROTOCOL_REPORT && keymap_config.nkro) {
        memset(nkro_report.bits, 0, sizeof(nkro_report.bits));
        return;
    }
#endif
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
