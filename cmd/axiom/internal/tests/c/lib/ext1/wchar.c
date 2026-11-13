/* Annex K's <wchar.h> (C11 K.3.9): the wide formatted functions, the wide
   string functions and the restartable conversions, with their
   runtime-constraint violations. */
#define __STDC_WANT_LIB_EXT1__ 1
#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <errno.h>

static void handler(const char *restrict msg, void *restrict ptr, errno_t error) {
    (void)ptr;
    printf("handler: %s (%d)\n", msg, error != 0);
}

static void show(const char *name, const wchar_t *w) {
    printf("%s [", name);
    for (; *w != 0; w++)
        printf("%c", (char)*w);
    printf("]\n");
}

int main(void) {
    wchar_t a[8], b[16], *t, *ptr;
    rsize_t max;
    errno_t e;
    size_t len;
    mbstate_t st;
    char mb[8];
    const char *src;
    const wchar_t *wsrc;
    int r, x;
    set_constraint_handler_s(handler);
    setlocale(LC_CTYPE, "C.UTF-8");

    e = wcscpy_s(a, 8, L"wide");
    show("wcscpy_s", a);
    e = wcscpy_s(a, 4, L"wide");
    printf("wcscpy_s too long %d %d\n", e != 0, (int)a[0]);
    e = wcsncpy_s(a, 8, L"abcdef", 3);
    show("wcsncpy_s", a);
    e = wcscat_s(a, 8, L"xyz");
    show("wcscat_s", a);
    e = wcscat_s(a, 8, L"12");
    printf("wcscat_s too long %d %d\n", e != 0, (int)a[0]);
    wcscpy_s(a, 8, L"ab");
    e = wcsncat_s(a, 8, L"cdefghij", 3);
    show("wcsncat_s", a);
    e = wmemcpy_s(b, 16, L"0123456789", 10);
    e = wmemmove_s(b + 2, 14, b, 5);
    b[10] = 0;
    show("wmemmove_s", b);
    e = wmemcpy_s(b, 4, L"0123456789", 5);
    printf("wmemcpy_s too long %d %d %d\n", e != 0, (int)b[0], (int)b[3]);
    printf("wcsnlen_s %d %d %d\n", (int)wcsnlen_s(L"abc", 9), (int)wcsnlen_s(L"abc", 1), (int)wcsnlen_s(NULL, 3));
    wcscpy_s(b, 16, L"a,b,,c");
    max = 7;
    t = wcstok_s(b, &max, L",", &ptr);
    show("wcstok_s 1", t);
    t = wcstok_s(NULL, &max, L",", &ptr);
    show("wcstok_s 2", t);
    t = wcstok_s(NULL, &max, L",", &ptr);
    show("wcstok_s 3", t);
    printf("wcstok_s end %d %d\n", wcstok_s(NULL, &max, L",", &ptr) == NULL, (int)max);

    /* formatted */
    r = swprintf_s(a, 8, L"%d%ls", 12, L"ab");
    show("swprintf_s", a);
    printf("-> %d\n", r);
    r = swprintf_s(a, 4, L"%d", 12345);
    printf("swprintf_s too long %d %d\n", r < 0, (int)a[0]);
    r = swprintf_s(a, 8, L"%n", &x);
    printf("swprintf_s %%n %d\n", r);
    r = snwprintf_s(a, 4, L"%d", 12345);
    show("snwprintf_s", a);
    printf("-> %d\n", r);
    r = swscanf_s(L"word 7", L"%ls %d", b, (rsize_t)16, &x);
    show("swscanf_s", b);
    printf("-> %d %d\n", r, x);
    r = swscanf_s(L"toolong", L"%ls", a, (rsize_t)4);
    printf("swscanf_s too long %d %d\n", r, (int)a[0]);
    r = swscanf_s(L"abc", L"%s", mb, (rsize_t)sizeof mb);
    printf("swscanf_s narrow [%s] %d\n", mb, r);
    r = wprintf_s(L"%ls\n", L"wprintf_s");
    printf("wprintf_s on a byte stream %d\n", r < 0);
    r = wprintf_s(L"%ls", (wchar_t *)NULL);

    /* conversions */
    memset(&st, 0, sizeof st);
    e = wcrtomb_s(&len, mb, sizeof mb, (wchar_t)0x20AC, &st);
    printf("wcrtomb_s %d %d %d\n", e, (int)len, (unsigned char)mb[0]);
    e = wcrtomb_s(&len, mb, 2, (wchar_t)0x20AC, &st);
    printf("wcrtomb_s too small %d %d %d\n", e != 0, (int)len == -1, mb[0]);
    e = wcrtomb_s(&len, NULL, 0, L'x', &st);
    printf("wcrtomb_s null %d %d\n", e, (int)len);
    src = "h\xe2\x82\xac!";
    e = mbsrtowcs_s(&len, a, 8, &src, 8, &st);
    printf("mbsrtowcs_s %d %d %d %d\n", e, (int)len, (int)a[1], src == NULL);
    src = "abcdef";
    e = mbsrtowcs_s(&len, a, 8, &src, 3, &st);
    printf("mbsrtowcs_s 3 %d %d %d %s\n", e, (int)len, (int)a[3], src);
    src = NULL;
    e = mbsrtowcs_s(&len, a, 8, &src, 3, &st);
    wsrc = L"xy";
    e = wcsrtombs_s(&len, mb, sizeof mb, &wsrc, 8, &st);
    printf("wcsrtombs_s %d %d %s %d\n", e, (int)len, mb, wsrc == NULL);
    wsrc = L"wxyz";
    e = wcsrtombs_s(&len, mb, sizeof mb, &wsrc, 2, &st);
    printf("wcsrtombs_s 2 %d %d %s ", e, (int)len, mb);
    show("left", wsrc);
    wsrc = L"toolong";
    e = wcsrtombs_s(&len, mb, 4, &wsrc, 8, &st);
    printf("wcsrtombs_s no room %d %d\n", e != 0, mb[0]);
    return 0;
}

