#!/usr/bin/env python3
"""Writes the C library's tests of the software floating arithmetic in the
floating-point environment: binary128 (_Float128, every target with an
environment), the x87's 80-bit format (long double on x86-64 Linux) and
binary16 (_Float16), each operation in the four rounding directions with the
exceptions it raises (IEEE 754-2019: correctly rounded, tininess detected
after rounding, "underflow" for a tiny inexact result).

    cmd/axiom/internal/tests/tools/gen_fenv_wide_tests.py

writes cmd/axiom/internal/tests/c/lib/math/fenv_wide_q.c, fenv_wide_x.c and
fenv_half.c. The expected results are computed here exactly, in rational
arithmetic (Python's fractions), and rounded by IEEE 754's rules; the
operands are boundary cases (halfway points, overflow and underflow
thresholds, subnormals, exact cancellation, infinities, NaNs and signaling
NaNs, zeros of both signs) and random values. Each program computes every
operation under #pragma STDC FENV_ACCESS ON in each direction, compares
the result's bits (any NaN for a NaN) and the flags with the table, and
prints the number of mismatches per operation (each mismatch in detail).
The output is deterministic.
"""
import os
import random
from fractions import Fraction

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, '..', 'c', 'lib', 'math')

# precision, largest exponent, width of the exponent field
FMTS = {
    'q': (113, 16383, 15),
    'x': (64, 16383, 15),
    'd': (53, 1023, 11),
    'f': (24, 127, 8),
    'h': (11, 15, 5),
}
MODES = ['N', 'D', 'U', 'Z']  # to nearest, toward -inf, toward +inf, toward 0
INV, DZ, OV, UN, IX = 1, 2, 4, 8, 16  # the flags as the tests print them

NAN = 'nan'


class V:
    """A floating value: a Fraction, or an infinity/NaN with its sign."""

    def __init__(self, kind, neg=False, val=Fraction(0), snan=False):
        self.kind, self.neg, self.val, self.snan = kind, neg, val, snan

    def __repr__(self):
        return f'V({self.kind},{self.neg},{float(self.val)},{self.snan})'


def fin(x, neg=None):
    x = Fraction(x)
    return V('f', (x < 0) if neg is None else neg, x)


def decode(bits, fmt):
    p, emax, w = FMTS[fmt]
    bias = emax
    if fmt == 'x':
        neg = (bits >> 79) & 1
        ef = (bits >> 64) & 0x7FFF
        m = bits & ((1 << 64) - 1)
        if ef == 0x7FFF:
            if (m << 1) & ((1 << 64) - 1) == 0:
                return V('i', bool(neg))
            return V('n', bool(neg), snan=not ((m >> 62) & 1))
        e = max(ef, 1) - bias - 63
        return V('f', bool(neg), (-1 if neg else 1) * Fraction(m) * Fraction(2) ** e)
    fb = p - 1
    total = 1 + w + fb
    neg = (bits >> (total - 1)) & 1
    ef = (bits >> fb) & ((1 << w) - 1)
    fr = bits & ((1 << fb) - 1)
    if ef == (1 << w) - 1:
        if fr == 0:
            return V('i', bool(neg))
        return V('n', bool(neg), snan=not ((fr >> (fb - 1)) & 1))
    m = fr | ((1 << fb) if ef else 0)
    e = max(ef, 1) - bias - fb
    return V('f', bool(neg), (-1 if neg else 1) * Fraction(m) * Fraction(2) ** e)


def ilog2(a):
    """floor(log2(a)) of a positive Fraction."""
    e = a.numerator.bit_length() - a.denominator.bit_length()
    while Fraction(2) ** e > a:
        e -= 1
    while Fraction(2) ** (e + 1) <= a:
        e += 1
    return e


def rint(x, mode, neg):
    """The positive Fraction x rounded to an integer in the direction (of a
    value of sign neg)."""
    n = x.numerator // x.denominator
    r = x - n
    if r == 0:
        return n
    if mode == 'N':
        if r > Fraction(1, 2) or (r == Fraction(1, 2) and n % 2 == 1):
            return n + 1
        return n
    if mode == 'Z':
        return n
    up = (mode == 'U') != neg
    return n + 1 if up else n


