#!/usr/bin/env python3
"""Writes the tests of C23's decimal floating types (_Decimal32, _Decimal64,
_Decimal128), whose expected results Python's decimal module (contexts of
each format's precision and exponent range, clamp 1: IEEE 754's decimal
arithmetic), exact integer and rational arithmetic compute:

    cmd/axiom/internal/tests/tools/gen_dec_tests.py

writes cmd/axiom/internal/tests/c/lib/math/dec_gen_<format>.c, one hosted
program for each format. Each has a table of operands (boundary values:
zeros of several exponents, the least subnormal, the largest value,
infinities, quiet and signaling NaNs with payloads, non-canonical
encodings; values near each other for cancellation; and pseudo-random
values of every magnitude and number of digits, so that cohorts and
quanta vary) and, for each operation, the expected encoding (BID) and
exception flags of every result in each of the five decimal rounding
directions. The program computes the results at run time from volatile
operands, in each direction (fe_dec_setround), compares encodings and
flags (where the target has exception flags: not on WebAssembly), and
prints the number of mismatches of each operation. The operators and
conversions are also folded by the compiler (static tables of constant
expressions), compared with the results of rounding to nearest.

Operations: + - * / (and fma, sqrt, remainder, fmod, quantize, the rounding
functions, nextup and nextdown of <math.h>), < and == (signaling and quiet
comparisons), the conversions to the other decimal formats, to and from
long long, double and float.
"""
import decimal
import math
import os
import random
from decimal import Context, Decimal
from fractions import Fraction

here = os.path.dirname(os.path.abspath(__file__))
out_dir = os.path.join(here, '..', 'c', 'lib', 'math')

FMT = {0: (7, 96, 32, 8), 1: (16, 384, 64, 10), 2: (34, 6144, 128, 14)}
NAMES = {0: ('d32', '_Decimal32'), 1: ('d64', '_Decimal64'), 2: ('d128', '_Decimal128')}
ROUND = [decimal.ROUND_HALF_EVEN, decimal.ROUND_FLOOR, decimal.ROUND_CEILING, decimal.ROUND_DOWN,
         decimal.ROUND_HALF_UP]
F_INVALID, F_DIVBYZERO, F_OVERFLOW, F_UNDERFLOW, F_INEXACT = 1, 4, 8, 16, 32


def params(f):
    p, emax, k, eb = FMT[f]
    return p, emax, k, eb, k - 1 - eb, emax + p - 2


def qmin(f):
    p, emax, *_ = FMT[f]
    return 1 - emax - (p - 1)


def qmax(f):
    p, emax, *_ = FMT[f]
    return emax - (p - 1)


def ctx(f, mode=0):
    p, emax, *_ = FMT[f]
    return Context(prec=p, Emax=emax, Emin=1 - emax, rounding=ROUND[mode], clamp=1, traps=[])


def flags(c):
    v = 0
    for sig, b in [(decimal.InvalidOperation, 1), (decimal.DivisionByZero, 4), (decimal.Overflow, 8),
                   (decimal.Underflow, 16), (decimal.Inexact, 32)]:
        if c.flags[sig]:
            v |= b
    return v


def decode(bits, f):
    p, emax, k, eb, cb, bias = params(f)
    s = (bits >> (k - 1)) & 1
    g = (bits >> (k - 6)) & 31
    em = (1 << eb) - 1
    if g >> 3 != 3:
        e = (bits >> cb) & em
        c = bits & ((1 << cb) - 1)
    elif g >> 1 != 15:
        e = (bits >> (cb - 2)) & em
        c = (4 << (cb - 2)) | (bits & ((1 << (cb - 2)) - 1))
    elif g == 30:
        return Decimal((s, (), 'F'))
    else:
        sig = (bits >> (k - 7)) & 1
        pl = bits & ((1 << (cb - 3)) - 1)
        if pl >= 10 ** (p - 1):
            pl = 0
        return Decimal((s, tuple(int(x) for x in str(pl)) if pl else (), 'N' if sig else 'n'))
    if c >= 10 ** p:
        c = 0
    return Decimal((s, tuple(int(x) for x in str(c)), e - bias))


