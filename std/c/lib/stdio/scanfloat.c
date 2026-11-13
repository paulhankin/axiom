/* scanfloat.c - the floating-point input of fscanf (C17 7.21.6.2p12: a
   e f g, as strtod's subject sequence, 7.22.1.3): the scanner of the text
   (from any source of characters, one character of look-ahead) and the
   correctly rounded conversion of the number it read to binary32,
   binary64, the x87 80-bit format or binary128.

   The scanner keeps at most a format's number of significant digits and
   whether any dropped digit was not zero: with enough digits (more than
   any halfway point between two values of the format has), a dropped tail
   is a 1 after them, which rounds the same way. The conversion is exact:
   the decimal digits D and exponent E give D * 10^E; for E >= 0 that is an
   integer, for E < 0 the quotient of D * 2^s by 10^-E is taken to more bits
   than the format has, its remainder telling whether the value is above
   the quotient; then the binary value is rounded once, in the current
   rounding direction (F.5: to nearest with the halfway case to even,
   toward zero, upward, downward; an overflow toward zero gives the largest
   finite value, F.7 / IEC 60559 7.4) (W. D. Clinger, "How to read floating point numbers accurately", PLDI
   1990, for the method; the arithmetic here is plain long division). */
#include "impl.h"

/* (32-bit limbs and bit counts as long: valid with every --c-int) */
typedef uint_least32_t limb;
#define LOW32(x) ((limb)((x) & 0xffffffffUL))

/* -------------------------------------------------------------------------
   Big binary integers: 32-bit limbs, least significant first */

struct bnum {
    limb *l;
    long n, cap;
};

static void bn_set(struct bnum *b, limb v) {
    b->n = v != 0;
    b->l[0] = v;
}

/* b = b * k + add */
static int bn_muladd(struct bnum *b, limb k, limb add) {
    unsigned __axiom_int64 c = add;
    long i;
    for (i = 0; i < b->n; i++) {
        unsigned __axiom_int64 t = (unsigned __axiom_int64)b->l[i] * k + c;
        b->l[i] = LOW32(t);
        c = t >> 32;
    }
    if (c != 0) {
        if (b->n >= b->cap)
            return -1;
        b->l[b->n++] = LOW32(c);
    }
    return 0;
}

/* b = b << s */
static int bn_shl(struct bnum *b, long s) {
    long w = s / 32, r = s % 32, i;
    if (b->n == 0)
        return 0;
    if (b->n + w + 1 > b->cap)
        return -1;
    if (r != 0) {
        b->l[b->n] = 0;
        for (i = b->n; i > 0; i--)
            b->l[i] = LOW32((b->l[i] << r) | (b->l[i - 1] >> (32 - r)));
        b->l[0] = LOW32(b->l[0] << r);
        b->n++;
    }
    if (w != 0) {
        for (i = b->n - 1; i >= 0; i--)
            b->l[i + w] = b->l[i];
        for (i = 0; i < w; i++)
            b->l[i] = 0;
        b->n += w;
    }
    while (b->n > 0 && b->l[b->n - 1] == 0)
        b->n--;
    return 0;
}

static long bn_bits(const struct bnum *b) {
    limb t;
    long n;
    if (b->n == 0)
        return 0;
    t = b->l[b->n - 1];
    n = 32 * (b->n - 1);
    while (t != 0) {
        n++;
        t >>= 1;
    }
    return n;
}

static int bn_bit(const struct bnum *b, long i) {
    if (i < 0 || i / 32 >= b->n)
        return 0;
    return (int)((b->l[i / 32] >> (i % 32)) & 1);
}

/* Is any bit below i set? */
static int bn_low_nonzero(const struct bnum *b, long i) {
    long w = i / 32, k;
    if (i <= 0)
        return 0;
    for (k = 0; k < w && k < b->n; k++) {
        if (b->l[k] != 0)
            return 1;
    }
    if (w < b->n && (i % 32) != 0 && (b->l[w] & (((limb)1 << (i % 32)) - 1)) != 0)
        return 1;
    return 0;
}

/* Limb i of t << s (s = 32 w + r) */
static limb shifted_limb(const struct bnum *t, long w, long r, long i) {
    long j = i - w;
    limb lo, hi;
    if (j < 0 || j > t->n)
        return 0;
    hi = j < t->n ? t->l[j] : 0;
    if (r == 0)
        return hi;
    lo = j > 0 ? t->l[j - 1] : 0;
    return LOW32((hi << r) | (lo >> (32 - r)));
}

/* a >= t << s ? */
static int bn_ge_shifted(const struct bnum *a, const struct bnum *t, long s) {
    long w = s / 32, r = s % 32, top = t->n + w + 1, i;
    if (a->n > top)
        return 1;
    for (i = top - 1; i >= 0; i--) {
        limb x = i < a->n ? a->l[i] : 0, y = shifted_limb(t, w, r, i);
        if (x != y)
            return x > y;
    }
    return 1;
}

