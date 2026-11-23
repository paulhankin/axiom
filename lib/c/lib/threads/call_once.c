/* call_once (C17 7.26.2.1; C23 also <stdlib.h>, 7.24.2.1): calls func the
   first time it is called with flag; a call while that one runs waits for
   it to return. */
#include "axc/threads_impl.h"

void call_once(once_flag *flag, void (*func)(void)) {
    __axc_call_once(flag, func);
}