def encode(d, f):
    p, emax, k, eb, cb, bias = params(f)
    t = d.as_tuple()
    s = t.sign << (k - 1)
    if t.exponent == 'F':
        return s | (30 << (k - 6))
    if t.exponent in ('n', 'N'):
        pl = int(''.join(map(str, t.digits))) if t.digits else 0
        if pl >= 10 ** (p - 1):
            pl = int(str(pl)[-(p - 1):])
        return s | (31 << (k - 6)) | ((1 if t.exponent == 'N' else 0) << (k - 7)) | pl
    c = int(''.join(map(str, t.digits))) if t.digits else 0
    e = t.exponent + bias
    if c < (1 << cb):
        return s | (e << cb) | c
    return s | (3 << (k - 3)) | (e << (cb - 2)) | (c & ((1 << (cb - 2)) - 1))


def quiet(d):
    t = d.as_tuple()
    return Decimal((t.sign, t.digits, 'n'))


def tup(d):
    t = d.as_tuple()
    return t.sign, int(''.join(map(str, t.digits))) if t.digits else 0, t.exponent


def mk(s, c, q):
    return Decimal((s, tuple(map(int, str(c))), q))


# -----------------------------------------------------------------------------
# Operands
# -----------------------------------------------------------------------------

def operands(f, R, n):
    p = FMT[f][0]
    lo, hi = qmin(f), qmax(f)
    fixed = [mk(0, 0, 0), mk(1, 0, 0), mk(0, 0, -3), mk(1, 0, lo), mk(0, 0, hi), mk(0, 1, lo), mk(1, 1, lo),
             mk(0, 10 ** p - 1, hi), mk(1, 10 ** p - 1, hi), mk(0, 1, 0), mk(0, 10, -1), mk(0, 100, -2),
             mk(1, 15, -1), mk(0, 25, -1), mk(0, 10 ** (p - 1), lo), mk(0, 10 ** p - 1, lo),
             Decimal('Infinity'), Decimal('-Infinity'), Decimal('NaN'), Decimal('-NaN123'), Decimal('sNaN'),
             Decimal('sNaN45'), mk(0, 5, 0), mk(0, 3, 0), mk(0, 7, -1), mk(0, 10 ** p - 1, -p),
             mk(0, 5 * 10 ** (p - 1), -p), mk(1, 999, -1)]
    xs = list(fixed)
    while len(xs) < n:
        r = R.random()
        nd = R.choice([1, 2, p, p, R.randint(1, p)])
        c = R.randint(10 ** (nd - 1), 10 ** nd - 1)
        if R.random() < 0.25:
            c = int(str(R.randint(1, 9)) + '0' * (nd - 1)) if R.random() < .5 else 10 ** nd - 1
        if r < 0.45:
            q = R.randint(-p - 3, 3)
        elif r < 0.6:
            q = R.randint(lo, lo + 2 * p)
        elif r < 0.75:
            q = R.randint(hi - 2 * p, hi)
        else:
            q = R.randint(lo, hi)
        xs.append(mk(R.randint(0, 1), c, q))
    return xs[:n]


def partner(f, a, R):
    # a value of about a's size (cancellation, alignment), or another
    p = FMT[f][0]
    if not a.is_finite() or a.is_zero() or R.random() < 0.4:
        return operands(f, R, 40)[R.randint(0, 39)]
    s, c, q = tup(a)
    q2 = max(qmin(f), min(qmax(f), q + R.randint(-p - 2, p + 2)))
    nd = R.randint(1, p)
    return mk(R.randint(0, 1), R.randint(10 ** (nd - 1), 10 ** nd - 1), q2)


# -----------------------------------------------------------------------------
# References (the results this implementation gives where Python's decimal
# has no operation: IEEE 754's rules)
# -----------------------------------------------------------------------------

def nan_of(*xs):
    # the NaN result of an operation with NaN operands: the first signaling
    # one quieted ("invalid"), else the first quiet one
    for x in xs:
        if x.is_snan():
            return quiet(x), F_INVALID
    for x in xs:
        if x.is_qnan():
            return x, 0
    return None


