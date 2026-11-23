/* iscntrl (<ctype.h>): whether c is a control character: 0 to 31 and 127 (7.4.1.4). c is
   EOF or an unsigned char's value; any other int is in no class. */
#include <ctype.h>
#include "axc/core_impl.h"

int iscntrl(int c) {
    return (__AXC_CLASS(c) & (__AXC_CNTRL)) != 0;
}
