/* exit called again by an atexit handler (undefined, 7.22.4.4p2): the
   library does not hang or call a handler twice; the remaining handlers
   run and the process ends with the later status. */
/* expect-status: 4 */
#include <stdlib.h>
#include "print.h"

static void first(void) {
    print_cstr("first registered");
}

static void again(void) {
    print_cstr("calls exit(4)");
    exit(4);
}

static void second(void) {
    print_cstr("second registered");
}

int main(void) {
    atexit(first);
    atexit(again);
    atexit(second);
    exit(2);
    /* expect: second registered */
    /* expect: calls exit(4) */
    /* expect: first registered */
}
