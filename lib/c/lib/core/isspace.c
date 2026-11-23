/* isspace (<ctype.h>): whether c is space, \f, \n, \r, \t or \v (7.4.1.10). c is
   EOF or an unsigned char's value; any other int is in no class. */
#include <ctype.h>
#include "axc/core_impl.h"

int isspace(int c) {
    return (__AXC_CLASS(c) & (__AXC_SPACE)) != 0;
}
