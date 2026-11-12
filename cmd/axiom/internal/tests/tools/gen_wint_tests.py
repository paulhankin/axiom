#!/usr/bin/env python3
"""Writes the tests of C's integers wider than 64 bits (__int128 and
unsigned __int128 of the GNU dialects, C23's _BitInt(N) for N > 64), whose
expected values Python's integers compute:

    cmd/axiom/internal/tests/tools/gen_wint_tests.py

writes cmd/axiom/internal/tests/c/run/wint_gen_<type>.c, one file for each
of a list of types (two limbs, three, four and more; widths that fill their
limbs and widths that do not). Each file has a table of operand pairs (the
boundary values of the type, values near powers of two, and pseudo-random
values of every magnitude) and, for each operation (+ - * / % & | ^ << >>,
the comparisons, unary - ~ !, division and remainder by constants, the
conversions to and from the narrower integers, double, float and long
double, and the compound assignments, ++ and --), computes the results
twice: at run time, from operands read from volatile objects (which no
optimization sees through), and as constant expressions that the compiler
folds (a static object's initializer). It prints for each operation a hash
of the results of each kind (FNV-1a over their 64-bit words), which must
both be the hash of the values Python computes (written in the test's
expect lines). C's undefined operations are left out: signed + - * and <<
that overflow (also done in the unsigned type of the width, where they
wrap), division by zero, the least value divided by -1, shift counts out of
range, conversions of floating values out of the type's range, of integers
out of float's.
"""
import os
import random
import struct

here = os.path.dirname(os.path.abspath(__file__))
out_dir = os.path.join(here, '..', 'c', 'run')

TYPES = [  # file tag, bits, signed, C type, standard, pairs
    ('u128', 128, False, 'unsigned __int128', 'gnu23', 44),
    ('i128', 128, True, '__int128', 'gnu23', 44),
    ('b65', 65, True, '_BitInt(65)', 'c23', 36),
    ('ub100', 100, False, 'unsigned _BitInt(100)', 'c23', 32),
    ('ub128', 128, False, 'unsigned _BitInt(128)', 'c23', 30),
    ('ub129', 129, False, 'unsigned _BitInt(129)', 'c23', 30),
    ('b192', 192, True, '_BitInt(192)', 'c23', 32),
    ('ub256', 256, False, 'unsigned _BitInt(256)', 'c23', 28),
    ('b521', 521, True, '_BitInt(521)', 'c23', 28),
]

DIVISORS = [3, 10, 641, 1000000007, 2**32 + 15, 2**61 - 1]

M64 = (1 << 64) - 1
FNV = 0xcbf29ce484222325


def fnv(h, w):
    return ((h ^ (w & M64)) * 0x100000001b3) & M64


def wrap(v, bits, signed):
    v &= (1 << bits) - 1
    if signed and v >> (bits - 1):
        v -= 1 << bits
    return v


def values(bits, signed, rng):
    lo = -(1 << (bits - 1)) if signed else 0
    hi = (1 << (bits - 1)) - 1 if signed else (1 << bits) - 1
    vs = {0, 1, 2, 3, 10, hi, hi - 1, lo, lo + 1, 2**63, 2**64 - 1, 2**64, 2**64 + 1, 2**63 - 1, 2**53 + 1}
    for k in range(32, bits, 96):
        vs.update([1 << k, (1 << k) - 1])
    if signed:
        vs.update([-1, -2, -10, -(2**63), -(2**64), -(2**64) - 1])
    for _ in range(6):
        n = rng.randrange(1, bits + 1)
        v = rng.getrandbits(n)
        if signed and rng.random() < 0.5:
            v = -v
        vs.add(v)
    return sorted(wrap(v, bits, signed) for v in vs)


def f32_bits(v):
    """The float nearest to integer v (ties to even), as bits; None out of
    float's range."""
    if v == 0:
        return 0
    neg = v < 0
    m = -v if neg else v
    e = m.bit_length() - 1
    sh = e - 23
    q = m << -sh if sh <= 0 else m >> sh
    if sh > 0:
        r = m & ((1 << sh) - 1)
        half = 1 << (sh - 1)
        if r > half or (r == half and q & 1):
            q += 1
        if q >> 24:
            q >>= 1
            e += 1
    if e > 127:
        return None
    return (1 << 31 if neg else 0) | ((e + 127) << 23) | (q & 0x7FFFFF)


