/* __axc_sig_deliver: what happens when signal sig arrives (from the
   system's handler, std/c/rt/core_signal.ax, or from raise where the system
   delivers none): its handler is called, with sig; an ignored signal does
   nothing; the default action ends the process, with status 3 for SIGABRT
   (as abort's) and 128 + sig for the others. On Linux and macOS the system
   carries out the default action and ignoring itself. */
#include <signal.h>
#include "axc/core_impl.h"

void __axc_sig_deliver(long long sig) {
    void (*h)(int);
    if (sig <= 0 || sig >= 32)
        return;
    h = __axc_sig_tab[sig];
    if (h == SIG_IGN)
        return;
    if (h == SIG_DFL)
        __axc_exit(sig == SIGABRT ? 3 : 128 + sig);
    h((int)sig);
}
