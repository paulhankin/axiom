/* isprint (<ctype.h>): whether c is a printing character, space included (7.4.1.8). c is
   EOF or an unsigned char's value; any other int is in no class. */
#include <ctype.h>
#include "axc/core_impl.h"

int isprint(int c) {
    return (__AXC_CLASS(c) & (__AXC_PRINT)) != 0;
}
