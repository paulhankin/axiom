/* --c-lib=system: the functions the system's C library keeps in its static
   part rather than its shared object (atexit, at_quick_exit,
   pthread_atfork, which register with that object's own registries) link,
   in a program built from C sources alone, with or without -l libraries. */
/* flags: --c-lib=system --c-std=gnu17 */
/* flag-variants: -O0 | -O2 -lm */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

static void first(void) { puts("first registered, last run"); }
static void second(void) { puts("second registered, first run"); }
static void quick(void) { puts("quick"); }
static void prepare(void) {}

int main(int argc, char **argv) {
    (void)argv;
    if (atexit(first) != 0 || atexit(second) != 0) return 1;
    if (at_quick_exit(quick) != 0) return 2;
    if (pthread_atfork(prepare, prepare, prepare) != 0) return 3;
    puts("main");
    if (argc > 5) quick_exit(0);
    return 0;
}
/* expect: main */
/* expect: second registered, first run */
/* expect: first registered, last run */
