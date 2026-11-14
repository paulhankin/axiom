/* --c-lib=system under an ISO standard: the system's headers take their
   compiler for a GNU one in places, whatever the standard (on AArch64
   Linux <signal.h> reaches the kernel's headers, which write __signed__,
   __uint128_t and named variadic macro parameters): there those are
   accepted, as the dialect's are, while the program's own code is
   checked as always. */
/* flags: --c-lib=system --c-std=c11 */
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>

static volatile sig_atomic_t got;
static void on(int s) { got = s; }

int main(void) {
    signal(SIGINT, on);
    raise(SIGINT);
    printf("%d\n", got == SIGINT); /* expect: 1 */
    return 0;
}
