/* The default runtime-constraint handler is abort_handler_s (the
   implementation's choice, C11 K.3.6.1.1p3): it writes the message to
   standard error and calls abort. */
#define __STDC_WANT_LIB_EXT1__ 1
#include <string.h>
#include <stdio.h>

int main(void) {
    char a[4];
    printf("before\n");
    fflush(stdout);
    strcpy_s(a, sizeof a, "too long");
    printf("not reached\n");
    return 0;
}
// expect: before
// expect-signal: 6
// expect-stderr: runtime-constraint violation: strcpy_s: s2 does not fit in s1max characters
