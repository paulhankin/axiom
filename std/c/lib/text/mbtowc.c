/* mbtowc (C17 7.22.7.2). UTF-8 and the single-byte encoding have no shift
   states: with s a null pointer it returns 0. n bytes that are not a whole
   valid character are -1 (EILSEQ); so is a character beyond U+FFFF where
   wchar_t has 16 bits, which one wchar_t cannot hold (mblen counts it). */
#include <axc/text.h>
#include <errno.h>

int mbtowc(wchar_t *restrict pwc, const char *restrict s, size_t n) {
    mbstate_t st = {0};
    unsigned long c;
    size_t r;
    if (!s)
        return 0;
    r = __axc_mb_get(&c, s, n, &st);
    if (r == (size_t)-2) {
        errno = EILSEQ;
        return -1;
    }
    if (r == (size_t)-1)
        return -1;
#if __AXIOM_WCHAR_WIDTH__ == 16
    if (c > 0xffff && pwc) {
        errno = EILSEQ;
        return -1;
    }
#endif
    if (pwc)
        *pwc = (wchar_t)c;
    return (int)r;
}
