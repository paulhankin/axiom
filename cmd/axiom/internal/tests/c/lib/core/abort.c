/* abort (C17 7.22.4.1) raises SIGABRT: its handler runs; and one that
   returns does not stop the abnormal termination (the default action:
   killed by the signal on Linux and macOS, status 3 elsewhere). No atexit
   function runs. */
/* expect-signal: 6 */
#include <signal.h>
#include <stdlib.h>
#include "print.h"

static void handler(int sig) {
    print_i64(sig == SIGABRT);
}

static void not_called(void) {
    print_cstr("atexit: not called");
}

int main(void) {
    atexit(not_called);
    signal(SIGABRT, handler);
    print_cstr("abort"); /* expect: abort */
    abort();
    /* expect: 1 */
}
