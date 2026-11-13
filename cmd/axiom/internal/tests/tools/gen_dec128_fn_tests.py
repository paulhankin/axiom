#!/usr/bin/env python3
"""Writes the C library's test of _Decimal128's elementary functions in every
decimal rounding direction, against values computed here in arbitrary
precision (mpmath, 80 digits) rounded to 34 digits by Python's decimal
module in each direction:

    cmd/axiom/internal/tests/tools/gen_dec128_fn_tests.py      # needs mpmath

writes cmd/axiom/internal/tests/c/lib/math/dec_fn_d128.c. The arguments are
fixed and of every kind the functions take (most go through std/c/rt/
dfp_fast.ax's wide form, some fall back to the decimal computation: near a
rounding boundary, exact results, huge and tiny arguments); each line of the
expected output is one argument's results in the five directions, printed
with %.33DDe. The output is deterministic.
"""
import decimal
import os

import mpmath as mp

mp.mp.dps = 80
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'c', 'lib', 'math', 'dec_fn_d128.c')
MODES = [('FE_DEC_TONEAREST', decimal.ROUND_HALF_EVEN), ('FE_DEC_DOWNWARD', decimal.ROUND_FLOOR),
         ('FE_DEC_UPWARD', decimal.ROUND_CEILING), ('FE_DEC_TOWARDZERO', decimal.ROUND_DOWN),
         ('FE_DEC_TONEARESTFROMZERO', decimal.ROUND_HALF_UP)]

# name, reference, arguments (decimal strings; pairs for the functions of two)
FUNCS = [
    ('exp', mp.exp, ['1.25', '-0.001', '123.456789012345678901234567890123', '1E-40']),
    ('exp2', lambda x: mp.power(2, x), ['0.5', '-77.25']),
    ('exp10', lambda x: mp.power(10, x), ['0.3', '-3.7']),
    ('expm1', mp.expm1, ['1E-10', '0.75', '-2.5']),
    ('log', mp.log, ['1.25', '0.999999', '123456789.123456789', '1E-300']),
    ('log2', lambda x: mp.log(x, 2), ['3', '0.1']),
    ('log10', mp.log10, ['2', '12345.6789']),
    ('log1p', mp.log1p, ['1E-12', '0.5', '-0.5']),
    ('sin', mp.sin, ['1', '-0.5', '1000.5', '1E-20']),
    ('cos', mp.cos, ['1', '3.14159']),
    ('tan', mp.tan, ['0.7', '-1.5']),
    ('asin', mp.asin, ['0.5', '-0.123456789012345678901234567890123']),
    ('acos', mp.acos, ['0.25', '-0.999']),
    ('atan', mp.atan, ['3', '-0.0001']),
    ('sinh', mp.sinh, ['0.1', '-5']),
    ('cosh', mp.cosh, ['2.5']),
    ('tanh', mp.tanh, ['0.3', '-1.1']),
    ('asinh', mp.asinh, ['1.5', '-1E-5']),
    ('acosh', mp.acosh, ['1.0001', '7']),
    ('atanh', mp.atanh, ['0.5', '-0.9']),
    ('cbrt', lambda x: mp.sign(x) * mp.cbrt(abs(x)), ['2', '-0.007000000000000000000000000000001']),
    ('erf', mp.erf, ['0.5', '-1.7']),
    ('erfc', mp.erfc, ['0.5', '3']),
    ('lgamma', lambda x: mp.log(abs(mp.gamma(x))), ['0.5', '7.25', '100.125']),
    ('tgamma', mp.gamma, ['0.5', '4.75', '-1.5']),
    ('sinpi', lambda x: mp.sin(mp.pi * x), ['0.1', '-0.3']),
    ('atanpi', lambda x: mp.atan(x) / mp.pi, ['2']),
    ('pow', mp.power, [('1.25', '0.3'), ('7', '-2.5'), ('0.5', '13.75'), ('2', '10')]),
    ('atan2', mp.atan2, [('1', '2'), ('-3', '0.5')]),
    ('hypot', mp.hypot, [('3', '4.5'), ('1E-5', '1')]),
]


def c34(v, rounding):
    """v rounded to 34 digits in the direction, as %.33DDe prints it."""
    ctx = decimal.Context(prec=34, rounding=rounding, Emax=6144, Emin=-6143)
    if isinstance(v, mp.mpc):
        raise ValueError(str(v))
    d = ctx.plus(decimal.Decimal(mp.nstr(v, 75)))
    s = format(d, '.33e')
    mant, ex = s.split('e')
    e = int(ex)
    return '%se%s%02d' % (mant, '-' if e < 0 else '+', abs(e))


def lit(a):
    """a as a _Decimal128 constant (an integer gets its point)."""
    return (a if ('.' in a or 'E' in a) else a + '.') + 'DL'


def main():
    lines = []
    calls = []
    for name, fn, args in FUNCS:
        for a in args:
            if isinstance(a, tuple):
                v = fn(mp.mpf(a[0]), mp.mpf(a[1]))
                call = '%sd128(%s, %s)' % (name, lit(a[0]), lit(a[1]))
            else:
                v = fn(mp.mpf(a))
                call = '%sd128(%s)' % (name, lit(a))
            lines.append(' '.join(c34(v, r) for _, r in MODES))
            calls.append(call)
    out = ['/* dec_fn_d128.c - written by cmd/axiom/internal/tests/tools/gen_dec128_fn_tests.py: _Decimal128\'s',
           '   elementary functions in every decimal rounding direction against correctly rounded values',
           '   (the wide form of std/c/rt/dfp_fast.ax and the decimal computation it falls back to). */',
           '/* flags: --c-std=c23 */']
    out += ['/* expect: %s */' % l for l in lines]
    out += ['#include <fenv.h>', '#include <math.h>', '#include <stdio.h>', '',
            '#pragma STDC FENV_ACCESS ON', '',
            'static const int modes[5] = {%s};' % ', '.join(m for m, _ in MODES), '',
            'static void p(_Decimal128 x, int last)', '{', '    printf(last ? "%.33DDe\\n" : "%.33DDe ", x);', '}', '',
            '#define T(e) do { for (int k = 0; k < 5; k++) { fe_dec_setround(modes[k]); _Decimal128 v = (e); fe_dec_setround(FE_DEC_TONEAREST); p(v, k == 4); } } while (0)',
            '', 'int main(void)', '{']
    out += ['    T(%s);' % c for c in calls]
    out += ['    return 0;', '}', '']
    open(OUT, 'w').write('\n'.join(out))
    print('wrote', OUT, len(calls))


main()
