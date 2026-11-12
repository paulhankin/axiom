/* fscanf's conversions over strings (C17 7.21.6.2, 7.21.6.7): integers
   in each base and length, characters, strings, scansets, %n, assignment
   suppression, widths, %p of fprintf's output, the results, and the
   rules of input items: the longest prefix of a matching sequence, one
   character given back, so that a prefix that is not a whole sequence
   fails (7.21.6.2p9 and the example of p20). */
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

static void clear(char *p, size_t n) {
    size_t i;
    for (i = 0; i < n; i++)
        p[i] = 0;
}

int main(void) {
    int a = -1, b = -1, c = -1, n = -1, r;
    unsigned u = 0;
    long l = 0;
    long long ll = 0;
    short h = 0;
    signed char hh = 0;
    size_t z = 0;
    char s[32], t[32], cs[8];
    void *p = NULL;
    double d = 0;
    r = sscanf("  42 -17 +5", "%d%d%d", &a, &b, &c);
    printf("%d %d %d %d\n", r, a, b, c);
    r = sscanf("0x1f 017 99 0X10 -0x1", "%i %i %i %i %i", &a, &b, &c, &n, &l);
    printf("%d %d %d %d %d\n", r, a, b, c, n);
    r = sscanf("777 ff FF 1011 0b11", "%o %x %X %b %b", &a, &b, &c, &n, &u);
    printf("%d %d %d %d %d %u\n", r, a, b, c, n, u);
    r = sscanf("200 40000 -9223372036854775808 2000000000 123", "%hhd %hd %lld %ld %zu", &hh, &h, &ll, &l, &z);
    printf("%d %d %d %lld %ld %u\n", r, hh, h, ll, l, (unsigned)z);
    r = sscanf("12345678", "%3d%2d%d", &a, &b, &c);
    printf("%d %d %d %d\n", r, a, b, c);
    r = sscanf("abc 12", "%*s %n%d%n", &n, &a, &b);
    printf("%d %d %d %d\n", r, n, a, b);
    clear(s, sizeof s);
    clear(t, sizeof t);
    clear(cs, sizeof cs);
    r = sscanf("hello, world!", "%5s%c %[a-z]%3c", s, cs, t, cs + 1);
    printf("%d [%s] [%s] [%s]\n", r, s, t, cs);
    clear(s, sizeof s);
    clear(t, sizeof t);
    r = sscanf("]x-y]z abc", "%[]x-y]%[^ ]", s, t);
    printf("%d [%s] [%s]\n", r, s, t);
    clear(s, sizeof s);
    r = sscanf("aaa-bb", "%[-a]", s);
    printf("%d [%s]\n", r, s);
    /* %% and ordinary characters; white space matches any amount */
    r = sscanf("10% x\t\n,20", "%d%% x ,%d", &a, &b);
    printf("%d %d %d\n", r, a, b);
    /* p round-trips fprintf's %p */
    sprintf(s, "%p", (void *)&r);
    r = sscanf(s, "%p", &p);
    printf("%d %d\n", r, p == (void *)&r);
    /* failures: matching (the count so far), input before any conversion
       (EOF), a prefix that is not a whole item */
    printf("%d\n", sscanf("x1", "%d", &a));
    printf("%d\n", sscanf("", "%d", &a));
    printf("%d\n", sscanf("   ", "%s", s));
    printf("%d\n", sscanf("7", "%d%d", &a, &b));
    printf("%d\n", sscanf("0x", "%x", &u));
    printf("%d\n", sscanf("-", "%d", &a));
    printf("%d\n", sscanf("100ergs", "%lf", &d));
    printf("%d\n", sscanf("1e+x", "%lf", &d));
    printf("%d\n", sscanf("a", "b%d", &a));
    printf("%d\n", sscanf("abc", "abc%d", &a));
    /* C23 wN */
    {
        int8_t i8 = 0;
        uint16_t u16 = 0;
        int64_t i64 = 0;
        r = sscanf("-8 65535 -123456789012", "%w8d %w16u %w64d", &i8, &u16, &i64);
        printf("%d %d %u %lld\n", r, i8, (unsigned)u16, (long long)i64);
    }
    return 0;
}
/* expect: 3 42 -17 5 */
/* expect: 5 31 15 99 16 */
/* expect: 5 511 255 255 11 3 */
/* expect: 5 -56 -25536 -9223372036854775808 2000000000 123 */
/* expect: 3 123 45 678 */
/* expect: 1 4 12 6 */
/* expect: 3 [hello] [world] [,!] */
/* expect: 2 []x] [-y]z] */
/* expect: 1 [aaa-] */
/* expect: 2 10 20 */
/* expect: 1 1 */
/* expect: 0 */
/* expect: -1 */
/* expect: -1 */
/* expect: 1 */
/* expect: 0 */
/* expect: 0 */
/* expect: 0 */
/* expect: 0 */
/* expect: 0 */
/* expect: -1 */
/* expect: 3 -8 65535 -123456789012 */
