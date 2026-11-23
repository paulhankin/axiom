/* at_quick_exit (<stdlib.h>, C23 7.24.4.4): registers func, to be called by
   quick_exit. 0 on success. */
#include <stdlib.h>
#include "axc/core_impl.h"

int at_quick_exit(void (*func)(void)) {
    return __axc_fn_push(&__axc_quick_tab, func);
}
