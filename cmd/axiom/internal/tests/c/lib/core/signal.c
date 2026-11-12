/* signal and raise (C17 7.14): signal returns the previous setting (SIG_DFL
   at first) or SIG_ERR for an unknown signal; raise calls the handler
   before it returns, with the signal's number, and the handler stays set
   (the library's choice: <signal.h>); SIG_IGN ignores. */
#include <errno.h>
#include <signal.h>
#include "print.h"

static volatile sig_atomic_t got, count;

static void handler(int sig) {
    got = sig;
    count++;
}

int main(void) {
    int sigs[6] = {SIGABRT, SIGFPE, SIGILL, SIGINT, SIGSEGV, SIGTERM};
    int i, ok = 1;
    print_bool(signal(SIGINT, handler) == SIG_DFL); /* expect: true */
    print_bool(signal(SIGINT, handler) == handler); /* expect: true */
    print_i64(raise(SIGINT)); /* expect: 0 */
    print_i64(got); /* expect: 2 */
    raise(SIGINT);
    print_i64(count); /* expect: 2 */
    print_bool(signal(SIGINT, SIG_IGN) == handler); /* expect: true */
    print_i64(raise(SIGINT)); /* expect: 0 */
    print_i64(count); /* expect: 2 */
    print_bool(signal(SIGINT, SIG_DFL) == SIG_IGN); /* expect: true */
    errno = 0;
    print_bool(signal(1000, handler) == SIG_ERR && errno > 0); /* expect: true */
    print_bool(signal(0, handler) == SIG_ERR); /* expect: true */
    print_bool(raise(1000) != 0); /* expect: true */
    for (i = 0; i < 6; i++) {
        count = 0;
        signal(sigs[i], handler);
        raise(sigs[i]);
        ok &= got == sigs[i] && count == 1;
        signal(sigs[i], SIG_DFL);
    }
    print_bool(ok); /* expect: true */
    return 0;
}
