/* c32rtomb (C17 7.28.1.4): a UTF-32 code unit to the locale's encoding;
   a surrogate, or beyond U+10FFFF, is EILSEQ. */
#include <axc/text.h>
#include <errno.h>
#include <uchar.h>

size_t c32rtomb(char *restrict s, char32_t c32, mbstate_t *restrict ps) {
    char buf[4];
    int n;
    (void)ps;
    if (!s) {
        s = buf;
        c32 = 0;
    }
    n = __axc_mb_put(s, c32);
    if (n < 0) {
        errno = EILSEQ;
        return (size_t)-1;
    }
    return (size_t)n;
}
