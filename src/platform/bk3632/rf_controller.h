#ifndef RF_CONTROLLER_H
#define RF_CONTROLLER_H

#include "report.h"

void rf_init();
void rf_send_report(report_keyboard_t *report);
void rf_keep_alive();

#endif
