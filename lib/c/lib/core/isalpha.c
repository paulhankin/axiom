/* isalpha (<ctype.h>): whether c is a letter: isupper or islower (7.4.1.2). c is
   EOF or an unsigned char's value; any other int is in no class. */
#include <ctype.h>
#include "axc/core_impl.h"

int isalpha(int c) {
    return (__AXC_CLASS(c) & (__AXC_UPPER | __AXC_LOWER)) != 0;
}
