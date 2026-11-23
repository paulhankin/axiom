/* memcmp (C23 7.26.4.1): compares n bytes as unsigned char. Words at a time
   while they are equal, at any alignment (each word copied out with
   memcpy, which the compiler makes one load: the processors this library
   runs on read unaligned words), then bytes from the word that differs. */
#include <string.h>
#include "word.h"

int memcmp(const void *s1, const void *s2, size_t n) {
    const unsigned char *a = s1;
    const unsigned char *b = s2;
    while (n >= __AXIOM_WSIZE) {
        __axiom_word x;
        __axiom_word y;
        memcpy(&x, a, sizeof x);
        memcpy(&y, b, sizeof y);
        if (x != y) {
            break;
        }
        a += __AXIOM_WSIZE;
        b += __AXIOM_WSIZE;
        n -= __AXIOM_WSIZE;
    }
    for (; n > 0; n--, a++, b++) {
        if (*a != *b) {
            return *a - *b;
        }
    }
    return 0;
}
