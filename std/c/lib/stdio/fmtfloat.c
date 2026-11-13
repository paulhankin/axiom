/* fmtfloat.c - the floating-point conversions of fprintf (C17 7.21.6.1p8:
   f F e E g G a A) for every floating format the targets have: binary64,
   the x87 80-bit format and binary128, read from the value's bits
   (__axc_fp_double, __axc_fp_long_double) into one description (struct
   __axc_fp: significand * 2^exp).

   The decimal conversions are exact, for any precision: a finite binary
   value m * 2^e is the integer m * 2^e when e >= 0, and m * 5^-e / 10^-e
   when e < 0, so its decimal digits are those of the integer N = m * 2^e or
   m * 5^-e, which is computed in base 10^9 (the classic exact method of
   multiple-precision arithmetic; D. E. Knuth, The Art of Computer
   Programming, vol. 2, 4.4, "Radix conversion"), then rounded to the digits
   asked for, the halfway case to even (IEC 60559's round to nearest, the
   default rounding direction: rounding modes are not followed).

   A decimal floating value (C23 _Decimal32, _Decimal64, _Decimal128:
   __axc_fp_dec, from its BID encoding) is its coefficient * 10^q: N is the
   coefficient, and e, f and g round its digits as for a binary value. Its
   a conversion (C23 7.23.6.1p8) shows the coefficient's digits and the
   quantum exponent as they are: in the style of f with -q digits after the
   point when -(n + 5) <= q <= 0 (n the coefficient's digits, 1 for zero),
   else of e with n - 1 digits after the point; a precision P below n
   first rounds the coefficient to P digits (half to even). */
#include "impl.h"

/* Limbs of N for the widest format: m * 5^16494 for binary128 (11564
   digits), 5^16445 * 2^64 for x87, 2^1024 * ... for binary64 */
#if LDBL_MANT_DIG > 53
#define LIMBS 1320
#else
#define LIMBS 96
#endif
#define BASE 1000000000UL

/* (limbs of at least 32 bits, valid with every --c-int) */
typedef uint_least32_t limb;

struct big {
    limb l[LIMBS]; /* least significant first */
    int n;
};

/* b = b * k + add (k < 2^32, add < 2^32) */
static void big_muladd(struct big *b, limb k, limb add) {
    unsigned __axiom_int64 carry = add;
    int i;
    for (i = 0; i < b->n; i++) {
        unsigned __axiom_int64 t = (unsigned __axiom_int64)b->l[i] * k + carry;
        b->l[i] = (limb)(t % BASE);
        carry = t / BASE;
    }
    while (carry != 0 && b->n < LIMBS) {
        b->l[b->n++] = (limb)(carry % BASE);
        carry /= BASE;
    }
}

static const limb pow10_tab[10] = {1UL, 10UL, 100UL, 1000UL, 10000UL, 100000UL, 1000000UL, 10000000UL, 100000000UL,
                                   1000000000UL};

/* The number of decimal digits of x (x > 0; 1 for 0) */
static int digits_of(limb x) {
    int d = 1;
    while (d < 10 && x >= pow10_tab[d])
        d++;
    return d;
}

/* The decimal digits of N, read from the most significant */
struct digits {
    const struct big *b;
    int top;   /* digits of the top limb */
    int total; /* digits of N (0 if N is 0) */
};

static void digits_init(struct digits *d, const struct big *b) {
    d->b = b;
    if (b->n == 0) {
        d->top = d->total = 0;
        return;
    }
    d->top = digits_of(b->l[b->n - 1]);
    d->total = d->top + 9 * (b->n - 1);
}

/* Digit i (0: the most significant); 0 past the last */
static int digit_at(const struct digits *d, int i) {
    int limb, pos;
    if (i < 0 || i >= d->total)
        return 0;
    if (i < d->top) {
        limb = d->b->n - 1;
        pos = d->top - 1 - i;
    } else {
        limb = d->b->n - 2 - (i - d->top) / 9;
        pos = 8 - (i - d->top) % 9;
    }
    return (int)(d->b->l[limb] / pow10_tab[pos] % 10);
}

