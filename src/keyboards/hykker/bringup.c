#include "sh68f881.h"
#include "clock.h"
#include "watchdog.h"
#include "isp.h"

// Bring-up image: take the board and hand it straight back to the bootloader, so
// running it can never strand the keyboard away from ISP.
void main(void)
{
    EA = 0;
    watchdog_kick();

    clock_init();

    watchdog_kick();
    isp_jump();
}
