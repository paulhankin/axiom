/* fprintf's other conversions and the string forms (C17 7.21.6.1,
   7.21.6.5, 7.21.6.6, 7.21.6.12, 7.21.6.13): c s p n %, %lc and %ls in
   the "C" locale, and snprintf's truncation and result. */
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <wchar.h>

static int vs(char *buf, size_t n, const char *fmt, ...) {
    va_list ap;
    int r;
    va_start(ap, fmt);
    r = vsnprintf(buf, n, fmt, ap);
    va_end(ap);
    return r;
}

int main(void) {
    char buf[32];
    int n;
    size_t k;
    printf("[%s][%10s][%-10s][%.2s][%10.2s][%.0s][%s]\n", "abc", "abc", "abc", "abc", "abc", "abc", "");
    printf("[%c][%3c][%-3c][%c]\n", 'x', 'y', 'z', 0x41);
    printf("[%ls][%5ls][%.2ls][%lc][%3lc]\n", L"wide", L"ab", L"wide", (wint_t)L'w', (wint_t)L'v');
    printf("[%p][%12p][%-12p]\n", (void *)0, (void *)0x1234, (void *)0xab);
    /* snprintf: at most n - 1 characters and a null; the whole length */
    for (k = 0; k <= 6; k++) {
        buf[0] = '#';
        buf[1] = 0;
        n = snprintf(k == 0 ? NULL : buf, k, "%d-%s", 123, "xy");
        printf("%u: %d [%s]\n", (unsigned)k, n, buf);
    }
    n = vs(buf, sizeof buf, "%s=%5.1f", "pi", 3.14159);
    printf("%d [%s]\n", n, buf);
    n = sprintf(buf, "%c%c%c", 'a', 0, 'b');
    printf("%d %d %d\n", n, buf[1], buf[2]);
    /* a long field through a short buffer */
    n = snprintf(buf, 8, "%40d", 7);
    printf("%d [%s]\n", n, buf);
    n = snprintf(buf, sizeof buf, "%.40f", 1.0);
    printf("%d [%s]\n", n, buf);
    /* a byte the "C" locale has no wide character for fails (EILSEQ) */
    n = snprintf(buf, sizeof buf, "%lc", (wint_t)0x1234);
    printf("%d\n", n);
    return 0;
}
/* expect: [abc][       abc][abc       ][ab][        ab][][] */
/* expect: [x][  y][z  ][A] */
/* expect: [wide][   ab][wi][w][  v] */
/* expect: [0x0][      0x1234][0xab        ] */
/* expect: 0: 6 [#] */
/* expect: 1: 6 [] */
/* expect: 2: 6 [1] */
/* expect: 3: 6 [12] */
/* expect: 4: 6 [123] */
/* expect: 5: 6 [123-] */
/* expect: 6: 6 [123-x] */
/* expect: 8 [pi=  3.1] */
/* expect: 3 0 98 */
/* expect: 40 [       ] */
/* expect: 42 [1.00000000000000000000000000000] */
/* expect: -1 */