/* a -= t << s (a >= t << s) */
static void bn_sub_shifted(struct bnum *a, const struct bnum *t, long s) {
    long w = s / 32, r = s % 32, i;
    unsigned __axiom_int64 borrow = 0;
    for (i = w; i < a->n; i++) {
        unsigned __axiom_int64 y = (unsigned __axiom_int64)shifted_limb(t, w, r, i) + borrow;
        unsigned __axiom_int64 x = a->l[i];
        borrow = x < y;
        a->l[i] = LOW32(x - y);
    }
    while (a->n > 0 && a->l[a->n - 1] == 0)
        a->n--;
}

/* -------------------------------------------------------------------------
   The formats */

struct fmt {
    int p;    /* significand bits, the leading one included */
    int emin; /* the exponent of the smallest normal's leading bit */
    int emax; /* the exponent of the largest finite value's leading bit */
    int digits; /* significant digits that decide any rounding */
};

static const struct fmt F32 = {24, -126, 127, 120};
static const struct fmt F64 = {53, -1022, 1023, 800};
#if LDBL_MANT_DIG == 64
static const struct fmt FLD = {64, -16382, 16383, 11600};
#elif LDBL_MANT_DIG == 113
static const struct fmt FLD = {113, -16382, 16383, 11600};
#else
#define FLD F64
#endif

/* The rounded result: m * 2^e (m < 2^p), or infinity */
struct res {
    unsigned __axiom_int64 hi, lo;
    long e;
    int inf, inexact;
    int over; /* overflowed (to infinity, or to the largest finite value) */
};

/* An overflow's result: infinity, or toward zero the largest finite value */
static void overflow(const struct fmt *f, struct res *r, int dir) {
    r->inexact = 1;
    r->over = 1;
    if (dir >= 0) {
        r->inf = 1;
        return;
    }
    r->inf = 0;
    r->e = f->emax - (f->p - 1);
    r->lo = f->p >= 64 ? ~(unsigned __axiom_int64)0 : ((unsigned __axiom_int64)1 << f->p) - 1;
    r->hi = f->p > 64 ? ((unsigned __axiom_int64)1 << (f->p - 64)) - 1 : 0;
}

/* Rounds q * 2^-s (plus a little more if sticky) to the format, with the
   subnormal values below emin; dir: 0 half to even, 1 away from zero, -1
   toward zero (__axc_round_dir). */
static void round_to(const struct fmt *f, const struct bnum *q, long s, int sticky, struct res *r, int dir) {
    long nb = bn_bits(q), lead, keep, drop, i;
    r->hi = r->lo = 0;
    r->inf = r->over = 0;
    r->inexact = sticky;
    r->e = 0;
    if (nb == 0)
        return;
    lead = nb - 1 - s;
    if (lead > f->emax) {
        overflow(f, r, dir);
        return;
    }
    keep = f->p;
    if (lead < f->emin)
        keep = f->p - (f->emin - lead);
    drop = nb - keep;
    if (drop <= 0) {
        /* exact: q shifted up */
        for (i = nb - 1; i >= 0; i--) {
            long b = i - drop;
            if (bn_bit(q, i)) {
                if (b >= 64)
                    r->hi |= (unsigned __axiom_int64)1 << (b - 64);
                else
                    r->lo |= (unsigned __axiom_int64)1 << b;
            }
        }
        r->e = -s + drop;
    } else {
        int half = bn_bit(q, drop - 1), rest = sticky || bn_low_nonzero(q, drop - 1), up;
        for (i = drop; i < nb; i++) {
            long b = i - drop;
            if (bn_bit(q, i)) {
                if (b >= 64)
                    r->hi |= (unsigned __axiom_int64)1 << (b - 64);
                else
                    r->lo |= (unsigned __axiom_int64)1 << b;
            }
        }
        r->inexact = half || rest;
        if (dir == 0)
            up = half && (rest || (r->lo & 1));
        else
            up = dir > 0 && (half || rest);
        if (up) {
            r->lo++;
            if (r->lo == 0)
                r->hi++;
        }
        r->e = -s + drop;
        /* a carry to 2^keep: one more bit (for a subnormal, it may become
           the smallest normal, which has the same exponent of its lsb) */
        if (keep > 0 && ((keep >= 64 ? r->hi >> (keep - 64) : r->lo >> keep) & 1) && keep == f->p) {
            r->lo = (r->lo >> 1) | (r->hi << 63);
            r->hi >>= 1;
            r->e++;
        }
    }
    if (r->e + f->p - 1 > f->emax)
        overflow(f, r, dir);
}

/* -------------------------------------------------------------------------
   The scanned number */

/* The decimal or hexadecimal number n to the format f; *range set when
   the result overflows or underflows (is not normal and inexact). */
