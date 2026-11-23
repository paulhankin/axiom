/* isdigit (<ctype.h>): whether c is a decimal digit (7.4.1.5). c is
   EOF or an unsigned char's value; any other int is in no class. */
#include <ctype.h>
#include "axc/core_impl.h"

int isdigit(int c) {
    return (__AXC_CLASS(c) & (__AXC_DIGIT)) != 0;
}
