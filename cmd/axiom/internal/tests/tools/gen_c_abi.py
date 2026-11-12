#!/usr/bin/env python3
"""Writes random tests of the C calling conventions: structures and unions of
random shapes (nested, arrays, bit-fields, floating and integer members mixed,
sizes from 1 to about 64 bytes, over-aligned members) passed and returned by
value, directly, through function pointers and as variadic arguments (read
with va_arg, va_copy and a va_list passed on), with the callers in one
translation unit and the callees in another:

    gen_c_abi.py SEED COUNT DIR [--prefix=NAME] [--no-align] [--ld] [--atomic] [--host]

writes DIR/NAME.h (the types and the functions that fill and check them),
DIR/NAME_callee.c (the functions called) and DIR/NAME.c (main, the calls).
NAME is abi_SEED by default. Every value is a function of a salt and the
position of the scalar it is in, so each side checks what it receives
without the other's help: the program prints the number of failures (0)
and the number of calls made, after a line for each failure.

Each unit may be built by a different compiler, or at a different
optimization level, to check that they agree on the calling convention
(differential testing; the units only need a function print_i64). With
--host the program is a test of the suite: DIR/NAME.c prints through
host.h, names its second unit (/* units: */) and carries its expected
output, and the other two files go in DIR/abi/. The suite's tests are

    gen_c_abi.py 101 8 cmd/axiom/internal/tests/c/run --host
    gen_c_abi.py 102 8 cmd/axiom/internal/tests/c/run --host
    gen_c_abi.py 103 8 cmd/axiom/internal/tests/c/run --host --no-align

With --atomic they also have _Atomic members (scalars, and records set
and read whole). With --ld the shapes also have long double and long double _Complex members
and arguments (C99; <complex.h>), with values that double cannot hold.

The shapes stay within what the C standard defines and every target
implements alike: bit-fields are of int, unsigned int and _Bool, at most
as wide as their type; alignment specifiers are powers of two up to 64
(none with --no-align); there are no flexible array members, no long
double and no zero-size objects.
"""
import os
import random
import sys

# name: (spelling, kind, bits, signed); kind: i integer, f floating, b _Bool, p pointer
SCALARS = {
    'c': ('signed char', 'i', 8, True),
    'uc': ('unsigned char', 'i', 8, False),
    's': ('short', 'i', 16, True),
    'us': ('unsigned short', 'i', 16, False),
    'i': ('int', 'i', 32, True),
    'u': ('unsigned', 'i', 32, False),
    'll': ('long long', 'i', 64, True),
    'ull': ('unsigned long long', 'i', 64, False),
    'f': ('float', 'f', 32, True),
    'd': ('double', 'f', 64, True),
    'b': ('_Bool', 'b', 1, False),
    'p': ('char *', 'p', 0, False),
}
# with --ld, also long double and its complex type (kind c: complex)
LONG_DOUBLES = {
    'ld': ('long double', 'f', 128, True),
    'cld': ('long double _Complex', 'c', 256, True),
}
ALL = {**SCALARS, **LONG_DOUBLES}
# the promoted type of a variadic argument
PROMOTE = {'c': 'i', 'uc': 'i', 's': 'i', 'us': 'i', 'b': 'i', 'f': 'd'}


class Scalar:
    def __init__(self, k):
        self.k = k
        self.spell = ALL[k][0]

    def decl(self, name):
        return f'{self.spell} {name}'


class Atomic:
    """_Atomic(t) (--atomic): a scalar or a record."""
    def __init__(self, t):
        self.t = t
        self.spell = f'_Atomic({t.spell})'

    def decl(self, name):
        return f'{self.spell} {name}'


class Array:
    def __init__(self, elem, n):
        self.elem = elem
        self.n = n

    def decl(self, name):
        return self.elem.decl(f'{name}[{self.n}]')


class Record:
    def __init__(self, name, union):
        self.name = name
        self.union = union
        self.members = []  # (name, type, bit-width or None, alignment or None)
        self.spell = ('union ' if union else 'struct ') + name

    def decl(self, name):
        return f'{self.spell} {name}'


