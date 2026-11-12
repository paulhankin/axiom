/* assert (C17 7.2): a true assertion does nothing; NDEBUG, defined before
   <assert.h> is included again, makes it nothing at all; a false one
   writes the expression, the file, the line and the function to standard
   error and calls abort (here caught: the handler ends the program). */
/* expect-status: 7 */
/* expect-stderr: assert.c:34: main: assertion failed: x == 2 */
#include <assert.h>
#include <signal.h>
#include <stdlib.h>
#include "print.h"

static void handler(int sig) {
    print_cstr("SIGABRT");
    _Exit(sig == SIGABRT ? 7 : 1);
}

static int calls;

static int count(void) {
    return ++calls;
}

int main(void) {
    int x = 1;
    static_assert(sizeof(int) >= 2, "int");
    signal(SIGABRT, handler);
    assert(x == 1);
#define NDEBUG
#include <assert.h>
    assert(count() == 99);
#undef NDEBUG
#include <assert.h>
    print_i64(calls); /* expect: 0 */
    assert(x == 2);
    print_cstr("not reached");
    return 0;
    /* expect: SIGABRT */
}
