/* quick_exit (<stdlib.h>, C23 7.24.4.7): the at_quick_exit functions, last
   registered first, then _Exit(status): no atexit functions, no flushing. */
#include <stdlib.h>
#include "axc/core_impl.h"

void quick_exit(int status) {
    __axc_fn_run(&__axc_quick_tab);
    __axc_exit(status);
}
