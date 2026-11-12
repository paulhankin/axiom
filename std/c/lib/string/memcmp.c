/* memcmp (C23 7.26.4.1): compares n bytes as unsigned char. Words at a time
   while they are equal, when both are aligned alike. */
#include <string.h>
#include "word.h"

int memcmp(const void *s1, const void *s2, size_t n) {
    const unsigned char *a = s1;
    const unsigned char *b = s2;
    if (n >= 2 * __AXIOM_WSIZE && (((size_t)a ^ (size_t)b) & (__AXIOM_WSIZE - 1)) == 0) {
        while (!__AXIOM_ALIGNED(a)) {
            if (*a != *b) {
                return *a - *b;
            }
            a++;
            b++;
            n--;
        }
        while (n >= __AXIOM_WSIZE && *(const __axiom_word *)a == *(const __axiom_word *)b) {
            a += __AXIOM_WSIZE;
            b += __AXIOM_WSIZE;
            n -= __AXIOM_WSIZE;
        }
    }
    for (; n > 0; n--, a++, b++) {
        if (*a != *b) {
            return *a - *b;
        }
    }
    return 0;
}
