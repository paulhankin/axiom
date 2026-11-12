/* mbrtowc, mbrlen, mbtowc, mbstowcs and c8rtomb against the Unicode Standard's
   table 3-7 of well-formed UTF-8 byte sequences, in the "C.UTF-8" locale:
   every sequence of one and two bytes, of three bytes beginning with E0 to
   F4, and of four beginning with F0 to F4 whose second and third bytes are
   at the edges of their ranges and just outside them. Each is complete,
   the beginning of a character ((size_t)-2), or invalid ((size_t)-1 with
   EILSEQ) at the first byte that cannot follow, as the table (written
   here as ranges, independently of the library's decoder) says; and the
   decoder starts again from a state set to zero after it. */
/* flags: --c-std=c23 */
#include <errno.h>
#include <locale.h>
#include <uchar.h>
#include <wchar.h>
#include "../../mix/print.h"
#include <stdlib.h>
#include <string.h>

/* table 3-7: a lead byte's length and the range of its second byte (the
   others are 80..BF) */
static int lead(unsigned char b, unsigned char *lo, unsigned char *hi) {
    *lo = 0x80;
    *hi = 0xbf;
    if (b <= 0x7f)
        return 1;
    if (b >= 0xc2 && b <= 0xdf)
        return 2;
    if (b == 0xe0)
        *lo = 0xa0;
    if (b == 0xed)
        *hi = 0x9f;
    if (b >= 0xe0 && b <= 0xef)
        return 3;
    if (b == 0xf0)
        *lo = 0x90;
    if (b == 0xf4)
        *hi = 0x8f;
    if (b >= 0xf0 && b <= 0xf4)
        return 4;
    return 0;
}

/* what the table says of s[0..n): the length of a complete character, -2
   for the beginning of one, -1 if invalid */
static long ref(const unsigned char *s, int n) {
    unsigned char lo, hi;
    int len = lead(s[0], &lo, &hi), i;
    if (len == 0)
        return -1;
    for (i = 1; i < n && i < len; i++) {
        if (s[i] < lo || s[i] > hi)
            return -1;
        lo = 0x80;
        hi = 0xbf;
    }
    return n >= len ? (s[0] ? len : 0) : -2;
}

static long counts[3], bad;

static void check(const unsigned char *s, int n) {
    mbstate_t st;
    wchar_t w;
    long want = ref(s, n);
    size_t r;
    int i;
    memset(&st, 0, sizeof st);
    errno = 0;
    r = mbrtowc(&w, (const char *)s, (size_t)n, &st);
    if (want >= 0)
        counts[0]++;
    else
        counts[want == -2 ? 1 : 2]++;
    if (want == -1 ? r != (size_t)-1 || errno != EILSEQ : r != (size_t)want)
        bad++;
    memset(&st, 0, sizeof st);
    if (mbrlen((const char *)s, (size_t)n, &st) != r)
        bad++;
    if (mbtowc(0, (const char *)s, (size_t)n) != (want >= 0 ? (int)want : -1))
        bad++;
    /* a byte at a time: -2 until the byte that completes or breaks it */
    memset(&st, 0, sizeof st);
    for (i = 0; i < n; i++) {
        unsigned char p[4];
        long wi;
        memcpy(p, s, (size_t)i + 1);
        wi = ref(p, i + 1);
        errno = 0;
        r = mbrtowc(&w, (const char *)s + i, 1, &st);
        if (wi == -1) {
            if (r != (size_t)-1 || errno != EILSEQ)
                bad++;
            break;
        }
        if (wi >= 0 ? r != (wi ? 1 : 0) : r != (size_t)-2)
            bad++;
        if (wi >= 0)
            break;
    }
    /* UTF-8 code units, a unit at a time (c8rtomb) */
    memset(&st, 0, sizeof st);
    for (i = 0; i < n; i++) {
        char out[4];
        unsigned char p[4];
        long wi;
        memcpy(p, s, (size_t)i + 1);
        wi = ref(p, i + 1);
        errno = 0;
        r = c8rtomb(out, s[i], &st);
        if (wi == -1 ? r != (size_t)-1 || errno != EILSEQ : r != (wi >= 0 ? (size_t)(i + 1) : 0))
            bad++;
        if (wi != -2)
            break;
    }
    /* mbstowcs of the bytes as a string: the one character they are, or -1
       if they are not one (a beginning is cut by the null character) */
    {
        char z[5];
        wchar_t ws[4];
        memcpy(z, s, (size_t)n);
        z[n] = 0;
        r = mbstowcs(ws, z, 4);
        if (want == n ? r != (size_t)(sizeof(wchar_t) == 2 && n == 4 ? 2 : 1) || ws[0] != w
                      : want < 0 && r != (size_t)-1)
            bad++;
    }
    /* (after an error, a state set to zero works again) */
    memset(&st, 0, sizeof st);
    if (mbrtowc(&w, "A", 1, &st) != 1 || w != 'A')
        bad++;
}

int main(void) {
    unsigned char s[4];
    int a, b, c, d;
    static const unsigned char edges[] = {0x00, 0x7f, 0x80, 0x8f, 0x90, 0x9f, 0xa0, 0xbf, 0xc0, 0xff};
    if (!setlocale(LC_CTYPE, "C.UTF-8"))
        return 1;
    for (a = 0; a < 256; a++) {
        s[0] = (unsigned char)a;
        check(s, 1);
        for (b = 0; b < 256; b++) {
            s[1] = (unsigned char)b;
            check(s, 2);
            if (a < 0xe0 || a > 0xf4)
                continue;
            for (c = 0; c < 256; c++) {
                s[2] = (unsigned char)c;
                check(s, 3);
            }
        }
    }
    for (a = 0xf0; a <= 0xf4; a++)
        for (b = 0; b < 10; b++)
            for (c = 0; c < 10; c++)
                for (d = 0; d < 256; d++) {
                    s[0] = (unsigned char)a;
                    s[1] = edges[b];
                    s[2] = edges[c];
                    s[3] = (unsigned char)d;
                    check(s, 4);
                }
    print_i64(counts[0]);
    print_i64(counts[1]);
    print_i64(counts[2]);
    print_i64(bad);
    return 0;
    /* expect: 105472 */
    /* expect: 17651 */
    /* expect: 1446925 */
    /* expect: 0 */
}