def encode(neg, ef, m, fmt):
    p, emax, w = FMTS[fmt]
    if fmt == 'x':
        return (int(neg) << 79) | (ef << 64) | m
    fb = p - 1
    return (int(neg) << (w + fb)) | (ef << fb) | (m & ((1 << fb) - 1))


def inf_bits(neg, fmt):
    p, emax, w = FMTS[fmt]
    if fmt == 'x':
        return encode(neg, 0x7FFF, 1 << 63, fmt)
    return encode(neg, (1 << w) - 1, 0, fmt)


def zero_bits(neg, fmt):
    return encode(neg, 0, 0, fmt)


def round_val(x, fmt, mode):
    """A nonzero Fraction rounded to fmt: (bits, flags)."""
    p, emax, w = FMTS[fmt]
    emin = 1 - emax
    neg = x < 0
    a = abs(x)
    e = ilog2(a)
    q = max(e, emin) - (p - 1)
    m = rint(a / Fraction(2) ** q, mode, neg)
    flags = 0
    inexact = Fraction(m) * Fraction(2) ** q != a
    if inexact:
        flags |= IX
        # tininess after rounding: rounded to p bits, unbounded exponent
        q2 = e - (p - 1)
        m2 = rint(a / Fraction(2) ** q2, mode, neg)
        if Fraction(m2) * Fraction(2) ** q2 < Fraction(2) ** emin:
            flags |= UN
    if m == 1 << p:
        m >>= 1
        q += 1
    big = (Fraction(2) - Fraction(2) ** (1 - p)) * Fraction(2) ** emax
    r = Fraction(m) * Fraction(2) ** q
    if r > big:
        flags |= OV | IX
        to_inf = mode == 'N' or (mode != 'Z' and ((mode == 'U') != neg))
        if to_inf:
            return inf_bits(neg, fmt), flags
        return encode(neg, (1 << w) - 2, (1 << p) - 1, fmt), flags
    if m < 1 << (p - 1):
        return encode(neg, 0, m, fmt), flags
    return encode(neg, q + (p - 1) + emax, m, fmt), flags


def result(v, fmt, mode, flags=0):
    """An exact or rounded V in fmt: (bits or NAN, flags)."""
    if v.kind == 'n':
        return NAN, flags
    if v.kind == 'i':
        return inf_bits(v.neg, fmt), flags
    if v.val == 0:
        return zero_bits(v.neg, fmt), flags
    b, f = round_val(v.val, fmt, mode)
    return b, flags | f


def nan_in(*xs):
    return any(x.kind == 'n' for x in xs), any(x.kind == 'n' and x.snan for x in xs)


def op(o, a, b, mode):
    """a o b as a V, and the flags of the special cases."""
    isn, sig = nan_in(a, b)
    if isn:
        return V('n'), INV if sig else 0
    if o in '+-':
        bn = b.neg if o == '+' else not b.neg
        bv = b.val if o == '+' else -b.val
        if a.kind == 'i' and b.kind == 'i':
            if a.neg != bn:
                return V('n'), INV
            return V('i', a.neg), 0
        if a.kind == 'i':
            return V('i', a.neg), 0
        if b.kind == 'i':
            return V('i', bn), 0
        s = a.val + bv
        if s == 0:
            if a.val == 0 and bv == 0 and a.neg == bn:
                return V('f', a.neg, Fraction(0)), 0
            return V('f', mode == 'D', Fraction(0)), 0
        return fin(s), 0
    neg = a.neg != b.neg
    if o == '*':
        if a.kind == 'i' or b.kind == 'i':
            if (a.kind == 'f' and a.val == 0) or (b.kind == 'f' and b.val == 0):
                return V('n'), INV
            return V('i', neg), 0
        pr = a.val * b.val
        return (V('f', neg, Fraction(0)) if pr == 0 else fin(pr)), 0
    # division
    if a.kind == 'i':
        if b.kind == 'i':
            return V('n'), INV
        return V('i', neg), 0
    if b.kind == 'i':
        return V('f', neg, Fraction(0)), 0
    if b.val == 0:
        if a.val == 0:
            return V('n'), INV
        return V('i', neg), DZ
    if a.val == 0:
        return V('f', neg, Fraction(0)), 0
    return fin(a.val / b.val), 0


