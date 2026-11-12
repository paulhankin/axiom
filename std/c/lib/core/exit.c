/* exit (<stdlib.h>, C23 7.24.4.4): normal termination. The atexit
   functions, last registered first; then the destructors (the GNU
   dialects' destructor attribute: __axc_at_exit_dtors); then the open streams flushed and
   closed and the tmpfile files removed (the standard I/O's hook,
   __axc_at_exit_flush); then the process ends with status (0 and
   EXIT_SUCCESS are success, EXIT_FAILURE is 1). Returning from main calls
   it (5.1.2.3.4, the start-up: std/internal/runtime/c_start.ax). A second
   call from a handler goes on with the handlers not yet called. */
#include <stdlib.h>
#include "axc/core_impl.h"

void exit(int status) {
    void (*flush)(void);
    __axc_fn_run(&__axc_atexit_tab);
    /* the destructors (a GNU dialect's), as after main's return */
    flush = __axc_at_exit_dtors;
    if (flush) {
        __axc_at_exit_dtors = (void (*)(void))0;
        flush();
    }
    flush = __axc_at_exit_flush;
    if (flush) {
        __axc_at_exit_flush = (void (*)(void))0; /* (once, if a handler of it exits) */
        flush();
    }
    __axc_exit(status);
}
