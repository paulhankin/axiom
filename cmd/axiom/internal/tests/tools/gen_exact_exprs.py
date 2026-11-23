#!/usr/bin/env python3
"""Writes tests of random integer expressions whose expected values Python
computes exactly from the specification's rules (spec §6.2, §7.5, §7.6,
Appendix A), not from the compiler: the front end and the lowering are
shared by every backend, so comparing backends with each other cannot find
their bugs, and these tests can.

    python3 cmd/axiom/internal/tests/tools/gen_exact_exprs.py
    cmd/axiom/build/axiom fmt cmd/axiom/internal/tests/run/exact_exprs_*.ax

writes cmd/axiom/internal/tests/run/exact_exprs_<n>.ax (the same files every
time: each has its own fixed seed). Each holds a few hundred expression
trees over ten variables of the integer types (one of each width and
signedness, and a second i32 and u64), mixing the wrapping operators
(+% -% *% -%x), the bitwise ones (& | ^ ~), shifts and rotations by counts
of any unsigned type, / and %, @min @max @abs, @clz @ctz @popcount,
@mul_high, the saturating builtins, @add_checked and the others with a
`catch` value, the conversions (@as, @int_cast, @truncate, @bit_cast) and
comparisons as integers (@as of a bool). The operations that can fail (+ -
* and unary - that overflow, / and % by 0, MIN / -1, @abs(MIN), @int_cast
out of range) appear only where Python has checked that no value set makes
them fail, so every expression has one value.

The variables' values (a dozen sets: edge values of each type and
pseudo-random ones of every magnitude) come from memory, and the expressions
are grouped in functions of the variables, so nothing is folded; the same
expressions over constants (the first value set, declared `const`) are
evaluated by the compiler. Each output line is a hash of the results of a
group, over all value sets, or over the constants.

    gen_exact_exprs.py --seeds=<first>:<count> <dir>

writes more programs (exact_exprs_s<seed>.ax, with other seeds) into <dir>,
for a longer search; copied into tests/run/, the suite runs them.
"""
import os
import random
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
RUN = os.path.join(HERE, '..', 'run')
M64 = (1 << 64) - 1
FNV = 1099511628211

TYPES = {'u8': (8, False), 'i8': (8, True), 'u16': (16, False), 'i16': (16, True), 'u32': (32, False),
         'i32': (32, True), 'u64': (64, False), 'i64': (64, True)}
VARS = [('x0', 'i8'), ('x1', 'u8'), ('x2', 'i16'), ('x3', 'u16'), ('x4', 'i32'), ('x5', 'u32'), ('x6', 'i64'),
        ('x7', 'u64'), ('x8', 'i32'), ('x9', 'u64')]
UNSIGNED = [t for t in TYPES if not TYPES[t][1]]
SIGNED = [t for t in TYPES if TYPES[t][1]]

PROGRAMS = [  # file, seed, expressions, value sets
    ('exact_exprs_1.ax', 1001, 300, 12),
    ('exact_exprs_2.ax', 2002, 300, 12),
]
GROUP = 12  # expressions per function


def rng_of(t):
    bits, signed = TYPES[t]
    if signed:
        return -(1 << (bits - 1)), (1 << (bits - 1)) - 1
    return 0, (1 << bits) - 1


def wrap(v, t):
    bits, signed = TYPES[t]
    v &= (1 << bits) - 1
    if signed and v >> (bits - 1):
        v -= 1 << bits
    return v


def in_range(v, t):
    lo, hi = rng_of(t)
    return lo <= v <= hi


def clamp(v, t):
    lo, hi = rng_of(t)
    return max(lo, min(hi, v))


def tdiv(a, b):
    q = abs(a) // abs(b)
    return q if (a < 0) == (b < 0) else -q


def trem(a, b):
    return a - b * tdiv(a, b)


def shl(a, k, t):
    return wrap(a << k, t) if k < 1024 else 0


def shr(a, k):
    if k >= 1024:
        return -1 if a < 0 else 0
    return a >> k