/* Is any digit from i on not zero? */
static int rest_nonzero(const struct digits *d, int i) {
    int limb, pos, k;
    if (i <= 0)
        return d->total > 0;
    if (i >= d->total)
        return 0;
    if (i < d->top) {
        limb = d->b->n - 1;
        pos = d->top - i; /* the low pos digits of the limb */
    } else {
        limb = d->b->n - 2 - (i - d->top) / 9;
        pos = 9 - (i - d->top) % 9;
    }
    if (d->b->l[limb] % pow10_tab[pos] != 0)
        return 1;
    for (k = limb - 1; k >= 0; k--) {
        if (d->b->l[k] != 0)
            return 1;
    }
    return 0;
}

/* N for v (finite, not zero), and the decimal exponent: v = 0.D * 10^dp
   where D are N's digits. */
static int make_n(struct big *b, const struct __axc_fp *v) {
    int e = v->exp, k;
    struct digits d;
    b->n = 0;
    if (v->dec) {
        /* the coefficient, 32 bits at a time; value = N * 10^q */
        big_muladd(b, 1, (limb)(v->hi >> 32));
        big_muladd(b, 0x10000UL, 0);
        big_muladd(b, 0x10000UL, (limb)(v->hi & 0xffffffffUL));
        big_muladd(b, 0x10000UL, 0);
        big_muladd(b, 0x10000UL, (limb)(v->lo >> 32));
        big_muladd(b, 0x10000UL, 0);
        big_muladd(b, 0x10000UL, (limb)(v->lo & 0xffffffffUL));
        digits_init(&d, b);
        return d.total + e;
    }
    /* the significand, 32 bits at a time from the top */
    big_muladd(b, 1, (limb)(v->hi >> 32));
    big_muladd(b, 0x10000UL, 0);
    big_muladd(b, 0x10000UL, (limb)(v->hi & 0xffffffffUL));
    big_muladd(b, 0x10000UL, 0);
    big_muladd(b, 0x10000UL, (limb)(v->lo >> 32));
    big_muladd(b, 0x10000UL, 0);
    big_muladd(b, 0x10000UL, (limb)(v->lo & 0xffffffffUL));
    if (e >= 0) {
        for (k = e; k >= 31; k -= 31)
            big_muladd(b, 0x80000000UL, 0);
        if (k > 0)
            big_muladd(b, (limb)1 << k, 0);
        digits_init(&d, b);
        return d.total;
    }
    for (k = -e; k >= 13; k -= 13)
        big_muladd(b, 1220703125UL, 0); /* 5^13 */
    if (k > 0) {
        limb p = 1;
        while (k-- > 0)
            p *= 5;
        big_muladd(b, p, 0);
    }
    digits_init(&d, b);
    return d.total + e;
}

/* The digits of N rounded to keep digits (keep may be 0 or less: the value
   rounds to a unit in the last place kept or to 0): written to out
   (room for keep + 1), the count of digits returned; *dp is adjusted when
   the rounding carries into a new leading digit. Round half to even. */
static int round_digits(const struct digits *d, int keep, char *out, int *dp) {
    int i, up, last;
    if (keep >= d->total) {
        for (i = 0; i < d->total; i++)
            out[i] = (char)('0' + digit_at(d, i));
        return d->total;
    }
    if (keep < 0)
        return 0;
    {
        int first = digit_at(d, keep);
        last = keep > 0 ? digit_at(d, keep - 1) : 0;
        up = first > 5 || (first == 5 && (rest_nonzero(d, keep + 1) || (last & 1)));
    }
    for (i = 0; i < keep; i++)
        out[i] = (char)('0' + digit_at(d, i));
    if (!up)
        return keep;
    for (i = keep - 1; i >= 0; i--) {
        if (out[i] != '9') {
            out[i]++;
            return keep;
        }
        out[i] = '0';
    }
    /* all nines (or nothing kept): 1 followed by the zeros */
    for (i = keep; i > 0; i--)
        out[i] = out[i - 1];
    out[0] = '1';
    (*dp)++;
    return keep + 1;
}

/* An output buffer: the caller's, or one of the library's when that is
   too small */
static char *room_for(size_t need, char *buf, size_t cap, char **big, size_t *bigcap) {
    if (need <= cap)
        return buf;
    *big = __axc_sys_alloc(need);
    if (*big == NULL)
        return NULL;
    *bigcap = need;
    return *big;
}

