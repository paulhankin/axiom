/* thrd_exit (C17 7.26.5.5): ends the calling thread with result res, after
   its thread-specific storage destructors. On the main thread (which
   thrd_create did not start) the program ends once the last other thread
   has ended, as if by exit(EXIT_SUCCESS) then. */
#include <stdlib.h>
#include "axc/threads_impl.h"

void thrd_exit(int res) {
    __axc_thrd_exit(res);
    /* (the main thread, the others ended) */
    exit(EXIT_SUCCESS);
}
