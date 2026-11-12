/* c8rtomb (C23 7.30.2.3): UTF-8 code units to the locale's encoding. The
   units of a character are kept in *ps (0 bytes) until its last; a unit
   that cannot continue a well-formed sequence is EILSEQ. (char8_t is
   C23's unsigned char: this source is C17.) */
#include <axc/text.h>
#include <errno.h>
#include <uchar.h>

size_t c8rtomb(char *restrict s, unsigned char c8, mbstate_t *restrict ps) {
    static mbstate_t internal;
    char buf[4];
    long c;
    int n;
    if (!ps)
        ps = &internal;
    if (!s) {
        s = buf;
        c8 = 0;
    }
    c = __axc_utf8_feed(ps, c8);
    if (c == -2)
        return 0;
    if (c == -1 || (n = __axc_mb_put(s, (unsigned long)c)) < 0) {
        errno = EILSEQ;
        return (size_t)-1;
    }
    return (size_t)n;
}
