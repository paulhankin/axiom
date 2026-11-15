#include <stdio.h>

static int calls;

__attribute__((constructor)) static void init(void) { calls = 1; }

int util_value(void) { return 42; }
int util_calls(void) { return calls; }