def to_ll(a):
    """(value or None for invalid, flags) of the conversion to long long."""
    if a.kind != 'f':
        return None, INV
    t = abs(a.val).numerator // abs(a.val).denominator
    if a.val < 0:
        t = -t
    if t < -(1 << 63) or t >= (1 << 63):
        return None, INV
    return t, (IX if t != a.val else 0)


def conv(a, fmt, mode):
    """a converted to fmt: (bits or NAN, flags)."""
    if a.kind == 'n':
        return NAN, INV if a.snan else 0
    return result(a, fmt, mode)


def cmp_lt(a, b):
    isn, sig = nan_in(a, b)
    if isn:
        return 0, INV
    return int(value_of(a) < value_of(b)), 0


def cmp_eq(a, b):
    isn, sig = nan_in(a, b)
    if isn:
        return 0, INV if sig else 0
    return int(value_of(a) == value_of(b)), 0


def value_of(a):
    if a.kind == 'i':
        return float('-inf') if a.neg else float('inf')
    return a.val


# ---------------------------------------------------------------------------
# operands


def bits_of(x, fmt):
    """The bits of a Fraction rounded to nearest (or of 'inf', '-inf',
    'nan', 'snan', '-0')."""
    if x == 'inf' or x == '-inf':
        return inf_bits(x[0] == '-', fmt)
    if x == '-0':
        return zero_bits(True, fmt)
    p, emax, w = FMTS[fmt]
    if x in ('nan', 'snan', '-nan'):
        if fmt == 'x':
            return encode(x[0] == '-', 0x7FFF, (0xC << 60) if x != 'snan' else ((1 << 63) | 1), fmt)
        fb = p - 1
        if x == 'snan':
            return encode(False, (1 << w) - 1, 1, fmt)
        return encode(x[0] == '-', (1 << w) - 1, 1 << (fb - 1), fmt)
    x = Fraction(x)
    if x == 0:
        return zero_bits(False, fmt)
    return round_val(x, fmt, 'N')[0]


def operands(fmt, rng, n):
    """Pairs of operand bits for format fmt."""
    p, emax, w = FMTS[fmt]
    emin = 1 - emax
    one = Fraction(1)
    ulp1 = Fraction(2) ** (1 - p)
    big = (2 - ulp1) * Fraction(2) ** emax
    tiny = Fraction(2) ** emin
    sub = Fraction(2) ** (emin - p + 1)
    third = Fraction(1, 3)
    vals = [
        (one, ulp1 / 2), (one, ulp1 / 2 * 3), (one, -ulp1 / 4), (-one, -ulp1 / 2), (one, one), (one, -one),
        (third, Fraction(2, 3)), (big, big), (big, -big), (big, Fraction(2)), (-big, Fraction(1, 2)), (big, ulp1),
        (tiny, Fraction(1, 2)), (tiny, -tiny / 2 * 3), (tiny * (1 + ulp1), Fraction(1, 2) - ulp1 / 4),
        (tiny, 1 - ulp1 / 2), (-tiny, 1 - ulp1 / 2), (sub, Fraction(1, 2)), (sub * 3, Fraction(1, 2)), (sub, sub),
        (tiny * (2 - ulp1), Fraction(1, 2)), (one, Fraction(0)), ('-0', Fraction(0)), ('-0', '-0'),
        (Fraction(0), '-0'), ('inf', 'inf'), ('inf', '-inf'), ('inf', Fraction(0)), (Fraction(0), Fraction(0)),
        (Fraction(-3), Fraction(0)), ('nan', one), (one, 'snan'), ('snan', 'nan'), ('-inf', Fraction(2)),
        (Fraction(10), Fraction(3)), (Fraction(-7), Fraction(16)), (Fraction(2) ** 62 + Fraction(1, 2), one),
        (Fraction(-(2 ** 63)), one), (Fraction(2) ** 63, one), (Fraction(-(2 ** 63)) - Fraction(1, 2), one),
        (Fraction(123456789, 1000), Fraction(-987654321, 1000000)), (Fraction(65504), Fraction(2)),
        (Fraction(65519), one), (Fraction(65520), one), (Fraction(2) ** -24, Fraction(1, 2)),
        (Fraction(2) ** -25 * 3, one), (Fraction(2) ** -14 * (1 - Fraction(2) ** -11), one),
    ]
    pairs = []
    for a, b in vals:
        pairs.append((bits_of(a, fmt), bits_of(b, fmt)))
    # random values: all exponents, and near one another
    for _ in range(n):
        def r():
            k = rng.randrange(8)
            if k == 0:
                e = rng.randrange(emin - p, emin + 4)
            elif k == 1:
                e = rng.randrange(emax - 4, emax + 1)
            elif k < 5:
                e = rng.randrange(-8, 8)
            else:
                e = rng.randrange(emin, emax + 1)
            m = rng.randrange(1 << (p - 1), 1 << p)
            if rng.randrange(3) == 0:
                m &= ~((1 << rng.randrange(p)) - 1)
                m |= 1 << (p - 1)
            v = Fraction(m) * Fraction(2) ** (e - p + 1)
            return -v if rng.randrange(2) else v
        a = r()
        b = r() if rng.randrange(3) else a * Fraction(rng.choice([1, 3, -1, -3]), rng.choice([1, 2, 4, 1024]))
        pairs.append((bits_of(a, fmt), bits_of(b, fmt)))
    return pairs


