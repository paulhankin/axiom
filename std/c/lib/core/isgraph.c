/* isgraph (<ctype.h>): whether c is a printing character other than space (7.4.1.6). c is
   EOF or an unsigned char's value; any other int is in no class. */
#include <ctype.h>
#include "axc/core_impl.h"

int isgraph(int c) {
    return (__AXC_CLASS(c) & (__AXC_UPPER | __AXC_LOWER | __AXC_DIGIT | __AXC_PUNCT)) != 0;
}
