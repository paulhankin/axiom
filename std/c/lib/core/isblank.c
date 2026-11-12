/* isblank (<ctype.h>): whether c is a space or a horizontal tab (7.4.1.3). c is
   EOF or an unsigned char's value; any other int is in no class. */
#include <ctype.h>
#include "axc/core_impl.h"

int isblank(int c) {
    return (__AXC_CLASS(c) & (__AXC_BLANK)) != 0;
}
