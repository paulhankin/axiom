/* exit (<stdlib.h>, C23 7.24.4.4): normal termination. The atexit
   functions, last registered first; then the destructors (the GNU
   dialects' destructor attribute: __axc_at_exit_dtors); then the open streams flushed and
   closed and the tmpfile files removed (the standard I/O's hook,
   __axc_at_exit_flush): __axc_exit_handlers; then the process ends with
   status (0 and EXIT_SUCCESS are success, EXIT_FAILURE is 1). Returning
   from main calls it (5.1.2.3.4, the start-up:
   std/internal/runtime/c_start.ax). A second call from a handler goes on
   with the handlers not yet called. In a program with threads, a call
   from another thread while one runs waits (the process ends). */
#include <stdlib.h>
#include "axc/core_impl.h"

void exit(int status) {
    __axc_exit_handlers();
    __axc_exit(status);
}
