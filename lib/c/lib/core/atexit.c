/* atexit (<stdlib.h>, C23 7.24.4.3): registers func, to be called by exit
   (and by returning from main) with no arguments. 0 on success. */
#include <stdlib.h>
#include "axc/core_impl.h"

int atexit(void (*func)(void)) {
    return __axc_fn_push(&__axc_atexit_tab, func);
}
