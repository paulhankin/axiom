/* A handler of SIGSEGV for a real fault, delivered by the system (it
   replaces the crash report while it is set; the handler leaves with
   longjmp, as it may not return from a fault: 7.14.1.1p3). The signal
   stays blocked after that (setjmp saves no signal mask), so the program
   does not fault again. */
/* linux-only */
#include <setjmp.h>
#include <signal.h>
#include "print.h"

static jmp_buf jb;
static volatile int *volatile bad = 0;

static void handler(int sig) {
    longjmp(jb, sig);
}

int main(void) {
    int r;
    signal(SIGSEGV, handler);
    r = setjmp(jb);
    if (r == 0) {
        *bad = 1;
        print_cstr("not reached");
    }
    print_i64(r); /* expect: 11 */
    return 0;
}