def rot(a, k, t, left):
    bits, _ = TYPES[t]
    k %= bits
    if not left:
        k = (bits - k) % bits
    u = a & ((1 << bits) - 1)
    return wrap((u << k) | (u >> (bits - k)), t)


def clz(a, t):
    bits, _ = TYPES[t]
    return bits - (a & ((1 << bits) - 1)).bit_length()


def ctz(a, t):
    bits, _ = TYPES[t]
    u = a & ((1 << bits) - 1)
    return bits if u == 0 else (u & -u).bit_length() - 1


def popcount(a, t):
    bits, _ = TYPES[t]
    return bin(a & ((1 << bits) - 1)).count('1')


def lossless(s, d):
    sb, ss = TYPES[s]
    db, ds = TYPES[d]
    if ss and not ds:
        return False
    return db >= sb if ss == ds else db > sb


def mix(h, x):
    """FNV-style, with the high half folded in: a multiplication alone
    carries a change of the top bit to the top bit only, so that two
    results wrong in their sign bit would cancel."""
    y = ((h ^ (x & M64)) * FNV) & M64
    return y ^ (y >> 32)


class Node:
    """An expression: its Axiom text (`text` with the variables' names
    substituted by `render`), its type and its value in every set."""

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
        self.sets = {v: [self.value(t) for _ in range(nsets)] for v, t in VARS}

    def value(self, t):
        lo, hi = rng_of(t)
        bits, signed = TYPES[t]
        k = self.r.random()
        if k < 0.35:
            e = self.r.randint(0, bits - 1)
            base = [0, 1, -1, lo, hi, 1 << e, (1 << e) - 1, -(1 << e), lo + 1, hi - 1]
            vs = [v for v in base if lo <= v <= hi]
            return self.r.choice(vs)
        if k < 0.6:
            return self.r.randint(lo, hi)
        m = self.r.randint(1, bits)
        v = self.r.randint(0, (1 << m) - 1)
        if signed and self.r.random() < 0.5:
            v = -v
        return clamp(v, t)

    def var(self, t):
        names = [v for v, vt in VARS if vt == t]
        v = self.r.choice(names)
        return Node(['$' + v], t, list(self.sets[v]), leaf=True)

    def literal(self, t):
        lo, hi = rng_of(t)
        bits, _ = TYPES[t]
        v = self.r.choice([0, 1, 2, 3, 5, 7, 31, 63, 100, 255, hi, lo, -1, -7, 1 << self.r.randint(0, bits - 1),
                           self.r.randint(lo, hi)])
        v = clamp(v, t)
        return Node([str(v)], t, [v] * self.nsets, leaf=True, lit=True)

    def ok(self, vals):
        return all(v is not None for v in vals)

    def gen(self, t, depth):
        if depth <= 0 or self.r.random() < 0.15:
            return self.var(t)
        for _ in range(20):
            n = self.try_node(t, depth)
            if n is not None and self.ok(n.vals):
                assert all(in_range(v, n.t) for v in n.vals), (n.render({v: v for v, _ in VARS}), n.vals)
                return n
        return self.var(t)

    def operand(self, t, depth, lit_ok=True):
        if lit_ok and self.r.random() < 0.25:
            return self.literal(t)
        return self.gen(t, depth)

    def try_node(self, t, depth):
        r = self.r
        bits, signed = TYPES[t]
        lo, hi = rng_of(t)
        d = depth - 1
        kind = r.choice(['wrap', 'wrap', 'wrap', 'bit', 'bit', 'shift', 'rot', 'div', 'plain', 'minmax', 'sat',
                         'checked', 'unary', 'count', 'conv', 'conv', 'conv', 'cmp', 'mulhigh'])
        if kind == 'wrap':
            op = r.choice(['+%', '-%', '*%'])
            f = {'+%': lambda a, b: a + b, '-%': lambda a, b: a - b, '*%': lambda a, b: a * b}[op]
            a = self.gen(t, d)
            b = self.operand(t, d)
            if r.random() < 0.3 and op == '-%':
                a, b = b, a
            if a.lit and b.lit:
                return None
            return Node(cat('(', a, f' {op} ', b, ')'), t, [wrap(f(x, y), t) for x, y in zip(a.vals, b.vals)])
        if kind == 'bit':
            op = r.choice(['&', '|', '^'])
            f = {'&': lambda a, b: a & b, '|': lambda a, b: a | b, '^': lambda a, b: a ^ b}[op]
            a = self.gen(t, d)
            b = self.operand(t, d)
            return Node(cat('(', a, f' {op} ', b, ')'), t, [f(x, y) for x, y in zip(a.vals, b.vals)])
        if kind == 'shift':
            op = r.choice(['<<', '>>'])
            a = self.gen(t, d)
            if r.random() < 0.4:
                k = r.choice([0, 1, 2, bits - 1, bits, bits + 1, r.randint(0, 2 * bits)])
                kn = Node([str(k)], 'u32', [k] * self.nsets, lit=True)
            else:
                ct = r.choice(UNSIGNED)
                kn = self.gen(ct, d)
                if r.random() < 0.5:
                    m = r.choice([7, 15, 31, 63, 127])
                    kn = Node(cat('(', kn, f' & {m})'), ct, [x & m for x in kn.vals])
            f = (lambda x, k: shl(x, k, t)) if op == '<<' else shr
            return Node(cat('(', a, f' {op} ', kn, ')'), t, [f(x, k) for x, k in zip(a.vals, kn.vals)])
        if kind == 'rot':
            left = r.random() < 0.5
            a = self.gen(t, d)
            ct = r.choice(UNSIGNED)
            kn = self.gen(ct, d)
            name = 'rotl' if left else 'rotr'
            return Node(cat(f'@{name}(', a, ', ', kn, ')'), t, [rot(x, k, t, left) for x, k in zip(a.vals, kn.vals)])
        if kind == 'div':
            op = r.choice(['/', '%'])
            a = self.gen(t, d)
            b = self.operand(t, d)
            if r.random() < 0.5 and not b.lit:
                b = Node(cat('(', b, ' | 1)'), t, [x | 1 for x in b.vals])
            if b.lit and b.vals[0] == 0:
                return None
            if a.lit and b.lit:
                return None
            vals = []
            for x, y in zip(a.vals, b.vals):
                if y == 0 or (op == '/' and signed and x == lo and y == -1):
                    vals.append(None)
                else:
                    vals.append(tdiv(x, y) if op == '/' else trem(x, y))
            return Node(cat('(', a, f' {op} ', b, ')'), t, vals)
        if kind == 'plain':
            op = r.choice(['+', '-', '*'])
            f = {'+': lambda a, b: a + b, '-': lambda a, b: a - b, '*': lambda a, b: a * b}[op]
            a = self.gen(t, d)
            b = self.operand(t, d)
            if r.random() < 0.6:
                m = r.choice([3, 15, 127]) if bits > 8 else r.choice([3, 7])
                a = Node(cat('(', a, f' & {m})'), t, [x & m for x in a.vals])
                if not b.lit:
                    b = Node(cat('(', b, f' & {m})'), t, [x & m for x in b.vals])
            if a.lit and b.lit:
                return None
            vals = [f(x, y) for x, y in zip(a.vals, b.vals)]
            return Node(cat('(', a, f' {op} ', b, ')'), t, [v if in_range(v, t) else None for v in vals])
        if kind == 'minmax':
            name = r.choice(['min', 'max'])
            a = self.gen(t, d)
            b = self.operand(t, d)
            f = min if name == 'min' else max
            return Node(cat(f'@{name}(', a, ', ', b, ')'), t, [f(x, y) for x, y in zip(a.vals, b.vals)])
        if kind == 'sat':
            name = r.choice(['add', 'sub', 'mul'])
            f = {'add': lambda a, b: a + b, 'sub': lambda a, b: a - b, 'mul': lambda a, b: a * b}[name]
            a = self.gen(t, d)
            b = self.operand(t, d)
            return Node(cat(f'@{name}_sat(', a, ', ', b, ')'), t, [clamp(f(x, y), t) for x, y in zip(a.vals, b.vals)])
        if kind == 'checked':
            name = r.choice(['add', 'sub', 'mul'])
            f = {'add': lambda a, b: a + b, 'sub': lambda a, b: a - b, 'mul': lambda a, b: a * b}[name]
            a = self.gen(t, d)
            b = self.operand(t, d)
            k = self.literal(t)
            vals = []
            for x, y in zip(a.vals, b.vals):
                v = f(x, y)
                vals.append(v if in_range(v, t) else k.vals[0])
            return Node(cat(f'(@{name}_checked(', a, ', ', b, ') catch ', k, ')'), t, vals)
        if kind == 'unary':
            ops = ['-%', '~'] + (['-', 'abs'] if signed else [])
            op = r.choice(ops)
            a = self.gen(t, d)
            if op == '-%':
                return Node(cat('-%(', a, ')'), t, [wrap(-x, t) for x in a.vals])
            if op == '~':
                return Node(cat('~(', a, ')'), t, [wrap(~x, t) for x in a.vals])
            if op == '-':
                return Node(cat('-(', a, ')'), t, [-x if x != lo else None for x in a.vals])
            return Node(cat('@abs(', a, ')'), t, [abs(x) if x != lo else None for x in a.vals])
        if kind == 'count':
            name = r.choice(['clz', 'ctz', 'popcount'])
            f = {'clz': clz, 'ctz': ctz, 'popcount': popcount}[name]
            a = self.gen(t, d)
            return Node(cat(f'@{name}(', a, ')'), t, [f(x, t) for x in a.vals])
        if kind == 'mulhigh':
            a = self.gen(t, d)
            b = self.operand(t, d)
            return Node(cat('@mul_high(', a, ', ', b, ')'), t, [wrap((x * y) >> bits, t) for x, y in zip(a.vals, b.vals)])
        if kind == 'conv':
            s = r.choice(list(TYPES))
            sb, ss = TYPES[s]
            forms = ['int_cast', 'int_cast_masked']
            if lossless(s, t):
                forms += ['as', 'as']
            if sb >= bits:
                forms += ['truncate', 'truncate']
            if sb == bits and ss != signed:
                forms += ['bit_cast']
            form = r.choice(forms)
            a = self.gen(s, d)
            if form == 'as':
                return Node(cat(f'@as({t}, ', a, ')'), t, list(a.vals))
            if form == 'truncate':
                return Node(cat(f'@truncate({t}, ', a, ')'), t, [wrap(x, t) for x in a.vals])
            if form == 'bit_cast':
                return Node(cat(f'@bit_cast({t}, ', a, ')'), t, [wrap(x, t) for x in a.vals])
            if form == 'int_cast_masked':
                m = (1 << (min(bits - signed, sb - ss))) - 1
                a = Node(cat('(', a, f' & {m})'), s, [x & m for x in a.vals])
            return Node(cat(f'@int_cast({t}, ', a, ')'), t, [x if in_range(x, t) else None for x in a.vals])
        if kind == 'cmp':
            s = r.choice(list(TYPES))
            op = r.choice(['==', '!=', '<', '<=', '>', '>='])
            f = {'==': lambda a, b: a == b, '!=': lambda a, b: a != b, '<': lambda a, b: a < b,
                 '<=': lambda a, b: a <= b, '>': lambda a, b: a > b, '>=': lambda a, b: a >= b}[op]
            a = self.gen(s, d)
            if r.random() < 0.5:
                # against another type, both converted to one that holds both
                s2 = r.choice(list(TYPES))
                c = next((c for c in TYPES if lossless(s, c) and lossless(s2, c)), None)
                if c is None:
                    return None
                b = self.gen(s2, d)
                a = Node(cat(f'@as({c}, ', a, ')'), c, list(a.vals))
                b = Node(cat(f'@as({c}, ', b, ')'), c, list(b.vals))
            else:
                b = self.operand(s, d)
            return Node(cat(f'@as({t}, ', a, f' {op} ', b, ')'), t, [int(f(x, y)) for x, y in zip(a.vals, b.vals)])
        raise AssertionError(kind)


