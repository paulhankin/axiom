/* __axc_at_exit_flush: the standard I/O's function that exit calls after
   the atexit handlers (axc/core_impl.h), null until that part sets it. */
#include "axc/core_impl.h"

void (*__axc_at_exit_flush)(void);
