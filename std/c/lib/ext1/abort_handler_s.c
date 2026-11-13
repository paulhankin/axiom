/* abort_handler_s (C11 K.3.6.1.2): writes "runtime-constraint violation:
   " and msg, and a new-line, to the standard error stream, then calls
   abort. */
#define __STDC_WANT_LIB_EXT1__ 1
#include <stdio.h>
#include <stdlib.h>

void abort_handler_s(const char *__AXC_RESTRICT msg, void *__AXC_RESTRICT ptr, errno_t error) {
    (void)ptr;
    (void)error;
    fputs("runtime-constraint violation: ", stderr);
    fputs(msg != NULL ? msg : "", stderr);
    fputc('\n', stderr);
    abort();
}
