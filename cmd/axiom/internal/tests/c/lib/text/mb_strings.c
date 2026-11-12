/* The multibyte string conversions (mbstowcs, wcstombs, mbsrtowcs,
   wcsrtombs) and the single-byte ones (btowc, wctob) in the "C" locale
   (single-byte: each byte is the character of its value) and in
   "C.UTF-8": the limits on what is stored, the counts with a null
   destination, where *src is left, EILSEQ and its position, and
   mbsinit. */
#include <errno.h>
#include <locale.h>
#include <wchar.h>
#include "../../mix/print.h"
#include <stdlib.h>
#include <string.h>

/* a size_t result, (size_t)-1 to -3 as negative numbers */
static long long sz(size_t r) {
    return r >= (size_t)-3 ? (long long)(r - (size_t)-3) - 3 : (long long)r;
}

static void wide(const wchar_t *w, size_t n) {
    size_t i;
    for (i = 0; i < n; i++)
        print_u64((unsigned long)w[i]);
}

int main(void) {
    wchar_t w[16];
    char b[32];
    const char *src;
    const wchar_t *wsrc;
    mbstate_t st;
    size_t r;
    int i, n = 0;
    /* "C": every byte, both ways */
    for (i = 1; i < 256; i++) {
        char one[2] = {(char)i, 0};
        if (mbstowcs(w, one, 2) != 1 || (unsigned long)w[0] != (unsigned long)i || wcstombs(b, w, 2) != 1 ||
            (unsigned char)b[0] != i || btowc(i) != (wint_t)i || wctob((wint_t)i) != i)
            n++;
    }
    print_i64(n);
    print_bool(btowc(-1) == WEOF && wctob(WEOF) == -1 && wctob(256) == -1);
    errno = 0;
    print_i64(wctomb(b, 0x100));
    print_bool(errno == EILSEQ);
    print_i64(sz(mbstowcs(0, "\xc3\xa9t\xc3\xa9", 0)));
    setlocale(LC_CTYPE, "C.UTF-8");
    print_i64(sz(mbstowcs(0, "\xc3\xa9t\xc3\xa9", 0)));
    print_bool(btowc(0x7f) == 0x7f && btowc(0x80) == WEOF && btowc(0xc3) == WEOF && wctob(0xe9) == -1 &&
               wctob('A') == 'A');
    /* mbsrtowcs: a limit, then the rest; the end; a count */
    src = "a\xc3\xa9\xe2\x82\xac\xf0\x9f\x98\x80z";
    memset(&st, 0, sizeof st);
    r = mbsrtowcs(w, &src, 2, &st);
    print_i64(sz(r));
    print_i64(src[0] == '\xe2');
    r = mbsrtowcs(w + 2, &src, 16, &st);
    /* (U+1F600 is two wchar_t where wchar_t has 16 bits) */
    print_i64(sz(r) - (sizeof(wchar_t) == 2));
    print_bool(src == 0 && mbsinit(&st));
    if (sizeof(wchar_t) == 4)
        wide(w, 5);
    else if (w[3] == 0xd83d && w[4] == 0xde00)
        print_cstr("97\n233\n8364\n128512\n122");
    /* back again (a surrogate pair where wchar_t has 16 bits) */
    wsrc = w;
    memset(&st, 0, sizeof st);
    r = wcsrtombs(b, &wsrc, sizeof b, &st);
    print_bool(r == 11 && wsrc == 0 && memcmp(b, "a\xc3\xa9\xe2\x82\xac\xf0\x9f\x98\x80z", 12) == 0);
    wsrc = w;
    r = wcsrtombs(b, &wsrc, 9, &st);
    print_bool(r == 6 && wsrc == w + 3);
    src = "a\xc3\xa9\xe2\x82\xac\xf0\x9f\x98\x80z";
    print_i64(sz(mbsrtowcs(0, &src, 1, &st)) - (sizeof(wchar_t) == 2));
    print_bool(src != 0);
    /* EILSEQ: -1, *src at the bad character */
    src = "ab\xc3(";
    errno = 0;
    r = mbsrtowcs(w, &src, 16, &st);
    print_bool(r == (size_t)-1 && errno == EILSEQ && src[0] == '\xc3');
    print_bool(mbstowcs(w, "\xff", 4) == (size_t)-1);
    /* wcsrtombs: stops before a character that does not fit */
    w[0] = 'a';
    w[1] = 0xe9;
    w[2] = 0x20ac;
    w[3] = 0;
    wsrc = w;
    memset(&st, 0, sizeof st);
    r = wcsrtombs(b, &wsrc, 4, &st);
    print_i64(sz(r));
    print_bool(wsrc == w + 2);
    r = wcsrtombs(b + 3, &wsrc, 3, &st);
    print_i64(sz(r));
    print_bool(wsrc == w + 3);
    r = wcsrtombs(b + 6, &wsrc, 1, &st);
    print_i64(sz(r));
    print_bool(wsrc == 0 && memcmp(b, "a\xc3\xa9\xe2\x82\xac", 7) == 0);
    wsrc = w;
    print_i64(sz(wcsrtombs(0, &wsrc, 0, &st)));
    print_i64(sz(wcstombs(0, w, 0)));
    print_i64(sz(wcstombs(b, w, 32)));
    w[1] = 0xd800;
    errno = 0;
    print_bool(wcstombs(b, w, 32) == (size_t)-1 && errno == EILSEQ);
    /* a state with a character begun is not initial */
    memset(&st, 0, sizeof st);
    print_bool(mbsinit(&st) && mbsinit(0));
    print_i64(sz(mbrtowc(w, "\xe2\x82", 2, &st)));
    print_bool(!mbsinit(&st));
    print_i64(sz(mbrtowc(w, "\xac", 1, &st)));
    print_bool(mbsinit(&st) && w[0] == 0x20ac);
    print_i64(sz(mbrtowc(w, "", 1, &st)));
    print_i64(sz(mbrtowc(0, 0, 0, &st)));
    print_i64(sz(mbrtowc(w, "x", 0, &st)));
    print_i64(sz(mbrlen("\xe2\x82\xac", 3, 0)));
    print_i64(sz(wcrtomb(0, L'x', &st)));
    print_i64(mblen(0, 0) + mbtowc(0, 0, 0) + wctomb(0, 0));
    print_i64(mblen("", 1));
    return 0;
    /* expect: 0 */
    /* expect: true */
    /* expect: -1 */
    /* expect: true */
    /* expect: 5 */
    /* expect: 3 */
    /* expect: true */
    /* expect: 2 */
    /* expect: 1 */
    /* expect: 3 */
    /* expect: true */
    /* expect: 97 */
    /* expect: 233 */
    /* expect: 8364 */
    /* expect: 128512 */
    /* expect: 122 */
    /* expect: true */
    /* expect: true */
    /* expect: 5 */
    /* expect: true */
    /* expect: true */
    /* expect: true */
    /* expect: 3 */
    /* expect: true */
    /* expect: 3 */
    /* expect: true */
    /* expect: 0 */
    /* expect: true */
    /* expect: 6 */
    /* expect: 6 */
    /* expect: 6 */
    /* expect: true */
    /* expect: true */
    /* expect: -2 */
    /* expect: true */
    /* expect: 1 */
    /* expect: true */
    /* expect: 0 */
    /* expect: 0 */
    /* expect: -2 */
    /* expect: 3 */
    /* expect: 1 */
    /* expect: 0 */
    /* expect: 0 */
}
