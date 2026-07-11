#include "user_sleep.h"

#ifdef SLEEP_ENABLE

// Template board: no board-specific sleep support. A real board enables it by
// returning a mode here and filling in prepare()/wake() — see
// src/keyboards/nuphy-air60/user_sleep.c for a worked example.
user_sleep_mode_t user_sleep_supported(void)
{
    return USER_SLEEP_NONE;
}

void user_sleep_prepare(void) {}
void user_sleep_wake(void) {}

#endif // SLEEP_ENABLE
