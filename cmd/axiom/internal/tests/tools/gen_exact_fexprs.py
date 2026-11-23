#!/usr/bin/env python3
"""Writes tests of random floating-point expressions whose expected values
Python computes exactly from the specification's rules (spec §6.1, §6.2,
§7.7, Appendix A), not from the compiler, with the rational model of IEEE
754 binary32 and binary64 in gen_exact_ops.py.

    python3 cmd/axiom/internal/tests/tools/gen_exact_fexprs.py
    cmd/axiom/build/axiom fmt cmd/axiom/internal/tests/run/exact_fexprs_*.ax

writes cmd/axiom/internal/tests/run/exact_fexprs_<n>.ax (the same files
every time: each has its own fixed seed). Each holds a few hundred
expression trees over variables of f32 and f64 and of six integer types,
mixing + - * / (each rounded once: no contraction of `a * b + c` into an
fma, no reassociation), @fma, @sqrt, @min @max (NaN, and -0 < +0), @abs,
@copysign, @floor @ceil @trunc @nearest, unary -, float literals (rounded
to their type from the decimal, ties to even), the comparisons as integers
(@as of a bool), the conversions between the floats (@as(f64, x),
@float_cast) and between floats and integers (@float_from_int, @as from the
integers a float holds exactly, @int_from_float: truncation, saturation,
NaN to 0), and the wrapping integer operators on the way. NaN results are
hashed as one NaN (payloads and their signs are unspecified, §7.7), so
@copysign takes its sign only from an operand that is not NaN.

The variables' values (a dozen sets: zeros, small integers and halves,
values that round, subnormals, the extremes, infinities, NaN, and
pseudo-random ones of every magnitude) come from memory, and the
expressions are grouped in functions of the variables, so nothing is
folded; the same expressions over constants (the first value set, which
is finite, declared `const`) may be evaluated by the compiler. Each output
line is a hash of the results of a group, over all value sets, or over the
constants.

    gen_exact_fexprs.py --seeds=<first>:<count> <dir>

writes more programs (exact_fexprs_s<seed>.ax, with other seeds) into
<dir>, for a longer search; copied into tests/run/, the suite runs them.
`--expand` with either form writes one output line per expression and value
set instead (with its text in a comment), to find the expression behind a
wrong hash.
"""
import math
import os
import random
import sys
from fractions import Fraction

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from gen_exact_ops import (FMT, INF, NAN, f_add, f_bits, f_div, f_fma, f_from_bits, f_minmax, f_mul, f_round,  # noqa: E402
                           f_sqrt, f_sub, float_cast, float_values, int_from_float, is_neg, mix, round_frac)

HERE = os.path.dirname(os.path.abspath(__file__))
RUN = os.path.join(HERE, '..', 'run')
M64 = (1 << 64) - 1

FT = ['f32', 'f64']
IT = {'u8': (8, False), 'i16': (16, True), 'i32': (32, True), 'u32': (32, False), 'i64': (64, True),
      'u64': (64, False)}
VARS = [('a0', 'f32'), ('a1', 'f32'), ('a2', 'f32'), ('b0', 'f64'), ('b1', 'f64'), ('b2', 'f64'), ('n0', 'u8'),
        ('n1', 'i16'), ('n2', 'i32'), ('n3', 'u32'), ('n4', 'i64'), ('n5', 'u64')]

PROGRAMS = [  # file, seed, expressions, value sets
    ('exact_fexprs_1.ax', 3003, 300, 12),
    ('exact_fexprs_2.ax', 4004, 300, 12),
]
GROUP = 12  # expressions per function

# float literals: exact ones, and decimals that round (to f32 from the
# decimal, not through f64)
LITERALS = ['0.0', '1.0', '2.0', '0.5', '0.25', '3.0', '10.0', '1.5', '100.0', '0.1', '0.2', '0.3', '1.1', '3.14159',
            '2.718281828459045', '1e10', '1e-7', '6.02e23', '1e-300', '16777217.0', '9007199254740993.0', '0.7']


def irange(t):
    bits, signed = IT[t]
    if signed:
        return -(1 << (bits - 1)), (1 << (bits - 1)) - 1
    return 0, (1 << bits) - 1