# ---------------------------------------------------------------------------
# the programs

OPS = ['add', 'sub', 'mul', 'div', 'tod', 'tof', 'toh', 'toll', 'lt', 'eq']
# the library's functions whose results depend on the direction (F.10)
FUNCS = ['sqrt', 'rint', 'nearbyint', 'llrint', 'scalbn', 'fma', 'fdim']
SCALES = [-20000, -16400, -200, -3, 0, 5, 300, 16400]


def fn(o, a, b, i, mode, fmt):
    """Function o of a (and b) in direction mode: (V or bits, flags)."""
    if o == 'sqrt':
        if a.kind == 'n':
            return NAN, INV if a.snan else 0
        if a.kind == 'i':
            return (NAN, INV) if a.neg else (inf_bits(False, fmt), 0)
        if a.val == 0:
            return zero_bits(a.neg, fmt), 0
        if a.val < 0:
            return NAN, INV
        return sqrt_bits(a.val, fmt, mode)
    if o in ('rint', 'nearbyint', 'llrint'):
        if a.kind != 'f':
            if o == 'llrint':
                return NAN, INV
            return (NAN, INV if a.snan else 0) if a.kind == 'n' else (inf_bits(a.neg, fmt), 0)
        x = a.val
        neg = x < 0 or (x == 0 and a.neg)
        r = rint(abs(x), mode, x < 0) if x != 0 else 0
        rv = -Fraction(r) if x < 0 else Fraction(r)
        if o == 'llrint':
            if rv < -(1 << 63) or rv >= (1 << 63):
                return NAN, INV
            return int(rv) & ((1 << 64) - 1), (IX if rv != x else 0)
        fl = IX if (rv != x and o == 'rint') else 0
        if rv == 0:
            return zero_bits(neg, fmt), fl
        return round_val(rv, fmt, 'N')[0], fl
    if o == 'scalbn':
        n = SCALES[i % len(SCALES)]
        if a.kind == 'n':
            return NAN, INV if a.snan else 0
        if a.kind == 'i' or a.val == 0:
            return result(a, fmt, mode)
        return round_val(a.val * Fraction(2) ** n, fmt, mode)
    if o == 'fma':
        c = a
        isn, sig = nan_in(a, b, c)
        prod_inv = (a.kind == 'i' and b.kind == 'f' and b.val == 0) or (b.kind == 'i' and a.kind == 'f' and a.val == 0)
        if prod_inv:
            return NAN, INV
        if isn:
            return NAN, INV if sig else 0
        v, f = op('*', a, b, mode)
        if v.kind == 'i' or c.kind == 'i':
            v2, f2 = op('+', v, c, mode)
            return result(v2, fmt, mode, f | f2)
        s = v.val + c.val
        if s == 0:
            pn = a.neg != b.neg
            if v.val == 0 and c.val == 0 and pn == c.neg:
                return zero_bits(pn, fmt), 0
            return zero_bits(mode == 'D', fmt), 0
        return round_val(s, fmt, mode)
    if o == 'fdim':
        isn, sig = nan_in(a, b)
        if isn:
            return NAN, INV if sig else 0
        if value_of(a) > value_of(b):
            v, f = op('-', a, b, mode)
            return result(v, fmt, mode, f)
        return zero_bits(False, fmt), 0
    raise ValueError(o)


