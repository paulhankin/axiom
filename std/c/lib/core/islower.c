/* islower (<ctype.h>): whether c is a lowercase letter (7.4.1.7). c is
   EOF or an unsigned char's value; any other int is in no class. */
#include <ctype.h>
#include "axc/core_impl.h"

int islower(int c) {
    return (__AXC_CLASS(c) & (__AXC_LOWER)) != 0;
}
