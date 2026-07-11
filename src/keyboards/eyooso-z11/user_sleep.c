#include "user_sleep.h"

#ifdef SLEEP_ENABLE

// The eyooso-z11 is a wired (USB-only) board, so sleep would need to be
// USB-suspend driven (sleep when the host suspends the bus, wake on resume) —
// dropping a bus-active wired device into Power-Down on an inactivity timeout
// would stop it answering the host and break the link. That path isn't
// implemented for this board yet, so opt out: sleep compiles in but never fires.
user_sleep_mode_t user_sleep_supported(void)
{
    return USER_SLEEP_NONE;
}

void user_sleep_prepare(void) {}
void user_sleep_wake(void) {}

#endif // SLEEP_ENABLE
