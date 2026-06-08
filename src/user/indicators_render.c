#include "indicators.h"

// Default: boards with no foreground LED render do nothing here. Boards that
// render their effect frame in the main loop (e.g. nuphy-air60) override it.
void indicators_render() {}
