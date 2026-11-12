#!/usr/bin/env python3
"""Writes the C library's test of the wide long double functions' values,
computed here in arbitrary precision (mpmath) and rounded correctly to each
format:

    cmd/axiom/internal/tests/tools/gen_ld_math_tests.py      # needs mpmath

writes cmd/axiom/internal/tests/c/lib/math/math_values_ldouble.c. It holds,
for every function that std/c/rt/math_ld.ax computes in the format's
precision, inputs (random over the function's domain, and its hard places:
huge arguments of the trigonometric functions, results near overflow and in
the subnormal range, arguments near 1 for the logarithms) and the correctly
rounded results, as the bits of the x87's 80-bit format (x86-64 Linux) and
of binary128 (AArch64 Linux, WebAssembly). The program checks each result:
within one ulp on the x87, two on binary128 (the library's bounds), NaN for
NaN, zeros and infinities exact; and prints the number of failures per
function, which must be 0. Where long double is double, the functions are
the double ones (math_values_double.c tests them) and it prints the same.
The output is deterministic.
"""
import os
import random

import mpmath as mp

mp.mp.prec = 2000
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'c', 'lib', 'math', 'math_values_ldouble.c')
FMTS = {'x': (64, 16383), 'q': (113, 16383)}


def encode(v, fmt):
    """v rounded to nearest (ties to even) in the format: (lo, hi) words."""
    p, bias = FMTS[fmt]
    if mp.isnan(v):
        return (0xC000000000000000, 0xFFFF) if fmt == 'x' else (0, 0x7FFF800000000000)
    neg = v < 0
    a = abs(v)
    if mp.isinf(a) or a == 0:
        ef, sig = (0x7FFF if a else 0), ((1 << 63) if (fmt == 'x' and a) else 0)
    else:
        e = int(mp.floor(mp.log(a, 2)))
        while mp.mpf(2) ** e > a:
            e -= 1
        while mp.mpf(2) ** (e + 1) <= a:
            e += 1
        e = max(e, 1 - bias)
        sig = int(mp.nint(a / mp.mpf(2) ** (e - p + 1)))
        if sig >= (1 << p):
            sig >>= 1
            e += 1
        if e + bias >= 0x7FFF:
            ef, sig = 0x7FFF, ((1 << 63) if fmt == 'x' else 0)
        elif sig < (1 << (p - 1)):
            ef = 0
        else:
            ef = e + bias
    if fmt == 'x':
        return sig, ef | (0x8000 if neg else 0)
    fr = sig & ((1 << 112) - 1)
    return fr & ((1 << 64) - 1), (fr >> 64) | (ef << 48) | ((1 << 63) if neg else 0)


def decode(lo, hi, fmt):
    p, bias = FMTS[fmt]
    if fmt == 'x':
        neg, ef, sig = (hi >> 15) & 1, hi & 0x7FFF, lo
        e = (ef if ef else 1) - bias - 63
    else:
        neg, ef = hi >> 63, (hi >> 48) & 0x7FFF
        sig = ((hi & ((1 << 48) - 1)) << 64) | lo
        if ef:
            sig |= 1 << 112
        e = (ef if ef else 1) - bias - 112
    v = mp.mpf(sig) * mp.mpf(2) ** e
    return -v if neg else v


def rnd(rng, kind):
    if kind[0] == 'u':
        _, a, b = kind
        return mp.mpf(a) + (mp.mpf(b) - mp.mpf(a)) * mp.mpf(rng.getrandbits(160)) / mp.mpf(2) ** 160
    _, e0, e1, sg = kind
    v = mp.mpf(2) ** rng.randint(e0, e1) * (1 + mp.mpf(rng.getrandbits(160)) / mp.mpf(2) ** 160)
    return -v if (sg and rng.random() < 0.5) else v


def two(x):
    return mp.mpf(2) ** x


