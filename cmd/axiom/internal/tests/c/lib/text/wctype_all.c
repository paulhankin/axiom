/* <wctype.h> over every code point. In "C.UTF-8": each class and case
   mapping is the one docs/c-text.md defines from Unicode's properties
   (wctype_ref.ax, from std/unicode), the classes relate as C17 7.30.2.1
   requires, and their sizes are those of Unicode 15.1's data. In "C": the
   ASCII characters have <ctype.h>'s "C" classes and mappings, and the
   others none. wctype, iswctype, wctrans and towctrans by name; WEOF. */
/* uses: wctype_ref.ax */
#include <locale.h>
#include <wchar.h>
#include <wctype.h>
#include "../../mix/print.h"

unsigned int ref_class(unsigned int c);
unsigned int ref_upper(unsigned int c);
unsigned int ref_lower(unsigned int c);

static const char *const names[12] = {"alnum", "alpha", "blank", "cntrl", "digit", "graph",
                                      "lower", "print", "punct", "space", "upper", "xdigit"};
static int (*const fns[12])(wint_t) = {iswalnum, iswalpha, iswblank, iswcntrl, iswdigit, iswgraph,
                                       iswlower, iswprint, iswpunct, iswspace, iswupper, iswxdigit};

static unsigned int classes(wint_t c) {
    unsigned int r = 0;
    int k;
    for (k = 0; k < 12; k++)
        if (fns[k](c))
            r |= 1u << k;
    return r;
}

/* the C17 7.30.2.1 relations among the classes of one character */
static int related(unsigned int r) {
    int alnum = r & 1, alpha = r & 2, blank = r & 4, cntrl = r & 8, digit = r & 16, graph = r & 32, lower = r & 64,
        print = r & 128, punct = r & 256, space = r & 512, upper = r & 1024, xdigit = r & 2048;
    if (!!alnum != (alpha || digit))
        return 0;
    if (alpha && (cntrl || digit || punct || space))
        return 0;
    if ((upper || lower) && !alpha)
        return 0;
    if (!!graph != (print && !space))
        return 0;
    if (punct && (!print || space || alnum))
        return 0;
    if ((blank && !space) || (xdigit && !alnum) || (print && cntrl))
        return 0;
    return 1;
}

/* <ctype.h>'s "C" classes of an ASCII character (C17 7.4.1) */
static unsigned int ascii(unsigned int c) {
    unsigned int r = 0;
    if (c < 32 || c == 127)
        r |= 8;
    else
        r |= 128;
    if (c == ' ' || (c >= 9 && c <= 13))
        r |= 512;
    if (c == ' ' || c == 9)
        r |= 4;
    if (c >= '0' && c <= '9')
        r |= 16 | 1 | 2048;
    if ((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))
        r |= 2048;
    if (c >= 'a' && c <= 'z')
        r |= 64 | 2 | 1;
    if (c >= 'A' && c <= 'Z')
        r |= 1024 | 2 | 1;
    if (c > 32 && c < 127)
        r |= 32;
    if (c > 32 && c < 127 && !(r & 1))
        r |= 256;
    return r;
}

int main(void) {
    long count[12] = {0}, ref_count[12] = {0}, wrong = 0, unrelated = 0, upper_maps = 0, lower_maps = 0, c_wrong = 0;
    unsigned long c;
    int k;
    wctype_t types[12];
    for (k = 0; k < 12; k++)
        types[k] = wctype(names[k]);
    print_bool(wctype("alphabet") == 0 && wctype("") == 0 && wctrans("toupper") && wctrans("tolower") &&
               wctrans("totitle") == 0 && wctrans("toupper") != wctrans("tolower"));
    for (c = 0; c < 0x110000; c++) {
        unsigned int r;
        /* (beyond U+FFFF, wint_t has no such value where it has 16 bits) */
        if (c > 0xffff && sizeof(wint_t) == 2)
            break;
        r = classes((wint_t)c);
        if (c < 128 ? r != ascii((unsigned int)c) : r != 0)
            c_wrong++;
        if (towupper((wint_t)c) != (c < 128 && c >= 'a' && c <= 'z' ? c - 32 : c) ||
            towlower((wint_t)c) != (c < 128 && c >= 'A' && c <= 'Z' ? c + 32 : c))
            c_wrong++;
    }
    setlocale(LC_ALL, "C.UTF-8");
    for (c = 0; c < 0x110000; c++) {
        unsigned int r;
        if (c > 0xffff && sizeof(wint_t) == 2)
            break;
        r = classes((wint_t)c);
        if (r != ref_class((unsigned int)c) || (c < 128 && r != ascii((unsigned int)c)))
            wrong++;
        if (!related(r))
            unrelated++;
        for (k = 0; k < 12; k++) {
            if (r & (1u << k))
                count[k]++;
            if (ref_class((unsigned int)c) & (1u << k))
                ref_count[k]++;
            if (!iswctype((wint_t)c, types[k]) != !(r & (1u << k)))
                wrong++;
        }
        if (towupper((wint_t)c) != ref_upper((unsigned int)c) || towlower((wint_t)c) != ref_lower((unsigned int)c) ||
            towctrans((wint_t)c, wctrans("toupper")) != towupper((wint_t)c) ||
            towctrans((wint_t)c, wctrans("tolower")) != towlower((wint_t)c) || towctrans((wint_t)c, 0) != c)
            wrong++;
        upper_maps += towupper((wint_t)c) != c;
        lower_maps += towlower((wint_t)c) != c;
    }
    print_i64(c_wrong);
    print_i64(wrong);
    print_i64(unrelated);
    if (sizeof(wint_t) == 2) {
        /* (a 16-bit wint_t: the characters up to U+FFFF only, checked against
           the reference above; the sizes below are the whole code space's) */
        static const long whole[14] = {138397, 138387, 18, 237, 10, 149624, 2544, 149641, 11227, 25, 1951, 22, 1450, 1433};
        for (k = 0; k < 14; k++)
            print_i64(wrong == 0 && (k >= 12 || count[k] == ref_count[k]) ? whole[k] : -1);
    } else {
        for (k = 0; k < 12; k++)
            print_i64(count[k]);
        print_i64(upper_maps);
        print_i64(lower_maps);
    }
    print_bool(classes(WEOF) == 0 && towupper(WEOF) == WEOF && towlower(WEOF) == WEOF && !iswctype(WEOF, 0));
    print_bool(iswalpha(0xe9) && iswlower(0xe9) && towupper(0xe9) == 0xc9 && iswspace(0x3000) && iswblank(0xa0) &&
               iswpunct(0xbf) && !iswdigit(0x660) && iswpunct(0x660) && iswcntrl(0x2028) && !iswprint(0xe000));
    return 0;
    /* expect: true */
    /* expect: 0 */
    /* expect: 0 */
    /* expect: 0 */
    /* expect: 138397 */
    /* expect: 138387 */
    /* expect: 18 */
    /* expect: 237 */
    /* expect: 10 */
    /* expect: 149624 */
    /* expect: 2544 */
    /* expect: 149641 */
    /* expect: 11227 */
    /* expect: 25 */
    /* expect: 1951 */
    /* expect: 22 */
    /* expect: 1450 */
    /* expect: 1433 */
    /* expect: true */
    /* expect: true */
}