def f64_bits(v):
    try:
        return struct.unpack('<Q', struct.pack('<d', float(v)))[0]
    except OverflowError:
        return None


def gen(t, rng):
    tag, bits, signed, ctype, std, npairs = t
    if signed:
        utype = 'unsigned __int128' if ctype == '__int128' else 'unsigned ' + ctype
    else:
        utype = ctype
    lo = -(1 << (bits - 1)) if signed else 0
    hi = (1 << (bits - 1)) - 1 if signed else (1 << bits) - 1
    words = (bits + 63) // 64
    vs = values(bits, signed, rng)
    # (each value as the first operand once, the second a random one; and more)
    pairs = [(a, rng.choice(vs)) for a in vs]
    while len(pairs) < npairs:
        pairs.append((rng.choice(vs), rng.choice(vs)))

    def count(b):
        return (b & 0xFFFF) % bits

    ops = []  # name, Python function of (a, b) (None: undefined), kind of result, C of (A, B, K)

    def op(name, f, kind, c):
        ops.append((name, f, kind, c))

    def bin_(o):
        return lambda A, B, K: '(%s %s %s)' % (A, o, B)

    def ubin(o):
        return lambda A, B, K: '((U)%s %s (U)%s)' % (A, o, B)

    def checked(f):
        def g(a, b):
            r = f(a, b)
            return r if lo <= r <= hi else None
        return g
    for name, o, f in [('add', '+', lambda a, b: a + b), ('sub', '-', lambda a, b: a - b),
                       ('mul', '*', lambda a, b: a * b)]:
        op(name + '_u', lambda a, b, f=f: f(a, b), 'U', ubin(o))
        if signed:
            op(name, checked(f), 'T', bin_(o))

    def tdiv(a, b):
        if b == 0 or (signed and a == lo and b == -1):
            return None
        q = abs(a) // abs(b)
        return q if (a < 0) == (b < 0) else -q

    def trem(a, b):
        q = tdiv(a, b)
        return None if q is None else a - b * q
    op('div', tdiv, 'T', bin_('/'))
    op('rem', trem, 'T', bin_('%'))
    op('and', lambda a, b: a & b, 'T', bin_('&'))
    op('or', lambda a, b: a | b, 'T', bin_('|'))
    op('xor', lambda a, b: a ^ b, 'T', bin_('^'))
    for name, o, f in [('lt', '<', lambda a, b: a < b), ('le', '<=', lambda a, b: a <= b),
                       ('gt', '>', lambda a, b: a > b), ('ge', '>=', lambda a, b: a >= b),
                       ('eq', '==', lambda a, b: a == b), ('ne', '!=', lambda a, b: a != b)]:
        op(name, lambda a, b, f=f: int(f(a, b)), 'w', bin_(o))

    def shl(a, b):
        k = count(b)
        return None if signed and (a < 0 or (a << k) > hi) else a << k
    op('shl', shl, 'T', lambda A, B, K: '(%s << %s)' % (A, K))
    op('shr', lambda a, b: a >> count(b), 'T', lambda A, B, K: '(%s >> %s)' % (A, K))
    op('shl_u', lambda a, b: a << count(b), 'U', lambda A, B, K: '((U)%s << %s)' % (A, K))
    op('neg', lambda a, b: None if signed and a == lo else -a, 'T', lambda A, B, K: '(-%s)' % A)
    op('not', lambda a, b: ~a, 'T', lambda A, B, K: '(~%s)' % A)
    op('lnot', lambda a, b: int(a == 0), 'w', lambda A, B, K: '(!%s)' % A)
    for d in DIVISORS:
        if d <= hi:
            op('div%d' % d, lambda a, b, d=d: tdiv(a, d), 'T', lambda A, B, K, d=d: '(%s / %d)' % (A, d))
            op('rem%d' % d, lambda a, b, d=d: trem(a, d), 'T', lambda A, B, K, d=d: '(%s %% %d)' % (A, d))
    op('to_ll', lambda a, b: a, 'w', lambda A, B, K: '(unsigned long long)(long long)%s' % A)
    op('to_u32', lambda a, b: a & 0xFFFFFFFF, 'w', lambda A, B, K: '(unsigned long long)(unsigned)%s' % A)
    op('to_bool', lambda a, b: int(a != 0), 'w', lambda A, B, K: '(unsigned long long)(_Bool)%s' % A)
    op('from_ll', lambda a, b: wrap(a, 64, True), 'T', lambda A, B, K: '((T)(long long)%s)' % A)
    op('from_ull', lambda a, b: a & M64, 'T', lambda A, B, K: '((T)(unsigned long long)%s)' % A)
    op('from_s', lambda a, b: wrap(a, 16, True), 'T', lambda A, B, K: '((T)(short)%s)' % A)
    op('to_f64', lambda a, b: f64_bits(a), 'd', lambda A, B, K: '((double)%s)' % A)
    op('to_f32', lambda a, b: f32_bits(a), 'f', lambda A, B, K: '((float)%s)' % A)

    def exact53(a, b):
        m = abs(a)
        k = max(0, m.bit_length() - 53)
        return a if (m >> k) << k == m else None
    op('via_f64', exact53, 'T', lambda A, B, K: '((T)(double)%s)' % A)
    op('via_f32', lambda a, b: a if abs(a) < (1 << 24) else None, 'T', lambda A, B, K: '((T)(float)%s)' % A)
    op('via_ld', exact53, 'T', lambda A, B, K: '((T)(long double)%s)' % A)

    def trunc(a, b):
        # ((double)a / 4 + 0.75) truncated toward zero
        fb = f64_bits(a)
        if fb is None:
            return None
        x = float(a) / 4 + 0.75
        v = int(x)
        return v if lo <= v <= hi and abs(x) < 2.0 ** 1000 else None
    op('f64_trunc', trunc, 'T', lambda A, B, K: '((T)((double)%s / 4 + 0.75))' % A)

    def hash_of(kind, vals):
        h = FNV
        for v in vals:
            if kind in 'wdf':
                h = fnv(h, v)
            else:
                u = v & ((1 << bits) - 1)
                for k in range(words):
                    h = fnv(h, u >> (64 * k))
        return h

    out = []
    w = out.append
    exp = []
    body = []
    for name, f, kind, c in ops:
        idx = [i for i, (a, b) in enumerate(pairs) if f(a, b) is not None]
        if not idx:
            continue
        h = hash_of(kind, [f(*pairs[i]) for i in idx])
        exp.append((name, h))
        rtype = {'T': 'T', 'U': 'U', 'w': 'unsigned long long', 'd': 'double', 'f': 'float'}[kind]
        body.append('static const short ix_%s[] = {%s};' % (name, ', '.join(map(str, idx))))
        body.append('static const %s cf_%s[] = {%s};' % (
            rtype, name, ', '.join(c('A%d' % i, 'B%d' % i, str(count(pairs[i][1]))) for i in idx)))
        hf = {'w': 'hw', 'd': 'hd', 'f': 'hf'}.get(kind, 'hv')
        body.append('RUN(%s, %s, %s, %s)' % (name, rtype, c('a', 'b', 'k'), hf))
        body.append('')

    # the compound assignments, ++ and -- (unsigned: they wrap)
    def comp(a, b):
        k = count(b)
        m = (1 << bits) - 1
        x = (a + b) & m
        x = (x * (b | 1)) & m
        x = (x - a) & m
        x = (x << k) & m
        x = x ^ b
        x = (x + 1) & m
        x = (x - 2) & m
        return x
    exp.append(('compound', hash_of('U', [comp(a, b) for a, b in pairs])))
    body.append('static void t_compound(void) {')
    body.append('    unsigned long long h = FNV0;')
    body.append('    for (int i = 0; i < NP; i++) {')
    body.append('        U x = (U)va[i], b = (U)vb[i];')
    body.append('        x += b;')
    body.append('        x *= b | 1;')
    body.append('        x -= (U)va[i];')
    body.append('        x <<= vk[i];')
    body.append('        x ^= b;')
    body.append('        U y = x++;')
    body.append('        if (y + 1 != x) h++;')
    body.append('        --x;')
    body.append('        x--;')
    body.append('        h = hv(h, x);')
    body.append('    }')
    body.append('    report("compound", h, h);')
    body.append('}')
    body.append('')

    w('/* flags: --c-std=%s */' % std)
    for name, h in exp:
        w('/* expect: %s %s %s */' % (name, h, h))
    w('/* Generated by cmd/axiom/internal/tests/tools/gen_wint_tests.py: the operations of')
    w('   %s at run time (operands from volatile objects) and as constant expressions' % ctype)
    w('   (static initializers): for each, the hashes of the two kinds of results, which must')
    w('   both be those of the values Python computes. */')
    w('#include "host.h"')
    w('')
    w('typedef %s T;' % ctype)
    w('typedef %s U;' % utype)
    w('')
    w('#define FNV0 0x%xull' % FNV)
    w('#define NP %d' % len(pairs))
    w('')
    for i, (a, b) in enumerate(pairs):
        w('#define A%d ((T)0x%xuwb)' % (i, a & ((1 << bits) - 1)))
        w('#define B%d ((T)0x%xuwb)' % (i, b & ((1 << bits) - 1)))
    w('')
    w('static volatile T va[NP] = {%s};' % ', '.join('A%d' % i for i in range(len(pairs))))
    w('static volatile T vb[NP] = {%s};' % ', '.join('B%d' % i for i in range(len(pairs))))
    w('static volatile int vk[NP] = {%s};' % ', '.join(str(count(b)) for a, b in pairs))
    w('')
    w('static unsigned long long hw(unsigned long long h, unsigned long long x) {')
    w('    return (h ^ x) * 0x100000001b3ull;')
    w('}')
    w('')
    w('/* the value\'s %d word(s) */' % words)
    w('static unsigned long long hv(unsigned long long h, U x) {')
    w('    for (int k = 0; k < %d; k++) h = hw(h, (unsigned long long)(x >> (64 * k)));' % words)
    w('    return h;')
    w('}')
    w('')
    w('static unsigned long long DB(double d) {')
    w('    union { unsigned long long u; double d; } x;')
    w('    x.d = d;')
    w('    return x.u;')
    w('}')
    w('')
    w('static unsigned long long FB(float f) {')
    w('    union { unsigned u; float f; } x;')
    w('    x.f = f;')
    w('    return x.u;')
    w('}')
    w('')
    w('static unsigned long long hd(unsigned long long h, double x) {')
    w('    return hw(h, DB(x));')
    w('}')
    w('')
    w('static unsigned long long hf(unsigned long long h, float x) {')
    w('    return hw(h, FB(x));')
    w('}')
    w('')
    w('static void put_u(char *b, int *n, unsigned long long v) {')
    w('    char t[24];')
    w('    int k = 0;')
    w('    do { t[k++] = (char)(\'0\' + v % 10); v /= 10; } while (v);')
    w('    while (k) b[(*n)++] = t[--k];')
    w('}')
    w('')
    w('static void report(const char *name, unsigned long long h, unsigned long long hc) {')
    w('    char b[80];')
    w('    int n = 0;')
    w('    while (*name) b[n++] = *name++;')
    w('    b[n++] = \' \';')
    w('    put_u(b, &n, h);')
    w('    b[n++] = \' \';')
    w('    put_u(b, &n, hc);')
    w('    print_str(b, (size_t)n);')
    w('}')
    w('')
    w('/* the test of an operation: its results at run time and constant (cf_), hashed */')
    w('#define RUN(NAME, R, EXPR, H)                                                    \\')
    w('    static void t_##NAME(void) {                                                  \\')
    w('        unsigned long long h = FNV0, hc = FNV0;                                   \\')
    w('        for (int j = 0; j < (int)(sizeof ix_##NAME / sizeof ix_##NAME[0]); j++) { \\')
    w('            int i = ix_##NAME[j];                                                 \\')
    w('            T a = va[i], b = vb[i];                                               \\')
    w('            int k = vk[i];                                                        \\')
    w('            (void)a;                                                              \\')
    w('            (void)b;                                                              \\')
    w('            (void)k;                                                              \\')
    w('            R r = EXPR;                                                           \\')
    w('            h = H(h, r);                                                          \\')
    w('            hc = H(hc, cf_##NAME[j]);                                             \\')
    w('        }                                                                         \\')
    w('        report(#NAME, h, hc);                                                     \\')
    w('    }')
    w('')
    out.extend(body)
    w('int main(void) {')
    for name, h in exp:
        w('    t_%s();' % name)
    w('    return 0;')
    w('}')
    return '\n'.join(out) + '\n'


def main():
    rng = random.Random(128)
    for t in TYPES:
        text = gen(t, rng)
        path = os.path.join(out_dir, 'wint_gen_%s.c' % t[0])
        with open(path, 'w') as f:
            f.write(text)
        print('wrote', os.path.relpath(path))


if __name__ == '__main__':
    main()
