/* quick_exit and at_quick_exit (C17 7.22.4.3, 7.22.4.7): the at_quick_exit
   functions in reverse order, no atexit functions; and _Exit (7.22.4.5),
   called by the last of them, ends the process at once. */
/* expect-status: 6 */
#include <stdlib.h>
#include "print.h"

static void not_called(void) {
    print_cstr("atexit: not called");
}

static void q1(void) {
    print_cstr("q1");
    _Exit(6);
}

static void q2(void) {
    print_cstr("q2");
}

int main(void) {
    atexit(not_called);
    print_i64(at_quick_exit(q1)); /* expect: 0 */
    at_quick_exit(q2);
    quick_exit(5);
    /* expect: q2 */
    /* expect: q1 */
}
