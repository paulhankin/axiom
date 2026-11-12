/* signal (<signal.h>, C23 7.14.1.1): sets how signal sig is handled: SIG_DFL
   (the default action), SIG_IGN (ignored) or a function to call, which then
   stays set while it runs (the signal waits until it returns). Returns the
   previous setting, or SIG_ERR (errno EINVAL) for a number that is not one
   of the six signals of <signal.h>. The system's part is
   std/c/rt/core_signal.ax. */
#include <errno.h>
#include <signal.h>
#include "axc/core_impl.h"

static int known(int sig) {
    return sig == SIGINT || sig == SIGILL || sig == SIGABRT || sig == SIGFPE || sig == SIGSEGV || sig == SIGTERM;
}

void (*signal(int sig, void (*func)(int)))(int) {
    void (*old)(int);
    int action = 2;
    if (!known(sig)) {
        errno = EINVAL;
        return SIG_ERR;
    }
    if (func == SIG_DFL)
        action = 0;
    else if (func == SIG_IGN)
        action = 1;
    old = __axc_sig_tab[sig];
    __axc_sig_tab[sig] = func;
    if (__axc_os_signal(sig, action) < -1) {
        __axc_sig_tab[sig] = old;
        errno = EINVAL;
        return SIG_ERR;
    }
    return old;
}
