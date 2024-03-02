#ifndef REPORT_H
#define REPORT_H

#include <stdint.h>
#include <stdbool.h>

#define KEYBOARD_REPORT_SIZE 8
#define KEYBOARD_REPORT_KEYS 6

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

extern __xdata report_keyboard_t keyboard_report;

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

#endif
