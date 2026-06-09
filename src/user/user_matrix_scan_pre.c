#include "user_matrix.h"

// Default (no-op) per-sweep pre-hook. matrix.c calls this right before it drives
// the columns; a board overrides it in its own user_matrix.c to do whatever
// column-pin setup it needs (e.g. nuphy-air60 switches its muxed columns to
// output). Boards whose columns are always outputs fall back here.
void user_matrix_scan_pre(void) {}
