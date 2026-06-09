#include "user_matrix.h"

// Default (no-op) per-sweep post-hook. matrix.c calls this right after the
// sweep; a board overrides it in its own user_matrix.c to undo whatever
// scan_pre did (e.g. nuphy-air60 releases its columns to input/high-Z). Boards
// that keep their columns driven fall back here.
void user_matrix_scan_post(void) {}
