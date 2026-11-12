/* __axc_strtofp: the subject sequence of strtod, strtof and strtold (C23
   7.24.1.5) and its value as the bits of binary64 or binary32, correctly
   rounded to nearest (ties to even). After white space (isspace) and an
   optional sign, the longest initial part of one of these forms:

   - decimal digits with an optional '.', then an optional exponent part
     (e or E, an optional sign, digits): converted by the Axiom parser
     (std/fmt.parse_float, through __axc_parse_f64), which is correctly
     rounded for any number of digits;
   - 0x or 0X, hexadecimal digits with an optional '.', then an optional
     binary exponent part (p or P, an optional sign, decimal digits):
     converted here, exactly then rounded once;
   - INF or INFINITY, NAN or NAN(n-char-sequence), ignoring case: the
     n-char-sequence (digits, letters, underscores) is read and ignored (a
     quiet NaN).

   A form needs at least one digit; an exponent part without digits is not
   part of the subject ("1e+" is the subject "1"), nor is "x" without a
   hexadecimal digit after "0x" (the subject "0"). With no subject, the
   value is 0 and *end is s. *range is 1 when a nonzero subject's value is
   infinite (overflow), or zero or subnormal (underflow). */
#include "axc/core_impl.h"

static int lower(int c) {
    return c >= 'A' && c <= 'Z' ? c - 'A' + 'a' : c;
}

/* does p start with the lowercase word w, ignoring case? */
static int word(const unsigned char *p, const char *w) {
    while (*w) {
        if (lower(*p) != *w)
            return 0;
        p++;
        w++;
    }
    return 1;
}

static int hexval(int c) {
    if (c >= '0' && c <= '9')
        return c - '0';
    c = lower(c);
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return -1;
}

static int isdig(int c) {
    return c >= '0' && c <= '9';
}

/* The binary64 (bits 64) or binary32 bits of m * 2^e, m != 0, rounded once
   to nearest, ties to even; sticky: nonzero bits were dropped below m's
   last. */
static unsigned long long round_bits(unsigned long long m, long e, int sticky, int bits, int *range) {
    int p = bits == 64 ? 53 : 24;              /* significand bits */
    long emin = bits == 64 ? -1022 : -126;     /* least normal exponent */
    long emax = bits == 64 ? 1023 : 127;
    unsigned long long inf = bits == 64 ? 0x7FF0000000000000ULL : 0x7F800000ULL;
    long big;
    long shift;
    unsigned long long q, r, half;
    /* m in [2^63, 2^64): the value is in [2^big, 2^(big+1)) */
    while (!(m >> 63)) {
        m <<= 1;
        e--;
    }
    big = e + 63;
    if (big > emax) {
        *range = 1;
        return inf;
    }
    shift = 64 - p; /* the bits dropped from m */
    if (big < emin)
        shift += emin - big; /* subnormal: fewer bits */
    if (shift >= 64) {
        /* (all of m is below the last bit: at most half of it when shift
           is 64, which rounds up only past half) */
        q = 0;
        if (shift == 64 && (m > (1ULL << 63) || (m == (1ULL << 63) && sticky)))
            q = 1;
        *range = 1;
        return q;
    }
    q = m >> shift;
    r = m & ((1ULL << shift) - 1);
    half = 1ULL << (shift - 1);
    if (r > half || (r == half && (sticky || (q & 1))))
        q++;
    if (big < emin) {
        /* subnormal (q < 2^(p-1)), or the least normal if it rounded up to
           2^(p-1): the encoding is q itself either way */
        *range = 1;
        return q;
    }
    /* the exponent field big + bias, and the fraction; a q rounded up to
       2^p carries into the exponent (infinity past the largest) */
    {
        unsigned long long bias = bits == 64 ? 1023 : 127;
        unsigned long long v = ((unsigned long long)(big + (long)bias) << (p - 1)) + (q - (1ULL << (p - 1)));
        if (v >= inf) {
            *range = 1;
            return inf;
        }
        return v;
    }
}