/* The exponent part of an e conversion: e+dd (at least two digits) */
static size_t put_exp10(char *p, int x, int upper) {
    char t[8];
    size_t n = 0, k = 0;
    p[n++] = upper ? 'E' : 'e';
    p[n++] = x < 0 ? '-' : '+';
    if (x < 0)
        x = -x;
    do {
        t[k++] = (char)('0' + x % 10);
        x /= 10;
    } while (x != 0);
    if (k < 2)
        t[k++] = '0';
    while (k > 0)
        p[n++] = t[--k];
    return n;
}

/* %a: [0x]h.hhhp+d, the leading digit 1 for a normal value (0 for a
   subnormal or zero, with the smallest normal's exponent) */
static size_t fmt_hex(const struct __axc_fp *v, int prec, int alt, int upper, char *buf, size_t cap, char **big,
                      size_t *bigcap) {
    const char *hex = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    unsigned __axiom_int64 hi = v->hi, lo = v->lo;
    int fb = v->mant_bits - 1, nd, lead, x, i;
    size_t need, n = 0;
    unsigned char dg[32];
    char *p;
    /* the fraction bits, left-aligned to a whole number of hex digits */
    nd = (fb + 3) / 4;
    lead = 0;
    x = 0;
    if (v->kind != 0) {
        /* the leading bit's position and the fraction below it */
        int top = fb; /* bit index of a normal's leading bit */
        int shift = nd * 4 - fb, k;
        unsigned __axiom_int64 fhi, flo;
        lead = (top >= 64 ? (int)(hi >> (top - 64)) : (int)(lo >> top)) & 1;
        x = lead ? v->exp + fb : v->min_exp;
        /* the fraction: the significand without its leading bit */
        if (top >= 64) {
            fhi = hi & (((unsigned __axiom_int64)1 << (top - 64)) - 1);
            flo = lo;
        } else {
            fhi = 0;
            flo = top == 64 ? lo : lo & (((unsigned __axiom_int64)1 << top) - 1);
        }
        /* shifted left by shift bits (0..3) */
        if (shift > 0) {
            fhi = (fhi << shift) | (flo >> (64 - shift));
            flo <<= shift;
        }
        for (k = 0; k < nd; k++) {
            int bit = (nd - 1 - k) * 4; /* the digit's low bit */
            dg[k] = (unsigned char)((bit >= 64 ? fhi >> (bit - 64) : (flo >> bit) | (bit > 60 ? fhi << (64 - bit) : 0)) & 15);
        }
    } else {
        for (i = 0; i < nd; i++)
            dg[i] = 0;
    }
    if (prec >= 0 && prec < nd) {
        /* round to prec digits, half to even */
        int up, rest = 0;
        for (i = prec + 1; i < nd; i++)
            rest |= dg[i];
        up = dg[prec] > 8 || (dg[prec] == 8 && (rest || ((prec > 0 ? dg[prec - 1] : (unsigned char)lead) & 1)));
        nd = prec;
        if (up) {
            for (i = nd - 1; i >= 0; i--) {
                if (dg[i] != 15) {
                    dg[i]++;
                    break;
                }
                dg[i] = 0;
            }
            if (i < 0)
                lead++;
        }
    } else if (prec < 0) {
        while (nd > 0 && dg[nd - 1] == 0)
            nd--;
    }
    need = 2 + 1 + 1 + (size_t)(prec > nd ? prec : nd) + 8;
    p = room_for(need, buf, cap, big, bigcap);
    if (p == NULL)
        return 0;
    p[n++] = '0';
    p[n++] = upper ? 'X' : 'x';
    p[n++] = hex[lead];
    if (nd > 0 || prec > 0 || alt)
        p[n++] = '.';
    for (i = 0; i < nd; i++)
        p[n++] = hex[dg[i]];
    for (; i < prec; i++)
        p[n++] = '0';
    p[n++] = upper ? 'P' : 'p';
    p[n++] = x < 0 ? '-' : '+';
    {
        char t[8];
        int k = 0;
        unsigned long ux = (unsigned long)(x < 0 ? -(long)x : (long)x);
        do {
            t[k++] = (char)('0' + ux % 10);
            ux /= 10;
        } while (ux != 0);
        while (k > 0)
            p[n++] = t[--k];
    }
    return n;
}

