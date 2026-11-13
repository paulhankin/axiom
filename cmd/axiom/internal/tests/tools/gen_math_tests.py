#!/usr/bin/env python3
"""Writes the C library's tests of <math.h>'s values, computed here in
arbitrary precision (mpmath) and rounded correctly:

    cmd/axiom/internal/tests/tools/gen_math_tests.py      # needs mpmath

writes cmd/axiom/internal/tests/c/lib/math/math_values_double.c and
math_values_float.c (C99's functions), math_values_c23_double.c and
math_values_c23_float.c (C23's). Each holds, for every function, inputs (random over the
function's domain, and its hard places: near zeros, poles, overflow and
underflow thresholds, huge arguments of the trigonometric functions) and
the correctly rounded results; the program checks that each result is
within one ulp (NaN for NaN, and the zeros' signs exact) and prints the
number of failures per function, which must be 0. The output is
deterministic.
"""
import math
import os
import random
import struct

import mpmath as mp

mp.mp.prec = 300
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'c', 'lib', 'math')


def db(x):
    return struct.unpack('<Q', struct.pack('<d', x))[0]


def fb(x):
    return struct.unpack('<I', struct.pack('<f', x))[0]


def to_f32(v):
    """mpf v rounded to binary32 (correctly: via its exact rational)."""
    if isinstance(v, float):
        return v
    if mp.isnan(v):
        return float('nan')
    if mp.isinf(v):
        return float(v)
    with mp.workprec(24):
        r = mp.mpf(v)  # (rounds to 24 bits; then check the exponent range)
    if abs(r) >= mp.mpf(2) ** 128:
        return float('inf') if r > 0 else float('-inf')
    if abs(v) < mp.mpf(2) ** -126:
        # subnormal: round to a multiple of 2^-149
        q = mp.nint(v * mp.mpf(2) ** 149)
        return math.copysign(float(q * mp.mpf(2) ** -149), float(mp.sign(v)))
    return float(r)


def to_f64(v):
    if isinstance(v, float):
        return v
    if mp.isnan(v):
        return float('nan')
    if mp.isinf(v):
        return float(v)
    if abs(v) < mp.mpf(2) ** -1022:
        q = mp.nint(v * mp.mpf(2) ** 1074)
        return math.copysign(float(q * mp.mpf(2) ** -1074), float(mp.sign(v)))
    with mp.workprec(53):
        r = mp.mpf(v)
    if abs(r) >= mp.mpf(2) ** 1024:
        return float('inf') if r > 0 else float('-inf')
    return float(r)


def F(name, *xs):
    x = [mp.mpf(a) for a in xs]
    a = x[0]
    if name in ('exp', 'expm1', 'log', 'log10', 'log1p', 'sin', 'cos', 'tan', 'asin', 'acos', 'atan', 'sinh', 'cosh',
                'tanh', 'asinh', 'acosh', 'atanh', 'erf', 'erfc', 'sqrt'):
        if name in ('log', 'log10', 'sqrt') and a < 0:
            return mp.nan
        if name == 'log1p' and a < -1:
            return mp.nan
        if name in ('asin', 'acos', 'atanh') and abs(a) > 1:
            return mp.nan
        if name == 'acosh' and a < 1:
            return mp.nan
        if name in ('log', 'log10') and a == 0:
            return -mp.inf
        if name in ('sin', 'cos', 'tan') and abs(a) > 1e10:
            with mp.workprec(int(mp.log(abs(a), 2)) + 300):
                return getattr(mp, name)(a)
        return getattr(mp, name)(a)
    if name == 'exp2':
        return mp.power(2, a)
    if name == 'log2':
        return mp.log(a, 2) if a > 0 else (mp.nan if a < 0 else -mp.inf)
    if name == 'cbrt':
        return mp.cbrt(a) if a >= 0 else -mp.cbrt(-a)
    if name == 'tgamma':
        if a == 0:
            return mp.inf if math.copysign(1, float(xs[0])) > 0 else -mp.inf
        if a < 0 and a == mp.floor(a):
            return mp.nan  # (a domain error)
        return mp.gamma(a)
    if name == 'lgamma':
        if a <= 0 and a == mp.floor(a):
            return mp.inf
        return mp.re(mp.loggamma(a))
    if name == 'exp2m1':
        return mp.expm1(a * mp.log(2))
    if name == 'exp10m1':
        return mp.expm1(a * mp.log(10))
    if name in ('logp1', 'log2p1', 'log10p1'):
        if a < -1:
            return mp.nan
        if a == -1:
            return -mp.inf
        return mp.log1p(a) / {'logp1': 1, 'log2p1': mp.log(2), 'log10p1': mp.log(10)}[name]
    if name == 'rsqrt':
        return mp.nan if a < 0 else (mp.inf if a == 0 else 1 / mp.sqrt(a))
    if name in ('sinpi', 'cospi', 'tanpi'):
        r = a - 2 * mp.floor(a / 2)  # (exact: a is a double)
        v = {'sinpi': mp.sinpi, 'cospi': mp.cospi, 'tanpi': lambda t: mp.sinpi(t) / mp.cospi(t)}[name](r)
        if v == 0:
            # the zeros' signs (F.10.1.13-15): sinpi(n) is x's sign; tanpi(n)
            # +0 for positive even and negative odd n, -0 for the others;
            # cospi(n + 1/2) +0
            neg = math.copysign(1, float(xs[0])) < 0
            if name == 'tanpi' and (int(a) % 2 == 1):
                neg = not neg
            if name == 'cospi':
                neg = False
            return -0.0 if neg else 0.0  # (a float: mpmath's zeros have no sign)
        return v
    if name in ('asinpi', 'acospi'):
        return mp.nan if abs(a) > 1 else getattr(mp, name[:4])(a) / mp.pi
    if name == 'atanpi':
        return mp.atan(a) / mp.pi
    if name == 'atan2pi':
        return mp.atan2(a, x[1]) / mp.pi
    if name == 'powr':
        return mp.nan if a < 0 else mp.power(a, x[1])
    if name == 'pown':
        return mp.power(a, int(x[1]))
    if name == 'rootn':
        n = int(x[1])
        r = mp.root(abs(a), n) if n > 0 else 1 / mp.root(abs(a), -n)
        return -r if a < 0 else r
    if name == 'compoundn':
        return mp.exp(int(x[1]) * mp.log1p(a))
    if name == 'pow':
        return mp.power(a, x[1])
    if name == 'atan2':
        return mp.atan2(a, x[1])
    if name == 'hypot':
        return mp.hypot(a, x[1])
    raise SystemExit(name)


