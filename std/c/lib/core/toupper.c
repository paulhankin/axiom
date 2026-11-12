/* toupper (<ctype.h>, 7.4.2.2): the uppercase letter of a lowercase one;
   any other value (EOF included) as it is. */
#include <ctype.h>
#include "axc/core_impl.h"

int toupper(int c) {
    return (__AXC_CLASS(c) & __AXC_LOWER) ? c - ('a' - 'A') : c;
}
