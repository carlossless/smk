#ifndef USER_LAYOUT_H
#define USER_LAYOUT_H

#include "keycodes.h"
#include <stdint.h>
#include <stdbool.h>

/*
  returns a boolean value to instruct whether further key process should continue or end here
*/
bool layout_process_record(uint16_t keycode, bool key_pressed);

#endif