static void convert(const struct fmt *f, struct __axc_num *n, struct res *r, int *range) {
    limb store[3 * 140];
    limb *heap = NULL;
    size_t heapn = 0;
    struct bnum q, t;
    long s = 0, i, limbs;
    int sticky = 0, dir = __axc_round_dir(n->neg);
    r->hi = r->lo = 0;
    r->e = 0;
    r->inf = r->over = 0;
    r->inexact = 0;
    *range = 0;
    if (n->kind == 1) {
        limb l[4];
        q.l = l;
        q.cap = 4;
        q.n = 4;
        l[0] = LOW32(n->mlo);
        l[1] = LOW32(n->mlo >> 32);
        l[2] = LOW32(n->mhi);
        l[3] = LOW32(n->mhi >> 32);
        while (q.n > 0 && q.l[q.n - 1] == 0)
            q.n--;
        if (q.n == 0)
            return;
        /* (an exponent far out of range: clamp it, keeping the result) */
        if (n->bexp > 100000)
            n->bexp = 100000;
        if (n->bexp < -100000)
            n->bexp = -100000;
        round_to(f, &q, -n->bexp, n->sticky, r, dir);
        goto done;
    }
    if (n->nd == 0)
        return;
    /* out of range at once: above 10^(emax+1)log2.. or below the smallest
       subnormal's half */
    if (n->dexp > (long)(f->emax + 1) * 30103 / 100000 + 1) {
        overflow(f, r, dir);
        *range = 1;
        return;
    }
    if (n->dexp < (long)(f->emin - f->p) * 30103 / 100000 - 1) {
        /* (0, or away from zero the smallest subnormal) */
        if (dir > 0) {
            r->lo = 1;
            r->e = f->emin - (f->p - 1);
        }
        r->inexact = 1;
        *range = 1;
        return;
    }
    if (n->sticky) {
        /* the dropped digits as a 1 after those kept */
        n->d[n->nd++] = 1;
    }
    {
        long e10 = n->dexp - n->nd;
        /* sizes: D < 10^nd, 10^|e10|, and the quotient's bits */
        long dbits = (long)n->nd * 3322 / 1000 + 2, tbits = (e10 < 0 ? -e10 : e10) * 3322 / 1000 + 2;
        long need = (e10 >= 0 ? dbits + tbits : (dbits > tbits ? dbits : tbits) + f->p + 8) / 32 + 4;
        limbs = need;
        if (2 * limbs > (long)(sizeof store / sizeof store[0])) {
            heapn = (size_t)2 * (size_t)limbs * sizeof(limb);
            heap = __axc_sys_alloc(heapn);
            if (heap == NULL) {
                r->inf = 0;
                *range = 1;
                return;
            }
            q.l = heap;
        } else {
            q.l = store;
        }
        t.l = q.l + limbs;
        q.cap = t.cap = limbs;
        /* D, nine digits at a time */
        bn_set(&q, 0);
        for (i = 0; i < n->nd;) {
            limb chunk = 0, mul = 1;
            int k;
            for (k = 0; k < 9 && i < n->nd; k++, i++) {
                chunk = chunk * 10 + (limb)n->d[i];
                mul *= 10;
            }
            bn_muladd(&q, mul, chunk);
        }
        if (e10 >= 0) {
            for (; e10 >= 9; e10 -= 9)
                bn_muladd(&q, 1000000000u, 0);
            if (e10 > 0) {
                limb p10 = 1;
                while (e10-- > 0)
                    p10 *= 10;
                bn_muladd(&q, p10, 0);
            }
            round_to(f, &q, 0, 0, r, dir);
        } else {
            /* T = 10^-e10; q = floor(D * 2^s / T), the remainder sticky */
            long k = -e10;
            struct bnum quo;
            limb ql[6];
            long db, tb, qb;
            bn_set(&t, 1);
            for (; k >= 9; k -= 9)
                bn_muladd(&t, 1000000000u, 0);
            if (k > 0) {
                limb p10 = 1;
                while (k-- > 0)
                    p10 *= 10;
                bn_muladd(&t, p10, 0);
            }
            db = bn_bits(&q);
            tb = bn_bits(&t);
            /* q = D * 2^s / T has p + 3 or p + 4 bits: for s < 0 (D far
               above T), T * 2^-s is the divisor */
            s = f->p + 3 - db + tb;
            if (s >= 0)
                bn_shl(&q, s);
            else
                bn_shl(&t, -s);
            tb = bn_bits(&t);
            /* the quotient has qb or qb - 1 bits */
            qb = bn_bits(&q) - tb + 1;
            quo.l = ql;
            quo.cap = 6;
            quo.n = 0;
            for (i = 0; i < 6; i++)
                ql[i] = 0;
            for (i = qb - 1; i >= 0; i--) {
                if (bn_ge_shifted(&q, &t, i)) {
                    bn_sub_shifted(&q, &t, i);
                    ql[i / 32] |= (limb)1 << (i % 32);
                }
            }
            quo.n = 6;
            while (quo.n > 0 && quo.l[quo.n - 1] == 0)
                quo.n--;
            sticky = q.n != 0;
            round_to(f, &quo, s, sticky, r, dir);
        }
    }
    if (heap != NULL)
        __axc_sys_free(heap, heapn);
done:
    /* (7.22.1.3p10: overflow; underflow when the result is not normal and
       inexact, or zero) */
    if (r->inf || r->over)
        *range = 1;
    else if (r->inexact && !((f->p > 64 ? r->hi >> (f->p - 1 - 64) : r->lo >> (f->p - 1)) & 1))
        *range = 1;
}

/* -------------------------------------------------------------------------
   The scanner */

static int lower(int c) {
    return c >= 'A' && c <= 'Z' ? c + ('a' - 'A') : c;
}

