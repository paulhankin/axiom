/* The locales agree with themselves (C17 7.11, 7.24.4, 7.29.4.4): in each
   locale setlocale accepts, localeconv has the "C" conventions, and
   strcoll, strxfrm, wcscoll and wcsxfrm order strings alike: a pair of
   UTF-8 strings and their wide forms (with characters of every range,
   U+E000-U+FFFF and beyond U+FFFF too, which UTF-16's code units would
   order otherwise) compare the same way by all four. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <limits.h>

static int sign(int x) {
    return (x > 0) - (x < 0);
}

int main(void) {
    static const char *const words[] = {"a", "b", "\xc3\xa9", "\xee\x80\x80", "\xef\xbf\xbd", "\xf0\x9f\x98\x80",
                                        "a\xf0\x9f\x98\x80", "a\xef\xbf\xbd", "", "\x7f"};
    static const char *const names[] = {"C.UTF-8", "en_US.UTF-8", "POSIX"};
    size_t n = sizeof words / sizeof words[0], i, j, k;
    for (k = 0; k < 3; k++) {
        int bad = 0, pairs = 0;
        struct lconv *lc;
        if (setlocale(LC_ALL, names[k]) == NULL) {
            printf("%s refused\n", names[k]);
            continue;
        }
        lc = localeconv();
        printf("%s: [%s] [%s] %d %d\n", names[k], lc->decimal_point, lc->thousands_sep, lc->frac_digits == CHAR_MAX,
               *lc->grouping);
        for (i = 0; i < n; i++)
            for (j = 0; j < n; j++) {
                wchar_t wa[8], wb[8], xa[16], xb[16];
                char sa[16], sb[16];
                int c, w, x, y;
                if (mbstowcs(wa, words[i], 8) == (size_t)-1 || mbstowcs(wb, words[j], 8) == (size_t)-1)
                    continue;
                pairs++;
                c = sign(strcoll(words[i], words[j]));
                w = sign(wcscoll(wa, wb));
                strxfrm(sa, words[i], sizeof sa);
                strxfrm(sb, words[j], sizeof sb);
                x = sign(strcmp(sa, sb));
                wcsxfrm(xa, wa, 16);
                wcsxfrm(xb, wb, 16);
                y = sign(wcscmp(xa, xb));
                if (c != w || c != x || c != y || c != sign(strcmp(words[i], words[j])))
                    bad++;
            }
        printf("  %d pairs, %d disagree\n", pairs, bad);
    }
    printf("de_DE: %d\n", setlocale(LC_ALL, "de_DE") == NULL);
    return 0;
}
// expect: C.UTF-8: [.] [] 1 0
// expect:   100 pairs, 0 disagree
// expect: en_US.UTF-8: [.] [] 1 0
// expect:   100 pairs, 0 disagree
// expect: POSIX: [.] [] 1 0
// expect:   100 pairs, 0 disagree
// expect: de_DE: 1
