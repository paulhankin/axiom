#!/usr/bin/env python3
"""Writes tests of Axiom's vector operations (spec §5.8, §13, Appendix A.5)
whose expected outputs Python computes lane by lane from the
specification's rules, not from the compiler, with the scalar models of
gen_exact_ops.py (integer lanes wrap; float lanes are IEEE 754 binary32 or
binary64, rounded to nearest, ties to even).

    python3 cmd/axiom/internal/tests/tools/gen_exact_simd.py
    cmd/axiom/build/axiom fmt cmd/axiom/internal/tests/run/exact_simd_*.ax

writes into cmd/axiom/internal/tests/run/ (the same files every time):

- exact_simd_int.ax: on every integer vector type of 128 and 256 bits, and
  of 512 bits for the lane-wise operators, rows of edge values (MIN, MAX,
  0, +-1, powers of two and their neighbours, pseudo-random ones) and every
  pair of rows: + - * and their % spellings (wrapping), & | ^ ~, unary -
  and -%, << >> by a scalar count of each unsigned type and by a vector of
  counts (at and past the lane width), @add_sat @sub_sat (8- and 16-bit
  lanes), @min @max (not 64-bit), @abs (wrapping), @clz @ctz @popcount, the
  comparisons (@lanes_eq ... masks), @select (with a mask and with any
  bits), @any_true @all_true @bitmask, @reduce_add, @widen_low and
  @widen_high, and @float_from_int to the float vectors of as many lanes.
- exact_simd_float.ax: on f32x4 f64x2 f32x8 f64x4 f32x16 f64x8, rows of
  edge values (zeros, subnormals, extremes, infinities, NaN): + - * /,
  @min @max (NaN, and -0 < +0), unary -, @abs, @sqrt, @floor @ceil @trunc
  @nearest, @fma (and a * b + c, not contracted), the comparisons,
  @select, @reduce_add and @dot (the halves folded, §13.2), and
  @int_from_float to the integer vectors of as many lanes (truncation,
  saturation, NaN to 0).
- exact_simd_lanes.ax: @shuffle with constant indices (results of 128, 256
  and 512 bits), @swizzle and @swizzle2 (indices taken as unsigned; out of
  range gives 0), @narrow (saturating, by the source's own type), @splat,
  @vec, @extract_lane, @replace_lane, @load_partial and @store_partial (of
  lengths from 0 to the lanes), and the @dot of i16 lanes.
- exact_simd_exprs_<n>.ax: random trees of vector operations over vector
  and scalar variables (a dozen value sets from memory), with results
  hashed lane by lane, and the same trees over constant leaves (@vec of
  literals), which the compiler may fold.

Each output line is a hash of the results of one operation over all its
operands (exprs: of a group of trees over all value sets); an integer
vector is hashed by its bytes, a float vector lane by lane with NaN as one
NaN (payloads are unspecified, §7.7).

    gen_exact_simd.py --seeds=<first>:<count> <dir>

writes more random programs (exact_simd_exprs_s<seed>.ax) into <dir>;
`--expand` writes one output line per tree and value set.
"""
import itertools
import math
import os
import random
import sys
from fractions import Fraction

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from gen_exact_ops import (HELPERS, NAN, edge_values, f_add, f_bits, f_div, f_fma, f_from_bits,  # noqa: E402
                           f_minmax, f_mul, f_round, f_sqrt, f_sub, float_values, mix, round_frac)

HERE = os.path.dirname(os.path.abspath(__file__))
RUN = os.path.join(HERE, '..', 'run')
M64 = (1 << 64) - 1

ILANES = ['i8', 'u8', 'i16', 'u16', 'i32', 'u32', 'i64', 'u64']
FLANES = ['f32', 'f64']
WIDTH = {'i8': 8, 'u8': 8, 'i16': 16, 'u16': 16, 'i32': 32, 'u32': 32, 'i64': 64, 'u64': 64, 'f32': 32, 'f64': 64}


def vname(lane, bits):
    return f'{lane}x{bits // WIDTH[lane]}'


