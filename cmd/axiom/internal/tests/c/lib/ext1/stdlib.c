/* Annex K's <stdlib.h> (C11 K.3.6): set_constraint_handler_s and the
   handlers, getenv_s, bsearch_s, qsort_s (with their contexts), wctomb_s,
   mbstowcs_s and wcstombs_s, with their runtime-constraint violations. */
#define __STDC_WANT_LIB_EXT1__ 1
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <locale.h>
#include <wchar.h>

static void handler(const char *restrict msg, void *restrict ptr, errno_t error) {
    (void)ptr;
    printf("handler: %s (%d)\n", msg, error != 0);
}

/* a comparison whose context says the direction */
static int cmp(const void *x, const void *y, void *context) {
    int a = *(const int *)x, b = *(const int *)y, dir = *(int *)context;
    return dir * ((a > b) - (a < b));
}

int main(void) {
    char v[8];
    size_t len;
    errno_t r;
    int a[10] = {5, 3, 9, 1, 7, 2, 8, 6, 4, 0}, i, dir = -1, key = 7, *p, status;
    char mb[8];
    wchar_t w[8];
    set_constraint_handler_s(handler);

    /* getenv_s: PATH-free: a name that is not set, then one too long */
    r = getenv_s(&len, v, sizeof v, "AXC_SURELY_NOT_SET_42");
    printf("not set %d %d [%s]\n", r != 0, (int)len, v);
    r = getenv_s(&len, NULL, 0, "AXC_SURELY_NOT_SET_42");
    printf("count only %d %d\n", r != 0, (int)len);
    r = getenv_s(&len, v, sizeof v, NULL);
    printf("null name %d %d\n", r != 0, (int)len);
    r = getenv_s(&len, NULL, 4, "X");

    /* qsort_s and bsearch_s with a context: descending */
    r = qsort_s(a, 10, sizeof a[0], cmp, &dir);
    printf("qsort_s %d:", r);
    for (i = 0; i < 10; i++)
        printf(" %d", a[i]);
    printf("\n");
    p = bsearch_s(&key, a, 10, sizeof a[0], cmp, &dir);
    printf("bsearch_s %d\n", p != NULL ? (int)(p - a) : -1);
    key = 11;
    printf("bsearch_s missing %d\n", bsearch_s(&key, a, 10, sizeof a[0], cmp, &dir) == NULL);
    printf("qsort_s zero %d\n", qsort_s(NULL, 0, sizeof a[0], NULL, NULL));
    printf("bsearch_s zero %d\n", bsearch_s(NULL, NULL, 0, 4, NULL, NULL) == NULL);
    r = qsort_s(NULL, 3, 4, cmp, &dir);
    r = qsort_s(a, (rsize_t)-1, 4, cmp, &dir);
    p = bsearch_s(&key, a, 10, sizeof a[0], NULL, &dir);

    /* wctomb_s in the "C" locale and in UTF-8 */
    r = wctomb_s(&status, mb, sizeof mb, L'A');
    printf("wctomb_s %d %d %c\n", r, status, mb[0]);
    r = wctomb_s(&status, NULL, 0, L'A');
    printf("wctomb_s null %d %d\n", r, status);
    setlocale(LC_CTYPE, "C.UTF-8");
    r = wctomb_s(&status, mb, sizeof mb, (wchar_t)0xE9);
    printf("wctomb_s e-acute %d %d %d %d\n", r, status, (unsigned char)mb[0], (unsigned char)mb[1]);
    r = wctomb_s(&status, mb, 1, (wchar_t)0xE9);
    printf("wctomb_s too small %d %d\n", r != 0, status);
    r = wctomb_s(&status, NULL, 3, L'A');
    r = wctomb_s(&status, mb, sizeof mb, (wchar_t)0xDC00);
    printf("wctomb_s invalid %d %d\n", r != 0, status);

    /* mbstowcs_s */
    r = mbstowcs_s(&len, w, 8, "h\xc3\xa9llo", 10);
    printf("mbstowcs_s %d %d %d %d\n", r, (int)len, (int)w[1], (int)w[5]);
    r = mbstowcs_s(&len, NULL, 0, "h\xc3\xa9llo", 0);
    printf("mbstowcs_s count %d %d\n", r, (int)len);
    r = mbstowcs_s(&len, w, 3, "abcdef", 2);
    printf("mbstowcs_s len < dstmax %d %d %d %d\n", r, (int)len, (int)w[1], (int)w[2]);
    r = mbstowcs_s(&len, w, 4, "abcdef", 10);
    printf("mbstowcs_s no room %d %d %d\n", r != 0, (int)len == -1, (int)w[0]);
    r = mbstowcs_s(&len, w, 8, "a\xff", 10);
    printf("mbstowcs_s invalid %d %d\n", r != 0, (int)len == -1);
    r = mbstowcs_s(NULL, w, 8, "a", 1);

    /* wcstombs_s */
    w[0] = L'h';
    w[1] = (wchar_t)0xE9;
    w[2] = L'!';
    w[3] = 0;
    r = wcstombs_s(&len, mb, sizeof mb, w, 10);
    printf("wcstombs_s %d %d %d %d %d\n", r, (int)len, (unsigned char)mb[1], mb[3], mb[4]);
    r = wcstombs_s(&len, NULL, 0, w, 0);
    printf("wcstombs_s count %d %d\n", r, (int)len);
    r = wcstombs_s(&len, mb, sizeof mb, w, 2);
    printf("wcstombs_s len 2 %d %d %d %d\n", r, (int)len, mb[0], mb[1]);
    r = wcstombs_s(&len, mb, 4, w, 10);
    printf("wcstombs_s no room %d %d %d\n", r != 0, (int)len == -1, mb[0]);
    r = wcstombs_s(&len, mb, 3, w, 2);
    printf("wcstombs_s stops before a char that does not fit %d %d %d %d\n", r, (int)len, mb[0], mb[1]);
    return 0;
}

