/* exit flushes every stream that has buffered output and closes it (C17
   7.22.4.4p4), after the atexit functions, whose own output is flushed
   too (quick_exit.c: _Exit and quick_exit do not). */
#include <stdio.h>
#include <stdlib.h>

static void late(void) {
    printf("from atexit\n");
}

int main(void) {
    FILE *f = fopen("e.txt", "w");
    fputs("in a file", f);
    atexit(late);
    printf("buffered\n");
    exit(0);
}
// expect: buffered
// expect: from atexit