static int hexval(int c) {
    if (c >= '0' && c <= '9')
        return c - '0';
    c = lower(c);
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return -1;
}

/* Adds a decimal digit (point: after the point) */
static void add_digit(struct __axc_num *n, int v, int point) {
    if (v == 0 && n->nd == 0) {
        /* leading zeros only move the point */
        if (point)
            n->dexp--;
        return;
    }
    if (n->nd < n->cap - 1)
        n->d[n->nd++] = (char)v;
    else if (v != 0)
        n->sticky = 1;
    if (!point && n->dexp < 1000000000L)
        n->dexp++;
}

/* Adds a hexadecimal digit */
static void add_hex(struct __axc_num *n, int v, int point) {
    if (v == 0 && n->mhi == 0 && n->mlo == 0) {
        if (point)
            n->bexp -= 4;
        return;
    }
    if ((n->mhi >> 52) == 0) {
        n->mhi = (n->mhi << 4) | (n->mlo >> 60);
        n->mlo = (n->mlo << 4) | (unsigned)v;
        if (point)
            n->bexp -= 4;
    } else {
        if (v != 0)
            n->sticky = 1;
        if (!point)
            n->bexp += 4;
    }
}

/* Reads a number as strtod's subject sequence (7.22.1.3p3) from in, at
   most width characters (width < 0: no limit), taking a character only
   while what was taken is a prefix of a number (so that the character that
   ends it is the only one given back, 7.21.6.2p9). *taken counts the
   characters taken, *valid those of the longest prefix that is a whole
   number (strtod's end), whose value n holds. 0 if no character was taken
   for want of input (end of file), 1 otherwise. */
int __axc_scan_num(struct __axc_in *in, long width, struct __axc_num *n, size_t *taken, size_t *valid) {
    size_t t = 0;
    int c, point = 0, digits = 0, i;
    long save_dexp, save_bexp;
    int save_nd;
#define NEXT() ((width < 0 || (long)t < width) ? (c = in->get(in), c >= 0 ? (t++, 1) : 0) : (c = -1, 0))
#define BACK() (in->unget(in, c), t--)
    n->kind = 0;
    n->neg = 0;
    n->nd = 0;
    n->sticky = 0;
    n->dexp = 0;
    n->mhi = n->mlo = 0;
    n->bexp = 0;
    *valid = 0;
    if (!NEXT()) {
        *taken = 0;
        return 0;
    }
    if (c == '+' || c == '-') {
        n->neg = c == '-';
        if (!NEXT())
            goto end;
    }
    if (lower(c) == 'i' || lower(c) == 'n') {
        /* inf, infinity, nan, nan(n-char-sequence) */
        const char *w = lower(c) == 'i' ? "infinity" : "nan";
        n->kind = lower(c) == 'i' ? 2 : 3;
        for (i = 1; w[i] != 0; i++) {
            if (!NEXT())
                goto end;
            if (lower(c) != w[i]) {
                BACK();
                goto end;
            }
            if (i == 2)
                *valid = t;
        }
        *valid = t;
        if (n->kind == 3) {
            if (!NEXT())
                goto end;
            if (c != '(') {
                BACK();
                goto end;
            }
            for (;;) {
                if (!NEXT())
                    goto end;
                if (c == ')') {
                    *valid = t;
                    goto end;
                }
                if (!((c >= '0' && c <= '9') || (lower(c) >= 'a' && lower(c) <= 'z') || c == '_')) {
                    BACK();
                    goto end;
                }
            }
        }
        goto end;
    }
    if (c == '0') {
        /* 0x: hexadecimal, if a digit follows */
        digits = 1;
        *valid = t;
        if (!NEXT())
            goto end;
        if (lower(c) == 'x' && !n->nohex) {
            int hd = 0;
            n->kind = 1;
            for (;;) {
                if (!NEXT())
                    break;
                if (c == '.' && !point) {
                    point = 1;
                    continue;
                }
                if (hexval(c) < 0) {
                    BACK();
                    break;
                }
                hd = 1;
                add_hex(n, hexval(c), point);
                *valid = t;
            }
            if (!hd) {
                /* "0x" alone: the number is the 0 */
                n->kind = 0;
                goto end;
            }
            /* the binary exponent: p, a sign, digits */
            if (c != 'p' && c != 'P')
                goto end;
            if (!NEXT())
                goto end;
            if (c != 'p' && c != 'P') {
                BACK();
                goto end;
            }
            save_bexp = n->bexp;
            {
                int eneg = 0, ed = 0;
                long ev = 0;
                if (!NEXT())
                    goto end;
                if (c == '+' || c == '-') {
                    eneg = c == '-';
                    if (!NEXT())
                        goto end;
                }
                while (c >= '0' && c <= '9') {
                    ed = 1;
                    if (ev < 100000000L)
                        ev = ev * 10 + (c - '0');
                    *valid = t;
                    n->bexp = save_bexp + (eneg ? -ev : ev);
                    if (!NEXT())
                        goto end;
                }
                BACK();
                (void)ed;
            }
            goto end;
        }
        BACK();
        if (!NEXT())
            goto end;
    }
    /* decimal digits, a point, digits */
    for (;;) {
        if (c >= '0' && c <= '9') {
            digits = 1;
            add_digit(n, c - '0', point);
            *valid = t;
        } else if (c == '.' && !point) {
            point = 1;
            if (digits)
                *valid = t;
        } else {
            break;
        }
        if (!NEXT())
            goto end;
    }
    if (!digits) {
        BACK();
        goto end;
    }
    if (c != 'e' && c != 'E') {
        BACK();
        goto end;
    }
    /* the exponent: e, a sign, digits */
    save_dexp = n->dexp;
    save_nd = n->nd;
    (void)save_nd;
    {
        int eneg = 0;
        long ev = 0;
        if (!NEXT())
            goto end;
        if (c == '+' || c == '-') {
            eneg = c == '-';
            if (!NEXT())
                goto end;
        }
        while (c >= '0' && c <= '9') {
            if (ev < 100000000L)
                ev = ev * 10 + (c - '0');
            *valid = t;
            n->dexp = save_dexp + (eneg ? -ev : ev);
            if (!NEXT())
                goto end;
        }
        BACK();
    }
end:
    *taken = t;
    return 1;
#undef NEXT
#undef BACK
}

