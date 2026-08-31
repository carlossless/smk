#include "interrupts.h"
#include "debug.h"
#include <stdint.h>

#define UNUSED_INTERRUPT_NONE 0xFF

static volatile __data uint8_t unused_interrupt_vector = UNUSED_INTERRUPT_NONE;

// Mask the source: a flag no handler clears would re-enter the vector until the watchdog fires.
#define UNUSED_INTERRUPT_DEFN(name, vector, ien, bit)              \
    void name##_unused_interrupt_handler(void) __interrupt(vector) \
    {                                                              \
        unused_interrupt_vector = vector;                          \
        ien &= ~(bit);                                             \
    }
UNUSED_INTERRUPTS(UNUSED_INTERRUPT_DEFN)
#undef UNUSED_INTERRUPT_DEFN

#if DEBUG == 1
void interrupts_task(void)
{
    if (unused_interrupt_vector == UNUSED_INTERRUPT_NONE) {
        return;
    }
    uint8_t vector          = unused_interrupt_vector;
    unused_interrupt_vector = UNUSED_INTERRUPT_NONE;
    dprintf("UNUSED INT %u, source disabled\r\n", vector);
}
#endif
