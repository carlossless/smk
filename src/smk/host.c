#include "host.h"
#include "debug.h"
#include "kb.h"
#include "usb.h"

static __xdata uint16_t last_system_usage   = 0;
static __xdata uint16_t last_consumer_usage = 0;

void host_keyboard_send(__xdata report_keyboard_t *report)
{
    kb_send_report(report);
}

void host_nkro_send(__xdata report_nkro_t *report)
{
    kb_send_nkro(report);
}

void host_system_send(uint16_t usage)
{
    if (usage == last_system_usage) return;
    last_system_usage = usage;

    __xdata report_extra_t report = {
        .report_id = REPORT_ID_SYSTEM,
        .usage     = usage,
    };
    kb_send_extra(&report);
}

void host_consumer_send(uint16_t usage)
{
    if (usage == last_consumer_usage) return;
    last_consumer_usage = usage;

    __xdata report_extra_t report = {
        .report_id = REPORT_ID_CONSUMER,
        .usage     = usage,
    };
    kb_send_extra(&report);
}
