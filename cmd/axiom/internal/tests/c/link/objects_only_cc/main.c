/* A C program of objects alone whose main is the system toolchain's
   object's (main is not this compiler's, and the start-up calls it as it
   calls a source's): arguments, atexit, the output flushed at exit, the exit
   status. */
/* main-object: cc */
/* flag-variants: -O2 | -O0 --c-lib=system */
#include <stdio.h>
#include <stdlib.h>

int ours_twelve(int x);

static void last(void) { puts(" (atexit)"); }

int main(int argc, char **argv) {
    if (atexit(last) != 0) return 1;
    printf("argc %d, argv[0] %s\n", argc, argv[0] ? "set" : "null");
    printf("value %d", ours_twelve(3));
    return 9;
}
/* expect: argc 1, argv[0] set */
/* expect: value 36 (atexit) */
/* expect-status: 9 */
