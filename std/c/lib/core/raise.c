/* raise (<signal.h>, C23 7.14.2.1): sends signal sig to the calling thread,
   and returns once its handler has (0); nonzero for a number that is not
   one of <signal.h>'s. Through the system on Linux and macOS (whose default
   actions apply), else at once (__axc_sig_deliver.c). */
#include <signal.h>
#include "axc/core_impl.h"

int raise(int sig) {
    if (sig != SIGINT && sig != SIGILL && sig != SIGABRT && sig != SIGFPE && sig != SIGSEGV && sig != SIGTERM)
        return -1;
    if (__axc_os_raise(sig) == 0)
        return 0;
    __axc_sig_deliver(sig);
    return 0;
}