ONE = {
    'exp': [(-745, 709.7)], 'exp2': [(-1074, 1023.9)], 'expm1': [(-40, 709), (-1e-3, 1e-3)],
    'log': [(1e-300, 1e300), (0.9, 1.1)], 'log2': [(1e-300, 1e300), (0.9, 1.1)], 'log10': [(1e-300, 1e300), (0.9, 1.1)],
    'log1p': [(-0.999, 10), (-1e-6, 1e-6)], 'sin': [(-10, 10), (-1e15, 1e15), (1e100, 1e300)],
    'cos': [(-10, 10), (-1e15, 1e15), (1e100, 1e300)], 'tan': [(-10, 10), (-1e15, 1e15), (1e100, 1e300)],
    'asin': [(-1, 1)], 'acos': [(-1, 1)], 'atan': [(-100, 100), (1e10, 1e300)], 'sinh': [(-710, 710), (-1, 1)],
    'cosh': [(-710, 710), (-1, 1)], 'tanh': [(-20, 20), (-0.1, 0.1)], 'asinh': [(-1e300, 1e300), (-1, 1)],
    'acosh': [(1, 1e300), (1, 2)], 'atanh': [(-1, 1)], 'cbrt': [(-1e300, 1e300)], 'sqrt': [(0, 1e300)],
    'erf': [(-6, 6), (-1, 1)], 'erfc': [(-6, 27), (0, 2)], 'tgamma': [(0.001, 171), (-30.5, -0.5)],
    'lgamma': [(0.001, 1e10), (0.5, 3)],
}
TWO = {'pow': [((0, 4), (-300, 300)), ((-10, 0), None)], 'atan2': [((-10, 10), (-10, 10))],
       'hypot': [((-1e300, 1e300), (-1e300, 1e300)), ((-10, 10), (-10, 10))]}
SPECIAL = {
    'exp': [0.0, -0.0, 709.782712893384, -745.1332191019411, -708.4, 1e-20],
    'expm1': [1e-300, -37.0, 0.25],
    'log': [1.0, 5e-324, 2.2250738585072014e-308, 1.7976931348623157e308, 0.9999999999999999, 1.0000000000000002],
    'log1p': [-0.5, 5e-324, 1.7976931348623157e308],
    'sin': [3.141592653589793, 1e22, 6381956970095103.0 * 2.0 ** 797, 1e-300, 5e-324],
    'cos': [1.5707963267948966, 1e22, 6381956970095103.0 * 2.0 ** 797],
    'tan': [1.5707963267948966, 1e22, 6381956970095103.0 * 2.0 ** 797],
    'atan': [1.0, 1.7976931348623157e308], 'acosh': [1.0, 1.0000000000000002],
    'tgamma': [0.5, 1.0, 2.0, 3.5, 170.5, 171.6, -0.5, -1.5, -170.5, -177.5, 1e-300],
    'lgamma': [1.0, 2.0, 0.5, 1e-300, 1e300, 2.5e305, -0.5, -2.5],
    'erfc': [26.5, 27.2], 'erf': [1e-300, 5e-324],
}