def iwrap(v, t):
    bits, signed = IT[t]
    v &= (1 << bits) - 1
    if signed and v >> (bits - 1):
        v -= 1 << bits
    return v


def as_float_ok(s, d):
    bits, signed = IT[s]
    return bits - (1 if signed else 0) <= FMT[d][0]


def lit_value(text, t):
    return round_frac(Fraction(text), t)


def float_text(x):
    """An Axiom literal of finite float x, exactly (the shortest decimal of
    the f64; it also names an f32 value exactly)."""
    if x == 0:
        return '-0.0' if is_neg(x) else '0.0'
    r = repr(x)
    if 'e' not in r and '.' not in r:
        r += '.0'
    if 'e' in r:
        m, e = r.split('e')
        if '.' not in m:
            m += '.0'
        r = m + 'e' + str(int(e))
    return r


def copysign(a, b):
    if math.isnan(b):
        return None  # the sign of a NaN is unspecified
    if math.isnan(a):
        return NAN
    return math.copysign(a, b)


def f_neg(a):
    return NAN if math.isnan(a) else -a


def f_abs(a):
    return NAN if math.isnan(a) else abs(a)


class Node:
    """An expression: its text (with `$name` placeholders for the
    variables), its type and its value in every set (None: unspecified)."""

    def __init__(self, parts, t, vals, leaf=False, lit=False):
        self.parts, self.t, self.vals, self.leaf, self.lit = parts, t, vals, leaf, lit

    def render(self, names):
        return ''.join(names[p[1:]] if p.startswith('$') else p for p in self.parts)


def cat(*items):
    out = []
    for it in items:
        if isinstance(it, Node):
            out += it.parts
        else:
            out.append(it)
    return out