/* The value of n (scanned) in each format */
static void to_parts(const struct fmt *f, struct __axc_num *n, struct res *r, int *range) {
    if (n->kind >= 2) {
        r->inf = n->kind == 2;
        r->over = 0;
        r->hi = r->lo = 0;
        r->e = 0;
        *range = 0;
        return;
    }
    convert(f, n, r, range);
}

double __axc_num_double(struct __axc_num *n, int *range) {
    struct res r;
    unsigned __axiom_int64 bits;
    double x;
    unsigned char *s = (unsigned char *)&bits, *d = (unsigned char *)&x;
    int k;
    /* exact cases (Clinger's fast path): up to 15 digits times or over an
       exact power of ten round once */
    if (n->kind == 0 && !n->sticky && n->nd <= 15 && n->nd > 0) {
        long e10 = n->dexp - n->nd;
        if (e10 >= -22 && e10 <= 22) {
            static const double p10[23] = {1e0,  1e1,  1e2,  1e3,  1e4,  1e5,  1e6,  1e7,  1e8,  1e9,  1e10, 1e11,
                                           1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22};
            __axiom_int64 v = 0;
            for (k = 0; k < n->nd; k++)
                v = v * 10 + n->d[k];
            /* (the sign first: the one rounded operation then rounds in
               the current direction for the signed value) */
            x = (double)(n->neg ? -v : v);
            x = e10 < 0 ? x / p10[-e10] : x * p10[e10];
            *range = 0;
            return x;
        }
    }
    to_parts(&F64, n, &r, range);
    if (n->kind == 3)
        bits = 0x7ff8000000000000ULL;
    else if (r.inf)
        bits = 0x7ff0000000000000ULL;
    else if (r.lo >> 52)
        bits = ((unsigned __axiom_int64)(r.e + 52 + 1023) << 52) | (r.lo & 0xfffffffffffffULL);
    else
        bits = r.lo; /* subnormal or zero */
    if (n->neg)
        bits |= 0x8000000000000000ULL;
    for (k = 0; k < 8; k++)
        d[k] = s[k];
    return x;
}

float __axc_num_float(struct __axc_num *n, int *range) {
    struct res r;
    limb bits;
    float x;
    unsigned char *s = (unsigned char *)&bits, *d = (unsigned char *)&x;
    int k;
    to_parts(&F32, n, &r, range);
    if (n->kind == 3)
        bits = 0x7fc00000u;
    else if (r.inf)
        bits = 0x7f800000u;
    else if (r.lo >> 23)
        bits = ((limb)(r.e + 23 + 127) << 23) | ((limb)r.lo & 0x7fffffu);
    else
        bits = (limb)r.lo;
    if (n->neg)
        bits |= 0x80000000u;
    for (k = 0; k < 4; k++)
        d[k] = s[k];
    return x;
}

long double __axc_num_long_double(struct __axc_num *n, int *range) {
#if LDBL_MANT_DIG == 53
    return __axc_num_double(n, range);
#else
    struct res r;
    long double x = 0;
    unsigned char *d = (unsigned char *)&x;
    unsigned __axiom_int64 w0, w1;
    int k;
    to_parts(&FLD, n, &r, range);
#if LDBL_MANT_DIG == 64
    /* the explicit leading bit; exponent 0 for subnormals */
    if (n->kind == 3) {
        w0 = 0xc000000000000000ULL;
        w1 = 0x7fff;
    } else if (r.inf) {
        w0 = 0x8000000000000000ULL;
        w1 = 0x7fff;
    } else {
        w0 = r.lo;
        w1 = (r.lo >> 63) ? (unsigned __axiom_int64)(r.e + 63 + 16383) : 0;
    }
    if (n->neg)
        w1 |= 0x8000;
#else
    if (n->kind == 3) {
        w0 = 0;
        w1 = 0x7fff800000000000ULL;
    } else if (r.inf) {
        w0 = 0;
        w1 = 0x7fff000000000000ULL;
    } else {
        w0 = r.lo;
        w1 = r.hi & 0xffffffffffffULL;
        if (r.hi >> 48)
            w1 |= (unsigned __axiom_int64)(r.e + 112 + 16383) << 48;
    }
    if (n->neg)
        w1 |= 0x8000000000000000ULL;
#endif
    for (k = 0; k < 8; k++) {
        d[k] = (unsigned char)(w0 >> (8 * k));
        d[8 + k] = (unsigned char)(w1 >> (8 * k));
    }
    return x;
#endif
}