def hash_of(v):
    return v & M64


def write(fname, seed, nexpr, nsets, out_dir=RUN):
    g = Gen(seed, nsets)
    exprs = []
    for _ in range(nexpr):
        t = g.r.choice(list(TYPES))
        e = g.gen(t, g.r.randint(2, 5))
        while e.leaf:
            e = g.gen(t, g.r.randint(2, 5))
        exprs.append(e)
    run_names = {v: v for v, _ in VARS}
    const_names = {v: 'K' + v[1:] for v, _ in VARS}
    out = [
        '// Random integer expressions over ten variables, every operation evaluated',
        '// by the rules of the spec (§6.2, §7.5, §7.6, Appendix A); the operations',
        '// that can fail appear only where no value set makes them fail. Each line',
        '// is a hash of a group of expressions over all value sets (from memory)',
        '// or over constants (the first set; the compiler evaluates them). (Generated',
        '// by cmd/axiom/internal/tests/tools/gen_exact_exprs.py, with the',
        '// expectations from exact arithmetic in Python.)',
        'import "../lib/io.ax" as io;',
        '',
    ]
    for v, t in VARS:
        out.append(f'let mut {v.upper()}: [{nsets}]{t} = [{", ".join(str(x) for x in g.sets[v])}];')
    out.append('')
    for v, t in VARS:
        out.append(f'const {const_names[v]}: {t} = {g.sets[v][0]};')
    out += ['', 'fn mix(h: u64, x: u64) u64 {', '    let y = (h ^ x) *% 1099511628211;', '    return y ^ (y >> 32);', '}', '']
    for t, (bits, signed) in TYPES.items():
        conv = f'@bit_cast(u64, @as(i64, r))' if signed and bits < 64 else (
            '@bit_cast(u64, r)' if signed else ('r' if bits == 64 else '@as(u64, r)'))
        out += [f'fn h_{t}(h: u64, r: {t}) u64 {{', f'    return mix(h, {conv});', '}', '']
    params = ', '.join(f'{v}: {t}' for v, t in VARS)
    groups = [exprs[i:i + GROUP] for i in range(0, len(exprs), GROUP)]
    calls = []
    for gi, grp in enumerate(groups):
        for kind, names in (('g', run_names), ('c', const_names)):
            sig = f'fn {kind}{gi}({params}) u64 {{' if kind == 'g' else f'fn {kind}{gi}() u64 {{'
            out.append(sig)
            out.append('    let mut h: u64 = 0;')
            for e in grp:
                out.append(f'    h = h_{e.t}(h, {e.render(names)});')
            out += ['    return h;', '}', '']
        hr = 0
        for si in range(nsets):
            hs = 0
            for e in grp:
                hs = mix(hs, hash_of(e.vals[si]))
            hr = mix(hr, hs)
        hc = 0
        for e in grp:
            hc = mix(hc, hash_of(e.vals[0]))
        calls.append((gi, hr, hc))
    out.append('export fn run() void {')
    args = ', '.join(f'{v.upper()}[s]' for v, _ in VARS)
    for gi, hr, hc in calls:
        out += ['    {', '        let mut h: u64 = 0;', f'        for s in 0..X0.len {{',
                f'            h = mix(h, g{gi}({args}));', '        }', f'        io.uint(h);  // expect: {hr}', '    }',
                f'    io.uint(c{gi}());  // expect: {hc}']
    out.append('}')
    with open(os.path.join(out_dir, fname), 'w') as f:
        f.write('\n'.join(out) + '\n')


def main():
    if len(sys.argv) == 3 and sys.argv[1].startswith('--seeds='):
        first, count = (int(x) for x in sys.argv[1][len('--seeds='):].split(':'))
        for seed in range(first, first + count):
            write(f'exact_exprs_s{seed}.ax', seed, 300, 12, sys.argv[2])
        return
    for fname, seed, nexpr, nsets in PROGRAMS:
        write(fname, seed, nexpr, nsets)


if __name__ == '__main__':
    main()
