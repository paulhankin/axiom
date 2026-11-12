/* memalignment (<stdlib.h>, C23 7.24.9.1): the largest power of two that
   divides p's address (as an integer), or 0 for a null pointer. */
#include <stdlib.h>

size_t memalignment(const void *p) {
    size_t a = (size_t)p;
    return a & (0 - a);
}