class Gen:
    def __init__(self, seed, nsets):
        self.r = random.Random(seed)
        self.nsets = nsets
        self.sets = {}
        for v, t in VARS:
            vals = [self.value(t, i == 0) for i in range(nsets)]
            self.sets[v] = vals

    def fvalue(self, t, finite):
        r = self.r
        p, emin, emax = FMT[t]
        k = r.random()
        if k < 0.3:
            vs = float_values(t)
            x = f_from_bits(r.choice(vs), t)
        elif k < 0.6:
            x = r.randint(-40, 40) / r.choice([1, 1, 2, 4, 8, 3])
        elif k < 0.85:
            x = r.uniform(-1000, 1000)
        else:
            x = math.ldexp(r.uniform(1, 2), r.randint(emin - p + 1, emax)) * r.choice([1, -1])
        if math.isinf(x) or math.isnan(x):
            if finite:
                return self.fvalue(t, finite)
            return x
        x = round_frac(Fraction(x), t) if x != 0 else x
        if finite and math.isinf(x):
            return self.fvalue(t, finite)
        return x

    def ivalue(self, t):
        r = self.r
        lo, hi = irange(t)
        bits, signed = IT[t]
        k = r.random()
        if k < 0.35:
            e = r.randint(0, bits - 1)
            vs = [v for v in [0, 1, -1, lo, hi, 1 << e, (1 << e) - 1, -(1 << e), lo + 1, hi - 1, 1 << 24, (1 << 24) + 1,
                              (1 << 53) + 1, -(1 << 53) - 1] if lo <= v <= hi]
            return r.choice(vs)
        if k < 0.6:
            return r.randint(lo, hi)
        m = r.randint(1, bits)
        v = r.randint(0, (1 << m) - 1)
        if signed and r.random() < 0.5:
            v = -v
        return max(lo, min(hi, v))

    def value(self, t, finite):
        return self.fvalue(t, finite) if t in FT else self.ivalue(t)

    def var(self, t):
        v = self.r.choice([v for v, vt in VARS if vt == t])
        return Node(['$' + v], t, list(self.sets[v]), leaf=True)

    def literal(self, t):
        text = self.r.choice(LITERALS)
        neg = self.r.random() < 0.3
        x = lit_value(text, t)
        if neg:
            x = -x
            text = '(-' + text + ')'
        return Node([text], t, [x] * self.nsets, leaf=True, lit=True)

    def gen(self, t, depth):
        if depth <= 0 or self.r.random() < 0.15:
            return self.var(t)
        for _ in range(20):
            n = self.try_float(t, depth) if t in FT else self.try_int(t, depth)
            if n is not None and all(v is not None for v in n.vals):
                return n
        return self.var(t)

    def operand(self, t, depth):
        if t in FT and self.r.random() < 0.2:
            return self.literal(t)
        return self.gen(t, depth)

    def try_float(self, t, depth):
        r = self.r
        d = depth - 1
        kind = r.choice(['arith', 'arith', 'arith', 'arith', 'fma', 'sqrt', 'minmax', 'unary', 'unary', 'round',
                         'copysign', 'conv', 'conv', 'conv', 'mixed'])
        if kind == 'arith':
            op = r.choice(['+', '-', '*', '/'])
            f = {'+': f_add, '-': f_sub, '*': f_mul, '/': f_div}[op]
            a = self.gen(t, d)
            b = self.operand(t, d)
            if r.random() < 0.3:
                a, b = b, a
            if a.lit and b.lit:
                return None
            return Node(cat('(', a, f' {op} ', b, ')'), t, [f(x, y, t) for x, y in zip(a.vals, b.vals)])
        if kind == 'mixed':
            # a * b + c and the like, which must be rounded twice
            a, b, c = self.gen(t, d), self.operand(t, d), self.operand(t, d)
            op1 = r.choice(['*', '/'])
            op2 = r.choice(['+', '-'])
            f1 = f_mul if op1 == '*' else f_div
            f2 = f_add if op2 == '+' else f_sub
            if r.random() < 0.5:
                return Node(cat('(', a, f' {op1} ', b, f' {op2} ', c, ')'), t,
                            [f2(f1(x, y, t), z, t) for x, y, z in zip(a.vals, b.vals, c.vals)])
            return Node(cat('(', c, f' {op2} ', a, f' {op1} ', b, ')'), t,
                        [f2(z, f1(x, y, t), t) for x, y, z in zip(a.vals, b.vals, c.vals)])
        if kind == 'fma':
            a, b, c = self.gen(t, d), self.operand(t, d), self.operand(t, d)
            return Node(cat('@fma(', a, ', ', b, ', ', c, ')'), t,
                        [f_fma(x, y, z, t) for x, y, z in zip(a.vals, b.vals, c.vals)])
        if kind == 'sqrt':
            a = self.gen(t, d)
            if r.random() < 0.6:
                a = Node(cat('@abs(', a, ')'), t, [f_abs(x) for x in a.vals])
            return Node(cat('@sqrt(', a, ')'), t, [f_sqrt(x, t) for x in a.vals])
        if kind == 'minmax':
            name = r.choice(['min', 'max'])
            a, b = self.gen(t, d), self.operand(t, d)
            return Node(cat(f'@{name}(', a, ', ', b, ')'), t,
                        [f_minmax(x, y, name == 'max') for x, y in zip(a.vals, b.vals)])
        if kind == 'unary':
            a = self.gen(t, d)
            if r.random() < 0.5:
                return Node(cat('-', a), t, [f_neg(x) for x in a.vals]) if a.leaf else \
                    Node(cat('-(', a, ')'), t, [f_neg(x) for x in a.vals])
            return Node(cat('@abs(', a, ')'), t, [f_abs(x) for x in a.vals])
        if kind == 'round':
            mode = r.choice(['floor', 'ceil', 'trunc', 'nearest'])
            a = self.gen(t, d)
            return Node(cat(f'@{mode}(', a, ')'), t, [f_round(x, mode) for x in a.vals])
        if kind == 'copysign':
            a, b = self.gen(t, d), self.operand(t, d)
            return Node(cat('@copysign(', a, ', ', b, ')'), t, [copysign(x, y) for x, y in zip(a.vals, b.vals)])
        if kind == 'conv':
            forms = ['from_int', 'from_int']
            if t == 'f64':
                forms += ['widen', 'widen']
            else:
                forms += ['narrow', 'narrow']
            form = r.choice(forms)
            if form == 'widen':
                a = self.gen('f32', d)
                return Node(cat('@as(f64, ', a, ')'), t, list(a.vals))
            if form == 'narrow':
                a = self.gen('f64', d)
                return Node(cat('@float_cast(f32, ', a, ')'), t, [float_cast(x) for x in a.vals])
            s = r.choice(list(IT))
            a = self.gen(s, d)
            vals = [round_frac(Fraction(x), t) if x != 0 else 0.0 for x in a.vals]
            if as_float_ok(s, t) and r.random() < 0.5:
                return Node(cat(f'@as({t}, ', a, ')'), t, vals)
            return Node(cat(f'@float_from_int({t}, ', a, ')'), t, vals)
        raise AssertionError(kind)

    def try_int(self, t, depth):
        r = self.r
        d = depth - 1
        kind = r.choice(['from_float', 'from_float', 'from_float', 'cmp', 'cmp', 'wrap'])
        if kind == 'from_float':
            s = r.choice(FT)
            a = self.gen(s, d)
            if r.random() < 0.4:
                # into the range, so that the truncation, not the saturation, decides
                lo, hi = irange(t)
                m = min(hi, 1 << 20)
                a = Node(cat('@min(@max(', a, f', {lo if lo > -m else -m}.0), {m}.0)'), s,
                         [f_minmax(f_minmax(x, float(max(lo, -m)), True), float(m), False) for x in a.vals])
            return Node(cat(f'@int_from_float({t}, ', a, ')'), t, [int_from_float_t(x, t) for x in a.vals])
        if kind == 'cmp':
            s = r.choice(FT)
            op = r.choice(['==', '!=', '<', '<=', '>', '>='])
            f = {'==': lambda a, b: a == b, '!=': lambda a, b: a != b, '<': lambda a, b: a < b,
                 '<=': lambda a, b: a <= b, '>': lambda a, b: a > b, '>=': lambda a, b: a >= b}[op]
            a, b = self.gen(s, d), self.operand(s, d)
            return Node(cat(f'@as({t}, ', a, f' {op} ', b, ')'), t, [int(f(x, y)) for x, y in zip(a.vals, b.vals)])
        if kind == 'wrap':
            op = r.choice(['+%', '-%', '*%'])
            f = {'+%': lambda a, b: a + b, '-%': lambda a, b: a - b, '*%': lambda a, b: a * b}[op]
            a, b = self.gen(t, d), self.gen(t, d)
            return Node(cat('(', a, f' {op} ', b, ')'), t, [iwrap(f(x, y), t) for x, y in zip(a.vals, b.vals)])
        raise AssertionError(kind)


