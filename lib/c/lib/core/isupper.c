/* isupper (<ctype.h>): whether c is an uppercase letter (7.4.1.11). c is
   EOF or an unsigned char's value; any other int is in no class. */
#include <ctype.h>
#include "axc/core_impl.h"

int isupper(int c) {
    return (__AXC_CLASS(c) & (__AXC_UPPER)) != 0;
}
