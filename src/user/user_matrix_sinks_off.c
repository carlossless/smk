#include "user_matrix.h"

// Default (no-op) row-sink drop before a matrix sweep. matrix.c calls this
// unconditionally; boards with RGB row sinks override it in their own
// user_matrix.c, while boards without them (e.g. example) fall back here.
void user_matrix_sinks_off(void) {}
