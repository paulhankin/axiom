/* _Exit (<stdlib.h>, C23 7.24.4.5): ends the process with status at once:
   no handlers are called and no streams are flushed. */
#include <stdlib.h>
#include "axc/core_impl.h"

void _Exit(int status) {
    __axc_exit(status);
}
