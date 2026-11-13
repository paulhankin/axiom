#!/usr/bin/env python3
"""Writes the C library's test of the _Float16 elementary functions
(std/c/lib/math/m_f16.c), which are correctly rounded: for each function,
binary16 arguments (random over the function's domain, and every exponent)
and the correctly rounded binary16 results, computed here in arbitrary
precision (mpmath) and rounded to nearest with ties to even:

    cmd/axiom/internal/tests/tools/gen_f16_math_tests.py      # needs mpmath

writes cmd/axiom/internal/tests/c/lib/math/math_values_f16.c, which checks
each result's bits and prints the number of mismatches per function (0).
(Every binary16 argument of the one-argument functions was checked so in
development; the table here is a sample.) The output is deterministic.
"""
import os
import random

import mpmath as mp

mp.mp.prec = 200
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'c', 'lib', 'math', 'math_values_f16.c')

F1 = [('acos', mp.acos), ('asin', mp.asin), ('atan', mp.atan), ('cos', mp.cos), ('sin', mp.sin), ('tan', mp.tan),
      ('acosh', mp.acosh), ('asinh', mp.asinh), ('atanh', mp.atanh), ('cosh', mp.cosh), ('sinh', mp.sinh),
      ('tanh', mp.tanh), ('exp', mp.exp), ('exp2', lambda x: mp.power(2, x)), ('exp10', lambda x: mp.power(10, x)),
      ('expm1', mp.expm1), ('log', mp.log), ('log10', mp.log10), ('log1p', mp.log1p), ('log2', lambda x: mp.log(x, 2)),
      ('cbrt', mp.cbrt), ('erf', mp.erf), ('erfc', mp.erfc), ('lgamma', lambda x: mp.log(abs(mp.gamma(x)))),
      ('tgamma', mp.gamma)]
F2 = [('atan2', mp.atan2), ('pow', mp.power), ('hypot', mp.hypot)]


def hval(b):
    s = -1 if b >> 15 else 1
    e = (b >> 10) & 31
    f = b & 1023
    if e == 0:
        return s * mp.mpf(f) * mp.mpf(2) ** -24
    return s * (1024 + f) * mp.mpf(2) ** (e - 25)


def tohalf(v):
    """v rounded to binary16 (to nearest, ties to even): its bits."""
    neg = v < 0
    a = abs(v)
    if mp.isinf(a):
        return 0xfc00 if neg else 0x7c00
    if a == 0:
        return 0x8000 if neg else 0
    e = max(int(mp.floor(mp.log(a, 2))), -14)
    q = a / mp.mpf(2) ** (e - 10)
    n = int(mp.floor(q))
    r = q - n
    if r > 0.5 or (r == 0.5 and n % 2):
        n += 1
    if n >= 2048:
        n //= 2
        e += 1
    if e > 15:
        return 0xfc00 if neg else 0x7c00
    bits = n if n < 1024 else ((e + 15) << 10) | (n - 1024)
    return bits | (0x8000 if neg else 0)


def ref(fn, xs):
    try:
        v = fn(*[hval(x) for x in xs])
    except (ValueError, ZeroDivisionError):
        return None
    if isinstance(v, mp.mpc) or mp.isnan(v):
        return None
    return tohalf(v)


def main():
    rng = random.Random(16)
    finite = [b for b in range(65536) if (b & 0x7c00) != 0x7c00 and (b & 0x7fff) != 0]
    L = ['/* The _Float16 elementary functions (std/c/lib/math/m_f16.c), correctly',
         '   rounded: their results\' bits against an arbitrary-precision reference',
         '   (written by cmd/axiom/internal/tests/tools/gen_f16_math_tests.py; do',
         '   not edit). */',
         '/* flags: --c-std=c23 */',
         '#define __STDC_WANT_IEC_60559_TYPES_EXT__ 1',
         '#include "print.h"',
         '#include <math.h>',
         '#include <string.h>',
         '',
         'static _Float16 h(unsigned b)',
         '{',
         '    _Float16 v;',
         '    unsigned short u = (unsigned short)b;',
         '    memcpy(&v, &u, 2);',
         '    return v;',
         '}',
         '',
         'static unsigned bits(_Float16 v)',
         '{',
         '    unsigned short u;',
         '    memcpy(&u, &v, 2);',
         '    return u;',
         '}',
         '']
    names = []
    for name, fn in F1:
        rows = []
        for b in rng.sample(finite, 400):
            w = ref(fn, (b,))
            if w is not None:
                rows.append((b, w))
        L.append('static const unsigned short t_%s[][2] = {' % name)
        L += ['    {0x%04x, 0x%04x},' % r for r in rows]
        L.append('};')
        names.append((name, 1))
    for name, fn in F2:
        rows = []
        while len(rows) < 400:
            a, b = rng.choice(finite), rng.choice(finite)
            if name == 'pow' and rng.random() < 0.5:
                b = rng.choice([0x3c00 + k for k in range(0, 0x800, 0x40)] + [0x4000, 0x4200, 0x4400, 0xc000, 0x3800])
            w = ref(fn, (a, b))
            if w is not None:
                rows.append((a, b, w))
        L.append('static const unsigned short t_%s[][3] = {' % name)
        L += ['    {0x%04x, 0x%04x, 0x%04x},' % r for r in rows]
        L.append('};')
        names.append((name, 2))
    L += ['', '#define N(t) (sizeof t / sizeof t[0])', '', 'int main(void)', '{', '    unsigned i;', '    long bad;']
    for name, ar in names:
        L.append('    bad = 0;')
        L.append('    for (i = 0; i < N(t_%s); i++)' % name)
        if ar == 1:
            L.append('        if (bits(%sf16(h(t_%s[i][0]))) != t_%s[i][1])' % (name, name, name))
        else:
            L.append('        if (bits(%sf16(h(t_%s[i][0]), h(t_%s[i][1]))) != t_%s[i][2])' % (name, name, name, name))
        L.append('            bad++;')
        L.append('    print_str("%s", %d);' % (name, len(name)))
        L.append('    print_i64(bad);')
    L += ['    return 0;', '}']
    for name, _ in names:
        L.append('/* expect: %s */' % name)
        L.append('/* expect: 0 */')
    with open(OUT, 'w') as f:
        f.write('\n'.join(L) + '\n')
    print('wrote', OUT)


main()