def sqrt_bits(a, fmt, mode):
    """sqrt(a), a positive Fraction, correctly rounded."""
    import math
    p, emax, w = FMTS[fmt]
    # r = floor(sqrt(a) 2^k) for k giving p + 8 bits, and whether exact
    e = ilog2(a)
    k = (p + 8) - e // 2
    v = a * Fraction(4) ** k
    num, den = v.numerator, v.denominator
    # sqrt(num/den) = sqrt(num*den)/den
    t = math.isqrt(num * den)
    exact = t * t == num * den and t % den == 0
    if exact:
        return round_val(Fraction(t, den) / Fraction(2) ** k, fmt, mode)
    # t/den <= sqrt(v) < (t+1)/den: floor(sqrt(v)) is at least
    # floor(t/den); the root lies strictly inside (fl, fl + 1) unless exact,
    # where fl = floor(sqrt(v)) = isqrt(floor(v))
    fl = math.isqrt(num // den)
    if fl * fl * den == num:
        return round_val(Fraction(fl) / Fraction(2) ** k, fmt, mode)
    # a value strictly between fl and fl + 1, which rounds as the root does
    # (p + 8 bits: the rounding bit is above fl's last bit)
    return round_val((Fraction(fl) + Fraction(1, 2)) / Fraction(2) ** k, fmt, mode)


def expect(fmt, pairs, ops):
    rows = []
    for i, (ab, bb) in enumerate(pairs):
        a, b = decode(ab, fmt), decode(bb, fmt)
        for mi, mode in enumerate(MODES):
            for oi, o in enumerate(ops):
                if o in ('add', 'sub', 'mul', 'div'):
                    v, f = op({'add': '+', 'sub': '-', 'mul': '*', 'div': '/'}[o], a, b, mode)
                    bits, f = result(v, fmt, mode, f)
                elif o in ('tod', 'tof', 'toh', 'toq', 'tox'):
                    bits, f = conv(a, {'tod': 'd', 'tof': 'f', 'toh': 'h', 'toq': 'q', 'tox': 'x'}[o], mode)
                elif o == 'toll':
                    t, f = to_ll(a)
                    bits = NAN if t is None else t & ((1 << 64) - 1)
                elif o == 'lt':
                    bits, f = cmp_lt(a, b)
                elif o == 'eq':
                    bits, f = cmp_eq(a, b)
                else:
                    bits, f = fn(o, a, b, i, mode, fmt)
                rows.append((i, mi, oi, bits, f))
    return rows


def words(v):
    return v & ((1 << 64) - 1), v >> 64


HEADER = '''/* {what} in the floating-point environment: each operation in the four
   rounding directions, its result's bits and the exceptions it raises,
   against results computed exactly and rounded by IEEE 754's rules
   (written by cmd/axiom/internal/tests/tools/gen_fenv_wide_tests.py; do not
   edit). The program prints the number of mismatches per operation. */
/* flags: --c-std=c23 */
/* native-only */
{only}#include "print.h"
#include <fenv.h>
#include <stdio.h>
#include <string.h>

#pragma STDC FENV_ACCESS ON

typedef unsigned long long U;

struct pair {{
    U alo, ahi, blo, bhi;
}};

/* operation k of pair i in mode m: the result's bits (lo, hi; nan: any NaN)
   and the flags (1 invalid, 2 divide-by-zero, 4 overflow, 8 underflow, 16
   inexact) */
struct row {{
    unsigned short i;
    unsigned char m, k, nan, fl;
    U lo, hi;
}};

static const int modes[4] = {{FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO}};
static const char *const names[] = {{{names}}};

static int flags(void)
{{
    int f = fetestexcept(FE_ALL_EXCEPT), r = 0;
    if (f & FE_INVALID)
        r |= 1;
    if (f & FE_DIVBYZERO)
        r |= 2;
    if (f & FE_OVERFLOW)
        r |= 4;
    if (f & FE_UNDERFLOW)
        r |= 8;
    if (f & FE_INEXACT)
        r |= 16;
    feclearexcept(FE_ALL_EXCEPT);
    return r;
}}
'''

BODY = '''
static int bad[{nops}];

static void check(const struct row *w, U lo, U hi, int isnan_, int fl)
{{
    int ok = w->fl == fl && (w->nan ? isnan_ : (!isnan_ && w->lo == lo && w->hi == hi));
    if (!ok) {{
        bad[w->k]++;
        printf("pair %d mode %d %s: got %016llx %016llx%s flags %d, expected %016llx %016llx%s flags %d\\n", w->i, w->m,
               names[w->k], hi, lo, isnan_ ? " (nan)" : "", fl, w->hi, w->lo, w->nan ? " (nan)" : "", w->fl);
    }}
}}

int main(void)
{{
    const struct row *w = rows;
    for (int i = 0; i < (int)(sizeof pairs / sizeof pairs[0]); i++) {{
        volatile T a = get(pairs[i].alo, pairs[i].ahi), b = get(pairs[i].blo, pairs[i].bhi);
        for (int m = 0; m < 4; m++) {{
            fesetround(modes[m]);
            feclearexcept(FE_ALL_EXCEPT);
{calls}        }}
    }}
    fesetround(FE_TONEAREST);
    for (int k = 0; k < {nops}; k++)
        printf("%s %d\\n", names[k], bad[k]);
    return 0;
}}
'''


def emit(path, what, only, fmt, tname, tbytes, pairs, ops, getter, calls):
    rows = expect(fmt, pairs, ops)
    out = [HEADER.format(what=what, only=only, names=', '.join(f'"{o}"' for o in ops))]
    out.append(f'#define T {tname}\n')
    out.append(getter)
    out.append('static const struct pair pairs[] = {\n')
    for ab, bb in pairs:
        al, ah = words(ab)
        bl, bh = words(bb)
        out.append(f'    {{0x{al:x}, 0x{ah:x}, 0x{bl:x}, 0x{bh:x}}},\n')
    out.append('};\n\nstatic const struct row rows[] = {\n')
    for i, mi, oi, bits, f in rows:
        if bits == NAN:
            out.append(f'    {{{i}, {mi}, {oi}, 1, {f}, 0, 0}},\n')
        else:
            lo, hi = words(bits)
            out.append(f'    {{{i}, {mi}, {oi}, 0, {f}, 0x{lo:x}, 0x{hi:x}}},\n')
    out.append('};\n')
    out.append(BODY.format(nops=len(ops), calls=calls))
    for o in ops:
        out.append(f'// expect: {o} 0\n')
    with open(path, 'w') as fh:
        fh.write(''.join(out))


WIDE_GET = '''
static T get(U lo, U hi)
{{
    T v;
    unsigned char b[16];
    memset(&v, 0, sizeof v);
    memcpy(b, &lo, 8);
    memcpy(b + 8, &hi, 8);
    memcpy(&v, b, {n});
    return v;
}}

static void bits(volatile T *p, U *lo, U *hi)
{{
    unsigned char b[16];
    memset(b, 0, 16);
    memcpy(b, (const void *)p, {n});
    memcpy(lo, b, 8);
    memcpy(hi, b + 8, 8);
}}

'''

WIDE_CALLS = '''            {
                volatile T r;
                U lo, hi;
                int fl;
                r = a + b;
                fl = flags();
                bits(&r, &lo, &hi);
                check(w++, lo, hi, r != r, fl);
                r = a - b;
                fl = flags();
                bits(&r, &lo, &hi);
                check(w++, lo, hi, r != r, fl);
                r = a * b;
                fl = flags();
                bits(&r, &lo, &hi);
                check(w++, lo, hi, r != r, fl);
                r = a / b;
                fl = flags();
                bits(&r, &lo, &hi);
                check(w++, lo, hi, r != r, fl);
            }
            {
                volatile double d = a;
                int fl = flags();
                U lo;
                memcpy(&lo, (const void *)&d, 8);
                check(w++, lo, 0, d != d, fl);
            }
            {
                volatile float f = a;
                int fl = flags();
                unsigned u;
                memcpy(&u, (const void *)&f, 4);
                check(w++, u, 0, f != f, fl);
            }
            {
                volatile _Float16 h = a;
                int fl = flags();
                unsigned short u;
                memcpy(&u, (const void *)&h, 2);
                check(w++, u, 0, h != h, fl);
            }
            {
                volatile long long v = (long long)a;
                int fl = flags();
                check(w++, (U)v, 0, (fl & 1) != 0, fl);
            }
            {
                volatile int c = a < b;
                int fl = flags();
                check(w++, (U)c, 0, 0, fl);
                c = a == b;
                fl = flags();
                check(w++, (U)c, 0, 0, fl);
            }
'''

FN_CALLS = '''            {
                volatile T r;
                U lo, hi;
                int fl;
                static const int scales[8] = {-20000, -16400, -200, -3, 0, 5, 300, 16400};
                r = F(sqrt)(a);
                fl = flags();
                bits(&r, &lo, &hi);
                check(w++, lo, hi, r != r, fl);
                r = F(rint)(a);
                fl = flags();
                bits(&r, &lo, &hi);
                check(w++, lo, hi, r != r, fl);
                r = F(nearbyint)(a);
                fl = flags();
                bits(&r, &lo, &hi);
                check(w++, lo, hi, r != r, fl);
                {
                    volatile long long v = F(llrint)(a);
                    fl = flags();
                    check(w++, (U)v, 0, (fl & 1) != 0, fl);
                }
                r = F(scalbn)(a, scales[i % 8]);
                fl = flags();
                bits(&r, &lo, &hi);
                check(w++, lo, hi, r != r, fl);
                r = F(fma)(a, b, a);
                fl = flags();
                bits(&r, &lo, &hi);
                check(w++, lo, hi, r != r, fl);
                r = F(fdim)(a, b);
                fl = flags();
                bits(&r, &lo, &hi);
                check(w++, lo, hi, r != r, fl);
            }
'''

HALF_GET = '''
static T get(U lo, U hi)
{
    T v;
    unsigned short u = (unsigned short)lo;
    (void)hi;
    memcpy(&v, &u, 2);
    return v;
}

static void bits(volatile T *p, U *lo, U *hi)
{
    unsigned short u;
    memcpy(&u, (const void *)p, 2);
    *lo = u;
    *hi = 0;
}

'''


def main():
    rng = random.Random(20261113)
    qp = operands('q', rng, 50)
    emit(os.path.join(OUT, 'fenv_wide_q.c'), '_Float128 (binary128, software arithmetic: std/c/rt/ldouble.ax)', '', 'q',
         '_Float128', 16, qp, OPS, WIDE_GET.format(n=16), WIDE_CALLS)
    xp = operands('x', rng, 50)
    emit(os.path.join(OUT, 'fenv_wide_x.c'),
         'long double in the x87\'s 80-bit format (x86-64 Linux; software arithmetic: std/c/rt/ldouble.ax)',
         '/* linux-only */\n/* x86-64-only */\n', 'x', 'long double', 10, xp, OPS, WIDE_GET.format(n=10), WIDE_CALLS)
    fq = operands('q', rng, 40)
    emit(os.path.join(OUT, 'fenv_wide_fn_q.c'),
         '<math.h>\'s _Float128 functions that round (sqrtf128, rintf128, nearbyintf128, llrintf128, scalbnf128, fmaf128, '
         'fdimf128)', '#define __STDC_WANT_IEC_60559_TYPES_EXT__ 1\n', 'q', '_Float128', 16, fq, FUNCS,
         '#define F(f) f##f128\n#include <math.h>\n' + WIDE_GET.format(n=16), FN_CALLS)
    fx = operands('x', rng, 40)
    emit(os.path.join(OUT, 'fenv_wide_fn_x.c'),
         '<math.h>\'s long double functions that round (sqrtl ... fdiml), in the x87\'s format', '/* linux-only */\n/* x86-64-only */\n',
         'x', 'long double', 10, fx, FUNCS, '#define F(f) f##l\n#include <math.h>\n' + WIDE_GET.format(n=10), FN_CALLS)
    hp = operands('h', rng, 120)
    half_ops = ['add', 'sub', 'mul', 'div', 'lt', 'eq']
    half_calls = WIDE_CALLS[:WIDE_CALLS.index('            {\n                volatile double d')] + \
        WIDE_CALLS[WIDE_CALLS.index('            {\n                volatile int c'):]
    emit(os.path.join(OUT, 'fenv_half.c'), '_Float16 (binary16: computed in float, rounded by std/c/rt/ldouble.ax)', '',
         'h', '_Float16', 2, hp, half_ops, HALF_GET, half_calls)


if __name__ == '__main__':
    main()