/* %a of a decimal floating value (see the head of the file) */
static size_t fmt_dec_a(const struct __axc_fp *v, int prec, int alt, int upper, char *buf, size_t cap, char **big,
                        size_t *bigcap) {
    struct big b;
    struct digits d;
    char dig[48], *p;
    int n = 1, q = v->exp, i, dp, ip;
    size_t need, k = 0;
    dig[0] = '0';
    if (v->kind != 0) {
        dp = make_n(&b, v);
        digits_init(&d, &b);
        n = d.total;
        for (i = 0; i < n; i++)
            dig[i] = (char)('0' + digit_at(&d, i));
        if (prec >= 0) {
            int pk = prec == 0 ? 1 : prec;
            if (pk < n) {
                int nk = round_digits(&d, pk, dig, &dp);
                q += n - pk;
                if (nk > pk)
                    q++; /* (carried into a new digit: 10...0, one zero fewer) */
                n = pk;
            }
        }
    }
    need = (size_t)n + 16 + (size_t)(q < 0 ? -q : 0);
    p = room_for(need, buf, cap, big, bigcap);
    if (p == NULL)
        return 0;
    if (q <= 0 && q >= -(n + 5)) {
        ip = n + q; /* the integer digits */
        if (ip <= 0) {
            p[k++] = '0';
            p[k++] = '.';
            for (i = 0; i < -ip; i++)
                p[k++] = '0';
            for (i = 0; i < n; i++)
                p[k++] = dig[i];
        } else {
            for (i = 0; i < ip; i++)
                p[k++] = dig[i];
            if (q < 0 || alt)
                p[k++] = '.';
            for (i = ip; i < n; i++)
                p[k++] = dig[i];
        }
        return k;
    }
    p[k++] = dig[0];
    if (n > 1 || alt)
        p[k++] = '.';
    for (i = 1; i < n; i++)
        p[k++] = dig[i];
    k += put_exp10(p + k, q + n - 1, upper);
    return k;
}