def round_exact(f, mode, s, v, pref):
    """(-1)^s v (a Fraction >= 0) in format f: exact, the exponent nearest
    pref; else rounded once"""
    p = FMT[f][0]
    c = ctx(f, mode)
    if v == 0:
        return mk(s, 0, max(qmin(f), min(qmax(f), pref))), 0
    # the exact value as a decimal of enough digits (or with a sticky digit)
    t = math.floor(math.log10(v.numerator) - math.log10(v.denominator))
    e = t - p - 8
    scaled = v / (Fraction(10) ** e)
    ip, rem = divmod(scaled.numerator, scaled.denominator)
    if rem == 0:
        # exact: as written with the exponent e, Python's create_decimal
        # then picks the exponent; the exact case of decimal.ax's finish
        d = mk(s, ip, e)
        tz = len(str(ip)) - len(str(ip).rstrip('0'))
        nd = len(str(ip))
        elo = max(e + nd - p, qmin(f))
        ehi = min(e + tz, qmax(f))
        if elo <= ehi:
            ex = max(elo, min(ehi, pref))
            return mk(s, ip // 10 ** (ex - e) if ex >= e else ip * 10 ** (e - ex), ex), 0
        r = c.create_decimal(d)
        return r, flags(c)
    d = mk(s, ip * 10 + 1, e - 1)
    r = c.create_decimal(d)
    return r, flags(c)


def sqrt_ref(f, mode, a):
    if a.is_nan():
        return nan_of(a)
    if a.is_infinite():
        return (a, 0) if a > 0 else (Decimal('NaN'), F_INVALID)
    s, c, q = tup(a)
    pref = q >> 1
    if c == 0:
        return mk(s, 0, max(qmin(f), min(qmax(f), pref))), 0
    if s:
        return Decimal('NaN'), F_INVALID
    if q % 2:
        c *= 10
        q -= 1
    k = FMT[f][0] + 5
    N = c * 10 ** (2 * k)
    r = math.isqrt(N)
    if r * r == N:
        return round_exact(f, mode, 0, Fraction(r) * Fraction(10) ** (q // 2 - k), pref)
    cc = ctx(f, mode)
    res = cc.create_decimal(mk(0, r * 10 + 1, q // 2 - k - 1))
    return res, flags(cc)


def rem_ref(f, a, b, near):
    n = nan_of(a, b)
    if n:
        return n
    if a.is_infinite() or (b.is_finite() and b.is_zero()):
        return Decimal('NaN'), F_INVALID
    if b.is_infinite():
        return a, 0
    sa, ca, qa = tup(a)
    sb, cb, qb = tup(b)
    m = min(qa, qb)
    if ca == 0:
        return mk(sa, 0, max(qmin(f), min(qmax(f), m))), 0
    X = ca * 10 ** (qa - m)
    Y = cb * 10 ** (qb - m)
    sign = sa
    if near:
        q, r = divmod(X, Y)
        if 2 * r > Y or (2 * r == Y and q % 2 == 1):
            r = Y - r
            sign = 1 - sa
    else:
        r = X % Y
    if r == 0:
        sign = sa
    return mk(sign, r, m), 0


def convert_ref(a, f, g, mode):
    if a.is_nan():
        return (quiet(a), F_INVALID) if a.is_snan() else (a, 0)
    if a.is_infinite():
        return a, 0
    s, c, q = tup(a)
    return round_exact(g, mode, s, Fraction(c) * Fraction(10) ** q, q)


def to_int_ref(a, bits=64):
    lo, hi = -(1 << (bits - 1)), (1 << (bits - 1)) - 1
    if a.is_nan():
        return 0, F_INVALID
    if a.is_infinite():
        return (lo if a < 0 else hi), F_INVALID
    v = int(a)
    fl = F_INEXACT if v != a else 0
    if v > hi:
        return hi, F_INVALID
    if v < lo:
        return lo, F_INVALID
    return v, fl


BF = {'f': (24, -126, 127), 'd': (53, -1022, 1023)}


def bin_round(k, s, v):
    """(-1)^s v (Fraction >= 0) to float ('f') or double ('d'), nearest even: bits, flags"""
    p, emin, emax = BF[k]
    nb = 32 if k == 'f' else 64
    fb = p - 1
    if v == 0:
        return s << (nb - 1), 0
    e = v.numerator.bit_length() - v.denominator.bit_length()
    if Fraction(2) ** e > v:
        e -= 1
    if Fraction(2) ** (e + 1) <= v:
        e += 1
    t = max(e - p + 1, emin - p + 1)
    x = v / (Fraction(2) ** t)
    m = x.numerator // x.denominator
    rem = x - m
    fl = 0
    if rem != 0:
        fl |= F_INEXACT
        if e < emin:
            fl |= F_UNDERFLOW
        if rem > Fraction(1, 2) or (rem == Fraction(1, 2) and m % 2 == 1):
            m += 1
    if m >= 2 ** p:
        m //= 2
        t += 1
    lead = t + p - 1
    expbits = 8 if k == 'f' else 11
    if lead > emax and m >= 2 ** (p - 1):
        return (s << (nb - 1)) | (((1 << expbits) - 1) << fb), F_OVERFLOW | F_INEXACT
    ef = (lead + emax) if m >= 2 ** (p - 1) else 0
    return (s << (nb - 1)) | (ef << fb) | (m & ((1 << fb) - 1)), fl


def to_bin_ref(a, k):
    nb = 32 if k == 'f' else 64
    fb = BF[k][0] - 1
    expbits = 8 if k == 'f' else 11
    s = a.as_tuple().sign
    if a.is_nan():
        return (s << (nb - 1)) | (((1 << expbits) - 1) << fb) | (1 << (fb - 1)), (F_INVALID if a.is_snan() else 0)
    if a.is_infinite():
        return (s << (nb - 1)) | (((1 << expbits) - 1) << fb), 0
    s, c, q = tup(a)
    return bin_round(k, s, Fraction(c) * Fraction(10) ** q)


def from_double_ref(bits, f, mode):
    s = bits >> 63
    e = (bits >> 52) & 0x7ff
    fr = bits & ((1 << 52) - 1)
    if e == 0x7ff:
        if fr == 0:
            return Decimal((s, (), 'F')), 0
        return Decimal((s, (), 'n')), (F_INVALID if not (fr >> 51) & 1 else 0)
    m, ee = (fr, -1074) if e == 0 else (fr | (1 << 52), e - 1075)
    v = Fraction(m) * (Fraction(2) ** ee)
    return round_exact(f, mode, s, v, 0)


def int_ref(f, mode, v):
    c = ctx(f, mode)
    r = c.create_decimal(v)
    return r, flags(c)


def integral_ref(a, mode, exact):
    if a.is_nan():
        return nan_of(a)
    if a.is_infinite():
        return a, 0
    c = Context(prec=1000, Emax=10 ** 6, Emin=-10 ** 6, rounding=ROUND[mode], traps=[])
    if exact:
        r = c.to_integral_exact(a)
        return r, (F_INEXACT if c.flags[decimal.Inexact] else 0)
    return c.to_integral_value(a), 0


def next_ref(a, f, up):
    if a.is_nan():
        return nan_of(a)
    c = ctx(f)
    r = c.next_plus(a) if up else c.next_minus(a)
    return r, 0


# -----------------------------------------------------------------------------
# The program
# -----------------------------------------------------------------------------

def hex2(v, f):
    if f == 2:
        return '0x%xULL, 0x%xULL' % (v & ((1 << 64) - 1), v >> 64)
    return '0x%xULL, 0' % v


def write(f, seed, n):
    R = random.Random(seed)
    tag, ctype = NAMES[f]
    bits = FMT[f][2]
    xs = operands(f, R, n)
    ys = [partner(f, x, R) for x in xs]
    zs = [partner(f, x * y if x.is_finite() and y.is_finite() else x, R) if R.random() < .5 else operands(f, R, 40)[
        R.randint(0, 39)] for x, y in zip(xs, ys)]
    # (z in range: a product's neighbour may not be)
    zs = [z if (not z.is_finite()) or (len(z.as_tuple().digits) <= FMT[f][0] and qmin(f) <= z.as_tuple().exponent
                                        <= qmax(f)) else xs[i] for i, z in enumerate(zs)]
    ints = [R.choice([0, 1, -1, 2 ** 63 - 1, -2 ** 63, 10 ** 15, R.randint(-2 ** 63, 2 ** 63 - 1),
                      R.randint(-10 ** 9, 10 ** 9), R.randint(-10 ** 18, 10 ** 18)]) for _ in range(n)]
    dbls = []
    for _ in range(n):
        r = R.random()
        if r < 0.1:
            dbls.append(R.choice([0, 1 << 63, 0x7ff0000000000000, 0x7ff8000000000000, 0x7ff4000000000000, 1,
                                  0x7fefffffffffffff, 0x3fb999999999999a]))
        else:
            e = R.choice([R.randint(-60, 60), R.randint(-1074, 1023)]) + 1023
            e = max(0, min(2046, e))
            dbls.append((R.randint(0, 1) << 63) | (e << 52) | R.getrandbits(52))
    modes = range(5)
    tables = []  # (name, rows[mode][i] = (bits_lo_hi, flags), kind)

    def mode_table(fn):
        return [[fn(i, m) for i in range(n)] for m in modes]

    def arith(op):
        def fn(i, m):
            c = ctx(f, m)
            a, b = xs[i], ys[i]
            r = {'add': c.add, 'sub': c.subtract, 'mul': c.multiply, 'div': c.divide}[op](a, b)
            return encode(r, f), flags(c)
        return fn

    def fma_fn(i, m):
        c = ctx(f, m)
        return encode(c.fma(xs[i], ys[i], zs[i]), f), flags(c)

    def quantize_fn(i, m):
        c = ctx(f, m)
        return encode(c.quantize(xs[i], ys[i]), f), flags(c)

    def sqrt_fn(i, m):
        r, fl = sqrt_ref(f, m, xs[i])
        return encode(r, f), fl

    def rem_fn(near):
        def fn(i, m):
            r, fl = rem_ref(f, xs[i], ys[i], near)
            return encode(r, f), fl
        return fn

    def cmp_fn(op):
        def fn(i, m):
            a, b = xs[i], ys[i]
            fl = 0
            if a.is_nan() or b.is_nan():
                if op == 'lt' or a.is_snan() or b.is_snan():
                    fl = F_INVALID
                return 0, fl
            return int(a < b if op == 'lt' else a == b), 0
        return fn

    def conv_fn(g):
        def fn(i, m):
            r, fl = convert_ref(xs[i], f, g, m)
            return encode(r, g), fl
        return fn

    def toint_fn(i, m):
        v, fl = to_int_ref(xs[i])
        return v & ((1 << 64) - 1), fl

    def fromint_fn(i, m):
        r, fl = int_ref(f, m, ints[i])
        return encode(r, f), fl

    def tobin_fn(k):
        def fn(i, m):
            return to_bin_ref(xs[i], k)
        return fn

    def fromdbl_fn(i, m):
        r, fl = from_double_ref(dbls[i], f, m)
        return encode(r, f), fl

    def integral_fn(mode_of, exact):
        def fn(i, m):
            r, fl = integral_ref(xs[i], mode_of(m), exact)
            return encode(r, f), fl
        return fn

    def next_fn(up):
        def fn(i, m):
            r, fl = next_ref(xs[i], f, up)
            return encode(r, f), fl
        return fn

    # name, table, C expression of a b c (volatile operands) or none, result kind
    ops = [('add', arith('add'), 'a + b', 'T'), ('sub', arith('sub'), 'a - b', 'T'),
           ('mul', arith('mul'), 'a * b', 'T'), ('div', arith('div'), 'a / b', 'T'),
           ('fma', fma_fn, 'fma%s(a, b, c)' % tag, 'T'), ('sqrt', sqrt_fn, 'sqrt%s(a)' % tag, 'T'),
           ('remainder', rem_fn(True), 'remainder%s(a, b)' % tag, 'T'),
           ('fmod', rem_fn(False), 'fmod%s(a, b)' % tag, 'T'),
           ('quantize', quantize_fn, 'quantize%s(a, b)' % tag, 'T'),
           ('lt', cmp_fn('lt'), 'a < b', 'I'), ('eq', cmp_fn('eq'), 'a == b', 'I'),
           ('rint', integral_fn(lambda m: m, True), 'rint%s(a)' % tag, 'T'),
           ('nearbyint', integral_fn(lambda m: m, False), 'nearbyint%s(a)' % tag, 'T'),
           ('floor', integral_fn(lambda m: 1, False), 'floor%s(a)' % tag, 'T'),
           ('ceil', integral_fn(lambda m: 2, False), 'ceil%s(a)' % tag, 'T'),
           ('trunc', integral_fn(lambda m: 3, False), 'trunc%s(a)' % tag, 'T'),
           ('round', integral_fn(lambda m: 4, False), 'round%s(a)' % tag, 'T'),
           ('roundeven', integral_fn(lambda m: 0, False), 'roundeven%s(a)' % tag, 'T'),
           ('nextup', next_fn(True), 'nextup%s(a)' % tag, 'T'),
           ('nextdown', next_fn(False), 'nextdown%s(a)' % tag, 'T'),
           ('to_ll', toint_fn, '(long long)a', 'L'), ('from_ll', fromint_fn, '(T)n', 'T'),
           ('to_double', tobin_fn('d'), '(double)a', 'D'), ('to_float', tobin_fn('f'), '(float)a', 'F'),
           ('from_double', fromdbl_fn, '(T)dbl', 'T')]
    for g in range(3):
        if g != f:
            ops.append(('to_%s' % NAMES[g][0], conv_fn(g), '(%s)a' % NAMES[g][1], 'T%d' % g))
    folded = {'add': '+', 'sub': '-', 'mul': '*', 'div': '/', 'lt': '<', 'eq': '=='}
    L = []
    L.append('/* dec_gen_%s.c - written by cmd/axiom/internal/tests/tools/gen_dec_tests.py: the operations on'
             % tag)
    L.append('   %s against the results of Python\'s decimal module and exact arithmetic, in every'
             % ctype)
    L.append('   decimal rounding direction (expected encodings and flags in the tables). */')
    L.append('/* flags: --c-std=c23 */')
    for name, _, _, _ in ops:
        L.append('/* expect: %s 0 */' % name)
    for name in folded:
        L.append('/* expect: fold_%s 0 */' % name)
    L.append('/* expect: fold_to_ll 0 */')
    L += ['#include "print.h"', '#include <fenv.h>', '#include <math.h>', '#include <string.h>', '',
          'typedef %s T;' % ctype, '#define N %d' % n, '',
          '/* an expected result: its encoding (the low and high 64 bits) and flags */',
          'struct res {', '    unsigned long long lo, hi;', '    int fl;', '};', '']

    def arr(nm, vals, fv):
        L.append('static const unsigned long long %s[N][2] = {' % nm)
        for v in vals:
            L.append('    {%s},' % hex2(encode(v, fv) if isinstance(v, Decimal) else v, fv))
        L.append('};')

    arr('X', xs, f)
    arr('Y', ys, f)
    arr('Z', zs, f)
    L.append('static const long long INTS[N] = {')
    for v in ints:
        L.append('    %s,' % ('(-9223372036854775807LL - 1)' if v == -2 ** 63 else '%dLL' % v))
    L.append('};')
    L.append('static const unsigned long long DBLS[N] = {')
    for v in dbls:
        L.append('    0x%xULL,' % v)
    L.append('};')
    for name, fn, _, kind in ops:
        rows = mode_table(fn)
        L.append('static const struct res E_%s[5][N] = {' % name)
        for row in rows:
            L.append('    {')
            for v, fl in row:
                L.append('        {0x%xULL, 0x%xULL, %d},' % (v & ((1 << 64) - 1), v >> 64, fl))
            L.append('    },')
        L.append('};')
    # the folded tables: constant expressions of the operands' encodings
    def kexpr(v, fv):
        b = encode(v, fv)
        return '__builtin_axiom_dec(%d, 0x%xULL, 0x%xULL)' % (FMT[fv][2], b >> 64, b & ((1 << 64) - 1))

    for name, op in folded.items():
        ty = 'T' if name in ('add', 'sub', 'mul', 'div') else 'int'
        L.append('static const %s F_%s[N] = {' % (ty, name))
        for x, y in zip(xs, ys):
            L.append('    %s %s %s,' % (kexpr(x, f), op, kexpr(y, f)))
        L.append('};')
    L.append('static const long long F_to_ll[N] = {')
    for x in xs:
        v, fl = to_int_ref(x)
        # (the conversions the compiler folds: those whose value fits, 6.3.1.4)
        L.append('    %s,' % ('(long long)%s' % kexpr(x, f) if fl & F_INVALID == 0 else '0'))
    L.append('};')
    L += ['',
          'static T ld(const unsigned long long *w) {',
          '    T v;',
          '    memcpy(&v, w, sizeof v);',
          '    return v;',
          '}',
          '',
          '/* the exception flags are compared where the target has them */',
          'static int have_flags(void) {',
          '    return (math_errhandling & MATH_ERREXCEPT) != 0;',
          '}',
          '',
          '/* 1 if the result at r (size bytes) or its flags differ from e */',
          'static int differs(const struct res *e, const void *r, size_t size, int fl) {',
          '    unsigned long long w[2] = {0, 0};',
          '    memcpy(w, r, size);',
          '    if (w[0] != e->lo || w[1] != e->hi)',
          '        return 1;',
          '    return have_flags() && fl != e->fl;',
          '}',
          '',
          '/* one line: the operation and its mismatches */',
          'static void report(const char *name, int bad) {',
          '    char b[64];',
          '    size_t k = strlen(name), j;',
          '    char d[12];',
          '    int n = 0;',
          '    memcpy(b, name, k);',
          '    do {',
          '        d[n++] = (char)(\'0\' + bad % 10);',
          '        bad /= 10;',
          '    } while (bad > 0);',
          '    for (j = 0; j < (size_t)n; j++)',
          '        b[k + j] = d[n - 1 - j];',
          '    print_str(b, k + (size_t)n);',
          '}',
          '']
    for name, _, expr, kind in ops:
        rt = {'T': 'T', 'I': 'int', 'L': 'long long', 'D': 'double', 'F': 'float'}.get(kind)
        if kind.startswith('T') and len(kind) == 2:
            rt = NAMES[int(kind[1])][1]
        L += ['static void t_%s(void) {' % name,
              '    int bad = 0, m, i, fl;',
              '    for (m = 0; m < 5; m++) {',
              '        fe_dec_setround(m);',
              '        for (i = 0; i < N; i++) {',
              '            volatile T a = ld(X[i]), b = ld(Y[i]), c = ld(Z[i]);',
              '            volatile long long n = INTS[i];',
              '            volatile double dbl;',
              '            %s r;' % rt,
              '            unsigned long long db = DBLS[i];',
              '            memcpy((void *)&dbl, &db, sizeof db);',
              '            (void)b;',
              '            (void)c;',
              '            (void)n;',
              '            feclearexcept(FE_ALL_EXCEPT);',
              '            r = %s;' % expr,
              '            fl = fetestexcept(FE_ALL_EXCEPT);',
              '            bad += differs(&E_%s[m][i], &r, sizeof r, fl);' % name,
              '        }',
              '    }',
              '    fe_dec_setround(FE_DEC_TONEAREST);',
              '    report("%s ", bad);' % name,
              '}', '']
    L += ['static void folds(void) {', '    int i, bad;']
    for name in folded:
        L += ['    bad = 0;',
              '    for (i = 0; i < N; i++)',
              '        bad += differs(&E_%s[0][i], &F_%s[i], sizeof F_%s[i], E_%s[0][i].fl) != 0;'
              % (name, name, name, name),
              '    report("fold_%s ", bad);' % name]
    L += ['    bad = 0;',
          '    for (i = 0; i < N; i++)',
          '        if (E_to_ll[0][i].fl % 2 == 0)',
          '            bad += differs(&E_to_ll[0][i], &F_to_ll[i], sizeof F_to_ll[i], E_to_ll[0][i].fl) != 0;',
          '    report("fold_to_ll ", bad);',
          '}', '',
          'int main(void) {']
    for name, _, _, _ in ops:
        L.append('    t_%s();' % name)
    L += ['    folds();', '    return 0;', '}']
    with open(os.path.join(out_dir, 'dec_gen_%s.c' % tag), 'w') as fh:
        fh.write('\n'.join(L) + '\n')


if __name__ == '__main__':
    for f, seed, n in [(0, 32, 48), (1, 64, 48), (2, 128, 40)]:
        write(f, seed, n)
