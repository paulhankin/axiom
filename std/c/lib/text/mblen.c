/* mblen (C17 7.22.7.1): the bytes of the character at s, as mbtowc. */
#include <axc/text.h>

int mblen(const char *s, size_t n) {
    return mbtowc(0, s, n);
}
