#pragma once

#include <stdbool.h>

#ifdef SLEEP_ENABLE

void sleep_init(void);

void sleep_note_frame(bool frame_completed);

void sleep_note_activity(void);

void sleep_task(void);

#else

#    define sleep_init()           ((void)0)
#    define sleep_note_frame(done) ((void)(done))
#    define sleep_note_activity()  ((void)0)
#    define sleep_task()           ((void)0)

#endif
