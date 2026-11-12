/* thrd_detach (C17 7.26.5.3): thr's resources go when it ends; it cannot be
   joined. thrd_error if it is not a thread thrd_create started. */
#include "axc/threads_impl.h"

int thrd_detach(thrd_t thr) {
    return (int)__axc_thrd_detach(thr);
}