VT = {}  # name -> (lane, lanes, bits)
for _bits in (128, 256, 512):
    for _l in ILANES + FLANES:
        VT[vname(_l, _bits)] = (_l, _bits // WIDTH[_l], _bits)


def lane(v):
    return VT[v][0]


def nlanes(v):
    return VT[v][1]


def vbits(v):
    return VT[v][2]


def is_float(t):
    return t in FLANES or (t in VT and lane(t) in FLANES)


def signed(lt):
    return lt[0] == 'i'


def lrange(lt):
    w = WIDTH[lt]
    if signed(lt):
        return -(1 << (w - 1)), (1 << (w - 1)) - 1
    return 0, (1 << w) - 1


def wrap(x, lt):
    w = WIDTH[lt]
    x &= (1 << w) - 1
    if signed(lt) and x >> (w - 1):
        x -= 1 << w
    return x


def sat(x, lt):
    lo, hi = lrange(lt)
    return max(lo, min(hi, x))


def unsigned_of(lt):
    return 'u' + lt[1:]


def shl(x, k, lt):
    return wrap(x << k, lt) if k < WIDTH[lt] else 0


def shr(x, k, lt):
    if k >= WIDTH[lt]:
        return -1 if x < 0 else 0
    return x >> k


def clz(x, lt):
    w = WIDTH[lt]
    return w - (x & ((1 << w) - 1)).bit_length()


def ctz(x, lt):
    w = WIDTH[lt]
    u = x & ((1 << w) - 1)
    return w if u == 0 else (u & -u).bit_length() - 1


def popcount(x, lt):
    return bin(x & ((1 << WIDTH[lt]) - 1)).count('1')


def f_neg(x):
    return NAN if math.isnan(x) else -x


def f_abs(x):
    return NAN if math.isnan(x) else abs(x)


def fold_halves(vals, add):
    """@reduce_add's order (§13.2): lane i plus lane i + n/2, until one."""
    v = list(vals)
    while len(v) > 1:
        h = len(v) // 2
        v = [add(v[i], v[i + h]) for i in range(h)]
    return v[0]


def int_from_float(x, lt):
    if math.isnan(x):
        return 0
    lo, hi = lrange(lt)
    if math.isinf(x):
        return lo if x < 0 else hi
    return max(lo, min(hi, math.trunc(Fraction(x))))


def float_from_int(x, ft):
    return round_frac(Fraction(x), ft) if x != 0 else 0.0


def mask_of(b, lt):
    return wrap(-1, lt) if b else 0


def fcmp(op, a, b):
    return {'eq': a == b, 'ne': a != b, 'lt': a < b, 'le': a <= b, 'gt': a > b, 'ge': a >= b}[op]


CMPS = ['eq', 'ne', 'lt', 'le', 'gt', 'ge']


# ---- hashing --------------------------------------------------------------

def canon(x, ft):
    if math.isnan(x):
        return 0x7fc00000 if ft == 'f32' else 0x7ff8000000000000
    return f_bits(x, ft)


def hash_vec(h, vals, vt):
    lt = lane(vt)
    if lt in FLANES:
        for x in vals:
            h = mix(h, canon(x, lt))
        return h
    w = WIDTH[lt]
    word, fill = 0, 0
    for x in vals:
        word |= (x & ((1 << w) - 1)) << fill
        fill += w
        if fill == 64:
            h = mix(h, word)
            word, fill = 0, 0
    assert fill == 0
    return h


def hash_scalar(h, x, t):
    if t in FLANES:
        return mix(h, canon(x, t))
    if t == 'bool':
        return mix(h, int(x))
    return mix(h, x & M64)


def hash_any(h, x, t):
    return hash_vec(h, x, t) if t in VT else hash_scalar(h, x, t)


def vec_helpers(used):
    out = []
    for bits in (128, 256, 512):
        k = bits // 64
        out += [f'fn hb{bits}(h0: u64, v: u64x{k}) u64 {{', '    let mut h = h0;']
        out += [f'    h = mix(h, @extract_lane(v, {i}));' for i in range(k)]
        out += ['    return h;', '}', '']
    for vt in sorted(used):
        lt = lane(vt)
        out += [f'fn hv_{vt}(h0: u64, v: {vt}) u64 {{', '    let mut h = h0;']
        out += [f'    h = h_{lt}(h, @extract_lane(v, {i}));' for i in range(nlanes(vt))]
        out += ['    return h;', '}', '']
    return out


def hash_line(hv, expr, t, used):
    if t in VT and not is_float(t):
        b = vbits(t)
        return f'{hv} = hb{b}({hv}, @bit_cast(u64x{b // 64}, {expr}));'
    if t in VT:
        used.add(t)
        return f'{hv} = hv_{t}({hv}, {expr});'
    return f'{hv} = h_{t}({hv}, {expr});'


# ---- the operation tests ----------------------------------------------------

class Stmt:
    def __init__(self, key, expr, py, rtype, pre=()):
        self.key, self.expr, self.py, self.rtype, self.pre = key, expr, py, rtype, list(pre)


class Prog:
    def __init__(self, name, comment):
        self.name, self.comment = name, comment
        self.arrays = {}  # name -> (storage type, [ints], [python values], kind)
        self.fns, self.calls, self.used = [], [], set()

    def rows(self, name, vt, rows):
        """A global array of the lanes of `rows` (lists), loaded as vectors of
        type vt (float lanes kept as their bits)."""
        lt = lane(vt)
        st = ('u32' if lt == 'f32' else 'u64') if lt in FLANES else lt
        flat = [f_bits(x, lt) if lt in FLANES else x for r in rows for x in r]
        self.arrays[name] = (st, flat, [list(r) for r in rows], ('vec', vt))
        return name

    def scalars(self, name, t, vals):
        st = ('u32' if t == 'f32' else 'u64') if t in FLANES else t
        flat = [f_bits(x, t) if t in FLANES else x for x in vals]
        self.arrays[name] = (st, flat, list(vals), ('scal', t))
        return name

    def fn(self, name, loops, stmts, guard=None):
        """loops: [(variable, array)], outermost first; the statements run
        in the innermost loop, unless guard(values) says to skip them."""
        keys = []
        for s in stmts:
            if s.key not in keys:
                keys.append(s.key)
        hv = {k: f'h{i}' for i, k in enumerate(keys)}
        expect = {k: 0 for k in keys}
        lines = [f'fn {name}() void {{']
        lines += [f'    let mut {hv[k]}: u64 = 0;' for k in keys]
        ind = '    '
        for (var, arr), idx in zip(loops, 'ijklm'):
            kind, t = self.arrays[arr][3]
            n = nlanes(t) if kind == 'vec' else 1
            lines.append(f'{ind}for {idx} in 0..{arr}.len{f" / {n}" if n > 1 else ""} {{')
            ind += '    '
        for (var, arr), idx in zip(loops, 'ijklm'):
            kind, t = self.arrays[arr][3]
            if kind == 'vec':
                n = nlanes(t)
                ld = f'@load({t}, {arr}[{idx} * {n}..])' if not is_float(t) else \
                    f'@bit_cast({t}, @load({"u32" if lane(t) == "f32" else "u64"}x{n}, {arr}[{idx} * {n}..]))'
            else:
                ld = f'@bit_cast({t}, {arr}[{idx}])' if t in FLANES else f'{arr}[{idx}]'
            lines.append(f'{ind}let {var} = {ld};')
        if guard:
            lines += [ind + g for g in guard[0]]
        for s in stmts:
            lines += [ind + p for p in s.pre]
            lines.append(ind + hash_line(hv[s.key], s.expr, s.rtype, self.used))
        for _ in loops:
            ind = ind[:-4]
            lines.append(f'{ind}}}')
        for vals in itertools.product(*[self.arrays[arr][2] for _, arr in loops]):
            if guard and guard[1](*vals):
                continue
            for s in stmts:
                v = s.py(*vals)
                expect[s.key] = hash_any(expect[s.key], v, s.rtype)
        lines += [f'    io.uint({hv[k]});' for k in keys]
        lines.append('}')
        self.fns.append('\n'.join(lines))
        self.calls.append((name, [expect[k] for k in keys]))

    def write(self):
        out = ['// ' + l if l else '//' for l in self.comment]
        out += ['import "../lib/io.ax" as io;', '']
        for name, (st, flat, _, _) in self.arrays.items():
            out.append(f'let mut {name}: [{len(flat)}]{st} = [{", ".join(str(x) for x in flat)}];')
        out.append('')
        body = []
        for f in self.fns:
            body += [f, '']
        out.append(HELPERS)
        out += vec_helpers(self.used)
        out += body
        out.append('export fn run() void {')
        for name, hs in self.calls:
            out.append(f'    {name}();  // expect: {hs[0]}')
            out += [f'    // expect: {h}' for h in hs[1:]]
        out.append('}')
        with open(os.path.join(RUN, self.name), 'w') as f:
            f.write('\n'.join(out) + '\n')


GEN = ['(Generated by cmd/axiom/internal/tests/tools/gen_exact_simd.py, with the',
       'expectations computed lane by lane in Python, from the rules of the spec.)']

rnd = random.Random(20261124)


def lane_values(lt):
    if lt in FLANES:
        return [f_from_bits(b, lt) for b in float_values(lt)]
    return edge_values(lt)


def make_rows(lt, n, count):
    vs = lane_values(lt)
    out = []
    pool = []
    for _ in range(count):
        row = []
        for _ in range(n):
            if not pool:
                pool = list(vs)
                rnd.shuffle(pool)
            row.append(pool.pop())
        out.append(row)
    # a row that shares some lanes with the first, for equal lanes
    out.append([out[0][i] if i % 2 == 0 else out[1][i] for i in range(n)])
    return out


def rows_of(p, vt, count=None):
    name = 'R_' + vt.upper()
    if name not in p.arrays:
        n = nlanes(vt)
        if count is None:
            count = 6 if n <= 16 else 4
        p.rows(name, vt, make_rows(lane(vt), n, count))
    return name


def lanewise(f, *vs):
    return [f(*xs) for xs in zip(*vs)]


def int_binary_stmts(vt):
    lt = lane(vt)
    w = WIDTH[lt]
    out = []
    for op in ['+', '-', '+%', '-%']:
        f = (lambda a, b: a + b) if op[0] == '+' else (lambda a, b: a - b)
        out.append(Stmt(op, f'a {op} b', lambda a, b, f=f: lanewise(lambda x, y: wrap(f(x, y), lt), a, b), vt))
    if w > 8:
        for op in ['*', '*%']:
            out.append(Stmt(op, f'a {op} b', lambda a, b: lanewise(lambda x, y: wrap(x * y, lt), a, b), vt))
    for op, f in (('&', lambda x, y: x & y), ('|', lambda x, y: x | y), ('^', lambda x, y: x ^ y)):
        out.append(Stmt(op, f'a {op} b', lambda a, b, f=f: lanewise(f, a, b), vt))
    if w <= 16:
        out.append(Stmt('add_sat', '@add_sat(a, b)', lambda a, b: lanewise(lambda x, y: sat(x + y, lt), a, b), vt))
        out.append(Stmt('sub_sat', '@sub_sat(a, b)', lambda a, b: lanewise(lambda x, y: sat(x - y, lt), a, b), vt))
    if w < 64:
        out.append(Stmt('min', '@min(a, b)', lambda a, b: lanewise(min, a, b), vt))
        out.append(Stmt('max', '@max(a, b)', lambda a, b: lanewise(max, a, b), vt))
    for c in CMPS:
        out.append(Stmt(c, f'@lanes_{c}(a, b)', lambda a, b, c=c: lanewise(lambda x, y: mask_of(fcmp(c, x, y), lt), a, b),
                        vt))
    out.append(Stmt('select lt', '@select(@lanes_lt(a, b), a, b)', lambda a, b: lanewise(min, a, b), vt))
    m = (1 << w) - 1
    out.append(Stmt('select bits', '@select(a, b, ~b)',
                    lambda a, b: lanewise(lambda x, y: wrap((x & y) | (~x & ~y & m), lt), a, b), vt))
    uv = vname(unsigned_of(lt), vbits(vt))
    cnt = f'@bit_cast({uv}, b)'
    for op, f in (('<<', shl), ('>>', shr)):
        out.append(Stmt(f'{op} vec', f'a {op} {cnt}',
                        lambda a, b, f=f: lanewise(lambda x, y: f(x, y & m, lt), a, b), vt))
        msk = 2 * w - 1
        out.append(Stmt(f'{op} vec masked', f'a {op} ({cnt} & @splat({msk}))',
                        lambda a, b, f=f, msk=msk: lanewise(lambda x, y: f(x, y & msk, lt), a, b), vt))
    return out


def int_unary_stmts(vt):
    lt = lane(vt)
    w = WIDTH[lt]
    n = nlanes(vt)
    out = [Stmt('-%', '-%a', lambda a: lanewise(lambda x: wrap(-x, lt), a), vt),
           Stmt('~', '~a', lambda a: lanewise(lambda x: wrap(~x, lt), a), vt)]
    if signed(lt):
        out.append(Stmt('neg', '-a', lambda a: lanewise(lambda x: wrap(-x, lt), a), vt))
        out.append(Stmt('abs', '@abs(a)', lambda a: lanewise(lambda x: wrap(abs(x), lt), a), vt))
    for name, f in (('clz', clz), ('ctz', ctz), ('popcount', popcount)):
        out.append(Stmt(name, f'@{name}(a)', lambda a, f=f: lanewise(lambda x: f(x, lt), a), vt))
    out.append(Stmt('reduce_add', '@reduce_add(a)', lambda a: wrap(sum(a), lt), lt))
    bt = 'u64' if n == 64 else 'u32'
    out.append(Stmt('bitmask', '@bitmask(a)', lambda a: sum(1 << i for i, x in enumerate(a) if x < 0 or
                                                          (x >> (w - 1)) & 1), bt))
    out.append(Stmt('any_true', '@any_true(a)', lambda a: any(x != 0 for x in a), 'bool'))
    out.append(Stmt('all_true', '@all_true(a)', lambda a: all(x != 0 for x in a), 'bool'))
    if w <= 32 and vbits(vt) <= 256:
        wt = vname(lt[0] + str(2 * w), vbits(vt))
        out.append(Stmt('widen_low', '@widen_low(a)', lambda a: a[:n // 2], wt))
        out.append(Stmt('widen_high', '@widen_high(a)', lambda a: a[n // 2:], wt))
    for ft in FLANES:
        fv = f'{ft}x{n}'
        if fv in VT:
            out.append(Stmt(f'float_from_int {ft}', f'@float_from_int({fv}, a)',
                            lambda a, ft=ft: lanewise(lambda x: float_from_int(x, ft), a), fv))
    return out


def shift_counts(w):
    return sorted({0, 1, 2, w - 1, w, w + 1, 2 * w, 63, 64, 65, 200, 255})


def gen_int():
    p = Prog('exact_simd_int.ax', [
        'Lane-wise integer vector operations (spec §13.1, §13.2, Appendix A.5) on',
        'every integer vector type, over rows of edge values and every pair of',
        'rows: wrapping + - *, bitwise, shifts by scalar and vector counts (at and',
        'past the lane width: 0 or the sign), saturating, @min @max @abs, @clz',
        '@ctz @popcount, masks, @select, @any_true @all_true @bitmask, @reduce_add,',
        '@widen_low/@widen_high and @float_from_int. Each line hashes the results',
        'of one operation (an integer vector by its bytes).',
    ] + GEN)
    for bits in (128, 256, 512):
        for lt in ILANES:
            vt = vname(lt, bits)
            w = WIDTH[lt]
            arr = rows_of(p, vt)
            stmts = int_binary_stmts(vt)
            if bits == 512:
                stmts = [s for s in stmts if s.key in ('+', '-', '*', '^', 'min', 'lt', 'select lt', '<< vec',
                                                       '>> vec masked')]
            p.fn(f'bin_{vt}', [('a', arr), ('b', arr)], stmts)
            stmts = int_unary_stmts(vt)
            if bits == 512:
                stmts = [s for s in stmts if s.key in ('-%', 'abs', 'popcount', 'clz', 'reduce_add', 'bitmask',
                                                       'all_true')]
            p.fn(f'un_{vt}', [('a', arr)], stmts)
            if bits == 512:
                continue
            stmts = []
            for ct in ('u8', 'u32', 'u64'):
                cn = p.scalars(f'K_{ct.upper()}_{w}', ct, [k for k in shift_counts(w) if k <= lrange(ct)[1]])
                for op, f in (('<<', shl), ('>>', shr)):
                    p.fn(f'sh_{vt}_{ct}_{"l" if op == "<<" else "r"}', [('a', arr), ('k', cn)],
                         [Stmt(op, f'a {op} k', lambda a, k, f=f: lanewise(lambda x: f(x, k, lt), a), vt)])
    p.write()


def gen_float():
    p = Prog('exact_simd_float.ax', [
        'Lane-wise float vector operations (spec §13.1, §13.2, §7.7) on f32x4 f64x2',
        'f32x8 f64x4 f32x16 f64x8, over rows of edge values (zeros, subnormals,',
        'the extremes, infinities, NaN) and every pair (triple for @fma and the',
        'uncontracted a * b + c): + - * /, @min @max, unary -, @abs, @sqrt, the',
        'rounding builtins, masks, @select, @reduce_add and @dot (the halves',
        'folded), and @int_from_float. Each line hashes the results of one',
        'operation, NaN as one NaN.',
    ] + GEN)
    for bits in (128, 256, 512):
        for ft in FLANES:
            vt = vname(ft, bits)
            n = nlanes(vt)
            arr = rows_of(p, vt, 8)
            it = 'i32' if ft == 'f32' else 'i64'
            mt = vname(it, bits)
            fadd = lambda x, y: f_add(x, y, ft)  # noqa: E731
            stmts = [
                Stmt('+', 'a + b', lambda a, b: lanewise(lambda x, y: f_add(x, y, ft), a, b), vt),
                Stmt('-', 'a - b', lambda a, b: lanewise(lambda x, y: f_sub(x, y, ft), a, b), vt),
                Stmt('*', 'a * b', lambda a, b: lanewise(lambda x, y: f_mul(x, y, ft), a, b), vt),
                Stmt('/', 'a / b', lambda a, b: lanewise(lambda x, y: f_div(x, y, ft), a, b), vt),
                Stmt('min', '@min(a, b)', lambda a, b: lanewise(lambda x, y: f_minmax(x, y, False), a, b), vt),
                Stmt('max', '@max(a, b)', lambda a, b: lanewise(lambda x, y: f_minmax(x, y, True), a, b), vt),
                Stmt('select', '@select(@lanes_lt(a, b), a, b)',
                     lambda a, b: lanewise(lambda x, y: x if x < y else y, a, b), vt),
                Stmt('dot', '@dot(a, b)',
                     lambda a, b: fold_halves(lanewise(lambda x, y: f_mul(x, y, ft), a, b), fadd), ft),
            ]
            for c in CMPS:
                stmts.append(Stmt(c, f'@lanes_{c}(a, b)',
                                  lambda a, b, c=c: lanewise(lambda x, y: mask_of(fcmp(c, x, y), it), a, b), mt))
            p.fn(f'bin_{vt}', [('a', arr), ('b', arr)], stmts)
            stmts = [
                Stmt('neg', '-a', lambda a: lanewise(f_neg, a), vt),
                Stmt('abs', '@abs(a)', lambda a: lanewise(f_abs, a), vt),
                Stmt('sqrt', '@sqrt(a)', lambda a: lanewise(lambda x: f_sqrt(x, ft), a), vt),
                Stmt('reduce_add', '@reduce_add(a)', lambda a: fold_halves(a, fadd), ft),
            ]
            for mode in ('floor', 'ceil', 'trunc', 'nearest'):
                stmts.append(Stmt(mode, f'@{mode}(a)', lambda a, mode=mode: lanewise(lambda x: f_round(x, mode), a), vt))
            for il in ILANES:
                iv = f'{il}x{n}'
                if iv in VT:
                    stmts.append(Stmt(f'int_from_float {il}', f'@int_from_float({iv}, a)',
                                      lambda a, il=il: lanewise(lambda x: int_from_float(x, il), a), iv))
            p.fn(f'un_{vt}', [('a', arr)], stmts)
            small = 'S_' + vt.upper()
            p.rows(small, vt, p.arrays[arr][2][:4])
            p.fn(f'fma_{vt}', [('a', small), ('b', small), ('c', small)], [
                Stmt('fma', '@fma(a, b, c)', lambda a, b, c: lanewise(lambda x, y, z: f_fma(x, y, z, ft), a, b, c), vt),
                Stmt('mul add', 'a * b + c',
                     lambda a, b, c: lanewise(lambda x, y, z: f_add(f_mul(x, y, ft), z, ft), a, b, c), vt),
            ])
    p.write()


def shuffle_lists(n, k):
    """Index lists of k lanes into the 2n lanes of two n-lane vectors."""
    out = [[i % (2 * n) for i in range(k)], [(2 * n - 1 - i) % (2 * n) for i in range(k)],
           [(i // 2) + (n if i % 2 else 0) for i in range(k)] if k <= 2 * n else None]
    out = [x for x in out if x]
    out.append([rnd.randrange(2 * n) for _ in range(k)])
    return out


def gen_lanes():
    p = Prog('exact_simd_lanes.ax', [
        'Vector lane operations (spec §13.2, Appendix A.5): @shuffle (constant',
        'indices, results of 128, 256 and 512 bits), @swizzle and @swizzle2',
        '(unsigned indices; out of range gives 0), @narrow (saturating), @splat,',
        '@vec, @extract_lane, @replace_lane, @load_partial and @store_partial (of',
        'lengths 0 to the lanes), and the @dot of i16 lanes, over rows of edge values.',
    ] + GEN)
    for bits in (128, 256):
        for lt in ILANES + FLANES:
            vt = vname(lt, bits)
            n = nlanes(vt)
            w = WIDTH[lt]
            arr = rows_of(p, vt, 3)
            stmts = []
            for rb in (128, 256, 512):
                k = rb // w
                for ix in shuffle_lists(n, k):
                    rt = vname(lt, rb)
                    stmts.append(Stmt(f'shuffle {rb} {ix}', f'@shuffle(a, b, [{", ".join(str(i) for i in ix)}])',
                                      lambda a, b, ix=ix: [(a + b)[i] for i in ix], rt))
            p.fn(f'shuffle_{vt}', [('a', arr), ('b', arr)], stmts)
            # @swizzle and @swizzle2: indices in range, past it, and with the top bit
            ilt = 'u' + str(w) if lt in FLANES else lt
            idxt = vname(ilt, bits)
            idx_rows = []
            for _ in range(4):
                idx_rows.append([wrap(rnd.choice([rnd.randrange(n), rnd.randrange(2 * n), n, 2 * n, 2 * n - 1,
                                                  rnd.randrange(1 << w), (1 << w) - 1, 1 << (w - 1)]), ilt)
                                 for _ in range(n)])
            idx_rows.append([wrap(n - 1 - i, ilt) for i in range(n)])
            ia = p.rows('I_' + vt.upper(), idxt, idx_rows)
            m = (1 << w) - 1
            p.fn(f'swizzle_{vt}', [('a', arr), ('b', arr), ('x', ia)], [
                Stmt('swizzle', '@swizzle(a, x)',
                     lambda a, b, x: [a[i & m] if (i & m) < n else zero_of(lt) for i in x], vt),
                Stmt('swizzle2', '@swizzle2(a, b, x)',
                     lambda a, b, x: [(a + b)[i & m] if (i & m) < 2 * n else zero_of(lt) for i in x], vt),
            ])
            # lanes: extract, replace, splat, vec
            sc = p.scalars('L_' + lt.upper(), lt, [x for x in lane_values(lt)[:6]])
            stmts = []
            for li in sorted({0, 1, n // 2, n - 1}):
                stmts.append(Stmt(f'extract {li}', f'@extract_lane(a, {li})', lambda a, k, li=li: a[li], lt))
                stmts.append(Stmt(f'replace {li}', f'@replace_lane(a, {li}, k)',
                                  lambda a, k, li=li: a[:li] + [k] + a[li + 1:], vt))
            stmts.append(Stmt('splat', 'sv', lambda a, k: [k] * n, vt, pre=[f'let sv: {vt} = @splat(k);']))
            vec_args = ', '.join('k' if i % 3 == 0 else f'@extract_lane(a, {n - 1 - i})' for i in range(n))
            stmts.append(Stmt('vec', f'@vec({vt}, {vec_args})',
                              lambda a, k: [k if i % 3 == 0 else a[n - 1 - i] for i in range(n)], vt))
            p.fn(f'lanes_{vt}', [('a', arr), ('k', sc)], stmts)
            if lt in FLANES:
                continue
            # @load_partial and @store_partial of every length
            sentinel = wrap(0x5a5a5a5a5a5a5a5a, lt)
            buf = f'B_{vt.upper()}'
            p.arrays[buf] = (lt, [0] * n, None, ('buf', vt))
            stmts = []
            for ln in sorted({0, 1, 2, n // 2 - 1, n // 2, n - 1, n}):
                stmts.append(Stmt(f'load_partial {ln}', f'@load_partial({vt}, {arr}[i * {n}..i * {n} + {ln}])',
                                  lambda a, ln=ln: a[:ln] + [0] * (n - ln), vt))
                stmts.append(Stmt(f'store_partial {ln}', f'@load({vt}, {buf}[0..])',
                                  lambda a, ln=ln: a[:ln] + [sentinel] * (n - ln), vt,
                                  pre=[f'@mem_set({buf}[..], {sentinel});', f'@store_partial({buf}[0..{ln}], a);']))
            p.fn(f'partial_{vt}', [('a', arr)], stmts)
            # @narrow to both signednesses of the half width
            if w >= 16:
                for dl in ('i' + str(w // 2), 'u' + str(w // 2)):
                    dt = vname(dl, bits)
                    p.fn(f'narrow_{vt}_{dl}', [('a', arr), ('b', arr)], [
                        Stmt('narrow', f'@narrow({dt}, a, b)', lambda a, b, dl=dl: [sat(x, dl) for x in a + b], dt)])
            if lt == 'i16':
                dt = vname('i32', bits)
                p.fn(f'dot_{vt}', [('a', arr), ('b', arr)], [
                    Stmt('dot', '@dot(a, b)', lambda a, b: [wrap(a[2 * i] * b[2 * i] + a[2 * i + 1] * b[2 * i + 1], 'i32')
                                                            for i in range(n // 2)], dt)])
    p.write()


def lanes_bits(vals, lt):
    """The bits of a vector (lanes from low to high) as one integer."""
    w = WIDTH[lt]
    x = 0
    for i, v in enumerate(vals):
        x |= (f_bits(v, lt) if lt in FLANES else v & ((1 << w) - 1)) << (i * w)
    return x


def bits_lanes(x, lt, n):
    w = WIDTH[lt]
    out = []
    for i in range(n):
        b = (x >> (i * w)) & ((1 << w) - 1)
        out.append(f_from_bits(b, lt) if lt in FLANES else wrap(b, lt))
    return out


def gen_mem():
    p = Prog('exact_simd_mem.ax', [
        'Vectors in memory and the raw vector types: on vectors of 128, 256 and 512',
        'bits, of integer and float lanes, stores and loads through pointers made',
        'by @ptr_cast from lanes of a global array (at offsets of a lane, so',
        'unaligned and overlapping), @load and @store on slices at such offsets,',
        'arrays of vectors (for &mut, @ptr_add, an element through a pointer),',
        'slices of vectors (from @cast_slice and @slice_from_ptr, passed to a',
        'function), *mut and * vector parameters and a vector field of a struct',
        'changed through a pointer; v128, v256 and v512 (@bit_cast from and to',
        'lane types, & | ^ ~, as parameters, results and through pointers); and',
        'the @dot of 8-bit lanes (i8 by i8, u8 by i8: each i32 lane the sum of four',
        'products). Each line hashes the results of one operation over the rows.',
    ] + GEN)
    for bits in (128, 256, 512):
        for lt in ['u8', 'i16', 'i32', 'u64', 'f32', 'f64']:
            vt = vname(lt, bits)
            n = nlanes(vt)
            fl = lt in FLANES
            st = ('u32' if lt == 'f32' else 'u64') if fl else lt
            sv = f'{st}x{n}'
            arr = rows_of(p, vt, 3)
            nb = 3 * n + 3
            buf = f'M_{vt.upper()}'
            init = [wrap(0x0123_4567_89ab_cdef * (i + 1) + i, st) if not fl else
                    f_bits(float(i) - 7.5, lt) for i in range(nb)]
            p.arrays[buf] = (st, init, None, ('buf', vt))
            state = {'buf': list(init)}

            def ld(text):  # a load of a vt from a slice of buf's storage
                return f'@bit_cast({vt}, @load({sv}, {text}))' if fl else f'@load({vt}, {text})'

            def stv(text, v):
                return f'@store({text}, @bit_cast({sv}, {v}));' if fl else f'@store({text}, {v});'

            def put(k, vals, state=state, lt=lt, fl=fl):
                for i, v in enumerate(vals):
                    state['buf'][k + i] = f_bits(v, lt) if fl else v

            def get(k, state=state, lt=lt, fl=fl, n=n):
                return [f_from_bits(x, lt) if fl else x for x in state['buf'][k:k + n]]
            if fl:
                op, opf = '+', (lambda x, y, lt=lt: f_add(x, y, lt))
            else:
                op, opf = '+%', (lambda x, y, lt=lt: wrap(x + y, lt))
            k1, k2, k3, k4 = 1, n - 1 if n > 2 else 2, 2, n + 1
            stmts = []

            def s_ptr(a, b, k1=k1, k2=k2, put=put, get=get):
                put(k1, a)
                return get(k2)
            stmts.append(Stmt('ptr store load', f'@ptr_cast(*{vt}, &{buf}[{k2}]).*', s_ptr, vt,
                              pre=[f'let pm = @ptr_cast(*mut {vt}, &mut {buf}[{k1}]);', 'pm.* = a;']))

            def s_slice(a, b, k3=k3, k4=k4, put=put, get=get):
                put(k3, b)
                return get(k4)
            stmts.append(Stmt('slice store load', ld(f'{buf}[{k4}..]'), s_slice, vt,
                              pre=[stv(f'{buf}[{k3}..]', 'b')]))

            def s_arr(a, b, opf=opf):
                av = [a, [opf(x, y) for x, y in zip(b, a)], [opf(x, y) for x, y in zip(a, a)]]
                return av[2]
            stmts.append(Stmt('array of vectors', 'q.*', s_arr, vt,
                              pre=[f'let mut av: [3]{vt} = [a, b, a];', 'for &mut x in av[1..] {',
                                   f'    x.* = x.* {op} a;', '}', 'let q = @ptr_add(&av[0], 2);']))

            def s_mod(a, b, opf=opf):
                v1 = [opf(x, y) for x, y in zip(b, a)]
                return [opf(x, y) for x, y in zip(v1, a)]
            stmts.append(Stmt('mut param', 'av[1]', s_mod, vt, pre=[f'vmod_{vt}(&mut av[1], &av[0]);']))

            def s_sum(a, b, opf=opf):
                v1 = [opf(opf(x, y), y) for x, y in zip(b, a)]
                av = [a, v1, [opf(x, y) for x, y in zip(a, a)]]
                r = av[0]
                for x in av[1:]:
                    r = [opf(u, v) for u, v in zip(r, x)]
                return r
            stmts.append(Stmt('slice param', f'vsum_{vt}(av[..])', s_sum, vt))

            def s_cast(a, b, get=get, n=n):
                return [opf(x, y) for x, y in zip(get(n), get(2 * n))]
            stmts.append(Stmt('cast_slice', f'vsum_{vt}(@cast_slice({vt}, {buf}[{n}..{3 * n}]))', s_cast, vt))

            def s_sfp(a, b, get=get, n=n):
                return [opf(x, y) for x, y in zip(get(3), get(3 + n))]
            stmts.append(Stmt('slice_from_ptr', f'vsum_{vt}(@slice_from_ptr(@ptr_cast(*{vt}, &{buf}[3]), 2))',
                              s_sfp, vt))

            def s_field(a, b, opf=opf):
                return [opf(x, y) for x, y in zip(a, b)]
            stmts.append(Stmt('struct field', 'wv.v', s_field, vt,
                              pre=[f'let mut wv = WV_{vt}.{{ tag: 7, v: a, w: 9 }};', f'vbump_{vt}(&mut wv, b);']))
            stmts.append(Stmt('struct others', '@as(u64, wv.tag) + @as(u64, wv.w)', lambda a, b: 8 + 10, 'u64'))
            stmts.append(Stmt('buffer', ld(f'{buf}[{2 * n}..]'), lambda a, b, get=get, n=n: get(2 * n), vt))
            p.fn(f'mem_{vt}', [('a', arr), ('b', arr)], stmts)
            p.fns.append('\n'.join([
                f'fn vmod_{vt}(p: *mut {vt}, q: *{vt}) void {{', f'    p.* = p.* {op} q.*;', '}', '',
                f'fn vsum_{vt}(s: []{vt}) {vt} {{', '    let mut r = s[0];', '    for x in s[1..] {',
                f'        r = r {op} x;', '    }', '    return r;', '}', '',
                f'struct WV_{vt} {{', '    tag: u8,', f'    v: {vt},', '    w: u16,', '}', '',
                f'fn vbump_{vt}(p: *mut WV_{vt}, x: {vt}) void {{', f'    p.v = p.v {op} x;', '    p.tag += 1;',
                '    p.w += 1;', '}']))
    # the raw types
    for bits in (128, 256, 512):
        raw = f'v{bits}'
        k = bits // 64
        ut = f'u64x{k}'
        rows = {}
        for lt in ('u64', 'i8', 'f32'):
            rows[lt] = rows_of(p, vname(lt, bits), 3)
        stmts = []
        m = (1 << bits) - 1
        for la, lb in (('u64', 'u64'), ('i8', 'u64'), ('f32', 'i8')):
            ta, tb = vname(la, bits), vname(lb, bits)

            def cv(text, lt=None):
                return f'@bit_cast({raw}, {text})'
            ba = lambda a, la=la: lanes_bits(a, la)  # noqa: E731
            bb = lambda b, lb=lb: lanes_bits(b, lb)  # noqa: E731
            stmts.append(Stmt(f'and {la} {lb}', f'@bit_cast({ut}, {cv("a")} & {cv("b")})',
                              lambda a, b, ba=ba, bb=bb: bits_lanes(ba(a) & bb(b), 'u64', k), ut))
            stmts.append(Stmt(f'or not {la} {lb}', f'@bit_cast({ut}, {cv("a")} | ~{cv("b")})',
                              lambda a, b, ba=ba, bb=bb: bits_lanes(ba(a) | (~bb(b) & m), 'u64', k), ut))
            stmts.append(Stmt(f'xor {la} {lb}', f'@bit_cast({vname("i16", bits)}, {cv("a")} ^ {cv("b")})',
                              lambda a, b, ba=ba, bb=bb: bits_lanes(ba(a) ^ bb(b), 'i16', bits // 16),
                              vname('i16', bits)))

            def s_fn(a, b, ba=ba, bb=bb):
                x, y = ba(a), bb(b)
                loc = y ^ x
                return bits_lanes(((x | y) & (~loc & m)) ^ loc, 'u64', k)
            stmts.append(Stmt(f'fn {la} {lb}', f'@bit_cast({ut}, rf_{raw}({cv("a")}, {cv("b")}, &mut loc) ^ loc)', s_fn,
                              ut, pre=[f'let mut loc: {raw} = {cv("b")};']))
            p.fn(f'raw_{raw}_{la}_{lb}', [('a', rows[la]), ('b', rows[lb])], stmts)
            stmts = []
        p.fns.append('\n'.join([f'fn rf_{raw}(x: {raw}, y: {raw}, p: *mut {raw}) {raw} {{', '    p.* = p.* ^ x;',
                                 '    return (x | y) & ~p.*;', '}']))
    # @dot of 8-bit lanes
    for bits in (128, 256, 512):
        bt = vname('i8', bits)
        n = nlanes(bt)
        dt = vname('i32', bits)
        ri = rows_of(p, bt, 4)
        ru = rows_of(p, vname('u8', bits), 4)
        dot = lambda a, b, n=n: [sum(a[4 * i + j] * b[4 * i + j] for j in range(4)) for i in range(n // 4)]  # noqa
        p.fn(f'dot_{bt}', [('a', ri), ('b', ri)], [Stmt('dot i8', '@dot(a, b)', dot, dt)])
        p.fn(f'dot_u8_{bt}', [('a', ru), ('b', ri)], [Stmt('dot u8 i8', '@dot(a, b)', dot, dt)])
    p.write()


def zero_of(lt):
    return 0.0 if lt in FLANES else 0


# ---- random trees ---------------------------------------------------------

VVARS = [('v0', 'i32x4'), ('v1', 'u8x16'), ('v2', 'i16x8'), ('v3', 'u16x8'), ('v4', 'i8x16'), ('v5', 'u32x4'),
         ('v6', 'i64x2'), ('v7', 'u64x2'), ('v8', 'f32x4'), ('v9', 'f64x2'), ('v10', 'i32x8'), ('v11', 'f32x8'),
         ('v12', 'u16x16'), ('v13', 'f64x4'), ('v14', 'i64x4'), ('v15', 'u8x32'), ('v16', 'f32x16')]
SVARS = [('s0', 'i8'), ('s1', 'u8'), ('s2', 'i16'), ('s3', 'u16'), ('s4', 'i32'), ('s5', 'u32'), ('s6', 'i64'),
         ('s7', 'u64'), ('s8', 'f32'), ('s9', 'f64')]
ALLVARS = VVARS + SVARS
EPROGRAMS = [  # file, seed, trees, value sets
    ('exact_simd_exprs_1.ax', 5005, 120, 8),
    ('exact_simd_exprs_2.ax', 6006, 120, 8),
]
EGROUP = 8
ETYPES = [v for v in VT if vbits(v) <= 256] + ['f32x16', 'i32x16']


class Node:
    def __init__(self, parts, t, vals, leaf=False):
        self.parts, self.t, self.vals, self.leaf = parts, t, vals, leaf

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


def lit_text(x, lt):
    if lt not in FLANES:
        return str(x)
    if x == 0:
        return '-0.0' if math.copysign(1.0, x) < 0 else '0.0'
    r = repr(x)
    if 'e' in r:
        m, e = r.split('e')
        if '.' not in m:
            m += '.0'
        return m + 'e' + str(int(e))
    return r if '.' in r else r + '.0'


def scalar_lit(x, lt):
    text = lit_text(x, lt)
    return f'({text})' if text.startswith('-') else text


class VGen:
    def __init__(self, seed, nsets):
        self.r = random.Random(seed)
        self.nsets = nsets
        self.sets = {}
        for v, t in ALLVARS:
            self.sets[v] = [self.value(t, i == 0) for i in range(nsets)]

    def lane_value(self, lt, finite):
        r = self.r
        if lt in FLANES:
            k = r.random()
            if k < 0.3:
                x = f_from_bits(r.choice(float_values(lt)), lt)
                if finite and (math.isnan(x) or math.isinf(x)):
                    return self.lane_value(lt, finite)
                return x
            if k < 0.7:
                return round_frac(Fraction(r.randint(-40, 40), r.choice([1, 2, 4, 3])), lt) or 0.0
            return round_frac(Fraction(r.uniform(-1000, 1000)), lt)
        lo, hi = lrange(lt)
        w = WIDTH[lt]
        k = r.random()
        if k < 0.4:
            e = r.randrange(w)
            return r.choice([x for x in [0, 1, -1, lo, hi, 1 << e, (1 << e) - 1, -(1 << e), lo + 1, hi - 1, 2, 3]
                             if lo <= x <= hi])
        if k < 0.7:
            return r.randint(lo, hi)
        m = r.randint(1, w)
        x = r.randrange(1 << m)
        if signed(lt) and r.random() < 0.5:
            x = -x
        return sat(x, lt)

    def value(self, t, finite):
        if t in VT:
            return [self.lane_value(lane(t), finite) for _ in range(nlanes(t))]
        return self.lane_value(t, finite)

    def var(self, t):
        names = [v for v, vt in ALLVARS if vt == t]
        if not names:
            return None
        v = self.r.choice(names)
        return Node(['$' + v], t, list(self.sets[v]), leaf=True)

    def ok(self, n):
        return n is not None and all(v is not None for v in n.vals)

    def gen(self, t, depth):
        if depth <= 0 or self.r.random() < 0.15:
            return self.leaf(t)
        for _ in range(20):
            n = self.try_vec(t, depth) if t in VT else self.try_scalar(t, depth)
            if self.ok(n):
                return n
        return self.leaf(t)

    def leaf(self, t):
        n = self.var(t)
        if n is not None and self.r.random() < 0.8:
            return n
        if t not in VT:
            x = self.lane_value(t, True)
            return Node([scalar_lit(x, t)], t, [x] * self.nsets, leaf=True)
        if self.r.random() < 0.5:
            s = self.leaf(lane(t))
            return Node(cat(f'@splat_{t}(', s, ')'), t, [[x] * nlanes(t) for x in s.vals])
        xs = [self.lane_value(lane(t), True) for _ in range(nlanes(t))]
        return Node([f'@vec({t}, {", ".join(lit_text(x, lane(t)) for x in xs)})'], t, [list(xs)] * self.nsets,
                    leaf=True)

    def try_scalar(self, t, depth):
        r = self.r
        d = depth - 1
        cands = [v for v in ETYPES if lane(v) == t]
        vt = r.choice(cands)
        n = nlanes(vt)
        kind = r.choice(['extract', 'reduce', 'reduce'] + (['dot'] if t in FLANES else []))
        if kind == 'extract':
            a = self.gen(vt, d)
            li = r.randrange(n)
            return Node(cat('@extract_lane(', a, f', {li})'), t, [x[li] for x in a.vals])
        if kind == 'reduce':
            a = self.gen(vt, d)
            if t in FLANES:
                return Node(cat('@reduce_add(', a, ')'), t, [fold_halves(x, lambda p, q: f_add(p, q, t)) for x in a.vals])
            return Node(cat('@reduce_add(', a, ')'), t, [wrap(sum(x), t) for x in a.vals])
        a, b = self.gen(vt, d), self.gen(vt, d)
        return Node(cat('@dot(', a, ', ', b, ')'), t,
                    [fold_halves([f_mul(p, q, t) for p, q in zip(x, y)], lambda p, q: f_add(p, q, t))
                     for x, y in zip(a.vals, b.vals)])

    def try_vec(self, t, depth):
        return self.try_float(t, depth) if is_float(t) else self.try_int(t, depth)

    def same_size(self, t, pred):
        c = [v for v in ETYPES if vbits(v) == vbits(t) and pred(v)]
        return self.r.choice(c) if c else None

    def common(self, t, depth):
        """Forms of either kind of vector: shuffles, swizzles, select of a
        comparison, replace_lane, splat."""
        r = self.r
        d = depth - 1
        lt, n = lane(t), nlanes(t)
        kind = r.choice(['shuffle', 'swizzle', 'select', 'replace', 'splat'])
        if kind == 'shuffle':
            srcs = [v for v in ETYPES if lane(v) == lt and vbits(v) <= 256]
            st = r.choice(srcs)
            m = nlanes(st)
            ix = [r.randrange(2 * m) for _ in range(n)]
            a, b = self.gen(st, d), self.gen(st, d)
            return Node(cat('@shuffle(', a, ', ', b, f', [{", ".join(map(str, ix))}])'), t,
                        [[(x + y)[i] for i in ix] for x, y in zip(a.vals, b.vals)])
        if kind == 'swizzle':
            it = vname('u' + str(WIDTH[lt]), vbits(t))
            a = self.gen(t, d)
            x = self.gen(it, d)
            if r.random() < 0.6:
                x = Node(cat('(', x, f' & @splat_{it}({2 * n - 1}))'), it, [[i & (2 * n - 1) for i in xs] for xs in x.vals])
            if r.random() < 0.5:
                return Node(cat('@swizzle(', a, ', ', x, ')'), t,
                            [[av[i] if i < n else zero_of(lt) for i in xv] for av, xv in zip(a.vals, x.vals)])
            b = self.gen(t, d)
            return Node(cat('@swizzle2(', a, ', ', b, ', ', x, ')'), t,
                        [[(av + bv)[i] if i < 2 * n else zero_of(lt) for i in xv]
                         for av, bv, xv in zip(a.vals, b.vals, x.vals)])
        if kind == 'select':
            ct = self.same_size(t, lambda v: nlanes(v) == n)
            c = r.choice(CMPS)
            a, b = self.gen(ct, d), self.gen(ct, d)
            x, y = self.gen(t, d), self.gen(t, d)
            ms = [[fcmp(c, p, q) for p, q in zip(av, bv)] for av, bv in zip(a.vals, b.vals)]
            return Node(cat(f'@select(@lanes_{c}(', a, ', ', b, '), ', x, ', ', y, ')'), t,
                        [[p if m else q for m, p, q in zip(mv, xv, yv)] for mv, xv, yv in zip(ms, x.vals, y.vals)])
        if kind == 'replace':
            a = self.gen(t, d)
            k = self.gen(lt, d)
            li = r.randrange(n)
            return Node(cat('@replace_lane(', a, f', {li}, ', k, ')'), t,
                        [av[:li] + [kv] + av[li + 1:] for av, kv in zip(a.vals, k.vals)])
        k = self.gen(lt, d)
        return Node(cat(f'@splat_{t}(', k, ')'), t, [[kv] * n for kv in k.vals])

    def try_float(self, t, depth):
        r = self.r
        d = depth - 1
        ft, n = lane(t), nlanes(t)
        kind = r.choice(['arith', 'arith', 'arith', 'fma', 'minmax', 'unary', 'round', 'sqrt', 'conv', 'common',
                         'common'])
        if kind == 'common':
            return self.common(t, depth)
        if kind == 'arith':
            op = r.choice(['+', '-', '*', '/'])
            f = {'+': f_add, '-': f_sub, '*': f_mul, '/': f_div}[op]
            a, b = self.gen(t, d), self.gen(t, d)
            return Node(cat('(', a, f' {op} ', b, ')'), t,
                        [[f(x, y, ft) for x, y in zip(av, bv)] for av, bv in zip(a.vals, b.vals)])
        if kind == 'fma':
            a, b, c = self.gen(t, d), self.gen(t, d), self.gen(t, d)
            if r.random() < 0.5:
                return Node(cat('@fma(', a, ', ', b, ', ', c, ')'), t,
                            [[f_fma(x, y, z, ft) for x, y, z in zip(av, bv, cv)]
                             for av, bv, cv in zip(a.vals, b.vals, c.vals)])
            return Node(cat('(', a, ' * ', b, ' + ', c, ')'), t,
                        [[f_add(f_mul(x, y, ft), z, ft) for x, y, z in zip(av, bv, cv)]
                         for av, bv, cv in zip(a.vals, b.vals, c.vals)])
        if kind == 'minmax':
            name = r.choice(['min', 'max'])
            a, b = self.gen(t, d), self.gen(t, d)
            return Node(cat(f'@{name}(', a, ', ', b, ')'), t,
                        [[f_minmax(x, y, name == 'max') for x, y in zip(av, bv)] for av, bv in zip(a.vals, b.vals)])
        if kind == 'unary':
            a = self.gen(t, d)
            if r.random() < 0.5:
                return Node(cat('-(', a, ')'), t, [[f_neg(x) for x in av] for av in a.vals])
            return Node(cat('@abs(', a, ')'), t, [[f_abs(x) for x in av] for av in a.vals])
        if kind == 'round':
            mode = r.choice(['floor', 'ceil', 'trunc', 'nearest'])
            a = self.gen(t, d)
            return Node(cat(f'@{mode}(', a, ')'), t, [[f_round(x, mode) for x in av] for av in a.vals])
        if kind == 'sqrt':
            a = self.gen(t, d)
            return Node(cat('@sqrt(@abs(', a, '))'), t, [[f_sqrt(f_abs(x), ft) for x in av] for av in a.vals])
        st = r.choice([v for v in ETYPES if nlanes(v) == n and not is_float(v)] or [None])
        if st is None:
            return None
        a = self.gen(st, d)
        return Node(cat(f'@float_from_int({t}, ', a, ')'), t, [[float_from_int(x, ft) for x in av] for av in a.vals])

    def try_int(self, t, depth):
        r = self.r
        d = depth - 1
        lt, n = lane(t), nlanes(t)
        w = WIDTH[lt]
        m = (1 << w) - 1
        kind = r.choice(['wrap', 'wrap', 'wrap', 'bit', 'bit', 'shift', 'shift', 'minmax', 'sat', 'unary', 'count',
                         'cast', 'widen', 'narrow', 'from_float', 'common', 'common'])
        if kind == 'common':
            return self.common(t, depth)
        if kind == 'wrap':
            ops = ['+', '-', '+%', '-%'] + (['*', '*%'] if w > 8 else [])
            op = r.choice(ops)
            f = {'+': lambda x, y: x + y, '-': lambda x, y: x - y, '*': lambda x, y: x * y}[op[0]]
            a, b = self.gen(t, d), self.gen(t, d)
            return Node(cat('(', a, f' {op} ', b, ')'), t,
                        [[wrap(f(x, y), lt) for x, y in zip(av, bv)] for av, bv in zip(a.vals, b.vals)])
        if kind == 'bit':
            op = r.choice(['&', '|', '^'])
            f = {'&': lambda x, y: x & y, '|': lambda x, y: x | y, '^': lambda x, y: x ^ y}[op]
            a, b = self.gen(t, d), self.gen(t, d)
            return Node(cat('(', a, f' {op} ', b, ')'), t,
                        [[f(x, y) for x, y in zip(av, bv)] for av, bv in zip(a.vals, b.vals)])
        if kind == 'shift':
            op = r.choice(['<<', '>>'])
            f = shl if op == '<<' else shr
            a = self.gen(t, d)
            if r.random() < 0.5:
                ct = r.choice(['u8', 'u16', 'u32', 'u64'])
                k = self.gen(ct, d)
                if k.leaf:
                    # (an untyped literal count is a u32, §7.6; so would the constant be)
                    k = Node(cat(f'@as({ct}, ', k, ')'), ct, k.vals)
                if r.random() < 0.6:
                    k = Node(cat('(', k, f' & {2 * w - 1})'), ct, [x & (2 * w - 1) for x in k.vals])
                return Node(cat('(', a, f' {op} ', k, ')'), t, [[f(x, kv, lt) for x in av] for av, kv in zip(a.vals, k.vals)])
            ut = vname('u' + str(w), vbits(t))
            k = self.gen(ut, d)
            if r.random() < 0.6:
                k = Node(cat('(', k, f' & @splat_{ut}({2 * w - 1}))'), ut, [[x & (2 * w - 1) for x in kv] for kv in k.vals])
            return Node(cat('(', a, f' {op} ', k, ')'), t,
                        [[f(x, y, lt) for x, y in zip(av, kv)] for av, kv in zip(a.vals, k.vals)])
        if kind == 'minmax':
            if w == 64:
                return None
            name = r.choice(['min', 'max'])
            a, b = self.gen(t, d), self.gen(t, d)
            f = min if name == 'min' else max
            return Node(cat(f'@{name}(', a, ', ', b, ')'), t, [[f(x, y) for x, y in zip(av, bv)] for av, bv in zip(a.vals, b.vals)])
        if kind == 'sat':
            if w > 16:
                return None
            name = r.choice(['add', 'sub'])
            f = (lambda x, y: x + y) if name == 'add' else (lambda x, y: x - y)
            a, b = self.gen(t, d), self.gen(t, d)
            return Node(cat(f'@{name}_sat(', a, ', ', b, ')'), t,
                        [[sat(f(x, y), lt) for x, y in zip(av, bv)] for av, bv in zip(a.vals, b.vals)])
        if kind == 'unary':
            ops = ['~', '-%'] + (['-', 'abs'] if signed(lt) else [])
            op = r.choice(ops)
            a = self.gen(t, d)
            if op == '~':
                return Node(cat('~(', a, ')'), t, [[wrap(~x, lt) for x in av] for av in a.vals])
            if op == 'abs':
                return Node(cat('@abs(', a, ')'), t, [[wrap(abs(x), lt) for x in av] for av in a.vals])
            return Node(cat(f'{op}(', a, ')'), t, [[wrap(-x, lt) for x in av] for av in a.vals])
        if kind == 'count':
            name = r.choice(['clz', 'ctz', 'popcount'])
            f = {'clz': clz, 'ctz': ctz, 'popcount': popcount}[name]
            a = self.gen(t, d)
            return Node(cat(f'@{name}(', a, ')'), t, [[f(x, lt) for x in av] for av in a.vals])
        if kind == 'cast':
            st = self.same_size(t, lambda v: not is_float(v) and v != t)
            if st is None:
                return None
            a = self.gen(st, d)
            sw = WIDTH[lane(st)]
            out = []
            for av in a.vals:
                word = 0
                for i, x in enumerate(av):
                    word |= (x & ((1 << sw) - 1)) << (i * sw)
                out.append([wrap(word >> (i * w), lt) for i in range(n)])
            return Node(cat(f'@bit_cast({t}, ', a, ')'), t, out)
        if kind == 'widen':
            if w == 8:
                return None
            st = vname(lt[0] + str(w // 2), vbits(t))
            if st not in ETYPES:
                return None
            a = self.gen(st, d)
            hi = r.random() < 0.5
            return Node(cat(f'@widen_{"high" if hi else "low"}(', a, ')'), t,
                        [av[n:] if hi else av[:n] for av in a.vals])
        if kind == 'narrow':
            if w == 64:
                return None
            st = vname(r.choice('iu') + str(2 * w), vbits(t))
            if st not in ETYPES:
                return None
            a, b = self.gen(st, d), self.gen(st, d)
            return Node(cat(f'@narrow({t}, ', a, ', ', b, ')'), t, [[sat(x, lt) for x in av + bv] for av, bv in zip(a.vals, b.vals)])
        st = r.choice([v for v in ETYPES if nlanes(v) == n and is_float(v)] or [None])
        if st is None:
            return None
        a = self.gen(st, d)
        return Node(cat(f'@int_from_float({t}, ', a, ')'), t, [[int_from_float(x, lt) for x in av] for av in a.vals])


def splat_helpers(text):
    """`@splat_T(x)` in the rendered text stands for a splat to T: each one
    becomes a call of a function that returns T (the splat's type comes from
    the context, §13.2)."""
    import re
    out = []
    for t in sorted(set(re.findall(r'@splat_(\w+)\(', text))):
        out += [f'fn splat_{t}(x: {lane(t)}) {t} {{', '    return @splat(x);', '}', '']
    return out


def gen_trees(seed, ntrees, nsets):
    g = VGen(seed, nsets)
    trees = []
    tops = ETYPES + ILANES + FLANES
    for _ in range(ntrees):
        t = g.r.choice(tops)
        e = g.gen(t, g.r.randint(2, 4))
        while e.leaf:
            e = g.gen(t, g.r.randint(2, 4))
        trees.append(e)
    return g, trees


def write_trees(fname, seed, ntrees, nsets, out_dir=RUN, expand=False):
    g, trees = gen_trees(seed, ntrees, nsets)
    run_names = {v: v for v, _ in ALLVARS}
    const_names = {}
    for v, t in ALLVARS:
        x = g.sets[v][0]
        if t in VT:
            const_names[v] = f'@vec({t}, {", ".join(lit_text(y, lane(t)) for y in x)})'
        else:
            const_names[v] = scalar_lit(x, t)
    out = ['// Random trees of vector operations (spec §13) over vector and scalar',
           '// variables, every lane evaluated by the rules of the spec: integer lanes',
           '// wrap, float lanes round to nearest (no contraction; @reduce_add and @dot',
           '// fold the halves). Each line is a hash of a group of trees over all value',
           '// sets (from memory) or over constants (the first set, as @vec of',
           '// literals; the compiler may evaluate them). (Generated by',
           '// cmd/axiom/internal/tests/tools/gen_exact_simd.py, with the expectations',
           '// computed lane by lane in Python.)',
           'import "../lib/io.ax" as io;', '']
    for v, t in ALLVARS:
        vals = g.sets[v]
        if t in VT:
            lt = lane(t)
            st = ('u32' if lt == 'f32' else 'u64') if lt in FLANES else lt
            flat = [f_bits(x, lt) if lt in FLANES else x for row in vals for x in row]
        else:
            st = ('u32' if t == 'f32' else 'u64') if t in FLANES else t
            flat = [f_bits(x, t) if t in FLANES else x for x in vals]
        out.append(f'let mut {v.upper()}: [{len(flat)}]{st} = [{", ".join(map(str, flat))}];')
    out.append('')
    used = set()
    params = ', '.join(f'{v}: {t}' for v, t in ALLVARS)
    args = []
    for v, t in ALLVARS:
        if t in VT:
            n = nlanes(t)
            ld = f'@load({t}, {v.upper()}[s * {n}..])' if not is_float(t) else \
                f'@bit_cast({t}, @load({"u32" if lane(t) == "f32" else "u64"}x{n}, {v.upper()}[s * {n}..]))'
        else:
            ld = f'@bit_cast({t}, {v.upper()}[s])' if t in FLANES else f'{v.upper()}[s]'
        args.append(ld)
    args = ', '.join(args)
    fns, body = [], []

    def fix(text):
        return text.replace('@splat_', 'splat_')
    if expand:
        body.append('export fn run() void {')
        for i, e in enumerate(trees):
            fns += [f'fn e{i}({params}) u64 {{', '    let mut h: u64 = 0;',
                    '    ' + hash_line('h', fix(e.render(run_names)), e.t, used), '    return h;', '}', '']
            fns += [f'fn k{i}() u64 {{', '    let mut h: u64 = 0;',
                    '    ' + hash_line('h', fix(e.render(const_names)), e.t, used), '    return h;', '}', '']
            body += [f'    // {e.t}: {e.render(run_names)}'[:3000], f'    for s in 0..S0.len {{',
                     f'        io.uint(e{i}({args}));', '    }']
            body += [f'    // expect: {hash_any(0, e.vals[si], e.t)}' for si in range(nsets)]
            body.append(f'    io.uint(k{i}());  // expect: {hash_any(0, e.vals[0], e.t)}')
        body.append('}')
    else:
        body.append('export fn run() void {')
        for gi in range(0, len(trees), EGROUP):
            grp = trees[gi:gi + EGROUP]
            k = gi // EGROUP
            for kind, names in (('g', run_names), ('c', const_names)):
                fns.append(f'fn {kind}{k}({params}) u64 {{' if kind == 'g' else f'fn {kind}{k}() u64 {{')
                fns.append('    let mut h: u64 = 0;')
                fns += ['    ' + hash_line('h', fix(e.render(names)), e.t, used) for e in grp]
                fns += ['    return h;', '}', '']
            hr = 0
            for si in range(nsets):
                hs = 0
                for e in grp:
                    hs = hash_any(hs, e.vals[si], e.t)
                hr = mix(hr, hs)
            hc = 0
            for e in grp:
                hc = hash_any(hc, e.vals[0], e.t)
            body += ['    {', '        let mut h: u64 = 0;', f'        for s in 0..S0.len {{',
                     f'            h = mix(h, g{k}({args}));', '        }', f'        io.uint(h);  // expect: {hr}', '    }',
                     f'    io.uint(c{k}());  // expect: {hc}']
        body.append('}')
    out.append(HELPERS)
    out += splat_helpers('\n'.join(e.render(run_names) + e.render(const_names) for e in trees))
    out += vec_helpers(used)
    out += fns + body
    with open(os.path.join(out_dir, fname), 'w') as f:
        f.write('\n'.join(out) + '\n')


def main():
    args = sys.argv[1:]
    expand = '--expand' in args
    args = [a for a in args if a != '--expand']
    if len(args) == 2 and args[0].startswith('--seeds='):
        first, count = (int(x) for x in args[0][len('--seeds='):].split(':'))
        for seed in range(first, first + count):
            write_trees(f'exact_simd_exprs_s{seed}.ax', seed, 120, 8, args[1], expand)
        return
    if not expand:
        gen_int()
        gen_float()
        gen_lanes()
    for fname, seed, ntrees, nsets in EPROGRAMS:
        write_trees(fname, seed, ntrees, nsets, expand=expand)
    if not expand:
        gen_mem()


if __name__ == '__main__':
    main()
