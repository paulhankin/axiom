/* SIG_DFL after a handler of SIGSEGV puts back the action the program
   started with: the crash report. */
/* linux-only */
/* expect-signal: 11 */
/* expect-stderr: segmentation fault */
#include <signal.h>
#include "print.h"

static volatile int *volatile bad = 0;

static void handler(int sig) {
    (void)sig;
    print_cstr("not called");
}

int main(void) {
    print_bool(signal(SIGSEGV, handler) == SIG_DFL); /* expect: true */
    print_bool(signal(SIGSEGV, SIG_DFL) == handler); /* expect: true */
    *bad = 2;
    return 0;
}
