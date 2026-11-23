/* ispunct (<ctype.h>): whether c is a printing character that is neither space nor isalnum (7.4.1.9). c is
   EOF or an unsigned char's value; any other int is in no class. */
#include <ctype.h>
#include "axc/core_impl.h"

int ispunct(int c) {
    return (__AXC_CLASS(c) & (__AXC_PUNCT)) != 0;
}
