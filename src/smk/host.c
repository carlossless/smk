#include "host.h"
#include "kb.h"
#include "keyboard.h"
#include "usb.h"

// System and consumer usages are level-triggered: the host holds the last usage
// until we send a different one, so a repeat send is redundant traffic.
static __xdata uint16_t last_system_usage;
static __xdata uint16_t last_consumer_usage;

static __xdata report_extra_t extra_report;

bool host_nkro_active(void)
{
    return usb_device_state_get_protocol() == USB_PROTOCOL_REPORT && keymap_config.nkro;
}

void host_keyboard_send(__xdata report_keyboard_t *report)
{
    kb_send_report(report);
}

void host_nkro_send(__xdata report_nkro_t *report)
{
    kb_send_nkro(report);
}

static void host_extra_send(uint8_t report_id, uint16_t usage)
{
    extra_report.report_id = report_id;
    extra_report.usage     = usage;

    kb_send_extra(&extra_report);
}

void host_system_send(uint16_t usage)
{
    if (usage == last_system_usage) return;
    last_system_usage = usage;

    host_extra_send(REPORT_ID_SYSTEM, usage);
}

void host_consumer_send(uint16_t usage)
{
    if (usage == last_consumer_usage) return;
    last_consumer_usage = usage;

    host_extra_send(REPORT_ID_CONSUMER, usage);
}
