#!/usr/bin/env python3
"""Writes tests of Axiom's scalar operations whose expected outputs Python
computes exactly from the specification's rules (spec §6.2, §7.5 to §7.7,
Appendix A), not from the compiler: the front end and the lowering are
shared by every backend, so comparing backends with each other cannot find
their bugs, and these tests can.

    python3 cmd/axiom/internal/tests/tools/gen_exact_ops.py
    cmd/axiom/build/axiom fmt cmd/axiom/internal/tests/run/exact_*.ax

writes into cmd/axiom/internal/tests/run/ (the same files every time; the
pseudo-random values have a fixed seed):

- exact_int_conv.ax: @as (every lossless pair of integer types, sign and
  zero extension), @int_cast of the values in the target's range, @truncate
  to a type as wide or narrower, @bit_cast between the signed and unsigned
  types of a width; usize and isize with values that fit 32 bits, so that
  the results are the same on every target.
- exact_int_cast_trap.ax: an @int_cast out of range traps.
- exact_int_shift.ax: << >> @rotl @rotr on every integer type, with a count
  of every unsigned type from memory (0, 1, the width - 1, the width, more,
  the count type's largest value), with constant counts, and of constant
  values by counts from memory.
- exact_int_arith.ax: +% -% *% & | ^ @min @max @add_sat @sub_sat @mul_sat
  @mul_high of every pair of edge values, / and % (the signs; not by 0, and
  / not MIN by -1), + - * where they do not overflow, the compound
  assignments, and the operations with a constant operand.
- exact_int_unary.ax: -% ~ @clz @ctz @popcount, - and @abs (not of MIN),
  the comparisons as values and as branches, against constants, and between
  two types through the @as of both to a type that holds both.
- exact_int_fold.ax: the operations on constants, which the compiler
  evaluates (in a function's body and in const initializers).
- exact_float_conv.ax: @float_from_int (round to nearest, ties to even),
  @as from the integers a float holds exactly and from f32 to f64,
  @int_from_float (truncation, saturation, NaN to 0), @float_cast (f64 to
  f32: rounding, overflow to infinity, subnormals) and @bit_cast between
  floats and integers.
- exact_float_arith.ax: IEEE 754 binary32 and binary64 + - * / @sqrt @fma
  (and a * b + c, which must not be contracted), unary -, @abs, @min/@max
  (a NaN operand gives NaN; -0 < +0), @copysign, @floor @ceil @trunc
  @nearest and the comparisons, of every pair (triple for @fma) of edge
  values: zeros, ones, subnormals, the largest and smallest normals, values
  whose operations round (ties too), infinities and NaN. Every one of these
  has one correctly rounded result, which Python computes with rationals.

The operands come from memory (`let mut` globals), so that nothing is
folded except in exact_int_fold.ax. Each output line is a hash (FNV-style,
over the 64-bit patterns) of the results of one operation over all its
operands, in a fixed order; a NaN result is hashed as one canonical NaN, as
NaN payloads are unspecified (§7.7). To find the operand behind a wrong
hash, print the values in the failing function.
"""
import itertools
import math
import os
import random
import struct
from fractions import Fraction

HERE = os.path.dirname(os.path.abspath(__file__))
RUN = os.path.join(HERE, '..', 'run')
M64 = (1 << 64) - 1
FNV = 1099511628211
rnd = random.Random(20261123)

FIXED = [(8, False), (8, True), (16, False), (16, True), (32, False), (32, True), (64, False), (64, True)]


def tname(bits, signed):
    return ('i' if signed else 'u') + str(bits)


# Integer types: name -> (bits, signed). usize and isize are given 32 bits:
# their values and the operations on them are chosen so that the results
# do not depend on the target's width.
ITYPES = {tname(b, s): (b, s) for b, s in FIXED}
ITYPES['usize'] = (32, False)
ITYPES['isize'] = (32, True)
PTRSIZED = ('usize', 'isize')


def rng_of(t):
    bits, signed = ITYPES[t]
    if t == 'usize':
        return 0, (1 << 31) - 1
    if signed:
        return -(1 << (bits - 1)), (1 << (bits - 1)) - 1
    return 0, (1 << bits) - 1


def wrap(v, bits, signed):
    v &= (1 << bits) - 1
    if signed and v >> (bits - 1):
        v -= 1 << bits
    return v


def wrap_t(v, t):
    return wrap(v, *ITYPES[t])


def in_range(v, t):
    lo, hi = rng_of(t)
    return lo <= v <= hi


def dedupe(vs):
    out = []
    for v in vs:
        if v not in out:
            out.append(v)
    return out