/* binary16 and binary128 (_Float16, _Float128: strtof16, strtof128),
   whatever long double is */
static const struct fmt F16 = {11, -14, 15, 40};
static const struct fmt F128 = {113, -16382, 16383, 11600};

/* n in binary16 (which 0) or binary128 (which 1): its 2 or 16 bytes, little-endian, at out */
void __axc_num_bits(struct __axc_num *n, int *range, int which, unsigned char *out) {
    struct res r;
    unsigned __axiom_int64 w0, w1;
    int k;
    if (which == 0) {
        unsigned int bits;
        to_parts(&F16, n, &r, range);
        if (n->kind == 3)
            bits = 0x7e00u;
        else if (r.inf)
            bits = 0x7c00u;
        else if (r.lo >> 10)
            bits = ((unsigned int)(r.e + 10 + 15) << 10) | ((unsigned int)r.lo & 0x3ffu);
        else
            bits = (unsigned int)r.lo;
        if (n->neg)
            bits |= 0x8000u;
        out[0] = (unsigned char)(bits & 0xff);
        out[1] = (unsigned char)(bits >> 8);
        return;
    }
    to_parts(&F128, n, &r, range);
    if (n->kind == 3) {
        w0 = 0;
        w1 = 0x7fff800000000000ULL;
    } else if (r.inf) {
        w0 = 0;
        w1 = 0x7fff000000000000ULL;
    } else {
        w0 = r.lo;
        w1 = r.hi & 0xffffffffffffULL;
        if (r.hi >> 48)
            w1 |= (unsigned __axiom_int64)(r.e + 112 + 16383) << 48;
    }
    if (n->neg)
        w1 |= 0x8000000000000000ULL;
    for (k = 0; k < 8; k++) {
        out[k] = (unsigned char)(w0 >> (8 * k));
        out[8 + k] = (unsigned char)(w1 >> (8 * k));
    }
}

/* The digit room a conversion of each format needs (struct __axc_num.cap):
   ld 0 binary64's (and binary32's), 1 long double's, 2 binary128's */
int __axc_num_digits(int ld) {
    if (ld == 2)
        return F128.digits + 2;
    return ld ? FLD.digits + 2 : F64.digits + 2;
}

/* strtold's work (<stdlib.h>, C17 7.22.1.3): the longest initial part of s,
   after white space, that is a subject sequence, in long double's format
   (whichever it is: LDBL_MANT_DIG); *end after it (s if there is none),
   *range when the result overflows or underflows. */
static int text_get(struct __axc_in *i) {
    return *i->s ? *i->s++ : -1;
}

static void text_unget(struct __axc_in *i, int c) {
    (void)c;
    i->s--;
}

/* The subject sequence of strtod at s (white space skipped) in the
   format: 0 float, 1 double, 2 long double (*out of that type), 3
   binary16, 4 binary128 (the bytes at out: strtof16, strtof128); *end
   past it (s if there is none); *range on overflow and underflow. */
static void text_scan(const char *s, char **end, int format, void *out, int *range) {
    struct __axc_in in;
    struct __axc_num n;
    size_t taken, valid, cap = (size_t)__axc_num_digits(format == 2 || format == 4);
    const char *p = s;
    char small[804];
    *range = 0;
    while (*p == ' ' || (*p >= '\t' && *p <= '\r'))
        p++;
    in.get = text_get;
    in.unget = text_unget;
    in.count = 0;
    in.f = NULL;
    in.s = (const unsigned char *)p;
    in.ws = NULL;
    in.eof = in.error = 0;
    in.c23 = 0;
    in.viol = NULL;
    n.nohex = 0;
    n.d = small;
    if (cap > sizeof small)
        n.d = __axc_sys_alloc(cap);
    if (n.d == NULL || cap <= sizeof small) {
        /* (within binary64's digits: exact for it, and for wider formats
           the halfway cases that have no more) */
        n.d = small;
        cap = sizeof small;
    }
    n.cap = (int)cap;
    if (format == 0)
        *(float *)out = 0;
    else if (format == 1)
        *(double *)out = 0;
    else if (format == 2)
        *(long double *)out = 0;
    else
        for (taken = 0; taken < (format == 4 ? 16u : 2u); taken++)
            ((unsigned char *)out)[taken] = 0;
    if (__axc_scan_num(&in, -1, &n, &taken, &valid) && valid > 0) {
        if (format == 0)
            *(float *)out = __axc_num_float(&n, range);
        else if (format == 1)
            *(double *)out = __axc_num_double(&n, range);
        else if (format == 2)
            *(long double *)out = __axc_num_long_double(&n, range);
        else
            __axc_num_bits(&n, range, format - 3, (unsigned char *)out);
        p += valid;
    } else {
        p = s;
    }
    if (n.d != small)
        __axc_sys_free(n.d, cap);
    if (end != NULL)
        *end = (char *)p;
}

