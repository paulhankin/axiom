/* thrd_current (C17 7.26.5.2): the calling thread. */
#include "axc/threads_impl.h"

thrd_t thrd_current(void) {
    return __axc_thrd_current();
}