# C23's (the second argument of pown, rootn and compoundn an integer)
ONE23 = {
    'exp2m1': [(-60, 1023), (-0.1, 0.1)], 'exp10m1': [(-18, 308), (-0.03, 0.03)], 'logp1': [(-0.999, 1e300)],
    'log2p1': [(-0.999, 1e300), (-1e-3, 1e-3)], 'log10p1': [(-0.999, 1e300), (-1e-3, 1e-3)],
    'rsqrt': [(1e-300, 1e300), (0.5, 4)], 'sinpi': [(-10, 10), (-1e15, 1e15)], 'cospi': [(-10, 10), (-1e15, 1e15)],
    'tanpi': [(-10, 10), (0.2, 0.3)], 'asinpi': [(-1, 1)], 'acospi': [(-1, 1)], 'atanpi': [(-100, 100), (1e10, 1e300)],
}
TWO23 = {'atan2pi': [((-10, 10), (-10, 10))], 'powr': [((0, 4), (-300, 300)), ((1e-10, 1e10), (-30, 30))]}
INT23 = {'pown': [((-3, 3), (-300, 300)), ((0.9, 1.1), (-5000, 5000))],
         'rootn': [((1e-300, 1e300), (2, 40)), ((-1e300, -1e-300), (3, 41)), ((1e-10, 1e10), (-20, -2))],
         'compoundn': [((-0.99, 3), (-300, 300)), ((-1e-3, 1e-3), (-100000, 100000))]}
SPECIAL23 = {
    'exp2m1': [1.0, 2.0, 10.0, -1.0, 1e-300, 5e-324], 'exp10m1': [1.0, 2.0, 15.0, -1.0, 1e-300],
    'log2p1': [1.0, 3.0, 1023.0, 5e-324], 'log10p1': [9.0, 99.0, 999.0], 'rsqrt': [4.0, 0.25, 5e-324, 1.7976931348623157e308],
    'sinpi': [0.5, 1.0, 1.5, -2.0, 0.25, 1e-300], 'cospi': [0.5, 1.0, 0.25, 1e300],
    'tanpi': [0.25, -0.25, 1.0, 2.0, -3.0, 0.75, 1e-300], 'asinpi': [1.0, -1.0, 0.5], 'acospi': [1.0, -1.0, 0.5],
    'atanpi': [1.0, -1.0, 1e-300],
}


def rand_in(rng, lo, hi):
    if lo > 0 and hi / lo > 100:
        return float(2.0 ** rng.uniform(float(mp.log(lo, 2)), float(mp.log(hi, 2))))
    return float(rng.uniform(lo, hi))