def int_from_float_t(x, t):
    if math.isnan(x):
        return 0
    lo, hi = irange(t)
    if math.isinf(x):
        return lo if x < 0 else hi
    return max(lo, min(hi, math.trunc(Fraction(x))))


def hash_of(v, t):
    if t in FT:
        if math.isnan(v):
            return 0x7fc00000 if t == 'f32' else 0x7ff8000000000000
        return f_bits(v, t)
    return v & M64


def hashers():
    out = ['fn mix(h: u64, x: u64) u64 {', '    let y = (h ^ x) *% 1099511628211;', '    return y ^ (y >> 32);', '}', '']
    for t, (bits, signed) in IT.items():
        conv = '@bit_cast(u64, @as(i64, r))' if signed and bits < 64 else (
            '@bit_cast(u64, r)' if signed else ('r' if bits == 64 else '@as(u64, r)'))
        out += [f'fn h_{t}(h: u64, r: {t}) u64 {{', f'    return mix(h, {conv});', '}', '']
    out += ['fn h_f32(h: u64, r: f32) u64 {', '    if r != r {', '        return mix(h, 0x7fc00000);', '    }',
            '    return mix(h, @as(u64, @bit_cast(u32, r)));', '}', '',
            'fn h_f64(h: u64, r: f64) u64 {', '    if r != r {', '        return mix(h, 0x7ff8000000000000);', '    }',
            '    return mix(h, @bit_cast(u64, r));', '}', '']
    return out


def gen_exprs(seed, nexpr, nsets):
    g = Gen(seed, nsets)
    exprs = []
    for _ in range(nexpr):
        t = g.r.choice(FT * 4 + list(IT))
        e = g.gen(t, g.r.randint(2, 5))
        while e.leaf:
            e = g.gen(t, g.r.randint(2, 5))
        exprs.append(e)
    return g, exprs


