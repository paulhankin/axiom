/* The order of a C program's start and end with an Axiom run-time layer
   (docs/c.md, "Start-up and exit"): the Axiom runtime starts first, an
   Axiom module's initialized data is there before main, and returning from
   main is calling exit (5.1.2.2.3) with its value, the program's own exit
   when it has one (here rt.ax's, as the hosted C library's will be), which
   runs the handlers in the reverse order of their registration and ends
   the process through the Axiom runtime. */
/* uses: ../print.ax */
/* expect-status: 3 */
#include "../print.h"

int at_exit(void (*)(void));
int rt_ready(void);
_Noreturn void exit(int);

static int calls;

static void first(void) { print_cstr("first registered, last run"); }
static void second(void) {
    print_cstr("second registered, first run");
    print_i64(calls);
}

int main(void) {
    print_i64(rt_ready()); /* expect: 1234 */
    at_exit(first);
    at_exit(second);
    calls = 2;
    print_cstr("main returns"); /* expect: main returns */
    return 3;
    /* expect: second registered, first run */
    /* expect: 2 */
    /* expect: first registered, last run */
    /* expect: exit */
    /* expect: 3 */
}
