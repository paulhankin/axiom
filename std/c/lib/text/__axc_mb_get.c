/* __axc_mb_get - the next character in LC_CTYPE's encoding (axc/text.h),
   as mbrtowc (C17 7.29.6.3.2) reads it: the bytes that complete it, 0 for
   the null character, (size_t)-2 when all n bytes are the beginning of
   one (they are in *ps), (size_t)-1 with EILSEQ when they are not. */
#include <axc/text.h>
#include <errno.h>

size_t __axc_mb_get(unsigned long *c, const char *s, size_t n, mbstate_t *ps) {
    size_t i;
    if (!__axc_mb_utf8()) {
        if (n == 0)
            return (size_t)-2;
        *c = (unsigned char)s[0];
        return *c ? 1 : 0;
    }
    for (i = 0; i < n; i++) {
        long r = __axc_utf8_feed(ps, (unsigned char)s[i]);
        if (r == -1) {
            errno = EILSEQ;
            return (size_t)-1;
        }
        if (r >= 0) {
            *c = (unsigned long)r;
            return r == 0 ? 0 : i + 1;
        }
    }
    return (size_t)-2;
}