def write(fname, seed, nexpr, nsets, out_dir=RUN, expand=False):
    g, exprs = gen_exprs(seed, nexpr, nsets)
    run_names = {v: v for v, _ in VARS}
    const_names = {v: 'K' + v.upper() for v, _ in VARS}
    out = [
        '// Random floating-point expressions over f32, f64 and integer variables,',
        '// every operation evaluated by the rules of the spec (§6.2, §7.7, Appendix',
        '// A): IEEE 754 rounding to nearest, ties to even, of each operation (no',
        '// contraction, no reassociation). Each line is a hash of a group of',
        '// expressions over all value sets (from memory) or over constants (the',
        '// first set; the compiler may evaluate them). (Generated by',
        '// cmd/axiom/internal/tests/tools/gen_exact_fexprs.py, with the expectations',
        '// from exact arithmetic in Python.)',
        'import "../lib/io.ax" as io;',
        '',
    ]
    for v, t in VARS:
        if t in FT:
            it = 'u32' if t == 'f32' else 'u64'
            out.append(f'let mut {v.upper()}: [{nsets}]{it} = [{", ".join(str(f_bits(x, t)) for x in g.sets[v])}];')
        else:
            out.append(f'let mut {v.upper()}: [{nsets}]{t} = [{", ".join(str(x) for x in g.sets[v])}];')
    out.append('')
    for v, t in VARS:
        x = g.sets[v][0]
        out.append(f'const {const_names[v]}: {t} = {float_text(x) if t in FT else x};')
    out.append('')
    out += hashers()
    params = ', '.join(f'{v}: {t}' for v, t in VARS)
    args = ', '.join(f'@bit_cast({t}, {v.upper()}[s])' if t in FT else f'{v.upper()}[s]' for v, t in VARS)
    if expand:
        body = ['export fn run() void {']
        for i, e in enumerate(exprs):
            out += [f'fn e{i}({params}) {e.t} {{', f'    return {e.render(run_names)};', '}', '']
            body.append(f'    // {e.render(run_names)}')
            body.append(f'    for s in 0..A0.len {{')
            body.append(f'        io.uint(h_{e.t}(0, e{i}({args})));')
            body.append('    }')
            for si in range(nsets):
                body.append(f'    // expect: {mix(0, hash_of(e.vals[si], e.t))}')
            body.append(f'    io.uint(h_{e.t}(0, {e.render(const_names)}));  // expect: {mix(0, hash_of(e.vals[0], e.t))}')
        out += body + ['}']
    else:
        groups = [exprs[i:i + GROUP] for i in range(0, len(exprs), GROUP)]
        calls = []
        for gi, grp in enumerate(groups):
            for kind, names in (('g', run_names), ('c', const_names)):
                out.append(f'fn {kind}{gi}({params}) u64 {{' if kind == 'g' else f'fn {kind}{gi}() u64 {{')
                out.append('    let mut h: u64 = 0;')
                for e in grp:
                    out.append(f'    h = h_{e.t}(h, {e.render(names)});')
                out += ['    return h;', '}', '']
            hr = 0
            for si in range(nsets):
                hs = 0
                for e in grp:
                    hs = mix(hs, hash_of(e.vals[si], e.t))
                hr = mix(hr, hs)
            hc = 0
            for e in grp:
                hc = mix(hc, hash_of(e.vals[0], e.t))
            calls.append((gi, hr, hc))
        out.append('export fn run() void {')
        for gi, hr, hc in calls:
            out += ['    {', '        let mut h: u64 = 0;', '        for s in 0..A0.len {',
                    f'            h = mix(h, g{gi}({args}));', '        }', f'        io.uint(h);  // expect: {hr}',
                    '    }', f'    io.uint(c{gi}());  // expect: {hc}']
        out.append('}')
    with open(os.path.join(out_dir, fname), 'w') as f:
        f.write('\n'.join(out) + '\n')


def main():
    args = sys.argv[1:]
    expand = '--expand' in args
    args = [a for a in args if a != '--expand']
    if len(args) == 2 and args[0].startswith('--seeds='):
        first, count = (int(x) for x in args[0][len('--seeds='):].split(':'))
        for seed in range(first, first + count):
            write(f'exact_fexprs_s{seed}.ax', seed, 300, 12, args[1], expand)
        return
    for fname, seed, nexpr, nsets in PROGRAMS:
        write(fname, seed, nexpr, nsets, expand=expand)


if __name__ == '__main__':
    main()
