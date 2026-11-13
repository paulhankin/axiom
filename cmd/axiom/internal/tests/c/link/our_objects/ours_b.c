#include <string.h>
#include <stdio.h>
__attribute__((constructor)) static void c(void) { puts("ctor b"); }
int ours_b(const char *s) { return (int)strlen(s); }
