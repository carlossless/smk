// Reading the factory information block, which is what FLASHCON.FAC swaps into the address
// space MOVC sees. Read-only on purpose: flash_program_from() and flash_erase() reach the same
// window, and an erase aimed at the wrong sector takes out the running code.

#include "clock.h"
#include "delay.h"
#include "flash.h"
#include "ldo.h"
#include "peripherals.h"
#include "watchdog.h"
#include <stdbool.h>

#define INFO_PART_NUMBER 0x1209u
#define INFO_ID_CODE     0x127Bu
#define INFO_LEN         5u

static __xdata uint8_t part_number[INFO_LEN];
static __xdata uint8_t id_code[INFO_LEN];

void main(void)
{
    ldo_init();
    clock_init();
    peripherals_init();

    flash_read_into(FLASH_DATA, INFO_PART_NUMBER, part_number, INFO_LEN);
    flash_read_into(FLASH_DATA, INFO_ID_CODE, id_code, INFO_LEN);

    // reading it back through the comparison path instead, which is what the settings store
    // uses to decide whether a write is needed at all
    bool unchanged = flash_matches(FLASH_DATA, INFO_PART_NUMBER, part_number, INFO_LEN);
    (void)unchanged;

    for (;;) {
        watchdog_kick();
        delay_ms(1000);
    }
}
