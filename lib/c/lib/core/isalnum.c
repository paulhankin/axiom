/* isalnum (<ctype.h>): whether c is a letter or a decimal digit (7.4.1.1). c is
   EOF or an unsigned char's value; any other int is in no class. */
#include <ctype.h>
#include "axc/core_impl.h"

int isalnum(int c) {
    return (__AXC_CLASS(c) & (__AXC_UPPER | __AXC_LOWER | __AXC_DIGIT)) != 0;
}
