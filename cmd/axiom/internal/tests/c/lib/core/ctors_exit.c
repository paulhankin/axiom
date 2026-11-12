/* The GNU dialects' constructors and destructors with the hosted library:
   the constructors run before main, by priority; exit (here called from a
   function main calls) runs the atexit functions, then the destructors
   (highest priority first), then flushes the streams, whose output the
   destructors may still add to. */
/* flags: --c-std=gnu17 */
#include <stdio.h>
#include <stdlib.h>

__attribute__((constructor(200))) static void second(void) { printf("constructor 200\n"); }
__attribute__((constructor(101))) static void first(void) { printf("constructor 101\n"); }
__attribute__((destructor(101))) static void last(void) { printf("destructor 101\n"); }
__attribute__((destructor(200))) static void early(void) { printf("destructor 200\n"); }

static void handler(void) { printf("atexit\n"); }

static void leave(void) {
    exit(4);
}

int main(void) {
    atexit(handler);
    printf("main\n");
    leave();
    return 0;
}
/* expect-status: 4 */
/* expect: constructor 101 */
/* expect: constructor 200 */
/* expect: main */
/* expect: atexit */
/* expect: destructor 200 */
/* expect: destructor 101 */