class Gen:
    def __init__(self, seed, align, ld=False, atomic=False):
        self.r = random.Random(seed)
        self.align = align
        self.atomic = atomic
        self.kinds = list(SCALARS) + (list(LONG_DOUBLES) if ld else [])
        self.records = []

    def scalar(self):
        return Scalar(self.r.choice(self.kinds))

    def member_type(self, depth):
        if self.atomic and self.r.random() < 0.2:
            # (an atomic scalar, or an atomic record of the sizes the
            # platforms align to their size, and others)
            if depth < 2 and self.r.random() < 0.3:
                return Atomic(self.record(depth + 1, False))
            return Atomic(self.scalar())
        x = self.r.random()
        if x < 0.55 or depth >= 2:
            return self.scalar()
        if x < 0.75:
            return Array(self.scalar() if self.r.random() < 0.8 or depth >= 1 else self.record(depth + 1),
                         self.r.randint(1, 5))
        return self.record(depth + 1)

    def record(self, depth=0, union=None):
        if union is None:
            union = self.r.random() < 0.12
        rec = Record('', union)
        if depth == 0 and not union and self.r.random() < 0.25:
            return self.homogeneous()
        n = self.r.choice([1, 1, 2, 2, 3, 3, 4, 5, 6, 8])
        for j in range(n):
            name = f'm{j}'
            if not union and self.r.random() < 0.15:
                # a run of bit-fields, perhaps with an unnamed or zero-width one
                for q in range(self.r.randint(1, 4)):
                    t = self.r.choice(['i', 'u', 'b', 'u', 'i'])
                    if t == 'b':
                        w = 1
                    else:
                        w = self.r.randint(1, 31 if t == 'i' else 32)
                    if self.r.random() < 0.1:
                        rec.members.append((None, Scalar(self.r.choice(['i', 'u'])), self.r.choice([0, 3]), None))
                    rec.members.append((f'{name}_{q}', Scalar(t), w, None))
                continue
            al = None
            mt = self.member_type(depth)
            if self.align and self.r.random() < 0.06:
                al = self.r.choice([8, 16, 16, 32, 64])
                if al < max_align(mt):
                    al = None  # (an alignment specifier cannot weaken one)
            rec.members.append((name, mt, None, al))
        # (named once its members are: those that are records come first)
        return self.name(rec)

    def homogeneous(self):
        # members of one floating type (AArch64's homogeneous aggregates,
        # x86-64's SSE eightbytes), as scalars, arrays and nested records
        k = self.r.choice(['f', 'd'] + (['ld'] if 'ld' in self.kinds else []))
        rec = Record('', False)
        total = self.r.randint(1, 5)
        j = 0
        while total > 0:
            x = self.r.random()
            if x < 0.6 or total == 1:
                rec.members.append((f'm{j}', Scalar(k), None, None))
                total -= 1
            elif x < 0.8:
                n = self.r.randint(1, total)
                rec.members.append((f'm{j}', Array(Scalar(k), n), None, None))
                total -= n
            else:
                inner = Record('', False)
                n = self.r.randint(1, total)
                for q in range(n):
                    inner.members.append((f'm{q}', Scalar(k), None, None))
                self.name(inner)
                rec.members.append((f'm{j}', inner, None, None))
                total -= n
            j += 1
        return self.name(rec)

    def name(self, rec):
        rec.name = f'R{len(self.records)}'
        rec.spell = ('union ' if rec.union else 'struct ') + rec.name
        self.records.append(rec)
        return rec

    def arg_type(self):
        if self.r.random() < 0.3:
            return self.scalar()
        # (records of up to about 64 bytes, over-aligned ones aside)
        while True:
            n = len(self.records)
            rec = self.record()
            if max_size(rec) <= 64 or any(al for _, _, _, al in rec.members):
                return rec
            del self.records[n:]