// expect: wcscpy_s [wide]
// expect: handler: wcscpy_s: s2 does not fit in s1max wide characters (1)
// expect: wcscpy_s too long 1 0
// expect: wcsncpy_s [abc]
// expect: wcscat_s [abcxyz]
// expect: handler: wcscat_s: s2 does not fit after s1's string (1)
// expect: wcscat_s too long 1 0
// expect: wcsncat_s [abcde]
// expect: wmemmove_s [0101234789]
// expect: handler: wmemcpy_s: n is greater than s1max (1)
// expect: wmemcpy_s too long 1 0 0
// expect: wcsnlen_s 3 1 0
// expect: wcstok_s 1 [a]
// expect: wcstok_s 2 [b]
// expect: wcstok_s 3 [c]
// expect: wcstok_s end 1 1
// expect: swprintf_s [12ab]
// expect: -> 4
// expect: handler: swprintf_s: the output does not fit in n wide characters (1)
// expect: swprintf_s too long 1 0
// expect: handler: swprintf_s: %n in the format (1)
// expect: swprintf_s %n 0
// expect: snwprintf_s [123]
// expect: -> 5
// expect: swscanf_s [word]
// expect: -> 2 7
// expect: swscanf_s too long 0 0
// expect: swscanf_s narrow [abc] 1
// expect: wprintf_s on a byte stream 1
// expect: wcrtomb_s 0 3 226
// expect: handler: wcrtomb_s: the multibyte character does not fit in smax bytes (1)
// expect: wcrtomb_s too small 1 1 0
// expect: wcrtomb_s null 0 1
// expect: mbsrtowcs_s 0 3 8364 1
// expect: mbsrtowcs_s 3 0 3 0 def
// expect: handler: mbsrtowcs_s: *src is a null pointer (1)
// expect: wcsrtombs_s 0 2 xy 1
// expect: wcsrtombs_s 2 0 2 wx left [yz]
// expect: handler: wcsrtombs_s: the conversion does not fit in dstmax bytes (1)
// expect: wcsrtombs_s no room 1 0