def edge_values(t):
    """MIN, MAX, 0, +-1, small values, powers of two and their neighbours
    (every power for 8 and 16 bits; for wider types those near the byte
    boundaries, the float mantissa widths and the top), and a few
    pseudo-random values."""
    bits, signed = ITYPES[t]
    lo, hi = rng_of(t)
    if bits <= 16:
        ks = range(1, bits)
    else:
        ks = [1, 2, 3, 7, 8, 9, 15, 16, 17, 23, 24, 25, 31, 32, 33, 47, 52, 53, 54, 62, 63]
    vs = [0, 1, 2, 3, 5, 7, 10, hi, hi - 1, hi // 2, hi // 2 + 1]
    if signed:
        vs += [-1, -2, -3, -7, lo, lo + 1, lo // 2, lo // 2 - 1]
    for k in ks:
        p = 1 << k
        vs += [p - 1, p, p + 1, p + 3]
        if signed:
            vs += [-p + 1, -p, -p - 1]
    for _ in range(4):
        vs.append(rnd.randint(lo, hi))
    return dedupe(v for v in vs if lo <= v <= hi)


def consts_of(t):
    lo, hi = rng_of(t)
    _, signed = ITYPES[t]
    vs = [0, 1, 2, 3, 7, 255, hi // 2, hi // 2 + 1, hi]
    if signed:
        vs = [lo, -1, -2, lo + 1] + vs
    return dedupe(v for v in vs if lo <= v <= hi)


def lit(v):
    return str(v)


def mix(h, x):
    return ((h ^ (x & M64)) * FNV) & M64


# ---- floats --------------------------------------------------------------

FMT = {'f32': (24, -126, 127), 'f64': (53, -1022, 1023)}
INF = float('inf')
NAN = float('nan')


def is_neg(x):
    return math.copysign(1.0, x) < 0


def round_frac(x, t):
    """The value of float type t nearest the rational x (ties to even), as
    a Python float, or +-inf when it overflows."""
    p, emin, emax = FMT[t]
    if x == 0:
        return 0.0
    sign = -1 if x < 0 else 1
    a = abs(x)
    e = a.numerator.bit_length() - a.denominator.bit_length()
    if Fraction(2) ** e > a:
        e -= 1
    assert Fraction(2) ** e <= a < Fraction(2) ** (e + 1)
    e = max(e, emin)
    q = Fraction(2) ** (e - p + 1)
    m = a / q
    n = m.numerator // m.denominator
    r = m - n
    if r > Fraction(1, 2) or (r == Fraction(1, 2) and n % 2 == 1):
        n += 1
    v = n * q
    if v >= Fraction(2) ** (emax + 1):
        return sign * INF
    f = float(v)
    assert Fraction(f) == v
    return sign * f


def signed_zero(neg):
    return -0.0 if neg else 0.0


def rnd_result(exact, t, neg_if_zero):
    """Round the exact rational result; a zero (exact, or after
    underflow) gets the sign neg_if_zero gives."""
    if exact == 0:
        return signed_zero(neg_if_zero)
    r = round_frac(exact, t)
    if r == 0:
        return signed_zero(exact < 0)
    return r


def f_add(a, b, t):
    if math.isnan(a) or math.isnan(b):
        return NAN
    if math.isinf(a) or math.isinf(b):
        if math.isinf(a) and math.isinf(b) and is_neg(a) != is_neg(b):
            return NAN
        return a if math.isinf(a) else b
    exact = Fraction(a) + Fraction(b)
    return rnd_result(exact, t, a == 0 and b == 0 and is_neg(a) and is_neg(b))


def f_sub(a, b, t):
    return f_add(a, -b, t)


def f_mul(a, b, t):
    if math.isnan(a) or math.isnan(b):
        return NAN
    neg = is_neg(a) != is_neg(b)
    if math.isinf(a) or math.isinf(b):
        if a == 0 or b == 0:
            return NAN
        return -INF if neg else INF
    return rnd_result(Fraction(a) * Fraction(b), t, neg)


def f_div(a, b, t):
    if math.isnan(a) or math.isnan(b):
        return NAN
    neg = is_neg(a) != is_neg(b)
    if math.isinf(a):
        if math.isinf(b):
            return NAN
        return -INF if neg else INF
    if math.isinf(b):
        return signed_zero(neg)
    if b == 0:
        if a == 0:
            return NAN
        return -INF if neg else INF
    return rnd_result(Fraction(a) / Fraction(b), t, neg)


def f_sqrt(a, t):
    if math.isnan(a) or (a < 0):
        return NAN
    if a == 0 or math.isinf(a):
        return a
    x = Fraction(a)
    k = 1200  # x * 4^k has far more bits than either format, even for subnormal x
    big = x * Fraction(4) ** k
    n = big.numerator // big.denominator
    s = math.isqrt(n)
    exact = s * s == n and big == n
    v = Fraction(s, 2 ** k)
    if not exact:
        v += Fraction(1, 2 ** (k + 1))  # sticky: strictly above s, below s + 1
    return round_frac(v, t)


def f_fma(a, b, c, t):
    if math.isnan(a) or math.isnan(b) or math.isnan(c):
        return NAN
    pneg = is_neg(a) != is_neg(b)
    if math.isinf(a) or math.isinf(b):
        if a == 0 or b == 0:
            return NAN
        if math.isinf(c) and is_neg(c) != pneg:
            return NAN
        return -INF if pneg else INF
    if math.isinf(c):
        return c
    prod = Fraction(a) * Fraction(b)
    exact = prod + Fraction(c)
    return rnd_result(exact, t, prod == 0 and c == 0 and pneg and is_neg(c))


def f_minmax(a, b, is_max):
    if math.isnan(a) or math.isnan(b):
        return NAN
    if a == b == 0:
        if is_max:
            return a if not is_neg(a) else b
        return a if is_neg(a) else b
    return max(a, b) if is_max else min(a, b)


def f_round(a, mode):
    if math.isnan(a) or math.isinf(a) or a == 0:
        return a
    x = Fraction(a)
    if mode == 'floor':
        n = math.floor(x)
    elif mode == 'ceil':
        n = math.ceil(x)
    elif mode == 'trunc':
        n = math.trunc(x)
    else:
        n = round(x)  # Fraction rounds half to even
    if n == 0:
        return signed_zero(is_neg(a))
    return float(n)


def f_cmp(a, b):
    r = 0
    for i, c in enumerate([a == b, a != b, a < b, a <= b, a > b, a >= b]):
        if c:
            r |= 1 << i
    return r


def f_bits(x, t):
    if t == 'f32':
        return struct.unpack('<I', struct.pack('<f', x))[0]
    return struct.unpack('<Q', struct.pack('<d', x))[0]


def f_from_bits(b, t):
    if t == 'f32':
        return struct.unpack('<f', struct.pack('<I', b))[0]
    return struct.unpack('<d', struct.pack('<Q', b))[0]


_FLOATS = {}


def float_values(t):
    if t not in _FLOATS:
        _FLOATS[t] = float_values_new(t)
    return _FLOATS[t]


def float_values_new(t):
    """Bit patterns of the operands: zeros, small integers and halves,
    values that do not round exactly, the extremes of the normal and
    subnormal ranges, the integers near the end of the mantissa, values
    near the integer types' limits, infinities and a NaN."""
    p, emin, emax = FMT[t]
    vs = [0.0, -0.0, 1.0, -1.0, 0.5, -0.5, 1.5, -1.5, 2.5, -2.5, 3.0, 0.75, 7.0, -7.0, 10.0,
          0.1, -0.1, 1 / 3, 2 / 3, 1e10, -1e10, 123.456]
    vs += [2.0 ** (p - 1), 2.0 ** (p - 1) + 1, 2.0 ** p, 2.0 ** p + 2, -2.0 ** p, 2.0 ** 31, -2.0 ** 31,
           2.0 ** 63, -2.0 ** 63, 2.0 ** 64]
    mx = (2 - 2.0 ** (1 - p)) * 2.0 ** emax
    vs += [mx, -mx, 2.0 ** emin, -2.0 ** emin, 2.0 ** (emin - p + 1), -2.0 ** (emin - p + 1),
           (1 - 2.0 ** (1 - p)) * 2.0 ** emin, 2.0 ** (emax - 1), 2.0 ** (emin + 1) * 3]
    vs += [INF, -INF, NAN]
    for _ in range(3):
        vs.append(rnd.uniform(-1000, 1000))
    out = []
    for v in vs:
        if not math.isnan(v) and not math.isinf(v):
            v = round_frac(Fraction(v), t)
        b = f_bits(v, t)
        if b not in out:
            out.append(b)
    return out


# ---- the programs ---------------------------------------------------------

def hash_bits(v, t):
    if t in ('f32', 'f64'):
        if math.isnan(v):
            return 0x7fc00000 if t == 'f32' else 0x7ff8000000000000
        return f_bits(v, t)
    if t == 'bool':
        return int(v)
    assert in_range(v, t) or t in PTRSIZED, (v, t)
    return v & M64


HELPERS = '''fn mix(h: u64, x: u64) u64 {
    return (h ^ x) *% 1099511628211;
}

fn h_u8(h: u64, r: u8) u64 {
    return mix(h, @as(u64, r));
}

fn h_u16(h: u64, r: u16) u64 {
    return mix(h, @as(u64, r));
}

fn h_u32(h: u64, r: u32) u64 {
    return mix(h, @as(u64, r));
}

fn h_u64(h: u64, r: u64) u64 {
    return mix(h, r);
}

fn h_usize(h: u64, r: usize) u64 {
    return mix(h, @as(u64, r));
}

fn h_i8(h: u64, r: i8) u64 {
    return mix(h, @bit_cast(u64, @as(i64, r)));
}

fn h_i16(h: u64, r: i16) u64 {
    return mix(h, @bit_cast(u64, @as(i64, r)));
}

fn h_i32(h: u64, r: i32) u64 {
    return mix(h, @bit_cast(u64, @as(i64, r)));
}

fn h_i64(h: u64, r: i64) u64 {
    return mix(h, @bit_cast(u64, r));
}

fn h_isize(h: u64, r: isize) u64 {
    return mix(h, @bit_cast(u64, @as(i64, r)));
}

fn h_bool(h: u64, r: bool) u64 {
    return mix(h, @as(u64, r));
}

fn h_f32(h: u64, r: f32) u64 {
    if r != r {
        return mix(h, 0x7fc00000);
    }
    return mix(h, @as(u64, @bit_cast(u32, r)));
}

fn h_f64(h: u64, r: f64) u64 {
    if r != r {
        return mix(h, 0x7ff8000000000000);
    }
    return mix(h, @bit_cast(u64, r));
}
'''


class Stmt:
    """One result to hash: `pre` lines run first (in the innermost loop),
    then the result `expr` of type `rtype` is mixed into hash `key`;
    `py` computes it from the loop values."""

    def __init__(self, key, expr, py, rtype, pre=()):
        self.key, self.expr, self.py, self.rtype, self.pre = key, expr, py, rtype, list(pre)


class Block:
    """Loops over arrays (outermost first; `loops` is a list of (variable,
    array name)), an optional guard (Axiom lines that `continue` and a
    Python predicate of the values that says when they do) and the results
    hashed in the innermost loop. No loops: the statements run once."""

    def __init__(self, loops, stmts, guard=None):
        self.loops, self.stmts, self.guard = loops, stmts, guard


class Prog:
    def __init__(self, name, comment):
        self.name = name
        self.comment = comment
        self.arrays = {}  # name -> (element type, Axiom literals, Python values, load type)
        self.consts = []
        self.fns = []
        self.calls = []  # (function name, [expected hashes])

    def array(self, name, t, vals, load=None):
        """A global array of values of integer type t, or (load f32/f64)
        of the bit patterns of floats."""
        lits = [lit(v) for v in vals]
        for n, (t2, l2, _, ld2) in self.arrays.items():
            if (t2, l2, ld2) == (t, lits, load):
                return n
        if name not in self.arrays:
            if load:
                py = [f_from_bits(b, load) for b in vals]
            else:
                py = list(vals)
            self.arrays[name] = (t, [lit(v) for v in vals], py, load)
        return name

    def const(self, line):
        self.consts.append(line)

    def fn(self, name, blocks):
        keys = []
        for b in blocks:
            for s in b.stmts:
                if s.key not in keys:
                    keys.append(s.key)
        hv = {k: f'h{i}' for i, k in enumerate(keys)}
        expect = {k: 0 for k in keys}
        lines = [f'fn {name}() void {{']
        for k in keys:
            lines.append(f'    let mut {hv[k]}: u64 = 0;')
        for b in blocks:
            ind = '    '
            for (var, arr), idx in zip(b.loops, 'ijklm'):
                lines.append(f'{ind}for {idx} in 0..{arr}.len {{')
                ind += '    '
            for (var, arr), idx in zip(b.loops, 'ijklm'):
                load = self.arrays[arr][3]
                if load:
                    lines.append(f'{ind}let {var} = @bit_cast({load}, {arr}[{idx}]);')
                else:
                    lines.append(f'{ind}let {var} = {arr}[{idx}];')
            if b.guard:
                lines += [ind + g for g in b.guard[0]]
            for s in b.stmts:
                lines += [ind + p for p in s.pre]
                lines.append(f'{ind}{hv[s.key]} = h_{s.rtype}({hv[s.key]}, {s.expr});')
            for _ in b.loops:
                ind = ind[:-4]
                lines.append(f'{ind}}}')
            for vals in itertools.product(*[self.arrays[arr][2] for _, arr in b.loops]):
                if b.guard and b.guard[1](*vals):
                    continue
                for s in b.stmts:
                    v = s.py(*vals)
                    assert v is not None, (name, s.expr, vals)
                    expect[s.key] = mix(expect[s.key], hash_bits(v, s.rtype))
        for k in keys:
            lines.append(f'    io.uint({hv[k]});')
        lines.append('}')
        self.fns.append('\n'.join(lines))
        self.calls.append((name, [expect[k] for k in keys]))

    def write(self, extra_head=()):
        out = ['// ' + l if l else '//' for l in self.comment]
        out.append('import "../lib/io.ax" as io;')
        out.append('')
        out += list(extra_head)
        for name, (t, lits, _, _) in self.arrays.items():
            out.append(f'let mut {name}: [{len(lits)}]{t} = [{", ".join(lits)}];')
        out.append('')
        out += self.consts
        if self.consts:
            out.append('')
        out.append(HELPERS)
        for f in self.fns:
            out.append(f)
            out.append('')
        out.append('export fn run() void {')
        for name, hs in self.calls:
            if not hs:
                continue
            out.append(f'    {name}();  // expect: {hs[0]}')
            for h in hs[1:]:
                out.append(f'    // expect: {h}')
        out.append('}')
        with open(os.path.join(RUN, self.name), 'w') as f:
            f.write('\n'.join(out) + '\n')


GEN = ['(Generated by cmd/axiom/internal/tests/tools/gen_exact_ops.py, with the',
       'expectations from exact arithmetic in Python, from the rules of the spec.)']


def V(prog, t):
    return prog.array('V_' + t.upper(), t, edge_values_cached(t))


_EDGE = {}


def edge_values_cached(t):
    if t not in _EDGE:
        _EDGE[t] = edge_values(t)
    return _EDGE[t]


def lossless(s, d):
    """@as(d, x) from s is allowed: every value of s is one of d on every
    target (usize and isize have 32 or 64 bits)."""
    if s == d:
        return True
    if s in PTRSIZED and d in PTRSIZED:
        return False
    sb, ss = ITYPES[s]
    db, ds = ITYPES[d]
    if s in PTRSIZED:
        sb = 64
    if ss and not ds:
        return False
    if ss == ds:
        return db >= sb
    return db > sb


INT_NAMES = [tname(b, s) for b, s in FIXED] + list(PTRSIZED)
FIXED_NAMES = [tname(b, s) for b, s in FIXED]


def gen_conv():
    p = Prog('exact_int_conv.ax', [
        'Integer conversions on every pair of integer types, at the edges of the',
        "ranges: @as (lossless: sign and zero extension), @int_cast of the source's",
        "values in the target's range, @truncate to a type as wide or narrower,",
        '@bit_cast between the signed and unsigned types of a width. usize and isize',
        'hold values that fit 32 bits, so the results are the same on every target.',
    ] + GEN)
    for s in INT_NAMES:
        vs = V(p, s)
        blocks = []
        stmts = []
        for d in INT_NAMES:
            if lossless(s, d):
                stmts.append(Stmt(f'as {d}', f'@as({d}, a)', lambda a: a, d))
        for d in INT_NAMES:
            sb, _ = ITYPES[s]
            db, ds = ITYPES[d]
            if d in PTRSIZED or (s in PTRSIZED and db > 32) or (s not in PTRSIZED and db > sb):
                continue
            stmts.append(Stmt(f'truncate {d}', f'@truncate({d}, a)', lambda a, db=db, ds=ds: wrap(a, db, ds), d))
        if s not in PTRSIZED:
            b, sg = ITYPES[s]
            d = tname(b, not sg)
            stmts.append(Stmt(f'bit_cast {d}', f'@bit_cast({d}, a)', lambda a, b=b, sg=sg: wrap(a, b, not sg), d))
        blocks.append(Block([('a', vs)], stmts))
        for d in INT_NAMES:
            if d == s:
                continue
            inr = [v for v in edge_values_cached(s) if in_range(v, d)]
            arr = vs if len(inr) == len(edge_values_cached(s)) else p.array(f'IC_{s.upper()}_{d.upper()}', s, inr)
            blocks.append(Block([('a', arr)], [Stmt(f'int_cast {d}', f'@int_cast({d}, a)', lambda a: a, d)]))
        p.fn(f'conv_{s}', blocks)
    p.write()


def gen_cast_trap():
    lines = [
        '// @int_cast of a value out of the target type\'s range traps (T0004), here',
        '// 256 to u8, from memory. (Generated by',
        '// cmd/axiom/internal/tests/tools/gen_exact_ops.py.)',
        '// expect-trap',
        'import "../lib/io.ax" as io;',
        '',
        'let mut V: [2]u32 = [255, 256];',
        '',
        'export fn run() void {',
        '    for i in 0..V.len {',
        '        io.uint(@as(u64, @int_cast(u8, V[i])));',
        '    }',
        '    // expect: 255',
        '}',
    ]
    with open(os.path.join(RUN, 'exact_int_cast_trap.ax'), 'w') as f:
        f.write('\n'.join(lines) + '\n')


def shl(a, k, t):
    return wrap_t(a << k, t) if k < 1024 else 0


def shr(a, k, t):
    if k >= 1024:
        return -1 if a < 0 else 0
    return a >> k


def rot(a, k, t, left):
    bits, signed = ITYPES[t]
    k %= bits
    u = a & ((1 << bits) - 1)
    if not left:
        k = (bits - k) % bits
    r = ((u << k) | (u >> (bits - k))) & ((1 << bits) - 1)
    return wrap(r, bits, signed)


def count_values(c):
    lo, hi = rng_of(c)
    vs = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 15, 16, 17, 31, 32, 33, 63, 64, 65, 100, 127, 128, 200, 255, 256,
          257, 1000, 4095, 65535, 65536, (1 << 31) - 1, 1 << 31, (1 << 32) - 1, 1 << 32, 1 << 63, hi]
    return dedupe(v for v in vs if lo <= v <= hi)


def gen_shift():
    p = Prog('exact_int_shift.ax', [
        'Shifts (spec §7.6: every count has its mathematical meaning; at or above',
        'the width, << and unsigned >> give 0 and signed >> gives 0 or -1) and',
        'rotations (the count modulo the width) of every integer type at the edges',
        'of its range: by counts of every unsigned type from memory, by constant',
        'counts, and of constant values by counts from memory.',
    ] + GEN)
    counts = ['u8', 'u16', 'u32', 'u64', 'usize']
    karr = {c: p.array('K_' + c.upper(), c, count_values(c)) for c in counts}
    for t in FIXED_NAMES:
        bits, signed = ITYPES[t]
        vs = V(p, t)
        blocks = []
        for c in counts:
            blocks.append(Block([('a', vs), ('k', karr[c])], [
                Stmt(f'shl {c}', 'a << k', lambda a, k, t=t: shl(a, k, t), t),
                Stmt(f'shr {c}', 'a >> k', lambda a, k, t=t: shr(a, k, t), t),
                Stmt(f'rotl {c}', '@rotl(a, k)', lambda a, k, t=t: rot(a, k, t, True), t),
                Stmt(f'rotr {c}', '@rotr(a, k)', lambda a, k, t=t: rot(a, k, t, False), t),
            ]))
        ks = dedupe([0, 1, 2, bits // 2, bits - 2, bits - 1, bits, bits + 1, 2 * bits, 255])
        stmts = []
        for k in ks:
            stmts.append(Stmt('shl const', f'a << {k}', lambda a, k=k, t=t: shl(a, k, t), t))
            stmts.append(Stmt('shr const', f'a >> {k}', lambda a, k=k, t=t: shr(a, k, t), t))
            stmts.append(Stmt('rotl const', f'@rotl(a, @as(u32, {k}))', lambda a, k=k, t=t: rot(a, k, t, True), t))
        blocks.append(Block([('a', vs)], stmts))
        lo, hi = rng_of(t)
        stmts = []
        for c in dedupe([1, 3, hi, lo, -1 if signed else hi - 1]):
            stmts.append(Stmt('const shl', f'@as({t}, {lit(c)}) << k', lambda k, c=c, t=t: shl(c, k, t), t))
            stmts.append(Stmt('const shr', f'@as({t}, {lit(c)}) >> k', lambda k, c=c, t=t: shr(c, k, t), t))
        blocks.append(Block([('k', karr['u32'])], stmts))
        p.fn(f'shift_{t}', blocks)
    p.write()


def tdiv(a, b):
    q = abs(a) // abs(b)
    return q if (a < 0) == (b < 0) else -q


def trem(a, b):
    return a - b * tdiv(a, b)


def clamp(v, t):
    lo, hi = rng_of(t)
    return max(lo, min(hi, v))


def checked(v, t):
    return v if in_range(v, t) else None


def mul_high(a, b, t):
    bits, signed = ITYPES[t]
    return wrap((a * b) >> bits, bits, signed)


def int_bin_ops(t):
    """(name, Axiom form with {a} and {b}, Python function) of the
    operations defined on every pair of values."""
    return [
        ('+%', '{a} +% {b}', lambda a, b: wrap_t(a + b, t)),
        ('-%', '{a} -% {b}', lambda a, b: wrap_t(a - b, t)),
        ('*%', '{a} *% {b}', lambda a, b: wrap_t(a * b, t)),
        ('&', '{a} & {b}', lambda a, b: a & b),
        ('|', '{a} | {b}', lambda a, b: a | b),
        ('^', '{a} ^ {b}', lambda a, b: a ^ b),
        ('min', '@min({a}, {b})', min),
        ('max', '@max({a}, {b})', max),
        ('add_sat', '@add_sat({a}, {b})', lambda a, b: clamp(a + b, t)),
        ('sub_sat', '@sub_sat({a}, {b})', lambda a, b: clamp(a - b, t)),
        ('mul_sat', '@mul_sat({a}, {b})', lambda a, b: clamp(a * b, t)),
        ('mul_high', '@mul_high({a}, {b})', lambda a, b: mul_high(a, b, t)),
    ]


def gen_arith():
    p = Prog('exact_int_arith.ax', [
        'Binary integer operations of every pair of edge values of every integer',
        'type: the wrapping operators, the bitwise ones, @min @max, the saturating',
        'builtins, @mul_high; / and % (truncating; the remainder has the sign of the',
        'dividend; MIN % -1 is 0), not by 0 and / not MIN by -1; + - * where they do',
        'not overflow (spec §7.6: there they would trap); the compound assignments;',
        'and the operations with a constant operand on either side.',
    ] + GEN)
    for t in FIXED_NAMES:
        bits, signed = ITYPES[t]
        lo, hi = rng_of(t)
        vs = V(p, t)
        ops = int_bin_ops(t)
        if signed:
            no_div = (f'if b == 0 or (a == @min_value({t}) and b == -1) {{', '    continue;', '}')
        else:
            no_div = ('if b == 0 {', '    continue;', '}')
        no_div_py = (lambda a, b: b == 0 or (signed and a == lo and b == -1))
        blocks = []
        blocks.append(Block([('a', vs), ('b', vs)], [Stmt(n, e.format(a='a', b='b'), f, t) for n, e, f in ops]))
        blocks.append(Block([('a', vs), ('b', vs)], [
            Stmt('%', 'a % b', trem, t),
            Stmt('%=', 'x', trem, t, pre=['let mut x = a;', 'x %= b;']),
        ], guard=(['if b == 0 {', '    continue;', '}'], lambda a, b: b == 0)))
        blocks.append(Block([('a', vs), ('b', vs)], [
            Stmt('/', 'a / b', tdiv, t),
            Stmt('/=', 'x', tdiv, t, pre=['let mut x = a;', 'x /= b;']),
            Stmt('/ %', 'q *% b +% r', lambda a, b: a, t, pre=['let q = a / b;', 'let r = a % b;']),
        ], guard=(list(no_div), no_div_py)))
        for op, name in (('+', 'add'), ('-', 'sub'), ('*', 'mul')):
            f = {'+': lambda a, b: a + b, '-': lambda a, b: a - b, '*': lambda a, b: a * b}[op]
            blocks.append(Block([('a', vs), ('b', vs)], [
                Stmt(op, f'a {op} b', f, t),
                Stmt(name + '_checked', 'c', f, t),
                Stmt(op + '=', 'x', f, t, pre=['let mut x = a;', f'x {op}= b;']),
            ], guard=([f'let c = @{name}_checked(a, b) catch {{', '    continue;', '};'],
                      lambda a, b, f=f: not in_range(f(a, b), t))))
        # compound assignments, in a variable and in an array element
        stmts = []
        for i, (n, e, f) in enumerate(ops[:6]):
            stmts.append(Stmt('compound', f'x{i}', f, t, pre=[f'let mut x{i} = a;', f'x{i} {n}= b;']))
        stmts.append(Stmt('compound', 'y[1]', lambda a, b: wrap_t(a * 3 + b, t), t,
                          pre=[f'let mut y: [2]{t} = [a, a];', 'y[1] *%= 3;', 'y[1] +%= b;']))
        blocks.append(Block([('a', vs), ('b', vs)], stmts))
        # a constant operand on either side
        stmts = []
        for k in consts_of(t):
            K = lit(k)
            for n, e, f in ops:
                stmts.append(Stmt(n + ' k', e.format(a='a', b=K), lambda a, f=f, k=k: f(a, k), t))
            for n, e, f in ops[:3] + ops[6:8]:
                stmts.append(Stmt('k ' + n, e.format(a=K, b='a'), lambda a, f=f, k=k: f(k, a), t))
            if k != 0:
                stmts.append(Stmt('% k', f'a % {K}', lambda a, k=k: trem(a, k), t))
                if not (signed and k == -1):
                    stmts.append(Stmt('/ k', f'a / {K}', lambda a, k=k: tdiv(a, k), t))
        blocks.append(Block([('a', vs)], stmts))
        blocks.append(Block([('a', vs)], [Stmt('k / %', f'{lit(k)} / a +% {lit(k)} % a',
                                                lambda a, k=k: wrap_t(tdiv(k, a) + trem(k, a), t), t)
                                           for k in consts_of(t)],
                            guard=([f'if a == 0 or a == -%1 {{', '    continue;', '}'],
                                   lambda a: a == 0 or a == wrap_t(-1, t))))
        p.fn(f'arith_{t}', blocks)
    p.write()


def clz(a, bits):
    u = a & ((1 << bits) - 1)
    return bits - u.bit_length()


def ctz(a, bits):
    u = a & ((1 << bits) - 1)
    if u == 0:
        return bits
    return (u & -u).bit_length() - 1


def popcount(a, bits):
    return bin(a & ((1 << bits) - 1)).count('1')


def cmp_bits(a, b):
    return f_cmp(a, b)


def cmp_expr(a, b):
    return (f'@as(u8, {a} == {b}) | (@as(u8, {a} != {b}) << 1) | (@as(u8, {a} < {b}) << 2) | '
            f'(@as(u8, {a} <= {b}) << 3) | (@as(u8, {a} > {b}) << 4) | (@as(u8, {a} >= {b}) << 5)')


CMP_EXPR = cmp_expr('a', 'b')
CMP_BRANCH = ['let mut c: u8 = 0;', 'if a == b {', '    c |= 1;', '}', 'if a != b {', '    c |= 2;', '}',
              'if a < b {', '    c |= 4;', '}', 'if a <= b {', '    c |= 8;', '}', 'if a > b {', '    c |= 16;', '}',
              'if a >= b {', '    c |= 32;', '}', 'if not (a < b) and a != b {', '    c |= 64;', '}']


def cmp_branch_py(a, b):
    return cmp_bits(a, b) | (64 if (not (a < b) and a != b) else 0)


def common_type(s, d):
    """The narrowest fixed type both @as convert to, or None."""
    for t in FIXED_NAMES:
        if lossless(s, t) and lossless(d, t):
            return t
    return None


def gen_unary():
    p = Prog('exact_int_unary.ax', [
        'Unary integer operations (-% ~ @clz @ctz @popcount, and - and @abs of all',
        'but MIN, which overflow) and comparisons of every integer type at the',
        'edges of its range: as values (@as of a bool), as branches, against',
        'constants, and between two types through the @as of both to a type that',
        'holds both.',
    ] + GEN)
    for t in FIXED_NAMES:
        bits, signed = ITYPES[t]
        lo, hi = rng_of(t)
        vs = V(p, t)
        blocks = []
        stmts = [
            Stmt('-%', '-%a', lambda a: wrap_t(-a, t), t),
            Stmt('~', '~a', lambda a: wrap_t(~a, t), t),
            Stmt('clz', '@clz(a)', lambda a: clz(a, bits), t),
            Stmt('ctz', '@ctz(a)', lambda a: ctz(a, bits), t),
            Stmt('popcount', '@popcount(a)', lambda a: popcount(a, bits), t),
            Stmt('bool', f'@as({t}, a == 0)', lambda a: int(a == 0), t),
        ]
        blocks.append(Block([('a', vs)], stmts))
        if signed:
            blocks.append(Block([('a', vs)], [
                Stmt('-', '-a', lambda a: -a, t),
                Stmt('abs', '@abs(a)', abs, t),
            ], guard=([f'if a == @min_value({t}) {{', '    continue;', '}'], lambda a: a == lo)))
        blocks.append(Block([('a', vs), ('b', vs)], [
            Stmt('cmp', CMP_EXPR, cmp_bits, 'u8'),
            Stmt('cmp branch', 'c', cmp_branch_py, 'u8', pre=CMP_BRANCH),
            Stmt('cmp bool', 'a < b', lambda a, b: a < b, 'bool'),
        ]))
        stmts = []
        for k in consts_of(t):
            stmts.append(Stmt('cmp k', cmp_expr('a', lit(k)), lambda a, k=k: cmp_bits(a, k), 'u8'))
            stmts.append(Stmt('k cmp', cmp_expr(lit(k), 'a'), lambda a, k=k: cmp_bits(k, a), 'u8'))
        blocks.append(Block([('a', vs)], stmts))
        p.fn(f'unary_{t}', blocks)
    for s in FIXED_NAMES:
        blocks = []
        for d in FIXED_NAMES:
            c = common_type(s, d)
            if d == s or c is None:
                continue
            e = cmp_expr(f'@as({c}, a)', f'@as({c}, b)')
            blocks.append(Block([('a', V(p, s)), ('b', V(p, d))], [Stmt(f'cmp {d}', e, cmp_bits, 'u8')]))
        p.fn(f'mixed_cmp_{s}', blocks)
    p.write()


def fold_values(t):
    lo, hi = rng_of(t)
    _, signed = ITYPES[t]
    if signed:
        return [lo, -1, hi]
    return [1, hi // 2 + 1, hi]


def gen_fold():
    p = Prog('exact_int_fold.ax', [
        'Integer operations on constants, which the compiler evaluates (spec §7.6:',
        'in const initializers, and in the body of a function, here as operands',
        'of the hashing): the operators, the builtins, the conversions, of edge',
        'values of every integer type.',
    ] + GEN)
    nconst = [0]
    for t in FIXED_NAMES:
        bits, signed = ITYPES[t]
        lo, hi = rng_of(t)
        names = []
        for i, v in enumerate(fold_values(t)):
            n = f'C_{t.upper()}_{i}'
            p.const(f'const {n}: {t} = {lit(v)};')
            names.append((n, v))
        ops = [(n, e, f) for n, e, f in int_bin_ops(t)]
        ops += [('/', '{a} / {b}', lambda a, b: tdiv(a, b) if b != 0 and not (a == lo and b == -1) else None),
                ('%', '{a} % {b}', lambda a, b: trem(a, b) if b != 0 else None)]
        stmts = []
        const_ok = ('+%', '-%', '*%', '&', '|', '^', '/', '%')
        for (na, a), (nb, b) in itertools.product(names, names):
            for n, e, f in ops:
                r = f(a, b)
                if r is None:
                    continue
                expr = e.format(a=na, b=nb)
                stmts.append(Stmt(n, expr, lambda r=r: r, t))
                if n in const_ok:
                    nconst[0] += 1
                    cn = f'E{nconst[0]}'
                    p.const(f'const {cn}: {t} = {expr};')
                    stmts.append(Stmt('const ' + n, cn, lambda r=r: r, t))
        for na, a in names:
            for k in dedupe([0, 1, bits - 1, bits, bits + 3]):
                stmts.append(Stmt('<<', f'{na} << {k}', lambda r=shl(a, k, t): r, t))
                stmts.append(Stmt('>>', f'{na} >> {k}', lambda r=shr(a, k, t): r, t))
                nconst[0] += 1
                p.const(f'const E{nconst[0]}: {t} = {na} << {k};')
                stmts.append(Stmt('const <<', f'E{nconst[0]}', lambda r=shl(a, k, t): r, t))
                nconst[0] += 1
                p.const(f'const E{nconst[0]}: {t} = {na} >> {k};')
                stmts.append(Stmt('const >>', f'E{nconst[0]}', lambda r=shr(a, k, t): r, t))
            for e, r in [(f'-%{na}', wrap_t(-a, t)), (f'~{na}', wrap_t(~a, t))]:
                stmts.append(Stmt('unary', e, lambda r=r: r, t))
                nconst[0] += 1
                p.const(f'const E{nconst[0]}: {t} = {e};')
                stmts.append(Stmt('const unary', f'E{nconst[0]}', lambda r=r: r, t))
            for e, r in [(f'@clz({na})', clz(a, bits)), (f'@ctz({na})', ctz(a, bits)),
                         (f'@popcount({na})', popcount(a, bits)), (f'@rotl({na}, @as(u32, 3))', rot(a, 3, t, True)),
                         (f'@rotr({na}, @as(u8, 5))', rot(a, 5, t, False))]:
                stmts.append(Stmt('bits', e, lambda r=r: r, t))
            for d in INT_NAMES:
                if d in PTRSIZED:
                    continue
                db, ds = ITYPES[d]
                if lossless(t, d):
                    stmts.append(Stmt('as', f'@as({d}, {na})', lambda r=a: r, d))
                if db <= bits:
                    stmts.append(Stmt('truncate', f'@truncate({d}, {na})', lambda r=wrap(a, db, ds): r, d))
                if in_range(a, d):
                    stmts.append(Stmt('int_cast', f'@int_cast({d}, {na})', lambda r=a: r, d))
            stmts.append(Stmt('bit_cast', f'@bit_cast({tname(bits, not signed)}, {na})',
                              lambda r=wrap(a, bits, not signed): r, tname(bits, not signed)))
        for (na, a), (nb, b) in itertools.product(names, names):
            e = cmp_expr(na, nb)
            stmts.append(Stmt('cmp', e, lambda r=cmp_bits(a, b): r, 'u8'))
        p.fn(f'fold_{t}', [Block([], stmts)])
    p.write()


# ---- float programs --------------------------------------------------------

FTYPES = ['f32', 'f64']


def F(p, t):
    it = 'u32' if t == 'f32' else 'u64'
    return p.array('F_' + t.upper(), it, float_values(t), load=t)


def round_int_to(v, t):
    return round_frac(Fraction(v), t)


def int_from_float(x, d):
    if math.isnan(x):
        return 0
    lo, hi = rng_of(d)
    if math.isinf(x):
        return lo if x < 0 else hi
    return clamp(math.trunc(Fraction(x)), d)


def float_cast(x):
    if math.isnan(x) or math.isinf(x) or x == 0:
        return x
    r = round_frac(Fraction(x), 'f32')
    if r == 0:
        return signed_zero(x < 0)
    return r


def as_float_ok(s, d):
    bits, signed = ITYPES[s]
    if s in PTRSIZED:
        return False
    p = FMT[d][0]
    return bits - (1 if signed else 0) <= p


def ftoi_values(f, d):
    """Bit patterns of float values near the limits of integer type d."""
    lo, hi = rng_of(d)
    vs = []
    for b in (lo, hi):
        for off in (Fraction(-1), Fraction(-1, 2), Fraction(0), Fraction(1, 2), Fraction(1), Fraction(2)):
            vs.append(round_frac(Fraction(b) + off, f))
    vs += [round_frac(Fraction(hi) * 2, f), round_frac(Fraction(lo) * 2, f), 0.9, -0.9, 0.0, -0.0]
    out = []
    for v in vs:
        bb = f_bits(v, f)
        if bb not in out:
            out.append(bb)
    return out


def gen_float_conv():
    p = Prog('exact_float_conv.ax', [
        'Conversions between floats and integers at the edges of the ranges:',
        '@float_from_int (round to nearest, ties to even) from every integer type,',
        '@as from the integer types a float holds exactly and from f32 to f64,',
        '@int_from_float to every integer type (toward zero, saturating; NaN is 0),',
        '@float_cast from f64 to f32 (rounding, overflow to infinity, subnormals),',
        'and @bit_cast between the floats and the integers of their size.',
    ] + GEN)
    for s in FIXED_NAMES:
        vs = V(p, s)
        stmts = []
        for d in FTYPES:
            stmts.append(Stmt(f'float_from_int {d}', f'@float_from_int({d}, a)',
                              lambda a, d=d: round_int_to(a, d), d))
            if as_float_ok(s, d):
                stmts.append(Stmt(f'as {d}', f'@as({d}, a)', lambda a, d=d: round_int_to(a, d), d))
        bits, _ = ITYPES[s]
        if bits in (32, 64):
            d = 'f32' if bits == 32 else 'f64'
            stmts.append(Stmt(f'bit_cast {d}', f'@bit_cast({d}, a)',
                              lambda a, d=d, bits=bits: f_from_bits(a & ((1 << bits) - 1), d), d))
        p.fn(f'from_{s}', [Block([('a', vs)], stmts)])
    for f in FTYPES:
        fa = F(p, f)
        blocks = []
        for d in FIXED_NAMES:
            arr = p.array(f'FI_{f.upper()}_{d.upper()}', 'u32' if f == 'f32' else 'u64', ftoi_values(f, d), load=f)
            for a in (fa, arr):
                blocks.append(Block([('x', a)], [Stmt(f'int_from_float {d}', f'@int_from_float({d}, x)',
                                                       lambda x, d=d: int_from_float(x, d), d)]))
        it = 'u32' if f == 'f32' else 'u64'
        bits = 32 if f == 'f32' else 64
        stmts = [Stmt('bit_cast', f'@bit_cast({it}, x)', lambda x, f=f: f_bits(x, f), it),
                 Stmt('bit_cast i', f'@bit_cast({tname(bits, True)}, x)',
                      lambda x, f=f, bits=bits: wrap(f_bits(x, f), bits, True), tname(bits, True))]
        if f == 'f32':
            stmts.append(Stmt('as f64', '@as(f64, x)', lambda x: x, 'f64'))
        else:
            stmts.append(Stmt('float_cast', '@float_cast(f32, x)', float_cast, 'f32'))
        blocks.append(Block([('x', fa)], stmts))
        p.fn(f'to_int_{f}', blocks)
    # f64 values near the rounding points of f32
    near = []
    for e in (0, 1, -126, -127, -149, -150, 127, 128):
        for m in (1, 1 + 2.0 ** -24, 1 + 2.0 ** -23, 1 + 3 * 2.0 ** -24, 2 - 2.0 ** -24, 2 - 2.0 ** -25,
                  1 + 2.0 ** -24 + 2.0 ** -50):
            v = math.ldexp(m, e)
            if not math.isinf(v):
                for sg in (1, -1):
                    b = f_bits(sg * v, 'f64')
                    if b not in near:
                        near.append(b)
    arr = p.array('FC_F64', 'u64', near, load='f64')
    p.fn('float_cast_near', [Block([('x', arr)], [Stmt('float_cast', '@float_cast(f32, x)', float_cast, 'f32')])])
    p.write()


def gen_float_arith():
    p = Prog('exact_float_arith.ax', [
        'IEEE 754 arithmetic (spec §7.7: round to nearest, ties to even; no',
        'contraction) on f32 and f64, of every pair (triple for @fma and a * b + c)',
        'of edge values: zeros, ones, values that round, subnormals, the extremes,',
        'infinities and NaN. + - * / @sqrt @fma, unary -, @abs, @min/@max (a NaN',
        'operand gives NaN; -0 < +0), @copysign, @floor @ceil @trunc @nearest and the',
        'comparisons each have one result, computed in Python with rationals.',
    ] + GEN)
    for t in FTYPES:
        fa = F(p, t)
        blocks = []
        blocks.append(Block([('a', fa)], [
            Stmt('neg', '-a', lambda a: -a, t),
            Stmt('abs', '@abs(a)', abs, t),
            Stmt('sqrt', '@sqrt(a)', lambda a: f_sqrt(a, t), t),
            Stmt('floor', '@floor(a)', lambda a: f_round(a, 'floor'), t),
            Stmt('ceil', '@ceil(a)', lambda a: f_round(a, 'ceil'), t),
            Stmt('trunc', '@trunc(a)', lambda a: f_round(a, 'trunc'), t),
            Stmt('nearest', '@nearest(a)', lambda a: f_round(a, 'nearest'), t),
        ]))
        blocks.append(Block([('a', fa), ('b', fa)], [
            Stmt('+', 'a + b', lambda a, b: f_add(a, b, t), t),
            Stmt('-', 'a - b', lambda a, b: f_sub(a, b, t), t),
            Stmt('*', 'a * b', lambda a, b: f_mul(a, b, t), t),
            Stmt('/', 'a / b', lambda a, b: f_div(a, b, t), t),
            Stmt('min', '@min(a, b)', lambda a, b: f_minmax(a, b, False), t),
            Stmt('max', '@max(a, b)', lambda a, b: f_minmax(a, b, True), t),
            Stmt('copysign', '@copysign(a, b)', lambda a, b: math.copysign(a, b) if not math.isnan(a) else NAN, t),
            Stmt('cmp', CMP_EXPR, f_cmp, 'u8'),
            Stmt('cmp branch', 'c', cmp_branch_py, 'u8', pre=CMP_BRANCH),
        ]))
        small = p.array('G_' + t.upper(), 'u32' if t == 'f32' else 'u64', float_values(t)[::2], load=t)
        blocks.append(Block([('a', small), ('b', small), ('c', small)], [
            Stmt('fma', '@fma(a, b, c)', lambda a, b, c: f_fma(a, b, c, t), t),
            Stmt('mul add', 'a * b + c', lambda a, b, c: f_add(f_mul(a, b, t), c, t), t),
        ]))
        p.fn(f'arith_{t}', blocks)
    p.write()


def selfcheck():
    """The rational model agrees with the host's binary64 arithmetic."""
    vals = [f_from_bits(b, 'f64') for b in float_values('f64')]
    for a in vals:
        for b in vals:
            for f, g in ((f_add, lambda x, y: x + y), (f_sub, lambda x, y: x - y), (f_mul, lambda x, y: x * y)):
                r, s = f(a, b, 'f64'), g(a, b)
                assert (math.isnan(r) and math.isnan(s)) or f_bits(r, 'f64') == f_bits(s, 'f64'), (a, b, r, s)
            if b != 0:
                r, s = f_div(a, b, 'f64'), a / b
                assert (math.isnan(r) and math.isnan(s)) or f_bits(r, 'f64') == f_bits(s, 'f64'), (a, b, r, s)
        if a >= 0:
            r, s = f_sqrt(a, 'f64'), math.sqrt(a)
            assert f_bits(r, 'f64') == f_bits(s, 'f64'), (a, r, s)
    for a in vals:
        for b in vals[::3]:
            for c in vals[::3]:
                if hasattr(math, 'fma') and not (math.isinf(a) or math.isinf(b) or math.isinf(c)):
                    try:
                        s = math.fma(a, b, c)
                    except (OverflowError, ValueError):
                        continue
                    r = f_fma(a, b, c, 'f64')
                    assert (math.isnan(r) and math.isnan(s)) or f_bits(r, 'f64') == f_bits(s, 'f64'), (a, b, c)
    # f32 results through binary64 (double rounding is innocuous for these)
    vals = [f_from_bits(b, 'f32') for b in float_values('f32')]
    for a in vals:
        for b in vals:
            s = a + b
            if not math.isnan(s) and not math.isinf(s):
                r = f_add(a, b, 'f32')
                try:
                    q = struct.unpack('<f', struct.pack('<f', s))[0]
                except OverflowError:
                    q = math.copysign(INF, s)
                assert f_bits(r, 'f32') == f_bits(q, 'f32'), (a, b)


def main():
    selfcheck()
    gen_conv()
    gen_cast_trap()
    gen_shift()
    gen_arith()
    gen_unary()
    gen_fold()
    gen_float_conv()
    gen_float_arith()


main()
