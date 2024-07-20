#include "host.h"
#include "debug.h"
#include "kb.h"

void host_keyboard_send(report_keyboard_t *report)
{
    kb_send_report(report);
}
