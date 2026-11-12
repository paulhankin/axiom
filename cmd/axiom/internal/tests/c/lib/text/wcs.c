/* The wide string functions of <wchar.h> (C17 7.29.4): copying with
   padding and truncation, concatenation, comparison (by wchar_t values,
   also of values beyond ASCII), wcscoll and wcsxfrm in the "C" locale,
   the searches with the null wide character and empty strings, wcstok's
   state, the wmem functions with overlap and zero counts; and the wide
   numeric conversions (wcstol ... wcstold), with white space by iswspace,
   bases, ranges, the end pointer and no conversion. */
#include <errno.h>
#include <locale.h>
#include <wchar.h>
#include "../../mix/print.h"

static void wstr(const wchar_t *s) {
    char b[64];
    int i;
    for (i = 0; s[i] && i < 63; i++)
        b[i] = s[i] < 128 ? (char)s[i] : '?';
    b[i] = 0;
    print_cstr(b);
}

int main(void) {
    wchar_t a[32], b[16], *e, *p, *save;
    const wchar_t *hay = L"aébcébc";
    int i;
    /* copying */
    print_i64((long long)wcslen(L""));
    print_i64((long long)wcslen(L"héllo"));
    wstr(wcscpy(a, L"abc"));
    wmemset(b, L'#', 8);
    print_bool(wcsncpy(b, L"xy", 5) == b && b[0] == 'x' && b[1] == 'y' && b[2] == 0 && b[4] == 0 && b[5] == '#');
    wcsncpy(b, L"wxyz", 2);
    print_bool(b[0] == 'w' && b[1] == 'x' && b[2] == 0);
    wstr(wcscat(a, L"de"));
    wstr(wcsncat(a, L"fghij", 2));
    wstr(wcsncat(a, L"", 3));
    wstr(wcsncat(a, L"z", 0));
    /* comparison: by values, also beyond 0x7f; the sign only */
    print_i64(wcscmp(L"abc", L"abc"));
    print_i64(wcscmp(L"abc", L"abd"));
    print_i64(wcscmp(L"ab", L"abc"));
    print_i64(wcscmp(L"é", L"z"));
    print_i64(wcsncmp(L"abcx", L"abcy", 3));
    print_i64(wcsncmp(L"abcx", L"abcy", 4));
    print_i64(wcsncmp(L"x", L"y", 0));
    print_i64(wcscoll(L"b", L"a"));
    print_i64((long long)wcsxfrm(b, L"hello", 16));
    wstr(b);
    print_i64((long long)wcsxfrm(0, L"hello", 0));
    print_i64(wmemcmp(L"ab\0c", L"ab\0d", 4));
    print_i64(wmemcmp(L"x", L"y", 0));
    /* searches */
    print_i64(wcschr(hay, 0x00e9) - hay);
    print_i64(wcsrchr(hay, 0x00e9) - hay);
    print_i64(wcschr(hay, 0) - hay);
    print_bool(wcschr(hay, 'z') == 0 && wcsrchr(hay, 'z') == 0);
    print_i64((long long)wcscspn(hay, L"cb"));
    print_i64((long long)wcscspn(hay, L""));
    print_i64((long long)wcsspn(hay, L"aé"));
    print_i64((long long)wcsspn(hay, L""));
    print_i64(wcspbrk(hay, L"cb") - hay);
    print_bool(wcspbrk(hay, L"xyz") == 0);
    print_i64(wcsstr(hay, L"bcé") - hay);
    print_i64(wcsstr(hay, L"") - hay);
    print_bool(wcsstr(hay, L"bcx") == 0 && wcsstr(L"", L"a") == 0);
    wmemcpy(b, L"ab\0cd", 5);
    print_i64(wmemchr(b, 'c', 5) - b);
    print_bool(wmemchr(L"abc", 'c', 2) == 0);
    /* wcstok */
    wcscpy(a, L"  one, two,,three  ");
    for (p = wcstok(a, L" ,", &save); p; p = wcstok(0, L" ,", &save))
        wstr(p);
    print_bool(wcstok(0, L" ,", &save) == 0);
    wcscpy(a, L",,,");
    print_bool(wcstok(a, L",", &save) == 0);
    /* wmemcpy, wmemmove both ways */
    for (i = 0; i < 10; i++)
        a[i] = (wchar_t)('0' + i);
    a[10] = 0;
    wmemmove(a + 2, a, 5);
    wstr(a);
    wmemmove(a, a + 3, 6);
    wstr(a);
    print_bool(wmemcpy(b, L"xyz", 0) == b && wmemmove(b, b, 3) == b);
    /* numeric conversions */
    print_i64(wcstol(L" \t-0x1fZ", &e, 0));
    print_i64(*e);
    print_i64(wcstol(L"0777", &e, 0));
    print_i64(wcstol(L"z", &e, 36));
    errno = 0;
    print_i64(wcstoll(L"9223372036854775808", &e, 10));
    print_bool(errno == ERANGE && *e == 0);
    print_u64(wcstoull(L"18446744073709551615", 0, 10));
    print_u64(wcstoul(L"-1", 0, 10) == (unsigned long)-1);
    p = L"  +";
    print_i64(wcstol(p, &e, 10));
    print_bool(e == p);
    p = L"　 12";
    print_i64(wcstol(p, &e, 10));
    print_bool(e == p);
    setlocale(LC_CTYPE, "C.UTF-8");
    print_i64(wcstol(p, &e, 10));
    print_bool(e == p + 4);
    print_f64(wcstod(L"  -1.5e3x", &e));
    print_i64(*e);
    print_f64((double)wcstof(L"0.25", 0));
    print_f64((double)wcstold(L"1e-2", 0));
    p = L".e1";
    print_f64(wcstod(p, &e));
    print_bool(e == p);
    print_f64(wcstod(L"inf", 0) > 1e308);
    print_bool(wcstod(L"nan", &e) != wcstod(L"nan", 0) && *e == 0);
    p = L"12é";
    print_i64(wcstol(p, &e, 10));
    print_bool(e == p + 2);
    return 0;
    /* expect: 0 */
    /* expect: 5 */
    /* expect: abc */
    /* expect: true */
    /* expect: true */
    /* expect: abcde */
    /* expect: abcdefg */
    /* expect: abcdefg */
    /* expect: abcdefg */
    /* expect: 0 */
    /* expect: -1 */
    /* expect: -1 */
    /* expect: 1 */
    /* expect: 0 */
    /* expect: -1 */
    /* expect: 0 */
    /* expect: 1 */
    /* expect: 5 */
    /* expect: hello */
    /* expect: 5 */
    /* expect: -1 */
    /* expect: 0 */
    /* expect: 1 */
    /* expect: 4 */
    /* expect: 7 */
    /* expect: true */
    /* expect: 2 */
    /* expect: 7 */
    /* expect: 2 */
    /* expect: 0 */
    /* expect: 2 */
    /* expect: true */
    /* expect: 2 */
    /* expect: 0 */
    /* expect: true */
    /* expect: 3 */
    /* expect: true */
    /* expect: one */
    /* expect: two */
    /* expect: three */
    /* expect: true */
    /* expect: true */
    /* expect: 0101234789 */
    /* expect: 1234784789 */
    /* expect: true */
    /* expect: -31 */
    /* expect: 90 */
    /* expect: 511 */
    /* expect: 35 */
    /* expect: 9223372036854775807 */
    /* expect: true */
    /* expect: 18446744073709551615 */
    /* expect: 1 */
    /* expect: 0 */
    /* expect: true */
    /* expect: 0 */
    /* expect: true */
    /* expect: 12 */
    /* expect: true */
    /* expect: -1500 */
    /* expect: 120 */
    /* expect: 0.25 */
    /* expect: 0.01 */
    /* expect: 0 */
    /* expect: true */
    /* expect: 1 */
    /* expect: true */
    /* expect: 12 */
    /* expect: true */
}