def gen(single, one=ONE, two=TWO, special=SPECIAL, ints=None, seed=0):
    rng = random.Random((1 if single else 2) + seed)
    out = []
    for name, ranges in one.items():
        xs = []
        for lo, hi in ranges:
            if single:
                # (float's range)
                lo = max(min(lo, 1e38), -1e38) if abs(lo) > 1e38 else lo
                hi = min(hi, 1e38)
                if lo >= hi:
                    lo = hi / 1e10
            xs += [rand_in(rng, lo, hi) for _ in range(40 // len(ranges))]
        xs += special.get(name, [])
        rows = []
        for x in xs:
            if single:
                x = to_f32(mp.mpf(x)) if abs(x) < 3e38 else x
                if abs(x) >= 3.4e38:
                    continue
            v = F(name, x)
            r = to_f32(v) if single else to_f64(v)
            rows.append(([x], r))
        out.append((name, 1, rows))
    for name, ranges in (ints or {}).items():
        rows = []
        for rx, rn in ranges:
            for _ in range(30 // len(ranges)):
                x = rand_in(rng, *rx)
                if single:
                    x = to_f32(mp.mpf(x))
                    if abs(x) > 3e38:
                        continue
                n = rng.randint(*rn)
                if name == 'rootn' and n == 0:
                    n = 3
                if name == 'rootn' and x < 0 and n % 2 == 0:
                    n += 1  # (an even root of a negative x is a domain error)
                v = F(name, x, n)
                r = to_f32(v) if single else to_f64(v)
                rows.append(([x, n], r))
        out.append((name, -2, rows))
    for name, ranges in two.items():
        rows = []
        for rx, ry in ranges:
            for _ in range(30):
                if single:
                    rx = (max(rx[0], -1e38), min(rx[1], 1e38))
                    ry = (max(ry[0], -1e38), min(ry[1], 1e38)) if ry else None
                x = rand_in(rng, *rx)
                y = rand_in(rng, *ry) if ry else float(rng.randint(-40, 40))
                if single:
                    x = to_f32(mp.mpf(x))
                    y = to_f32(mp.mpf(y))
                    if abs(x) > 3e38 or abs(y) > 3e38:
                        continue
                v = F(name, x, y)
                r = to_f32(v) if single else to_f64(v)
                rows.append(([x, y], r))
        out.append((name, 2, rows))
    return out


def write(single, c23=False):
    ty = 'float' if single else 'double'
    sfx = 'f' if single else ''
    ut = 'unsigned long' if single else 'unsigned long long'
    bits = fb if single else db
    if c23:
        funcs = gen(single, ONE23, TWO23, SPECIAL23, INT23, 10)
        fname = 'math_values_c23_%s.c' % ty
        which = 'C23\'s %s functions' % ty
        std = 'c23'
    else:
        funcs = gen(single)
        fname = 'math_values_%s.c' % ty
        which = '<math.h>\'s %s functions' % ty
        std = 'c99'
    L = ['/* %s - generated by cmd/axiom/internal/tests/tools/gen_math_tests.py (do not edit):' % fname,
         '   %s within one ulp of the correctly rounded results */' % which,
         '/* flags: --c-std=%s */' % std, '#include "print.h"', '#include <math.h>', '',
         'typedef %s U;' % ut, '',
         '/* x as an integer that orders the values (adjacent values differ by 1) */',
         'static long long ord(U b)', '{',
         '    U s = (U)1 << (sizeof(U) * 8 - 1);',
         '    return (b & s) ? -(long long)(b & ~s) : (long long)b;', '}', '',
         'static U bits_of(%s x)' % ty, '{', '    union { %s f; U u; } v;' % ty, '    v.u = 0;', '    v.f = x;',
         '    return v.u;', '}', '', 'static %s val(U b)' % ty, '{', '    union { %s f; U u; } v;' % ty,
         '    v.u = b;', '    return v.f;', '}', '',
         '/* r is within one ulp of want: equal, or both NaNs, or neighbours of the same sign */',
         'static int ok(%s r, U want)' % ty, '{', '    U b = bits_of(r);',
         '    long long d = ord(b) - ord(want);', '    if (isnan(val(want)))', '        return isnan(r);',
         '    if (val(want) == 0 || isinf(val(want)))', '        return b == want;',
         '    return d >= -1 && d <= 1 && signbit(r) == signbit(val(want));', '}', '']
    for name, ar, rows in funcs:
        L.append('static const U t_%s[][%d] = {' % (name, abs(ar) + 1))
        for xs, r in rows:
            if ar < 0:
                # (x, the integer n as two's complement bits)
                # (U is 32 bits wide for float on 32-bit targets: n's low 32 bits there)
                mask = 0xffffffff if single else 0xffffffffffffffff
                L.append('    {%s},' % ', '.join(['0x%x' % bits(float(xs[0])), '0x%x' % (xs[1] & mask),
                                                    '0x%x' % bits(r)]))
                continue
            L.append('    {%s},' % ', '.join(['0x%x' % bits(float(x)) for x in xs] + ['0x%x' % bits(r)]))
        L.append('};')
        L.append('')
    L.append('int main(void)')
    L.append('{')
    L.append('    unsigned i, bad;')
    for name, ar, rows in funcs:
        if ar < 0:
            if single:
                args = 'val(t_%s[i][0]), (long long)(int)(unsigned int)t_%s[i][1]' % (name, name)
            else:
                args = 'val(t_%s[i][0]), (long long)t_%s[i][1]' % (name, name)
        else:
            args = ', '.join('val(t_%s[i][%d])' % (name, k) for k in range(ar))
        L.append('    for (bad = i = 0; i < sizeof t_%s / sizeof t_%s[0]; i++)' % (name, name))
        L.append('        bad += !ok(%s%s(%s), t_%s[i][%d]);' % (name, sfx, args, name, abs(ar)))
        L.append('    print_str("%s%s", %d);' % (name, sfx, len(name + sfx)))
        L.append('    print_i64(bad);')
    L.append('    return 0;')
    L.append('}')
    for name, ar, rows in funcs:
        L.append('/* expect: %s%s */' % (name, sfx))
        L.append('/* expect: 0 */')
    res = L
    os.makedirs(OUT, exist_ok=True)
    with open(os.path.join(OUT, fname), 'w') as f:
        f.write('\n'.join(res) + '\n')


write(False)
write(True)
write(False, True)
write(True, True)
