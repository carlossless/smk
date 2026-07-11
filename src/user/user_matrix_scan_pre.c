#include "user_matrix.h"

// Default no-op per-sweep pre-hook, for boards whose columns are always outputs.
void user_matrix_scan_pre(void) {}