# name, arity, reference, random domains, hard arguments
FUNCS = [
    ('expl', 1, mp.exp, [('u', -11300, 11300), ('u', -2, 2), ('l', -60, -1, 1)],
     [11356.5, 11356.523, -11355.0, -11399.0, -11432.0, -11440.0, 1e-30, -1e-30, 0.5, 1]),
    ('exp2l', 1, lambda x: two(x), [('u', -16400, 16380), ('u', -3, 3)], [16383.9, -16382.5, -16440.2, -16494.5, 0.25]),
    ('expm1l', 1, mp.expm1, [('u', -60, 60), ('l', -80, -1, 1)], [1e-25, -1e-25, 0.0078, -0.0078, 120, -200]),
    ('logl', 1, mp.log, [('l', -16000, 16000, 0), ('u', 0.6, 1.5)], ['1.0000000000000000001', '0.99999999999999999999', 1.4179687, 1.41809, 2,
     two(-16440), two(-16490)]),
    ('log2l', 1, lambda x: mp.log(x, 2), [('l', -16000, 16000, 0), ('u', 0.6, 1.5)], [8, '1.00000000000000000001', two(-16400)]),
    ('log10l', 1, mp.log10, [('l', -16000, 16000, 0), ('u', 0.6, 1.5)], [1000, '0.9999999999999999999']),
    ('log1pl', 1, mp.log1p, [('l', -70, 60, 1), ('u', -0.99, 2)], [1e-30, -1e-30, '-0.9999999999999999999', 0.0028, -0.0029, 1e300]),
    ('sinl', 1, mp.sin, [('u', -10, 10), ('l', -40, 1000, 1), ('l', 1000, 16383, 1)],
     [1e300, mp.mpf(10) ** 4000, two(16383) * mp.mpf('1.9'), 1e-40, 355, 103993, mp.pi]),
    ('cosl', 1, mp.cos, [('u', -10, 10), ('l', -40, 1000, 1), ('l', 1000, 16383, 1)],
     [1e300, mp.mpf(10) ** 4000, 1e-40, 355, mp.pi / 2, 'pio4']),
    ('tanl', 1, mp.tan, [('u', -10, 10), ('l', -40, 16383, 1)], [mp.pi / 2, 1e22, 1e-40, 'pio4']),
    ('atanl', 1, mp.atan, [('l', -40, 40, 1), ('u', -3, 3)], ['1e-4000', '1e4000', 0.015625, 64, 1]),
    ('sinhl', 1, mp.sinh, [('u', -60, 60), ('l', -60, 13, 1)], [11357, -11357.5, 1e-30, 0.25]),
    ('coshl', 1, mp.cosh, [('u', -60, 60), ('l', -60, 13, 1)], [11357, -11358, 1e-30]),
    ('tanhl', 1, mp.tanh, [('u', -10, 10), ('l', -60, 6, 1)], [1e-30, 30, 50, -0.5]),
    ('cbrtl', 1, mp.cbrt, [('l', -16400, 16380, 1)], [27, -8, two(-16440), two(16383)]),
    ('asinl', 1, mp.asin, [('u', -1, 1), ('l', -60, 0, 1)], [1, -1, '0.99999999999999999999', 1e-30]),
    ('acosl', 1, mp.acos, [('u', -1, 1), ('l', -60, 0, 1)], [1, -1, '-0.99999999999999999999', 1e-30]),
    ('asinhl', 1, mp.asinh, [('l', -60, 100, 1)], ['1e4000', -1e-30]),
    ('acoshl', 1, mp.acosh, [('l', 0, 100, 0)], [1, '1.0000000000000000001', '1e4000']),
    ('atanhl', 1, mp.atanh, [('u', -1, 1), ('l', -60, 0, 1)], ['0.99999999999999999999', -1e-30]),
    ('powl', 2, mp.power, [(('l', -10, 10, 0), ('u', -200, 200)), (('u', 0.5, 2), ('l', 0, 14, 1)),
                           (('l', -16000, 16000, 0), ('u', -1, 1))],
     [(2, 16383.5), (2, -16440.5), ('1.0000000000000000001', 1e19), (-2, 3), (-2, 1e30), (10, 4932), (0.5, 16494)]),
    ('atan2l', 2, mp.atan2, [(('l', -40, 40, 1), ('l', -40, 40, 1))], [('1e-4000', -1), (-1, '1e-4000'), (1, 1), (1e-30, 1e30)]),
    ('hypotl', 2, mp.hypot, [(('l', -16000, 16000, 1), ('l', -16000, 16000, 1)), (('l', -5, 5, 1), ('l', -5, 5, 1))],
     [(two(16383), two(16383)), (two(-16440), two(-16440)), (3, 4), ('1e-4000', 1)]),
    ('fmal', 3, lambda x, y, z: x * y + z, [(('l', -100, 100, 1), ('l', -100, 100, 1), ('l', -200, 200, 1)),
                                           (('u', 1, 2), ('u', 1, 2), ('u', -4, -1))],
     [(1, 1, two(-200)), (two(8000), two(8000), -two(16000)), (two(-8000), two(-8400), two(-16300)),
      ('1.00000000000000000001', '0.99999999999999999999', -1)]),
]


def arg(v, fmt):
    if isinstance(v, str):
        if v == 'pio4':
            v = mp.pi / 4
        else:
            v = mp.mpf(v)
    return decode(*encode(mp.mpf(v), fmt), fmt)