def max_align(t):
    """The largest alignment t can have on any target."""
    if isinstance(t, Atomic):
        n = 1
        while n < max_size(t.t):
            n *= 2
        return max(max_align(t.t), min(n, 16))
    if isinstance(t, Scalar):
        return 8 if t.k == 'p' else min(16, max(1, ALL[t.k][2] // 8))
    if isinstance(t, Array):
        return max_align(t.elem)
    return max([max_align(mt) if w is None else 4 for _, mt, w, _ in t.members] +
               [al or 1 for _, _, _, al in t.members])


def max_size(t):
    """An upper bound of t's size on any target (bit-fields taking their
    type's size each)."""
    if isinstance(t, Atomic):
        return max_size(t.t) + 16
    if isinstance(t, Scalar):
        return 8 if t.k == 'p' else max(1, ALL[t.k][2] // 8)
    if isinstance(t, Array):
        return t.n * max_size(t.elem)
    sizes = [max_size(mt) + (al or 0) for _, mt, _, al in t.members]
    return max(sizes) if t.union else sum(sizes) + max_align(t)


def leaves(t, path):
    """The (lvalue, scalar kind, bit-width) of each scalar a fill sets: of a
    union, its first member's."""
    if isinstance(t, Scalar):
        return [(path, t.k, None)]
    if isinstance(t, Atomic):
        if isinstance(t.t, Scalar):
            return [(path, t.t.k, None)]
        return [(path, t.t, None)]  # (an atomic record: set and read whole)
    if isinstance(t, Array):
        out = []
        for k in range(t.n):
            out += leaves(t.elem, f'{path}[{k}]')
        return out
    out = []
    for name, mt, w, _ in t.members:
        if name is None:
            continue
        if w is not None:
            out.append((f'{path}.{name}', mt.k, w))
        else:
            out += leaves(mt, f'{path}.{name}')
        if t.union:
            break
    return out


def value(k, w, s, j):
    """The C expression of the value of a scalar of kind k (bit-field width w)
    for salt s (an expression) at position j."""
    m = f'abi_mix({s}, {j}u)'
    sp, kind, bits, signed = ALL[k]
    if kind == 'b' or w == 1 and not signed:
        return f'({sp})({m} & 1u)'
    if k == 'ld':
        # (with bits that double has not: 2^-50 beside values up to 250)
        return f'((long double)(long long)({m} % 4001u) / 8.0L - 250.0L + (long double)({m} % 7u) * 0x1p-50L)'
    if kind == 'c':
        re = value('ld', None, s, j)
        im = value('ld', None, s, f'{j}u + 1000')
        return f'(long double _Complex)({re} + {im} * I)'
    if kind == 'f':
        return f'({sp})((double)(long long)({m} % 4001u) / 8.0 - 250.0)'
    if kind == 'p':
        return f'(abi_anchor + ({m} % 8u))'
    if w is not None:
        bits = w
    if bits == 64:
        return f'({sp}){m}'
    if signed:
        # (in range: no implementation-defined conversion)
        half = 1 << (bits - 1)
        return f'({sp})((long long)({m} % {2 * half}u) - {half})'
    return f'({sp})({m} % {1 << bits}u)'


def fill_check(rec):
    ls = leaves(rec, '(*p)')
    f = [f'static void fill_{rec.name}({rec.spell} *p, unsigned s) {{']
    if not ls:
        f.append('    (void)p; (void)s;')
    f.append('    abi_zero(p, sizeof *p);')
    c = [f'static int check_{rec.name}(const {rec.spell} *p, unsigned s) {{', '    int bad = 0;']
    if not ls:
        c.append('    (void)p; (void)s;')
    for j, (lv, k, w) in enumerate(ls):
        if isinstance(k, Record):
            f.append(f'    {{ {k.spell} t; fill_{k.name}(&t, s + {j}u); {lv} = t; }}')
            c.append(f'    {{ {k.spell} t = {lv}; bad |= !check_{k.name}(&t, s + {j}u); }}')
            continue
        f.append(f'    {lv} = {value(k, w, "s", j)};')
        c.append(f'    bad |= {lv} != {value(k, w, "s", j)};')
    f.append('}')
    c.append('    return !bad;')
    c.append('}')
    return f + c


def decl_of(t, name):
    return t.decl(name)


def check_expr(t, name, s):
    if isinstance(t, Scalar):
        return f'{name} == {value(t.k, None, s, 0)}'
    return f'check_{t.name}(&{name}, {s})'


def fill_stmt(t, name, s, indent):
    if isinstance(t, Scalar):
        return f'{indent}{t.decl(name)} = {value(t.k, None, s, 0)};'
    return f'{indent}{t.decl(name)}; fill_{t.name}(&{name}, {s});'


def promoted(t):
    if isinstance(t, Scalar) and t.k in PROMOTE:
        return Scalar(PROMOTE[t.k])
    return t


class Test:
    pass


def main():
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    opts = [a for a in sys.argv[1:] if a.startswith('--')]
    if len(args) != 3:
        sys.exit(__doc__)
    seed, count, out = int(args[0]), int(args[1]), args[2]
    prefix = f'abi_gen_{seed}' if '--host' in opts else f'abi_{seed}'
    for o in opts:
        if o.startswith('--prefix='):
            prefix = o[len('--prefix='):]
    ld = '--ld' in opts
    g = Gen(seed, '--no-align' not in opts, ld, '--atomic' in opts)
    host = '--host' in opts
    tests = []
    for k in range(count):
        t = Test()
        t.k = k
        r = g.r
        t.ret = None if r.random() < 0.15 else g.arg_type()
        # scalars first that use up registers, sometimes
        t.params = []
        if r.random() < 0.3:
            for q in range(r.randint(1, 8)):
                t.params.append(Scalar(r.choice(['i', 'd', 'll', 'f'] + (['ld'] if ld else []))))
        for q in range(r.choice([0, 1, 2, 3, 4, 5, 8, 10])):
            t.params.append(g.arg_type())
        t.variadic = r.random() < 0.4
        t.vargs = []
        if t.variadic:
            if not t.params:
                t.params.append(Scalar('i'))
            for q in range(r.randint(1, 7)):
                t.vargs.append(promoted(g.arg_type()))
            t.copy_at = r.randint(0, len(t.vargs) - 1)
        tests.append(t)

    h = [f'/* {prefix}.h - written by gen_c_abi.py {seed} {count}: the types of the',
         f'   tests in {prefix}.c and {prefix}_callee.c, and the functions that fill',
         '   and check them. */',
         f'#ifndef {prefix.upper()}_H', f'#define {prefix.upper()}_H', '#include <stdarg.h>'] + \
        (['#include <complex.h>'] if ld else []) + ['',
         'extern char abi_anchor[8];', 'void abi_fail(int test, int what);', '',
         'static unsigned long long abi_mix(unsigned s, unsigned j) {',
         '    unsigned long long x = s * 0x9E3779B97F4A7C15ull + (j + 1u) * 0xBF58476D1CE4E5B9ull;',
         '    x ^= x >> 29;', '    x *= 0x94D049BB133111EBull;', '    return x ^ (x >> 32);', '}', '',
         'static void abi_zero(void *p, unsigned long n) {',
         '    unsigned char *q = (unsigned char *)p;', '    while (n--) *q++ = 0;', '}', '']
    for rec in g.records:
        h.append(f'{rec.spell} {{')
        for name, mt, w, al in rec.members:
            a = f'_Alignas({al}) ' if al else ''
            if w is not None:
                h.append(f'    {mt.spell} {name or ""} : {w};'.replace('  :', ' :'))
            else:
                h.append(f'    {a}{mt.decl(name)};')
        h.append('};')
    h.append('')
    for rec in g.records:
        h += fill_check(rec)
    h.append('')

    def proto(t, name):
        ps = [decl_of(p, f'a{q}') for q, p in enumerate(t.params)]
        if t.variadic:
            ps.append('...')
        rs = 'void' if t.ret is None else (t.ret.spell if not isinstance(t.ret, Scalar) else t.ret.spell)
        return f'{rs} {name}({", ".join(ps) or "void"})'

    for t in tests:
        h.append(proto(t, f'abi_f{t.k}') + ';')
    h.append('#endif')

    ce = [f'/* {prefix}_callee.c - written by gen_c_abi.py {seed} {count}: the functions',
          f'   that {prefix}.c calls, which check their arguments. */', f'#include "{prefix}.h"', '',
          'char abi_anchor[8];', 'int abi_failures;', 'int abi_first_test = -1, abi_first_what;', '',
          'void abi_fail(int test, int what) {',
          '    if (abi_failures++ == 0) {', '        abi_first_test = test;', '        abi_first_what = what;',
          '    }', '}', '']
    for t in tests:
        k = t.k
        if t.variadic:
            # the variadic arguments from a va_list passed by value
            ce.append(f'static void abi_v{k}(va_list ap, int from) {{')
            for q, vt in enumerate(t.vargs):
                ce.append(f'    if (from <= {q}) {{')
                ce.append(f'        {decl_of(vt, "v")} = va_arg(ap, {vt.spell});')
                ce.append(f'        if (!({check_expr(vt, "v", f"{k * 100 + 50 + q}u")})) abi_fail({k}, {200 + q});')
                ce.append('    }')
            ce.append('}')
        ce.append(proto(t, f'abi_f{k}') + ' {')
        for q, p in enumerate(t.params):
            ce.append(f'    if (!({check_expr(p, f"a{q}", f"{k * 100 + q}u")})) abi_fail({k}, {q});')
        if t.variadic:
            ce.append('    va_list ap, aq, ar;')
            ce.append(f'    va_start(ap, a{len(t.params) - 1});')
            for q, vt in enumerate(t.vargs):
                if q == t.copy_at:
                    ce.append('    va_copy(aq, ap);')
                ce.append('    {')
                ce.append(f'        {decl_of(vt, "v")} = va_arg(ap, {vt.spell});')
                ce.append(f'        if (!({check_expr(vt, "v", f"{k * 100 + 50 + q}u")})) abi_fail({k}, {100 + q});')
                ce.append('    }')
            ce.append('    va_end(ap);')
            ce.append('    va_copy(ar, aq);')
            ce.append(f'    abi_v{k}(aq, {t.copy_at});')
            ce.append('    va_end(aq);')
            # (a copy of a copy, read again from the same place)
            ce.append(f'    abi_v{k}(ar, {t.copy_at});')
            ce.append('    va_end(ar);')
        if t.ret is not None:
            ce.append(fill_stmt(t.ret, 'r', f'{k * 100 + 99}u', '    '))
            ce.append('    return r;')
        ce.append('}')
        ce.append('')
    ce.append('int abi_result(void) {')
    ce.append('    return abi_failures;')
    ce.append('}')
    ce.append('int abi_first(void) {')
    ce.append('    return abi_first_test * 1000 + abi_first_what;')
    ce.append('}')
    ce.append('unsigned long long abi_layout(int k) {')
    ce.append('    switch (k) {')
    for n, rec in enumerate(g.records):
        ce.append(f'    case {n}: return sizeof({rec.spell}) * 1000 + _Alignof({rec.spell});')
    ce.append('    }')
    ce.append('    return 0;')
    ce.append('}')

    ma = [f'/* {prefix}.c - written by gen_c_abi.py {seed} {count}: calls of the',
          f'   functions of {prefix}_callee.c, directly, through pointers and with',
          '   variadic arguments; prints the failures and the calls made. */']
    if host:
        ma.append(f'/* units: abi/{prefix}_callee.c */')
        ma.append('#include "host.h"')
        ma.append(f'#include "abi/{prefix}.h"')
    else:
        ma.append('void print_i64(long long x);')
        ma.append(f'#include "{prefix}.h"')
    ma += ['', 'int abi_result(void);', 'int abi_first(void);',
           'unsigned long long abi_layout(int k);', '']
    for t in tests:
        pt = proto(t, f'(*volatile abi_p{t.k})')
        ma.append(f'static {pt} = abi_f{t.k};')
    ma.append('')
    for t in tests:
        k = t.k
        ma.append(f'static int abi_t{k}(void) {{')
        for q, p in enumerate(t.params):
            ma.append(fill_stmt(p, f'a{q}', f'{k * 100 + q}u', '    '))
        for q, vt in enumerate(t.vargs):
            ma.append(fill_stmt(vt, f'v{q}', f'{k * 100 + 50 + q}u', '    '))
        call_args = ', '.join([f'a{q}' for q in range(len(t.params))] + [f'v{q}' for q in range(len(t.vargs))])
        ma.append('    int bad = 0;')
        for via in (f'abi_f{k}', f'abi_p{k}'):
            if t.ret is None:
                ma.append(f'    {via}({call_args});')
            else:
                ma.append('    {')
                ma.append(f'        {decl_of(t.ret, "r")} = {via}({call_args});')
                ma.append(f'        if (!({check_expr(t.ret, "r", f"{k * 100 + 99}u")})) bad = 1;')
                ma.append('    }')
        ma.append(f'    if (bad) abi_fail({k}, 99);')
        ma.append('    return 2;')
        ma.append('}')
        ma.append('')
    ma.append('int main(void) {')
    ma.append('    int calls = 0;')
    for n, rec in enumerate(g.records):
        ma.append(f'    if (abi_layout({n}) != sizeof({rec.spell}) * 1000 + _Alignof({rec.spell})) abi_fail(-1, {n});')
    for t in tests:
        ma.append(f'    calls += abi_t{t.k}();')
    ma.append('    if (abi_result() != 0) print_i64(abi_first());')
    ma.append('    print_i64(abi_result());')
    ma.append('    print_i64(calls);')
    ma.append('    return 0;')
    ma.append('}')
    if host:
        ma.append('/* expect: 0 */')
        ma.append(f'/* expect: {2 * count} */')

    sub = os.path.join(out, 'abi') if host else out
    os.makedirs(sub, exist_ok=True)
    for d, name, lines in ((sub, f'{prefix}.h', h), (sub, f'{prefix}_callee.c', ce), (out, f'{prefix}.c', ma)):
        with open(os.path.join(d, name), 'w') as fh:
            fh.write('\n'.join(lines) + '\n')


if __name__ == '__main__':
    main()