// expect: not set 1 0 []
// expect: count only 1 0
// expect: handler: getenv_s: name is a null pointer (1)
// expect: null name 1 0
// expect: handler: getenv_s: value is a null pointer and maxsize is not zero (1)
// expect: qsort_s 0: 9 8 7 6 5 4 3 2 1 0
// expect: bsearch_s 2
// expect: bsearch_s missing 1
// expect: qsort_s zero 0
// expect: bsearch_s zero 1
// expect: handler: qsort_s: base is a null pointer (1)
// expect: handler: qsort_s: nmemb is greater than RSIZE_MAX (1)
// expect: handler: bsearch_s: compar is a null pointer (1)
// expect: wctomb_s 0 1 A
// expect: wctomb_s null 0 0
// expect: wctomb_s e-acute 0 2 195 169
// expect: handler: wctomb_s: the multibyte character does not fit in smax bytes (1)
// expect: wctomb_s too small 1 2
// expect: handler: wctomb_s: s is a null pointer and smax is not zero (1)
// expect: wctomb_s invalid 1 -1
// expect: mbstowcs_s 0 5 233 0
// expect: mbstowcs_s count 0 5
// expect: mbstowcs_s len < dstmax 0 2 98 0
// expect: handler: mbstowcs_s: no null character within dstmax wide characters of the source (1)
// expect: mbstowcs_s no room 1 1 0
// expect: mbstowcs_s invalid 1 1
// expect: handler: mbstowcs_s: retval is a null pointer (1)
// expect: wcstombs_s 0 4 195 33 0
// expect: wcstombs_s count 0 4
// expect: wcstombs_s len 2 0 1 104 0
// expect: handler: wcstombs_s: the conversion does not fit in dstmax bytes (1)
// expect: wcstombs_s no room 1 1 0
// expect: wcstombs_s stops before a char that does not fit 0 1 104 0