def gen(fmt):
    rows = {}
    for name, ar, fn, doms, hard in FUNCS:
        rng = random.Random('%s-%s' % (name, fmt))
        inputs = []
        for h in hard:
            hv = h if isinstance(h, tuple) else (h,)
            inputs.append(tuple(arg(x, fmt) for x in hv))
        for i in range(16):
            d = doms[i % len(doms)]
            dd = d if ar > 1 else (d,)
            inputs.append(tuple(arg(rnd(rng, k), fmt) for k in dd))
        out = []
        for xs in inputs:
            try:
                ref = fn(*xs)
            except (ValueError, ZeroDivisionError):
                continue
            if isinstance(ref, mp.mpc):
                continue
            ws = []
            for x in xs:
                ws += list(encode(x, fmt))
            ws += list(encode(ref, fmt))
            out.append(ws)
        rows[name] = out
    return rows


def main():
    rx, rq = gen('x'), gen('q')
    L = ['/* The wide long double functions\' values (std/c/rt/math_ld.ax), against',
         '   correctly rounded results (written by',
         '   cmd/axiom/internal/tests/tools/gen_ld_math_tests.py; do not edit). */',
         '/* flags: --c-std=c11 */',
         '#include "print.h"',
         '#include <float.h>',
         '#include <math.h>',
         '#include <string.h>',
         '',
         'typedef unsigned long long U;',
         '',
         '#if __AXIOM_LDBL_WIDTH__ != 64',
         'static long double ld(U lo, U hi)',
         '{',
         '    long double v;',
         '    unsigned char b[16];',
         '    memcpy(b, &lo, 8);',
         '    memcpy(b + 8, &hi, 8);',
         '    memset(&v, 0, sizeof v);',
         '    memcpy(&v, b, sizeof v);',
         '    return v;',
         '}',
         '',
         '/* r within n ulps of want (NaN for NaN; zeros and infinities exact) */',
         'static int ok(long double r, long double want, int n)',
         '{',
         '    long double u;',
         '    if (isnan(want))',
         '        return isnan(r);',
         '    if (want == 0 || isinf(want))',
         '        return r == want && signbit(r) == signbit(want);',
         '    if (isnan(r) || isinf(r) || signbit(r) != signbit(want))',
         '        return 0;',
         '    u = ilogbl(want) < LDBL_MIN_EXP - 1 ? LDBL_TRUE_MIN : scalbnl(1.0L, ilogbl(want) - (LDBL_MANT_DIG - 1));',
         '    return fabsl(r - want) <= n * u;',
         '}',
         '',
         '#if __AXIOM_LDBL_WIDTH__ == 80',
         '#define ULPS 1',
         ]
    def tables(rows):
        T = []
        for name, ar, *_ in FUNCS:
            T.append('static const U t_%s[][%d] = {' % (name, 2 * ar + 2))
            for ws in rows[name]:
                T.append('    {%s},' % ', '.join('0x%x' % w for w in ws))
            T.append('};')
        return T
    L += tables(rx)
    L += ['#else', '#define ULPS 2']
    L += tables(rq)
    L += ['#endif', '', '#define N(t) (sizeof t / sizeof t[0])', '', 'int main(void)', '{', '    unsigned i;', '    long bad;']
    for name, ar, *_ in FUNCS:
        L.append('    bad = 0;')
        L.append('    for (i = 0; i < N(t_%s); i++) {' % name)
        a = ['ld(t_%s[i][%d], t_%s[i][%d])' % (name, 2 * k, name, 2 * k + 1) for k in range(ar)]
        w = 'ld(t_%s[i][%d], t_%s[i][%d])' % (name, 2 * ar, name, 2 * ar + 1)
        L.append('        if (!ok(%s(%s), %s, ULPS))' % (name, ', '.join(a), w))
        L.append('            bad++;')
        L.append('    }')
        L.append('    print_str("%s", %d);' % (name, len(name)))
        L.append('    print_i64(bad);')
    L += ['    return 0;', '}', '#else', '/* (long double is double: the double functions) */', 'int main(void)', '{']
    for name, *_ in FUNCS:
        L.append('    print_str("%s", %d);' % (name, len(name)))
        L.append('    print_i64(0);')
    L += ['    return 0;', '}', '#endif', '']
    for name, *_ in FUNCS:
        L.append('/* expect: %s */' % name)
        L.append('/* expect: 0 */')
    with open(OUT, 'w') as f:
        f.write('\n'.join(L) + '\n')
    print('wrote', OUT, sum(len(v) for v in rx.values()), sum(len(v) for v in rq.values()))


main()
