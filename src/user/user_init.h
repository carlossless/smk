#pragma once

#include <stdint.h>

void user_init();

// Board GPIO setup, split out of user_init() so the sleep wake path can
// re-establish the normal operating pin configuration.
void user_gpio_init(void);
