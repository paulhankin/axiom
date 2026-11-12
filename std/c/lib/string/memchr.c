/* memchr (C23 7.26.5.2): the first of n bytes at s that is (unsigned
   char)c, or a null pointer. Words at a time once aligned. */
#include <string.h>
#include "word.h"

#undef memchr

void *memchr(const void *s, int c, size_t n) {
    const unsigned char *p = s;
    unsigned char b = (unsigned char)c;
    while (n > 0 && !__AXIOM_ALIGNED(p)) {
        if (*p == b) {
            return (void *)p;
        }
        p++;
        n--;
    }
    __axiom_word m = __AXIOM_ONES * b;
    while (n >= __AXIOM_WSIZE) {
        /* (a byte of x is zero where one of the word's is b) */
        __axiom_word x = *(const __axiom_word *)p ^ m;
        if (__AXIOM_HAS_ZERO(x)) {
            break;
        }
        p += __AXIOM_WSIZE;
        n -= __AXIOM_WSIZE;
    }
    for (; n > 0; n--, p++) {
        if (*p == b) {
            return (void *)p;
        }
    }
    return NULL;
}