long double __axc_strtold(const char *s, char **end, int *range) {
    long double v;
    text_scan(s, end, 2, &v, range);
    return v;
}

/* strtod's and strtof's conversion when the rounding direction is not to
   nearest (std/c/lib/core/strtod.c, strtof.c: their own parser rounds to
   nearest only) */
double __axc_strtod_dir(const char *s, char **end, int *range) {
    double v;
    text_scan(s, end, 1, &v, range);
    return v;
}

float __axc_strtof_dir(const char *s, char **end, int *range) {
    float v;
    text_scan(s, end, 0, &v, range);
    return v;
}

/* -------------------------------------------------------------------------
   The decimal floating types (C23): a scanned number's value in _Decimal32,
   _Decimal64 or _Decimal128 (BID), rounded once in the decimal rounding
   direction (fe_dec_getround) when it has more digits than the format, or
   is out of its exponent range; exact, the quantum it was written with
   (C23 7.24.1.6p?: "the quantum exponent of the result is the exponent
   of the subject sequence's last digit"), or the nearest the format has.
   The same rules as std/c/rt/decimal.ax's (finish), which the arithmetic
   rounds with. */

unsigned __axiom_int64 __axc_dec_round_get(void);

struct decfmt {
    int p, bytes;
    long qmin, qmax, emin;
};

static void dec_store(const struct decfmt *f, int neg, unsigned __axiom_int64 hi, unsigned __axiom_int64 lo, long q,
                      int kind, void *dst) {
    /* kind 0 finite, 1 infinity, 2 NaN */
    unsigned char *o = (unsigned char *)dst;
    unsigned __axiom_int64 w0 = 0, w1 = 0, e = (unsigned __axiom_int64)(q - f->qmin);
    int k, kb = f->bytes * 8, cb = f->bytes == 4 ? 23 : f->bytes == 8 ? 53 : 113;
    if (f->bytes == 16) {
        w1 = (unsigned __axiom_int64)neg << 63;
        if (kind == 1)
            w1 |= (unsigned __axiom_int64)30 << 58;
        else if (kind == 2)
            w1 |= (unsigned __axiom_int64)31 << 58;
        else {
            w1 |= (e << 49) | hi;
            w0 = lo;
        }
    } else {
        w0 = (unsigned __axiom_int64)neg << (kb - 1);
        if (kind == 1)
            w0 |= (unsigned __axiom_int64)30 << (kb - 6);
        else if (kind == 2)
            w0 |= (unsigned __axiom_int64)31 << (kb - 6);
        else if (lo < ((unsigned __axiom_int64)1 << cb))
            w0 |= (e << cb) | lo;
        else
            w0 |= ((unsigned __axiom_int64)3 << (kb - 3)) | (e << (cb - 2)) | (lo & (((unsigned __axiom_int64)1 << (cb - 2)) - 1));
    }
    for (k = 0; k < f->bytes && k < 8; k++)
        o[k] = (unsigned char)(w0 >> (8 * k));
    for (k = 8; k < f->bytes; k++)
        o[k] = (unsigned char)(w1 >> (8 * (k - 8)));
}

/* the coefficient of digits d[0 .. n) as a 128-bit number */
static void dec_coef(const char *d, int n, unsigned __axiom_int64 *hi, unsigned __axiom_int64 *lo) {
    unsigned __axiom_int64 h = 0, l = 0;
    int i;
    for (i = 0; i < n; i++) {
        /* (h:l) * 10 + d */
        unsigned __axiom_int64 l8 = l << 3, l2 = l << 1, s, t;
        unsigned __axiom_int64 h10 = (h << 3) + (h << 1) + (l >> 61) + (l >> 63);
        s = l8 + l2;
        if (s < l8)
            h10++;
        t = s + (unsigned __axiom_int64)d[i];
        if (t < s)
            h10++;
        h = h10;
        l = t;
    }
    *hi = h;
    *lo = l;
}

