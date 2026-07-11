#pragma once

#ifdef SLEEP_ENABLE

void sleep_init(void);

void sleep_tick(void);

void sleep_note_activity(void);

void sleep_task(void);

#else

#    define sleep_init()          ((void)0)
#    define sleep_tick()          ((void)0)
#    define sleep_note_activity() ((void)0)
#    define sleep_task()          ((void)0)

#endif
