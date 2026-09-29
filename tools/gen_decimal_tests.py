#!/usr/bin/env python3
"""Generates std/decimal/testdata/cases.txt: random operations on decimals
with the results Python's decimal module gives.

Each line is: op a b n mode result
  op    add sub mul (exact), div (n significant digits), sqrt (n digits,
        b unused), quantize (to exponent n), round_sig (n digits),
        cmp (result -1, 0 or 1)
  mode  0-6: HALF_EVEN HALF_UP HALF_DOWN DOWN UP FLOOR CEILING
"""
import decimal
import os
import random

MODES = [decimal.ROUND_HALF_EVEN, decimal.ROUND_HALF_UP, decimal.ROUND_HALF_DOWN, decimal.ROUND_DOWN,
         decimal.ROUND_UP, decimal.ROUND_FLOOR, decimal.ROUND_CEILING]
rng = random.Random(2024)


def number():
    k = rng.choice([1, 1, 2, 3, 5, 8, 15, 30, 60])
    digits = ''.join(rng.choice('0123456789') for _ in range(k))
    if rng.random() < 0.3:
        digits = digits.rstrip('0') + '0' * rng.randint(0, 4)
    if rng.random() < 0.1:
        digits = '5' * k
    e = rng.randint(-20, 10)
    sign = '-' if rng.random() < 0.4 else ''
    return decimal.Decimal('%s%sE%d' % (sign, digits or '0', e))


def plain(d):
    s = str(d)
    return s if not s.startswith('-0') or d != 0 else s[1:]


lines = []
big = decimal.Context(prec=10000, Emax=10 ** 9, Emin=-10 ** 9)
for i in range(600):
    op = ['add', 'sub', 'mul', 'div', 'sqrt', 'quantize', 'round_sig', 'cmp'][i % 8]
    a, b = number(), number()
    m = rng.randrange(7)
    n = 0
    if op == 'add':
        r = big.add(a, b)
    elif op == 'sub':
        r = big.subtract(a, b)
    elif op == 'mul':
        r = big.multiply(a, b)
    elif op == 'div':
        if b == 0:
            b = decimal.Decimal(7)
        n = rng.choice([1, 2, 5, 10, 28, 50])
        r = decimal.Context(prec=n, rounding=MODES[m], Emax=10 ** 9, Emin=-10 ** 9).divide(a, b)
    elif op == 'sqrt':
        a = abs(a)
        n = rng.choice([1, 3, 10, 28, 40])
        m = 0
        r = decimal.Context(prec=n, Emax=10 ** 9, Emin=-10 ** 9).sqrt(a)
    elif op == 'quantize':
        n = rng.randint(-25, 5)
        r = a.quantize(decimal.Decimal('1E%d' % n), rounding=MODES[m], context=big)
    elif op == 'round_sig':
        n = rng.choice([1, 2, 3, 7, 20])
        r = decimal.Context(prec=n, rounding=MODES[m], Emax=10 ** 9, Emin=-10 ** 9).plus(a)
        if len(a.as_tuple().digits) <= n:
            r = a  # (plus would not change the digits, but drops a -0's sign)
    else:
        if rng.random() < 0.2:
            b = a.normalize() if a != 0 else a
        r = a.compare(b)
    lines.append('%s %s %s %d %d %s' % (op, plain(a), plain(b), n, m, plain(r)))

out = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'std', 'decimal', 'testdata', 'cases.txt')
with open(out, 'w') as f:
    f.write('\n'.join(lines) + '\n')
print(out, len(lines))
