/* thrd_yield (C17 7.26.5.8): lets other threads run. */
#include "axc/threads_impl.h"

void thrd_yield(void) {
    __axc_thrd_yield();
}