/* the hexadecimal form after "0x", at p (with a digit, or '.' and a digit) */
static unsigned long long hex(const unsigned char **pp, int bits, int *range) {
    const unsigned char *p = *pp;
    unsigned long long m = 0;
    long e = 0;    /* the binary exponent of m's last bit */
    int sticky = 0;
    int dot = 0;
    int d;
    for (;; p++) {
        if (*p == '.' && !dot) {
            dot = 1;
            continue;
        }
        d = hexval(*p);
        if (d < 0)
            break;
        if (m >> 60) {
            /* (no room: the digit is below m's last bit) */
            sticky |= d != 0;
            if (!dot)
                e += 4;
        } else {
            m = m * 16 + (unsigned)d;
            if (dot)
                e -= 4;
        }
    }
    if (lower(*p) == 'p') {
        const unsigned char *q = p + 1;
        int neg = 0;
        long x = 0;
        if (*q == '+' || *q == '-') {
            neg = *q == '-';
            q++;
        }
        if (isdig(*q)) {
            for (; isdig(*q); q++) {
                if (x < 100000000L)
                    x = x * 10 + (*q - '0'); /* (clamped: far past any result) */
            }
            e += neg ? -x : x;
            p = q;
        }
    }
    *pp = p;
    if (m == 0)
        return 0;
    return round_bits(m, e, sticky, bits, range);
}

unsigned long long __axc_strtofp(const char *s, char **end, int bits, int *range) {
    const unsigned char *p = (const unsigned char *)s;
    unsigned long long sign = bits == 64 ? 0x8000000000000000ULL : 0x80000000ULL;
    unsigned long long v = 0;
    int neg = 0;
    *range = 0;
    while (__AXC_CLASS(*p) & __AXC_SPACE)
        p++;
    if (*p == '+' || *p == '-') {
        neg = *p == '-';
        p++;
    }
    if (word(p, "inf")) {
        p += word(p, "infinity") ? 8 : 3;
        v = bits == 64 ? 0x7FF0000000000000ULL : 0x7F800000ULL;
    } else if (word(p, "nan")) {
        p += 3;
        if (*p == '(') {
            const unsigned char *q = p + 1;
            while (isdig(*q) || (lower(*q) >= 'a' && lower(*q) <= 'z') || *q == '_')
                q++;
            if (*q == ')')
                p = q + 1;
        }
        v = bits == 64 ? 0x7FF8000000000000ULL : 0x7FC00000ULL;
    } else if (p[0] == '0' && lower(p[1]) == 'x' && (hexval(p[2]) >= 0 || (p[2] == '.' && hexval(p[3]) >= 0))) {
        p += 2;
        v = hex(&p, bits, range);
    } else if (isdig(*p) || (*p == '.' && isdig(p[1]))) {
        const unsigned char *start = p;
        int nonzero = 0;
        int dot = 0;
        for (;; p++) {
            if (*p == '.' && !dot) {
                dot = 1;
                continue;
            }
            if (!isdig(*p))
                break;
            nonzero |= *p != '0';
        }
        if (lower(*p) == 'e') {
            const unsigned char *q = p + 1;
            if (*q == '+' || *q == '-')
                q++;
            if (isdig(*q)) {
                while (isdig(*q))
                    q++;
                p = q;
            }
        }
        if (bits == 64) {
            double d = __axc_parse_f64((const char *)start, (size_t)(p - start));
            unsigned long long b;
            unsigned char *bp = (unsigned char *)&b;
            const unsigned char *dp = (const unsigned char *)&d;
            int k;
            for (k = 0; k < 8; k++)
                bp[k] = dp[k];
            v = b;
            if (nonzero && (v == 0x7FF0000000000000ULL || (v >> 52) == 0))
                *range = 1;
        } else {
            float f = __axc_parse_f32((const char *)start, (size_t)(p - start));
            unsigned long b = 0;
            unsigned char *bp = (unsigned char *)&b;
            const unsigned char *fp = (const unsigned char *)&f;
            int k;
            for (k = 0; k < 4; k++)
                bp[k] = fp[k];
            v = b & 0xFFFFFFFFUL;
            if (nonzero && (v == 0x7F800000ULL || (v >> 23) == 0))
                *range = 1;
        }
    } else {
        if (end)
            *end = (char *)s;
        return 0;
    }
    if (end)
        *end = (char *)p;
    return neg ? v | sign : v;
}
