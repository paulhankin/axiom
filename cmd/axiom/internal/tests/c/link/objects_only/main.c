/* A C program of objects alone (axiom build main.o util.o): the build adds
   the C start-up and the C library (--c-lib) as it does for C sources. main
   is this compiler's object's (--emit=obj), with its arguments, its atexit
   function, the standard output flushed at exit (a line without its
   newline), and its value the exit status. */
/* main-object: ours */
/* flag-variants: -O0 | -O2 | -O2 --c-lib=system */
#include <stdio.h>
#include <stdlib.h>

int ours_twelve(int x);

static void last(void) { puts(" (atexit)"); }

int main(int argc, char **argv) {
    if (atexit(last) != 0) return 1;
    printf("argc %d, argv[0] %s, argv[argc] %s\n", argc, argv[0] ? "set" : "null", argv[argc] ? "set" : "null");
    printf("value %d", ours_twelve(4));
    return 7;
}
/* expect: argc 1, argv[0] set, argv[argc] null */
/* expect: value 48 (atexit) */
/* expect-status: 7 */
