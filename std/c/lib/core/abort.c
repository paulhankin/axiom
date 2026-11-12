/* abort (<stdlib.h>, C23 7.24.4.1): abnormal termination, by raise(SIGABRT)
   (whose default action ends the process: on Linux and macOS the system's,
   which kills it with the signal); a handler that returns does not stop
   it: the default action is restored and the signal raised again, and if
   the process still lives, it ends with status 3. No atexit functions run
   and no streams are flushed. */
#include <signal.h>
#include <stdlib.h>
#include "axc/core_impl.h"

void abort(void) {
    raise(SIGABRT);
    signal(SIGABRT, SIG_DFL);
    raise(SIGABRT);
    __axc_exit(3);
}