size_t __axc_fmt_fp(const struct __axc_fp *v, int spec, int prec, int alt, char *buf, size_t cap, char **big,
                    size_t *bigcap) {
    struct big b;
    struct digits d;
    int dp, nk, upper = spec >= 'A' && spec <= 'Z', style, strip = 0, i;
    char *dig, *p;
    size_t need, n = 0;
    char small[64];
    char *digbuf = small;
    size_t digcap = 0;
    spec |= 0x20;
    if (spec == 'a' && v->dec)
        return fmt_dec_a(v, prec, alt, upper, buf, cap, big, bigcap);
    if (spec == 'a')
        return fmt_hex(v, prec, alt, upper, buf, cap, big, bigcap);
    if (prec < 0)
        prec = 6;
    if (v->kind == 0) {
        b.n = 0;
        dp = 1;
    } else {
        dp = make_n(&b, v);
    }
    digits_init(&d, &b);
    /* the digits kept: f keeps those down to 10^-prec, e prec + 1 */
    style = spec;
    if (spec == 'g') {
        if (prec == 0)
            prec = 1;
        nk = prec;
    } else if (spec == 'e') {
        nk = prec + 1;
    } else {
        nk = dp + prec;
    }
    if (nk + 2 > (int)sizeof small) {
        digcap = (size_t)nk + 2;
        digbuf = __axc_sys_alloc(digcap);
        if (digbuf == NULL)
            return 0;
    }
    dig = digbuf;
    if (v->kind == 0) {
        nk = 0;
    } else {
        nk = round_digits(&d, nk, dig, &dp);
    }
    if (nk == 0)
        dp = spec == 'f' ? dp : 1; /* (zero: e and g print 0 with exponent 0) */
    if (spec == 'g') {
        /* 7.21.6.1p8: style e if X < -4 or X >= P, else f with P - 1 - X */
        int x = nk == 0 ? 0 : dp - 1;
        if (x < -4 || x >= prec) {
            style = 'e';
            prec = prec - 1;
        } else {
            style = 'f';
            prec = prec - 1 - x;
        }
        strip = !alt;
    }
    if (style == 'e') {
        int x = nk == 0 ? 0 : dp - 1;
        int fd = prec;
        if (strip) {
            /* (no trailing zeros among the fraction digits) */
            fd = nk - 1;
            if (fd < 0)
                fd = 0;
            while (fd > 0 && dig[fd] == '0')
                fd--;
            if (fd > prec)
                fd = prec;
        }
        need = (size_t)fd + 16;
        p = room_for(need, buf, cap, big, bigcap);
        if (p == NULL)
            goto fail;
        p[n++] = nk > 0 ? dig[0] : '0';
        if (fd > 0 || alt)
            p[n++] = '.';
        for (i = 1; i <= fd; i++)
            p[n++] = i < nk ? dig[i] : '0';
        n += put_exp10(p + n, x, upper);
    } else {
        /* f: the integer digits (dp of them), then prec fraction digits */
        int ip = dp > 0 ? dp : 0, fd = prec;
        if (strip) {
            /* the fraction's digits that are not trailing zeros */
            int last = nk - dp; /* fraction digits present */
            if (last > fd)
                last = fd;
            while (last > 0 && (dp + last - 1 < 0 || dig[dp + last - 1] == '0'))
                last--;
            fd = last > 0 ? last : 0;
        }
        need = (size_t)ip + (size_t)fd + 4;
        p = room_for(need, buf, cap, big, bigcap);
        if (p == NULL)
            goto fail;
        if (ip == 0) {
            p[n++] = '0';
        } else {
            for (i = 0; i < ip; i++)
                p[n++] = i < nk ? dig[i] : '0';
        }
        if (fd > 0 || alt)
            p[n++] = '.';
        for (i = 0; i < fd; i++) {
            int k = dp + i;
            p[n++] = k >= 0 && k < nk ? dig[k] : '0';
        }
    }
    if (digcap != 0)
        __axc_sys_free(digbuf, digcap);
    return n;
fail:
    if (digcap != 0)
        __axc_sys_free(digbuf, digcap);
    return 0;
}

/* binary64's bits */
void __axc_fp_double(struct __axc_fp *v, double x) {
    unsigned __axiom_int64 bits;
    unsigned char *s = (unsigned char *)&x, *t = (unsigned char *)&bits;
    int k, e;
    unsigned __axiom_int64 f;
    for (k = 0; k < 8; k++)
        t[k] = s[k];
    v->neg = (int)(bits >> 63);
    e = (int)(bits >> 52) & 0x7ff;
    f = bits & 0xfffffffffffffULL;
    v->hi = 0;
    v->mant_bits = 53;
    v->min_exp = -1022;
    v->dec = 0;
    if (e == 0x7ff) {
        v->kind = f != 0 ? 3 : 2;
        v->lo = f;
        v->exp = 0;
        return;
    }
    if (e == 0) {
        v->kind = f != 0;
        v->lo = f;
        v->exp = -1074;
        return;
    }
    v->kind = 1;
    v->lo = f | (1ULL << 52);
    v->exp = e - 1075;
}

/* long double's bits, as LDBL_MANT_DIG says the format is (the targets are
   little-endian): 53 binary64, 64 the x87 extended format (an explicit
   leading bit; 16 bytes of storage), 113 binary128 */
