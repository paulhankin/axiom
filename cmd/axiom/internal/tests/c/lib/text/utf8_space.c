/* The conversions between UTF-8 and wide characters, UTF-32, UTF-16 and
   UTF-8 code units (mbrtowc, wcrtomb, mbrtoc32, c32rtomb, mbrtoc16,
   c16rtomb, mbrtoc8, c8rtomb, mblen, mbtowc, wctomb) over the whole code
   space in the "C.UTF-8" locale: every scalar value round-trips, in the
   number of bytes RFC 3629 gives it, the bytes hashing (FNV-1a, 64 bits)
   to the hash of the UTF-8 form of U+0000..U+10FFFF without the
   surrogates; each surrogate and every value beyond U+10FFFF is EILSEQ. */
/* flags: --c-std=c23 */
#include <errno.h>
#include <locale.h>
#include <uchar.h>
#include <wchar.h>
#include "../../mix/print.h"
#include <stdlib.h>
#include <string.h>

static unsigned long long hash = 1469598103934665603ULL;

static int expected_length(unsigned long c) {
    return c < 0x80 ? 1 : c < 0x800 ? 2 : c < 0x10000 ? 3 : 4;
}

int main(void) {
    unsigned long c;
    long bad = 0, scalars = 0, rejected = 0;
    if (!setlocale(LC_ALL, "C.UTF-8"))
        return 1;
    for (c = 0; c <= 0x110001; c++) {
        char b[8], b2[8];
        mbstate_t st;
        char32_t d32;
        char16_t u1, u2;
        wchar_t w[2];
        size_t n, r;
        int i;
        memset(&st, 0, sizeof st);
        errno = 0;
        n = c32rtomb(b, (char32_t)c, &st);
        if (c > 0x10ffff || (c >= 0xd800 && c <= 0xdfff)) {
            if (n != (size_t)-1 || errno != EILSEQ)
                bad++;
            if (wcrtomb(b, (wchar_t)c, &st) != (size_t)-1 && sizeof(wchar_t) == 4)
                bad++;
            rejected++;
            continue;
        }
        scalars++;
        if (n != (size_t)expected_length(c))
            bad++;
        for (i = 0; i < (int)n; i++) {
            hash ^= (unsigned char)b[i];
            hash *= 1099511628211ULL;
        }
        /* back, at once and a byte at a time (mbtowc cannot store a
           character beyond U+FFFF in a 16-bit wchar_t) */
        memset(&st, 0, sizeof st);
        if (mbrtoc32(&d32, b, n, &st) != (c ? n : 0) || d32 != c || !mbsinit(&st))
            bad++;
        for (i = 0; i < (int)n; i++) {
            r = mbrtoc32(&d32, b + i, 1, &st);
            if (i + 1 < (int)n ? r != (size_t)-2 : (r != (c ? 1 : 0) || d32 != c))
                bad++;
        }
        if (mblen(b, n) != (c ? (int)n : 0) || mblen(b, n - 1) != -1 || mbtowc(w, b, n) != (c ? (sizeof(wchar_t) == 2 && c > 0xffff ? -1 : (int)n) : 0))
            bad++;
        /* wide characters: one, or a surrogate pair where wchar_t has 16
           bits */
        memset(&st, 0, sizeof st);
        r = mbrtowc(w, b, n, &st);
        if (r != (c ? n : 0))
            bad++;
        if (sizeof(wchar_t) == 2 && c > 0xffff) {
            if (mbrtowc(w + 1, b, n, &st) != (size_t)-3 || (unsigned long)w[0] != 0xd800 + ((c - 0x10000) >> 10) ||
                (unsigned long)w[1] != 0xdc00 + (c & 0x3ff))
                bad++;
            if (wcrtomb(b2, w[0], &st) != 0 || wcrtomb(b2, w[1], &st) != n || memcmp(b, b2, n))
                bad++;
        } else {
            if ((unsigned long)w[0] != c || wcrtomb(b2, w[0], &st) != n || memcmp(b, b2, n) ||
                wctomb(b2, w[0]) != (int)n || memcmp(b, b2, n))
                bad++;
        }
        /* UTF-16 */
        memset(&st, 0, sizeof st);
        r = mbrtoc16(&u1, b, n, &st);
        if (c > 0xffff) {
            if (r != n || mbrtoc16(&u2, b, n, &st) != (size_t)-3 || u1 != 0xd800 + ((c - 0x10000) >> 10) ||
                u2 != 0xdc00 + (c & 0x3ff) || c16rtomb(b2, u1, &st) != 0 || c16rtomb(b2, u2, &st) != n ||
                memcmp(b, b2, n))
                bad++;
        } else if (r != (c ? n : 0) || u1 != c || c16rtomb(b2, u1, &st) != n || memcmp(b, b2, n)) {
            bad++;
        }
        /* UTF-8 code units */
        memset(&st, 0, sizeof st);
        for (i = 0; i < (int)n; i++) {
            char8_t u;
            r = mbrtoc8(&u, b, n, &st);
            if (r != (i == 0 ? (c ? n : 0) : (size_t)-3) || u != (unsigned char)b[i])
                bad++;
            if (c8rtomb(b2, u, &st) != (i + 1 < (int)n ? 0 : n))
                bad++;
        }
        if (memcmp(b, b2, n) || !mbsinit(&st))
            bad++;
    }
    print_i64(scalars);
    print_i64(rejected);
    print_u64(hash);
    print_i64(bad);
    return 0;
    /* expect: 1112064 */
    /* expect: 2050 */
    /* expect: 16489504591323981763 */
    /* expect: 0 */
}
