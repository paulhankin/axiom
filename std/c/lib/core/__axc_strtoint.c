/* __axc_strtoint: the subject sequence of strtol and its kin (C23 7.24.1.7):
   after white space (isspace), an optional sign, then digits of the base
   (2 to 36: letters a to z and A to Z are 10 to 35); with base 16 an
   optional 0x or 0X, with base 0 the form of an integer constant (0x and
   octal 0 prefixes; in C23 also 0b, which base 2 then admits too). The
   longest initial part of that form is the subject: "0x" not followed by a
   hexadecimal digit is the subject "0", ending before the x. With no
   subject, nothing is converted and *end is s. */
#include "axc/core_impl.h"

static int digit(int c) {
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'z')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'Z')
        return c - 'A' + 10;
    return 99;
}

unsigned long long __axc_strtoint(const char *s, char **end, int base, unsigned long long max, int *neg,
                                  int *over, int c23) {
    const unsigned char *p = (const unsigned char *)s;
    unsigned long long v = 0;
    int any = 0;
    *neg = 0;
    *over = 0;
    if (base < 0 || base == 1 || base > 36) {
        if (end)
            *end = (char *)s;
        return 0;
    }
    while (__AXC_CLASS(*p) & __AXC_SPACE)
        p++;
    if (*p == '+' || *p == '-') {
        *neg = *p == '-';
        p++;
    }
    if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X') && (base == 0 || base == 16) && digit(p[2]) < 16) {
        p += 2;
        base = 16;
    } else if (c23 && p[0] == '0' && (p[1] == 'b' || p[1] == 'B') && (base == 0 || base == 2) && digit(p[2]) < 2) {
        p += 2;
        base = 2;
    } else if (base == 0) {
        base = p[0] == '0' ? 8 : 10;
    }
    {
        /* (v * base + d > max exactly when v > (max - d) / base) */
        unsigned long long lim = max / (unsigned)base;
        unsigned rem = (unsigned)(max % (unsigned)base);
        for (;; p++) {
            int d = digit(*p);
            if (d >= base)
                break;
            any = 1;
            if (v > lim || (v == lim && (unsigned)d > rem))
                *over = 1;
            else
                v = v * (unsigned)base + (unsigned)d;
        }
    }
    if (!any) {
        *neg = 0;
        if (end)
            *end = (char *)s;
        return 0;
    }
    if (end)
        *end = (char *)p;
    return v;
}