void __axc_fp_long_double(struct __axc_fp *v, long double x) {
#if LDBL_MANT_DIG == 53
    __axc_fp_double(v, (double)x);
#else
    unsigned char *s = (unsigned char *)&x;
    unsigned __axiom_int64 w0 = 0, w1 = 0;
    int k, e;
    v->dec = 0;
    for (k = 7; k >= 0; k--)
        w0 = (w0 << 8) | s[k];
    for (k = 15; k >= 8; k--)
        w1 = (w1 << 8) | s[k];
#if LDBL_MANT_DIG == 64
    e = (int)(w1 & 0x7fff);
    v->neg = (int)((w1 >> 15) & 1);
    v->hi = 0;
    v->mant_bits = 64;
    v->min_exp = -16382;
    v->lo = w0;
    if (e == 0x7fff) {
        v->kind = (w0 << 1) != 0 ? 3 : 2;
        v->exp = 0;
        return;
    }
    if (e != 0 && !(w0 >> 63)) {
        /* (an unnormal: not a value of the format) */
        v->kind = 3;
        v->exp = 0;
        return;
    }
    v->kind = w0 != 0;
    v->exp = (e == 0 ? 1 : e) - 16383 - 63;
#else
    e = (int)((w1 >> 48) & 0x7fff);
    v->neg = (int)(w1 >> 63);
    v->mant_bits = 113;
    v->min_exp = -16382;
    v->hi = w1 & 0xffffffffffffULL;
    v->lo = w0;
    if (e == 0x7fff) {
        v->kind = (v->hi | v->lo) != 0 ? 3 : 2;
        v->exp = 0;
        return;
    }
    if (e == 0) {
        v->kind = (v->hi | v->lo) != 0;
        v->exp = 1 - 16383 - 112;
        return;
    }
    v->kind = 1;
    v->hi |= 1ULL << 48;
    v->exp = e - 16383 - 112;
#endif
#endif
}

/* A decimal floating value of bytes 4, 8 or 16 (BID, IEEE 754-2019 3.5.2):
   the sign, the combination field's two forms of exponent and
   coefficient, infinity and NaN; a non-canonical coefficient (10^p or
   more) is 0 */
void __axc_fp_dec(struct __axc_fp *v, const void *p, int bytes) {
    const unsigned char *s = (const unsigned char *)p;
    unsigned __axiom_int64 w0 = 0, w1 = 0, c0, c1, g, e;
    int k, kb = bytes * 8, eb = bytes == 4 ? 8 : bytes == 8 ? 10 : 14, cb, bias;
    for (k = (bytes < 8 ? bytes : 8) - 1; k >= 0; k--)
        w0 = (w0 << 8) | s[k];
    if (bytes == 16)
        for (k = 15; k >= 8; k--)
            w1 = (w1 << 8) | s[k];
    cb = kb - 1 - eb;
    bias = bytes == 4 ? 101 : bytes == 8 ? 398 : 6176;
    v->dec = 1;
    v->mant_bits = 0;
    v->min_exp = 0;
    /* (a 128-bit value as w1:w0; the narrow ones in w0) */
    if (bytes == 16) {
        v->neg = (int)(w1 >> 63);
        g = (w1 >> 58) & 31;
    } else {
        v->neg = (int)((w0 >> (kb - 1)) & 1);
        g = (w0 >> (kb - 6)) & 31;
    }
    v->hi = v->lo = 0;
    v->exp = 0;
    if ((g >> 1) == 15) {
        v->kind = g == 30 ? 2 : 3;
        return;
    }
    if ((g >> 3) == 3) {
        /* the second form: 100 then cb - 2 bits; never canonical for 128 bits */
        if (bytes == 16) {
            e = (w1 >> 47) & 0x3fff;
            c1 = c0 = 0;
        } else {
            e = (w0 >> (cb - 2)) & ((1u << eb) - 1);
            c1 = 0;
            c0 = ((unsigned __axiom_int64)4 << (cb - 2)) | (w0 & (((unsigned __axiom_int64)1 << (cb - 2)) - 1));
        }
    } else if (bytes == 16) {
        e = (w1 >> 49) & 0x3fff;
        c1 = w1 & 0x1ffffffffffffULL;
        c0 = w0;
    } else {
        e = (w0 >> cb) & ((1u << eb) - 1);
        c1 = 0;
        c0 = w0 & (((unsigned __axiom_int64)1 << cb) - 1);
    }
    /* non-canonical: 10^7, 10^16 or 10^34 and more */
    if (bytes == 4 && c0 >= 10000000)
        c0 = 0;
    if (bytes == 8 && c0 >= 10000000000000000ULL)
        c0 = 0;
    if (bytes == 16 && (c1 > 0x1ed09bead87c0ULL || (c1 == 0x1ed09bead87c0ULL && c0 >= 0x378d8e6400000000ULL)))
        c1 = c0 = 0;
    v->hi = c1;
    v->lo = c0;
    v->exp = (int)e - bias;
    v->kind = (c1 | c0) != 0;
}
