/* isxdigit (<ctype.h>): whether c is a hexadecimal digit (7.4.1.12). c is
   EOF or an unsigned char's value; any other int is in no class. */
#include <ctype.h>
#include "axc/core_impl.h"

int isxdigit(int c) {
    return (__AXC_CLASS(c) & (__AXC_XDIGIT)) != 0;
}
