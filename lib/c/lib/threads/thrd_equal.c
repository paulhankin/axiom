/* thrd_equal (C17 7.26.5.4): nonzero if thr0 and thr1 are the same thread. */
#include "axc/threads_impl.h"

int thrd_equal(thrd_t thr0, thrd_t thr1) {
    return thr0 == thr1;
}
