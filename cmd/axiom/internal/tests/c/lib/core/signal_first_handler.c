/* A handler that is the first function whose address the program takes
   (on WebAssembly, table index 1, which once was SIG_IGN's value): raise
   calls it (C17 7.14p3: SIG_IGN compares unequal to every function's
   address), and SIG_IGN still ignores. */
/* expect: ignored */
/* expect-status: 7 */
#include <signal.h>
#include <stdlib.h>
#include "print.h"

static void handler(int sig) {
    _Exit(sig == SIGABRT ? 7 : 1);
}

int main(void) {
    signal(SIGABRT, handler);
    signal(SIGINT, SIG_IGN);
    raise(SIGINT);
    print_cstr(signal(SIGINT, SIG_DFL) == SIG_IGN && handler != SIG_IGN ? "ignored" : "wrong");
    abort();
}
