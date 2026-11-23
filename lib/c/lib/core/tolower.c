/* tolower (<ctype.h>, 7.4.2.1): the lowercase letter of an uppercase one;
   any other value (EOF included) as it is. */
#include <ctype.h>
#include "axc/core_impl.h"

int tolower(int c) {
    return (__AXC_CLASS(c) & __AXC_UPPER) ? c + ('a' - 'A') : c;
}
