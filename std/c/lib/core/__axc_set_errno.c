/* __axc_set_errno: errno = e, for the run-time layer (std/c/rt), which
   cannot write an int of the build's width itself. */
#include <errno.h>
#include "axc/core_impl.h"

void __axc_set_errno(long long e) {
    errno = (int)e;
}