void __axc_num_dec(struct __axc_num *n, int dfmt, void *dst, int *range) {
    static const struct decfmt fmts[3] = {{7, 4, -101, 90, -95}, {16, 8, -398, 369, -383}, {34, 16, -6176, 6111, -6143}};
    const struct decfmt *f = &fmts[dfmt - 1];
    int nd = n->nd, tz = 0, keep, first, rest, up, i, mode, neg = n->neg;
    long q, lo, e, hi_e;
    char dig[40];
    unsigned __axiom_int64 ch, cl;
    *range = 0;
    if (n->kind == 2 || n->kind == 3) {
        dec_store(f, neg, 0, 0, 0, n->kind - 1, dst);
        return;
    }
    q = n->dexp - nd;
    if (q < -2000000000L)
        q = -2000000000L;
    while (tz < nd && n->d[nd - 1 - tz] == 0)
        tz++;
    if (nd == tz && !n->sticky) {
        /* zero, with the written quantum (or the nearest the format has) */
        e = q < f->qmin ? f->qmin : q > f->qmax ? f->qmax : q;
        dec_store(f, neg, 0, 0, e, 0, dst);
        return;
    }
    lo = q + nd - f->p;
    if (!n->sticky) {
        long elo = lo > f->qmin ? lo : f->qmin, ehi = q + tz < f->qmax ? q + tz : f->qmax;
        if (elo <= ehi) {
            e = q < elo ? elo : q > ehi ? ehi : q;
            /* the digits down to 10^e: dropped zeros, or added ones */
            keep = (int)(nd - (e - q));
            for (i = 0; i < keep; i++)
                dig[i] = i < nd ? n->d[i] : 0;
            dec_coef(dig, keep, &ch, &cl);
            dec_store(f, neg, ch, cl, e, 0, dst);
            return;
        }
    }
    mode = (int)__axc_dec_round_get();
    if (lo > f->qmax)
        goto overflow;
    e = lo > f->qmin ? lo : f->qmin;
    keep = (long)nd - (e - q) > 0 ? (int)(nd - (e - q)) : 0;
    first = keep < nd && keep >= 0 && (long)nd - (e - q) >= 0 ? n->d[keep] : 0;
    rest = n->sticky;
    for (i = keep + ((long)nd - (e - q) >= 0 ? 1 : 0); i < nd; i++)
        rest |= n->d[i] != 0;
    for (i = 0; i < keep; i++)
        dig[i] = n->d[i];
    {
        int odd = keep > 0 && (dig[keep - 1] & 1), inexact = first != 0 || rest;
        if (!inexact)
            up = 0;
        else if (mode == 0)
            up = first > 5 || (first == 5 && (rest || odd));
        else if (mode == 4)
            up = first >= 5;
        else if (mode == 3)
            up = 0;
        else if (mode == 2)
            up = !neg;
        else
            up = neg;
        if (up) {
            for (i = keep - 1; i >= 0; i--) {
                if (dig[i] != 9) {
                    dig[i]++;
                    break;
                }
                dig[i] = 0;
            }
            if (i < 0) {
                /* 0 or 99...9 rounded up: 1 followed by the zeros */
                for (i = keep; i > 0; i--)
                    dig[i] = dig[i - 1];
                dig[0] = 1;
                keep++;
                if (keep > f->p) {
                    keep--;
                    e++;
                }
            }
        }
        if (e > f->qmax)
            goto overflow;
        if (inexact && q + nd - 1 < f->emin)
            *range = 1; /* (underflow: a tiny inexact result) */
    }
    dec_coef(dig, keep, &ch, &cl);
    dec_store(f, neg, ch, cl, e, 0, dst);
    return;
overflow:
    /* infinity, or the largest value, as the rounding direction says */
    *range = 1;
    mode = (int)__axc_dec_round_get();
    if (mode == 3 || (mode == 2 && neg) || (mode == 1 && !neg)) {
        for (i = 0; i < f->p; i++)
            dig[i] = 9;
        dec_coef(dig, f->p, &ch, &cl);
        dec_store(f, neg, ch, cl, f->qmax, 0, dst);
    } else {
        dec_store(f, neg, 0, 0, 0, 1, dst);
    }
    (void)hi_e;
}

void __axc_strtodec(const char *s, char **end, int dfmt, void *dst, int *range) {
    struct __axc_in in;
    struct __axc_num n;
    size_t taken, valid;
    const char *p = s;
    char small[64];
    unsigned char *o = (unsigned char *)dst;
    int k;
    *range = 0;
    while (*p == ' ' || (*p >= '\t' && *p <= '\r'))
        p++;
    in.get = text_get;
    in.unget = text_unget;
    in.count = 0;
    in.f = NULL;
    in.s = (const unsigned char *)p;
    in.ws = NULL;
    in.eof = in.error = 0;
    in.c23 = 1;
    in.viol = NULL;
    n.nohex = 1;
    n.d = small;
    n.cap = (int)sizeof small; /* (more than any format's p + 3 digits) */
    if (__axc_scan_num(&in, -1, &n, &taken, &valid) && valid > 0) {
        __axc_num_dec(&n, dfmt, dst, range);
        p += valid;
    } else {
        /* no subject sequence: +0 (exponent 0) */
        for (k = 0; k < (dfmt == 1 ? 4 : dfmt == 2 ? 8 : 16); k++)
            o[k] = 0;
        o[dfmt == 1 ? 3 : dfmt == 2 ? 7 : 15] = dfmt == 1 ? 0x32 : dfmt == 2 ? 0x31 : 0x30;
        if (dfmt == 1)
            o[2] = 0x80;
        else if (dfmt == 2)
            o[6] = 0xc0;
        else
            o[14] = 0x40;
        p = s;
    }
    if (end != NULL)
        *end = (char *)p;
}

/* strtof16 and strtof128's work (which 0 binary16, 1 binary128): the
   result's bytes at out */
void __axc_strtobits(const char *s, char **end, int *range, int which, unsigned char *out) {
    text_scan(s, end, 3 + which, out, range);
}
