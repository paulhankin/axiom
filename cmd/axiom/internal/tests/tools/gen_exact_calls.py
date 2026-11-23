#!/usr/bin/env python3
"""Writes tests of calls whose expected outputs Python computes from the
specification's rules (spec §5, §7.14, §9, §10, §11), not from the compiler:
random programs of functions that take and return random types, with every
result hashed, so that a value lost or changed on its way into a function,
out of it, or through a pointer changes an output line.

    python3 cmd/axiom/internal/tests/tools/gen_exact_calls.py
    cmd/axiom/build/axiom fmt cmd/axiom/internal/tests/run/exact_calls_*.ax

writes cmd/axiom/internal/tests/run/exact_calls_<n>.ax (the same files every
time: each has its own fixed seed). A program has a pool of random types:
the integers of every width, f32, f64, bool, enums (with and without a
representation, negative values), structs (nested, with fields in orders
that need padding, small and large), a generic struct instantiated at
several types, arrays, tagged unions (payloads of any type, and none),
optionals, error unions of a named error set and vectors of 128, 256 and 512
bits. Each type has three helpers that Python mirrors: `hs_` folds a value
into a hash, `mk_` makes one from a u64, `st_` makes another from one
(integers change only through the wrapping operators; floats through one
rounded `*` and `+`).

The functions, in levels where each calls only those of lower levels, take
parameters by value, pointers `*T` and `*mut T` (to globals, to fields and
elements of globals, to the callers' locals and parameters), slices `[]T`
and `[]mut T` of global and local arrays, and function values; some take
more parameters than any target passes in registers, mixing integers,
floats and vectors. A body hashes its parameters, writes through its
mutable pointers and slices, calls functions of lower levels (directly,
through tables of functions of one signature, and through its function
parameters; propagating errors with `!` or replacing them with `catch`),
calls the structs' methods by value, by `*Self` and by `*mut Self` (on
values, places and pointers), the generic functions instantiated at the
types it has (deduced and explicit, with `const` function and length
parameters), a recursive helper to a depth from memory, and returns a value
it made, a parameter, a field, a pointee or a callee's result (or an error).

The arguments of the calls in `run` come from memory (`let mut` globals,
or `mk_` of seeds in a global array), so nothing folds. Each output line
hashes one call's result with the locals it was given pointers to; the last
lines hash the globals, which the calls wrote through pointers and slices.

    gen_exact_calls.py --seeds=<first>:<count> <dir>

writes more programs (exact_calls_s<seed>.ax, with other seeds) into <dir>,
for a longer search; copied into tests/run/, the suite runs them.
"""
import math
import os
import random
import sys
from fractions import Fraction

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from gen_exact_ops import (ITYPES, edge_values, f_add, f_bits, f_from_bits, f_minmax, f_mul,  # noqa: E402
                           float_values, mix, round_frac, wrap_t)

HERE = os.path.dirname(os.path.abspath(__file__))
RUN = os.path.join(HERE, '..', 'run')
M64 = (1 << 64) - 1

PROGRAMS = [('exact_calls_1.ax', 3101), ('exact_calls_2.ax', 3202), ('exact_calls_3.ax', 3303)]
INTS = ['u8', 'i8', 'u16', 'i16', 'u32', 'i32', 'u64', 'i64']
ERRS = ['Bad', 'Worse', 'Ugly']
LANEW = {'i8': 8, 'u8': 8, 'i16': 16, 'u16': 16, 'i32': 32, 'u32': 32, 'i64': 64, 'u64': 64, 'f32': 32, 'f64': 64}
FCONST = [1.5, -0.75, 3.0, 0.5, -1.25, 2.0, 0.375]
FADD = [0.25, -2.5, 1.0, -0.125, 6.0, 0.0]


def flit(x):
    """An Axiom literal (constant expression) of the float x."""
    if math.isnan(x):
        return '(0.0 / 0.0)'
    if math.isinf(x):
        return '(1.0 / 0.0)' if x > 0 else '(-1.0 / 0.0)'
    r = repr(x)
    if 'e' in r and '.' not in r.split('e')[0]:
        m, e = r.split('e')
        r = m + '.0e' + e
    return r


def canon(x, t):
    if math.isnan(x):
        return 0x7fc00000 if t == 'f32' else 0x7ff8000000000000
    return f_bits(x, t)


class Prop(Exception):
    """An error leaving a function (`return error.X`, `e!`)."""

    def __init__(self, k):
        self.k = k


# ---- types ----------------------------------------------------------------

class Ty:
    const_ok = True
    named = False

    def decl(self):
        return []


class IntT(Ty):
    def __init__(self, name, r):
        self.id = self.ax = name
        self.bits, self.signed = ITYPES[name]
        lo, hi = (-(1 << (self.bits - 1)), (1 << (self.bits - 1)) - 1) if self.signed else (0, (1 << self.bits) - 1)
        self.lo, self.hi = lo, hi
        self.C = r.choice([1, 3, hi, lo + 1 if self.signed else hi - 2, r.randint(lo, hi)])
        self.D = r.choice([0, 1, -1 if self.signed else hi, r.randint(lo, hi)])
        self.edges = edge_values(name)

    def rand(self, r):
        return r.choice(self.edges) if r.random() < 0.6 else r.randint(self.lo, self.hi)

    def lit(self, v):
        return str(v)

    def hash(self, h, v):
        return mix(h, v & M64)

    def mk(self, x):
        return wrap_t(x, self.ax)

    def st(self, v):
        return wrap_t((v ^ self.D) + self.C, self.ax)

    def helpers(self):
        t = self.ax
        conv = {'u64': 'v', 'i64': '@bit_cast(u64, v)'}.get(t, '@bit_cast(u64, @as(i64, v))' if self.signed else '@as(u64, v)')
        mk = {'u64': 'x', 'i64': '@bit_cast(i64, x)'}.get(
            t, f'@bit_cast({t}, @truncate(u{self.bits}, x))' if self.signed else f'@truncate({t}, x)')
        return [f'fn hs_{t}(h: u64, v: {t}) u64 {{', f'    return mix(h, {conv});', '}', '',
                f'fn mk_{t}(x: u64) {t} {{', f'    return {mk};', '}', '',
                f'fn st_{t}(v: {t}) {t} {{', f'    return (v ^ {self.D}) +% {self.C};', '}', '']


class FloatT(Ty):
    def __init__(self, name, r):
        self.id = self.ax = name
        self.c1, self.c2 = r.choice(FCONST), r.choice(FADD)
        self.vals = [f_from_bits(b, name) for b in float_values(name)]

    def rand(self, r):
        if r.random() < 0.5:
            return r.choice(self.vals)
        return round_frac(Fraction(r.randint(-(1 << 20), 1 << 20), 1 << r.randint(0, 12)), self.ax) + 0.0

    def lit(self, v):
        return flit(v)

    def hash(self, h, v):
        return mix(h, canon(v, self.ax))

    def mk(self, x):
        i = wrap_t(x, 'i32')
        f = round_frac(Fraction(i), self.ax) if i else 0.0
        return f_mul(f, 0.0078125, self.ax)

    def st(self, v):
        return f_add(f_mul(v, self.c1, self.ax), self.c2, self.ax)

    def helpers(self):
        t = self.ax
        nan = '0x7fc00000' if t == 'f32' else '0x7ff8000000000000'
        conv = '@as(u64, @bit_cast(u32, v))' if t == 'f32' else '@bit_cast(u64, v)'
        return [f'fn hs_{t}(h: u64, v: {t}) u64 {{', '    if v != v {', f'        return mix(h, {nan});', '    }',
                f'    return mix(h, {conv});', '}', '',
                f'fn mk_{t}(x: u64) {t} {{',
                f'    return @float_from_int({t}, @bit_cast(i32, @truncate(u32, x))) * 0.0078125;', '}', '',
                f'fn st_{t}(v: {t}) {t} {{', f'    return (v * {flit(self.c1)}) + {flit(self.c2)};', '}', '']


class BoolT(Ty):
    id = ax = 'bool'

    def rand(self, r):
        return r.random() < 0.5

    def lit(self, v):
        return 'true' if v else 'false'

    def hash(self, h, v):
        return mix(h, int(v))

    def mk(self, x):
        return (x >> 3) & 1 == 1

    def st(self, v):
        return not v

    def helpers(self):
        return ['fn hs_bool(h: u64, v: bool) u64 {', '    return mix(h, @as(u64, v));', '}', '',
                'fn mk_bool(x: u64) bool {', '    return ((x >> 3) & 1) == 1;', '}', '',
                'fn st_bool(v: bool) bool {', '    return not v;', '}', '']


class EnumT(Ty):
    named = True

    def __init__(self, ident, r):
        self.id = ident
        self.ax = 'E' + ident[1:]
        n = r.randint(1, 6)
        self.repr = r.choice([None, None, 'u8', 'i8', 'i16', 'u32', 'i64'])
        lo, hi = (-100, 100) if self.repr is None or self.repr[0] == 'i' else (0, 200)
        vals, cur, used = [], None, set()
        for i in range(n):
            if cur is not None and r.random() < 0.6 and cur + 1 <= hi and cur + 1 not in used:
                v, explicit = cur + 1, False
            else:
                v = r.randint(lo, hi)
                while v in used:
                    v = r.randint(lo, hi)
                explicit = True
            if cur is None and v == 0 and r.random() < 0.5:
                explicit = False
            if cur is None and not explicit and v != 0:
                explicit = True
            used.add(v)
            vals.append((f'M{i}', v, explicit))
            cur = v
        self.members = vals

    def rand(self, r):
        return r.randrange(len(self.members))

    def lit(self, v):
        return f'{self.ax}.{self.members[v][0]}'

    def hash(self, h, v):
        return mix(h, v)

    def mk(self, x):
        return x % len(self.members)

    def st(self, v):
        return (v + 1) % len(self.members)

    def decl(self):
        head = f'enum {self.ax}' + (f': {self.repr}' if self.repr else '') + ' {'
        body = [f'    {m}' + (f' = {v}' if e else '') + ',' for m, v, e in self.members]
        return [head] + body + ['}', '']

    def helpers(self):
        n = len(self.members)
        out = [f'fn hs_{self.id}(h: u64, v: {self.ax}) u64 {{', '    match v {']
        out += [f'        .{m} => {{ return mix(h, {i}); }}' for i, (m, _, _) in enumerate(self.members)]
        out += ['    }', '}', '', f'fn mk_{self.id}(x: u64) {self.ax} {{', f'    match x % {n} {{']
        out += [f'        {i} => {{ return {self.ax}.{m}; }}' for i, (m, _, _) in enumerate(self.members[:-1])]
        out += [f'        else => {{ return {self.ax}.{self.members[-1][0]}; }}', '    }', '}', '',
                f'fn st_{self.id}(v: {self.ax}) {self.ax} {{', '    match v {']
        out += [f'        .{m} => {{ return {self.ax}.{self.members[(i + 1) % n][0]}; }}'
                for i, (m, _, _) in enumerate(self.members)]
        out += ['    }', '}', '']
        return out


class StructT(Ty):
    def __init__(self, ident, fields, generic=None):
        self.id = ident
        self.fields = fields  # [(name, Ty)]
        self.generic = generic  # (A, B) for Two.[A, B]
        self.named = generic is None
        self.ax = f'Two.[{generic[0].ax}, {generic[1].ax}]' if generic else 'S' + ident[1:]
        self.const_ok = all(t.const_ok for _, t in fields)
        self.int_field = next((i for i, (_, t) in enumerate(fields) if isinstance(t, IntT)), None)

    def rand(self, r):
        return tuple(t.rand(r) for _, t in self.fields)

    def lit(self, v):
        return f'{self.ax}.{{ ' + ', '.join(f'{n}: {t.lit(x)}' for (n, t), x in zip(self.fields, v)) + ' }'

    def hash(self, h, v):
        for (_, t), x in zip(self.fields, v):
            h = t.hash(h, x)
        return h

    def mk(self, x):
        return tuple(t.mk(mix(x, i + 1)) for i, (_, t) in enumerate(self.fields))

    def st(self, v):
        return tuple(t.st(x) for (_, t), x in zip(self.fields, v))

    def bump(self, v, k):
        v = list(self.st(v))
        if self.int_field is not None:
            t = self.fields[self.int_field][1]
            v[self.int_field] = v[self.int_field] ^ t.mk(k)
        return tuple(v)

    def decl(self):
        if not self.named:
            return []
        out = [f'struct {self.ax} {{']
        out += [f'    {n}: {t.ax},' for n, t in self.fields]
        out += ['', '    fn get(self: Self, k: u64) u64 {', f'        return hs_{self.id}(mix(k, 7), self);', '    }', '',
                '    fn peek(self: *Self) u64 {', '        let mut h: u64 = 13;']
        out += [f'        h = hs_{t.id}(h, self.{n});' for n, t in self.fields]
        out += ['        return h;', '    }', '', '    fn bump(self: *mut Self, k: u64) void {']
        out += [f'        self.{n} = st_{t.id}(self.{n});' for n, t in self.fields]
        if self.int_field is not None:
            n, t = self.fields[self.int_field]
            out.append(f'        self.{n} = self.{n} ^ mk_{t.id}(k);')
        out += ['    }', '', '    fn with(self: Self, k: u64) Self {', '        let mut r = self;', '        r.bump(k);',
                '        return r;', '    }', '}', '']
        return out

    def helpers(self):
        out = [f'fn hs_{self.id}(h0: u64, v: {self.ax}) u64 {{', '    let mut h = h0;']
        out += [f'    h = hs_{t.id}(h, v.{n});' for n, t in self.fields]
        out += ['    return h;', '}', '', f'fn mk_{self.id}(x: u64) {self.ax} {{',
                f'    return {self.ax}.{{ ' + ', '.join(f'{n}: mk_{t.id}(mix(x, {i + 1}))'
                                                   for i, (n, t) in enumerate(self.fields)) + ' };',
                '}', '', f'fn st_{self.id}(v: {self.ax}) {self.ax} {{',
                f'    return {self.ax}.{{ ' + ', '.join(f'{n}: st_{t.id}(v.{n})' for n, t in self.fields) + ' };', '}', '']
        return out


class ArrayT(Ty):
    def __init__(self, ident, elem, n):
        self.id, self.elem, self.n = ident, elem, n
        self.ax = f'[{n}]{elem.ax}'
        self.const_ok = elem.const_ok

    def rand(self, r):
        return tuple(self.elem.rand(r) for _ in range(self.n))

    def lit(self, v):
        return '[' + ', '.join(self.elem.lit(x) for x in v) + ']'

    def hash(self, h, v):
        for x in v:
            h = self.elem.hash(h, x)
        return h

    def mk(self, x):
        return tuple(self.elem.mk(mix(x, i + 1)) for i in range(self.n))

    def st(self, v):
        return tuple(self.elem.st(x) for x in v)

    def helpers(self):
        e = self.elem.id
        return [f'fn hs_{self.id}(h0: u64, v: {self.ax}) u64 {{', '    let mut h = h0;', '    for x in v {',
                f'        h = hs_{e}(h, x);', '    }', '    return h;', '}', '',
                f'fn mk_{self.id}(x: u64) {self.ax} {{',
                '    return [' + ', '.join(f'mk_{e}(mix(x, {i + 1}))' for i in range(self.n)) + '];', '}', '',
                f'fn st_{self.id}(v: {self.ax}) {self.ax} {{', '    let mut r = v;', '    for i in 0..r.len {',
                f'        r[i] = st_{e}(r[i]);', '    }', '    return r;', '}', '']


class UnionT(Ty):
    named = True

    def __init__(self, ident, variants):
        self.id = ident
        self.ax = 'U' + ident[1:]
        self.variants = variants  # [(name, Ty or None)]
        self.const_ok = False

    def rand(self, r):
        i = r.randrange(len(self.variants))
        t = self.variants[i][1]
        return (i, t.rand(r) if t else None)

    def hash(self, h, v):
        i, p = v
        t = self.variants[i][1]
        return t.hash(mix(h, i), p) if t else mix(h, i)

    def mk(self, x):
        i = x % len(self.variants)
        t = self.variants[i][1]
        return (i, t.mk(mix(x, 99)) if t else None)

    def st(self, v):
        i, p = v
        t = self.variants[i][1]
        return (i, t.st(p) if t else None)

    def decl(self):
        out = [f'union {self.ax} {{']
        out += [f'    {n}: {t.ax},' if t else f'    {n},' for n, t in self.variants]
        return out + ['}', '']

    def helpers(self):
        n = len(self.variants)
        out = [f'fn hs_{self.id}(h: u64, v: {self.ax}) u64 {{', '    match v {']
        for i, (vn, t) in enumerate(self.variants):
            out.append(f'        .{vn}(p) => {{ return hs_{t.id}(mix(h, {i}), p); }}' if t else
                       f'        .{vn} => {{ return mix(h, {i}); }}')
        out += ['    }', '}', '', f'fn mk_{self.id}(x: u64) {self.ax} {{', f'    match x % {n} {{']
        for i, (vn, t) in enumerate(self.variants):
            c = f'{self.ax}.{vn}(mk_{t.id}(mix(x, 99)))' if t else f'{self.ax}.{vn}'
            out.append(f'        {i if i < n - 1 else "else"} => {{ return {c}; }}')
        out += ['    }', '}', '', f'fn st_{self.id}(v: {self.ax}) {self.ax} {{', '    match v {']
        for vn, t in self.variants:
            out.append(f'        .{vn}(p) => {{ return {self.ax}.{vn}(st_{t.id}(p)); }}' if t else
                       f'        .{vn} => {{ return {self.ax}.{vn}; }}')
        return out + ['    }', '}', '']


class OptT(Ty):
    def __init__(self, ident, inner):
        self.id, self.inner = ident, inner
        self.ax = f'?{inner.ax}'
        self.const_ok = inner.const_ok

    def rand(self, r):
        return None if r.random() < 0.3 else ('S', self.inner.rand(r))

    def lit(self, v):
        return '.none' if v is None else f'.some({self.inner.lit(v[1])})'

    def hash(self, h, v):
        return mix(h, 0) if v is None else self.inner.hash(mix(h, 1), v[1])

    def mk(self, x):
        return None if x & 3 == 0 else ('S', self.inner.mk(mix(x, 5)))

    def st(self, v):
        return None if v is None else ('S', self.inner.st(v[1]))

    def helpers(self):
        i = self.inner.id
        return [f'fn hs_{self.id}(h: u64, v: {self.ax}) u64 {{', '    if let .some(p) = v {',
                f'        return hs_{i}(mix(h, 1), p);', '    }', '    return mix(h, 0);', '}', '',
                f'fn mk_{self.id}(x: u64) {self.ax} {{', '    if (x & 3) == 0 {', '        return .none;', '    }',
                f'    return mk_{i}(mix(x, 5));', '}', '',
                f'fn st_{self.id}(v: {self.ax}) {self.ax} {{', '    match v {',
                f'        .some(p) => {{ return .some(st_{i}(p)); }}', '        .none => { return .none; }', '    }', '}', '']


class ErrT(Ty):
    def __init__(self, ident, inner):
        self.id, self.inner = ident, inner
        self.ax = f'Errs!{inner.ax}'
        self.const_ok = inner.const_ok

    def rand(self, r):
        return ('E', r.randrange(len(ERRS))) if r.random() < 0.3 else ('O', self.inner.rand(r))

    def lit(self, v):
        return f'error.{ERRS[v[1]]}' if v[0] == 'E' else self.inner.lit(v[1])

    def hash(self, h, v):
        return mix(h, 101 + v[1]) if v[0] == 'E' else self.inner.hash(mix(h, 0), v[1])

    def mk(self, x):
        return ('E', (x >> 8) % len(ERRS)) if x % 5 == 0 else ('O', self.inner.mk(mix(x, 6)))

    def st(self, v):
        return v if v[0] == 'E' else ('O', self.inner.st(v[1]))

    def helpers(self):
        i = self.inner.id
        return [f'fn hs_{self.id}(h: u64, v: {self.ax}) u64 {{',
                '    let p = v catch |e| {', '        return mix(h, 100 + eidx(e));', '    };',
                f'    return hs_{i}(mix(h, 0), p);', '}', '',
                f'fn mk_{self.id}(x: u64) {self.ax} {{', '    if (x % 5) == 0 {', '        match (x >> 8) % 3 {',
                '            0 => { return error.Bad; }', '            1 => { return error.Worse; }',
                '            else => { return error.Ugly; }', '        }', '    }',
                f'    return mk_{i}(mix(x, 6));', '}', '',
                f'fn st_{self.id}(v: {self.ax}) {self.ax} {{', '    let p = v catch |e| {', '        return e;', '    };',
                f'    return st_{i}(p);', '}', '']


class VecT(Ty):
    const_ok = False

    def __init__(self, lane, bits):
        self.lane, self.bits = lane, bits
        self.n = bits // LANEW[lane.ax]
        self.id = self.ax = f'{lane.ax}x{self.n}'
        self.isf = isinstance(lane, FloatT)
        self.store = ('u32' if lane.ax == 'f32' else 'u64') if self.isf else lane.ax

    def rand(self, r):
        return tuple(self.lane.rand(r) for _ in range(self.n))

    def raw(self, v):
        return [f_bits(x, self.lane.ax) if self.isf else x for x in v]

    def hash(self, h, v):
        if self.isf:
            for x in v:
                h = self.lane.hash(h, x)
            return h
        w = LANEW[self.lane.ax]
        word, fill = 0, 0
        for x in v:
            word |= (x & ((1 << w) - 1)) << fill
            fill += w
            if fill == 64:
                h = mix(h, word)
                word, fill = 0, 0
        return h

    def mk(self, x):
        return tuple(self.lane.mk(mix(x, i + 1)) for i in range(self.n))

    def st(self, v):
        if self.isf:
            return tuple(self.lane.st(x) for x in v)
        return tuple(wrap_t(x + self.lane.C, self.lane.ax) for x in v)

    def load(self, g):
        if self.isf:
            return f'@bit_cast({self.ax}, @load({self.store}x{self.n}, {g}[..]))'
        return f'@load({self.ax}, {g}[..])'

    def helpers(self):
        l = self.lane.ax
        out = [f'fn hs_{self.id}(h0: u64, v: {self.ax}) u64 {{']
        if self.isf:
            out.append('    let mut h = h0;')
            out += [f'    h = hs_{l}(h, @extract_lane(v, {i}));' for i in range(self.n)]
            out.append('    return h;')
        else:
            out.append(f'    return hb{self.bits}(h0, @bit_cast(u64x{self.bits // 64}, v));')
        out += ['}', '', f'fn mk_{self.id}(x: u64) {self.ax} {{',
                f'    return @vec({self.ax}, ' + ', '.join(f'mk_{l}(mix(x, {i + 1}))' for i in range(self.n)) + ');', '}', '',
                f'fn st_{self.id}(v: {self.ax}) {self.ax} {{']
        if self.isf:
            out.append(f'    let a: {self.ax} = @splat({flit(self.lane.c1)});')
            out.append(f'    let b: {self.ax} = @splat({flit(self.lane.c2)});')
            out.append('    return (v * a) + b;')
        else:
            out.append(f'    let c: {self.ax} = @splat({self.lane.C});')
            out.append('    return v +% c;')
        return out + ['}', '']


def leaves(t):
    """The scalars in a value of t, at most (a bound on the sizes)."""
    if isinstance(t, StructT):
        return sum(leaves(f) for _, f in t.fields)
    if isinstance(t, ArrayT):
        return t.n * leaves(t.elem)
    if isinstance(t, UnionT):
        return 1 + max([leaves(v) for _, v in t.variants if v] or [0])
    if isinstance(t, (OptT, ErrT)):
        return 1 + leaves(t.inner)
    if isinstance(t, VecT):
        return t.n
    return 1


# ---- places and values in the Python model ---------------------------------

def get_path(v, path):
    for i in path:
        v = v[i]
    return v


def set_path(v, path, nv):
    if not path:
        return nv
    lst = list(v)
    lst[path[0]] = set_path(v[path[0]], path[1:], nv)
    return tuple(lst)


class Ref:
    """A pointer: a place in a store (the globals or a frame) and a path of
    field and element indices."""

    def __init__(self, store, key, path=()):
        self.store, self.key, self.path = store, key, tuple(path)

    def get(self):
        return get_path(self.store[self.key], self.path)

    def set(self, nv):
        self.store[self.key] = set_path(self.store[self.key], self.path, nv)


class Sl:
    """A slice of an array place."""

    def __init__(self, ref, lo, hi):
        self.ref, self.lo, self.hi = ref, lo, hi

    def __len__(self):
        return self.hi - self.lo

    def get(self, i):
        return self.ref.get()[self.lo + i]

    def set(self, i, v):
        arr = list(self.ref.get())
        arr[self.lo + i] = v
        self.ref.set(tuple(arr))

    def sub(self, a):
        return Sl(self.ref, self.lo + a, self.hi)


# ---- parameters and arguments ---------------------------------------------

class PTy:
    """A parameter's type: ('val', T), ('ptr', T, mut), ('slice', T, mut) or
    ('fn', Sig)."""

    def __init__(self, kind, t, mut=False):
        self.kind, self.t, self.mut = kind, t, mut

    def ax(self):
        if self.kind == 'val':
            return self.t.ax
        if self.kind == 'ptr':
            return ('*mut ' if self.mut else '*') + self.t.ax
        if self.kind == 'slice':
            return ('[]mut ' if self.mut else '[]') + self.t.ax
        return self.t.ax()

    def key(self):
        return (self.kind, self.t.ax if self.kind != 'fn' else self.t.ax(), self.mut)


class Sig:
    def __init__(self, params, ret):
        self.params, self.ret = params, ret  # [PTy], Ty or None

    def ax(self):
        return 'fn(' + ', '.join(p.ax() for p in self.params) + ') ' + (self.ret.ax if self.ret else 'void')


class Arg:
    """An argument: its text, its value in an environment, and the lines
    that come before and after the call (a local it points to, and its
    hash)."""

    def __init__(self, text, ev, pre=(), post=None):
        self.text, self.ev, self.pre, self.post = text, ev, list(pre), post


# ---- statements -------------------------------------------------------------

class St:
    """A statement of a body: `lines` its Axiom text, `run(env)` its effect."""

    def __init__(self, lines, run):
        self.lines, self.run = lines, run


class Fn:
    def __init__(self, name, sig, level):
        self.name, self.sig, self.level = name, sig, level
        self.pnames = [f'p{i}' for i in range(len(sig.params))]
        self.body = []
        self.ret = None  # (text, ev)
        self.seed = 0

    def call(self, args, prog):
        prog.steps += 1
        if prog.steps > 2000000:
            raise RuntimeError('too many calls')
        env = dict(zip(self.pnames, args))
        env['h'] = self.seed
        try:
            for s in self.body:
                s.run(env)
            if self.sig.ret is None:
                return None
            v = self.ret[1](env)
            return v
        except Prop as e:
            return ('E', e.k)


# ---- the program --------------------------------------------------------------

GENERICS = '''fn gpick[T](c: bool, a: T, b: T) T {
    if c {
        return a;
    }
    return b;
}

fn gswap[T](a: *mut T, b: *mut T) void {
    let t = a.*;
    a.* = b.*;
    b.* = t;
}

fn gfold[T, const H: fn(u64, T) u64](h: u64, xs: []T) u64 {
    let mut r = h;
    for x in xs {
        r = H(r, x);
    }
    return r;
}

fn gderef[T, const H: fn(u64, T) u64](h: u64, p: *T) u64 {
    return H(h, p.*);
}

fn gmk[T, const M: fn(u64) T](x: u64) T {
    return M(x);
}

fn gget[T, const N: usize](a: [N]T, i: usize) T {
    return a[i % N];
}

fn gsum[T: Int](a: T, b: T, c: T) T {
    return (a +% b) ^ c;
}

fn gmax[T: Ord](a: T, b: T) T {
    return @max(a, b);
}

struct Two[A, B] {
    a: A,
    b: B,

    fn first(self: Self) A {
        return self.a;
    }

    fn second(self: *Self) B {
        return self.b;
    }
}

fn eidx(e: Errs) u64 {
    match e {
        error.Bad => { return 1; }
        error.Worse => { return 2; }
        error.Ugly => { return 3; }
    }
}
'''


class Prog:
    def __init__(self, seed):
        self.r = random.Random(seed)
        self.steps = 0
        r = self.r
        self.ints = {n: IntT(n, r) for n in INTS}
        self.floats = {n: FloatT(n, r) for n in ('f32', 'f64')}
        self.boolt = BoolT()
        self.scalars = list(self.ints.values()) + list(self.floats.values()) + [self.boolt]
        self.types = list(self.scalars)
        self.ntypes = 0
        self.vecs = []
        for lane, bits in [('i32', 128), ('f32', 128), ('u8', 128), ('f64', 128), ('i64', 128), ('u16', 128),
                           ('i32', 256), ('f64', 256), ('f32', 256), ('i16', 512), ('f32', 512)]:
            lt = self.ints.get(lane) or self.floats[lane]
            self.vecs.append(VecT(lt, bits))
        self.types += self.vecs
        self.enums = []
        for _ in range(r.randint(2, 4)):
            self.add(EnumT(self.nid(), r))
        for _ in range(r.randint(14, 22)):
            self.add(self.composite())
        self.globals = {}  # name -> value
        self.gtypes = {}  # name -> Ty
        self.gorder = []
        self.vglobals = {}  # vector type id -> [names]
        self.seeds = [r.getrandbits(64) if r.random() < 0.8 else r.choice([0, 1, 5, 10, M64, 1 << 63]) for _ in range(64)]
        self.depth = [r.choice([0, 1, 2, 3, 5, 8, 13]) for _ in range(8)]
        self.make_globals()
        self.fns, self.tables, self.recs = [], [], {}
        self.generic_uses = set()
        self.nlocal = 0

    def nid(self):
        self.ntypes += 1
        return f't{self.ntypes}'

    def add(self, t):
        if t not in self.types:
            self.types.append(t)
        return t

    def pick_type(self, depth, vec=False, const=False):
        r = self.r
        pool = [t for t in self.types if (vec or not isinstance(t, VecT)) and (not const or t.const_ok)]
        if depth > 0 and r.random() < 0.25:
            t = self.composite(depth)
            if (not const or t.const_ok) and (vec or not isinstance(t, VecT)):
                return self.add(t)
        return r.choice(pool)

    def composite(self, depth=2):
        while True:
            t = self.composite1(depth)
            if leaves(t) <= 48:
                return t

    def composite1(self, depth):
        r = self.r
        k = r.choice(['struct', 'struct', 'struct', 'array', 'union', 'opt', 'err', 'two', 'struct'])
        if k == 'struct':
            n = r.choice([1, 2, 3, 4, 5, 6, 8, 11])
            fields = []
            for i in range(n):
                fields.append((f'f{i}', self.pick_type(depth - 1)))
            return StructT(self.nid(), fields)
        if k == 'two':
            a, b = self.pick_type(depth - 1), self.pick_type(depth - 1)
            return StructT(self.nid(), [('a', a), ('b', b)], generic=(a, b))
        if k == 'array':
            return ArrayT(self.nid(), self.pick_type(depth - 1), r.choice([1, 2, 3, 4, 5, 7]))
        if k == 'union':
            n = r.randint(1, 5)
            return UnionT(self.nid(), [(f'V{i}', self.pick_type(depth - 1) if r.random() < 0.7 else None)
                                       for i in range(n)])
        inner = self.pick_type(depth - 1)
        while isinstance(inner, (OptT, ErrT)):
            inner = self.pick_type(depth - 1)
        return OptT(self.nid(), inner) if k == 'opt' else ErrT(self.nid(), inner)

    # -- globals --

    def make_globals(self):
        r = self.r
        n = 0
        for t in list(self.types):
            if isinstance(t, VecT):
                self.vglobals[t.id] = []
                for _ in range(2):
                    name = f'GV{n}'
                    n += 1
                    self.globals[name] = t.rand(r)
                    self.gtypes[name] = t
                    self.gorder.append(name)
                    self.vglobals[t.id].append(name)
                continue
            if not t.const_ok:
                continue
            for _ in range(r.choice([1, 1, 2])):
                name = f'G{n}'
                n += 1
                self.globals[name] = t.rand(r)
                self.gtypes[name] = t
                self.gorder.append(name)
            if r.random() < 0.35 and leaves(t) <= 16:
                at = ArrayT(self.nid(), t, r.randint(3, 8))
                self.add(at)
                name = f'GA{n}'
                n += 1
                self.globals[name] = at.rand(r)
                self.gtypes[name] = at
                self.gorder.append(name)

    def globals_of(self, t):
        return [g for g in self.gorder if self.gtypes[g] is t and not isinstance(t, VecT)]

    def places_of(self, t):
        """(text, global, path) of the places of type t in the globals."""
        out = []

        def walk(text, g, path, gt, d):
            if gt is t:
                out.append((text, g, path))
            if d == 0:
                return
            if isinstance(gt, StructT):
                for i, (n, ft) in enumerate(gt.fields):
                    walk(f'{text}.{n}', g, path + (i,), ft, d - 1)
            elif isinstance(gt, ArrayT):
                for i in range(gt.n):
                    walk(f'{text}[{i}]', g, path + (i,), gt.elem, d - 1)

        for g in self.gorder:
            if not isinstance(self.gtypes[g], VecT):
                walk(g, g, (), self.gtypes[g], 2)
        return out

    def arrays_of(self, t):
        return [g for g in self.gorder if isinstance(self.gtypes[g], ArrayT) and self.gtypes[g].elem is t]

    # -- functions --

    def local(self, prefix='l'):
        self.nlocal += 1
        return f'{prefix}{self.nlocal}'

    def rand_ptype(self, many):
        r = self.r
        x = r.random()
        if many or x < 0.68:
            return PTy('val', self.pick_type(1, vec=True))
        if x < 0.82:
            t = self.pick_type(1)
            return PTy('ptr', t, r.random() < 0.6)
        if x < 0.94:
            t = self.pick_type(0)
            return PTy('slice', t, r.random() < 0.5)
        sigs = [f.sig for f in self.fns if f.level == self.cur_level - 1]
        if not sigs:
            return PTy('val', self.pick_type(1, vec=True))
        return PTy('fn', r.choice(sigs))

    def rand_sig(self):
        r = self.r
        x = r.random()
        many = False
        if x < 0.4:
            n = r.randint(0, 3)
        elif x < 0.75:
            n = r.randint(4, 8)
        else:
            n, many = r.randint(9, 30), True
        params = []
        for _ in range(n):
            if many and r.random() < 0.85:
                t = r.choice(list(self.ints.values()) + list(self.floats.values()) * 3 + self.vecs * 2 + [self.boolt])
                params.append(PTy('val', t))
            else:
                params.append(self.rand_ptype(False))
        ret = None if r.random() < 0.08 else self.pick_type(1, vec=True)
        if ret is not None and r.random() < 0.3:
            errs = [t for t in self.types if isinstance(t, ErrT)]
            if errs and r.random() < 0.5:
                ret = r.choice(errs)
            elif not isinstance(ret, (OptT, ErrT)):
                ret = self.add(ErrT(self.nid(), ret))
        return Sig(params, ret)

    def build(self):
        r = self.r
        nlev = 4
        for level in range(nlev):
            self.cur_level = level
            count = r.randint(5, 8) if level < nlev - 1 else r.randint(4, 6)
            made = []
            while len(made) < count:
                sig = self.rand_sig()
                fam = r.choice([1, 1, 1, 2, 3]) if level < nlev - 1 else 1
                group = []
                for _ in range(fam):
                    f = Fn(f'f{len(self.fns)}', sig, level)
                    f.seed = r.getrandbits(64)
                    self.fns.append(f)
                    group.append(f)
                    made.append(f)
                if fam > 1:
                    self.tables.append((f'FT{len(self.tables)}', sig, group))
            for f in made:
                self.make_body(f)

    def arg_for(self, pt, env_vars, ctx, j):
        """An argument for a parameter of type pt. env_vars: [(name,
        PTy)] of the caller's parameters; ctx: 'fn' (a body, which has h)
        or 'top' (run)."""
        r = self.r
        t = pt.t
        seed_text = f'mix(h, {j})' if ctx == 'fn' else f'SEEDS[{j % 64}]'
        seed_ev = (lambda env: mix(env['h'], j)) if ctx == 'fn' else (lambda env: self.seeds[j % 64])
        if pt.kind == 'val':
            opts = []
            for n, p in env_vars:
                if p.kind == 'val' and p.t is t:
                    opts.append(Arg(n, lambda env, n=n: env[n]))
                if p.kind == 'val' and isinstance(p.t, StructT):
                    for i, (fname, ft) in enumerate(p.t.fields):
                        if ft is t:
                            opts.append(Arg(f'{n}.{fname}', lambda env, n=n, i=i: env[n][i]))
                if p.kind == 'val' and isinstance(p.t, ArrayT) and p.t.elem is t:
                    k = r.randrange(p.t.n)
                    opts.append(Arg(f'{n}[{k}]', lambda env, n=n, k=k: env[n][k]))
                if p.kind == 'ptr' and p.t is t:
                    opts.append(Arg(f'{n}.*', lambda env, n=n: env[n].get()))
            for g in self.globals_of(t):
                opts.append(Arg(g, lambda env, g=g: self.globals[g]))
            if isinstance(t, VecT):
                for g in self.vglobals[t.id]:
                    opts.append(Arg(t.load(g), lambda env, g=g: self.globals[g]))
            mk = Arg(f'mk_{t.id}({seed_text})', lambda env: t.mk(seed_ev(env)))
            if not opts or r.random() < 0.35:
                return mk
            return r.choice(opts)
        if pt.kind == 'ptr':
            opts = []
            for n, p in env_vars:
                if p.kind == 'ptr' and p.t is t and (p.mut or not pt.mut):
                    opts.append(Arg(n, lambda env, n=n: env[n]))
                if p.kind == 'val' and p.t is t and not pt.mut:
                    opts.append(Arg(f'&{n}', lambda env, n=n: Ref(env, n)))
            amp = '&mut ' if pt.mut else '&'
            for text, g, path in self.places_of(t):
                opts.append(Arg(amp + text, lambda env, g=g, path=path: Ref(self.globals, g, path)))
            lname = self.local()
            init_text = f'mk_{t.id}({seed_text})'
            init = Arg(init_text, lambda env: t.mk(seed_ev(env)))
            gl = self.globals_of(t)
            if gl and r.random() < 0.5:
                g = r.choice(gl)
                init = Arg(g, lambda env, g=g: self.globals[g])
            loc = Arg(amp + lname, None, pre=[f'let mut {lname}: {t.ax} = {init.text};'], post=(lname, t))

            def ev(env, lname=lname, init=init):
                env[lname] = init.ev(env)
                return Ref(env, lname)
            loc.ev = ev
            if not opts or r.random() < 0.4:
                return loc
            return r.choice(opts)
        if pt.kind == 'slice':
            opts = []
            for n, p in env_vars:
                if p.kind == 'slice' and p.t is t and (p.mut or not pt.mut):
                    opts.append(Arg(n, lambda env, n=n: env[n]))
                    opts.append(Arg(f'{n}[@min({n}.len, 1)..]', lambda env, n=n: env[n].sub(min(len(env[n]), 1))))
            for g in self.arrays_of(t):
                at = self.gtypes[g]
                lo = r.randint(0, at.n)
                hi = r.randint(lo, at.n)
                if r.random() < 0.3:
                    lo, hi = 0, at.n
                opts.append(Arg(f'{g}[{lo}..{hi}]', lambda env, g=g, lo=lo, hi=hi: Sl(Ref(self.globals, g), lo, hi)))
            lname = self.local()
            n = r.randint(1, 6)
            at = self.add(ArrayT(self.nid(), t, n))
            init = '[' + ', '.join(f'mk_{t.id}(mix({seed_text}, {i}))' for i in range(n)) + ']'
            lo = r.randint(0, n)
            loc = Arg(f'{lname}[{lo}..]', None, pre=[f'let mut {lname}: [{n}]{t.ax} = {init};'], post=(lname, at))

            def ev(env, lname=lname, n=n, lo=lo):
                s = seed_ev(env)
                env[lname] = tuple(t.mk(mix(s, i)) for i in range(n))
                return Sl(Ref(env, lname), lo, n)
            loc.ev = ev
            if not opts or r.random() < 0.4:
                return loc
            return r.choice(opts)
        # a function value
        opts = []
        for n, p in env_vars:
            if p.kind == 'fn' and p.t is t:
                opts.append(Arg(n, lambda env, n=n: env[n]))
        for f in self.fns:
            if f.sig is t:
                opts.append(Arg(f.name, lambda env, f=f: f))
        for name, sig, group in self.tables:
            if sig is t:
                k = r.randrange(len(group))
                opts.append(Arg(f'{name}[{k}]', lambda env, f=group[k]: f))
        return r.choice(opts)

    def call_stmt(self, f, callee_text, callee_ev, sig, env_vars, ctx='fn', top_hash=None):
        """`let c = callee(args);` and the hashes of its result and of the
        locals it was given; at the top, the line printed."""
        r = self.r
        args = [self.arg_for(pt, env_vars, ctx, r.randrange(1000)) for pt in sig.params]
        lines = []
        for a in args:
            lines += a.pre
        call = f'{callee_text}(' + ', '.join(a.text for a in args) + ')'
        ret = sig.ret
        mode = 'plain'
        if ret is not None and isinstance(ret, ErrT) and ctx == 'fn':
            if f.sig.ret is not None and isinstance(f.sig.ret, ErrT) and r.random() < 0.5:
                mode = 'try'
            elif r.random() < 0.3:
                mode = 'catch'
        cname = self.local('c')
        rt = ret
        if ret is None:
            lines.append(call + ';')
        elif mode == 'try':
            lines.append(f'let {cname} = {call}!;')
            rt = ret.inner
        elif mode == 'catch':
            lines.append(f'let {cname} = {call} catch mk_{ret.inner.id}(mix(h, 77));')
            rt = ret.inner
        else:
            lines.append(f'let {cname} = {call};')
        posts = [a.post for a in args if a.post]
        hv = 'h' if ctx == 'fn' else 'hr'
        if ctx == 'top':
            lines.insert(0, 'let mut hr: u64 = 0;')
        if rt is not None:
            lines.append(f'{hv} = hs_{rt.id}({hv}, {cname});')
        for lname, lt in posts:
            lines.append(f'{hv} = hs_{lt.id}({hv}, {lname});')

        def run(env):
            fn = callee_ev(env)
            vals = [a.ev(env) for a in args]
            res = fn.call(vals, self)
            v = res
            if mode == 'try':
                if res[0] == 'E':
                    raise Prop(res[1])
                v = res[1]
            elif mode == 'catch':
                v = res[1] if res[0] == 'O' else ret.inner.mk(mix(env['h'], 77))
            hcur = env.get(hv, 0)
            if rt is not None:
                hcur = rt.hash(hcur, v)
                env[cname] = v
            for lname, lt in posts:
                hcur = lt.hash(hcur, env[lname])
            env[hv] = hcur
        return St(lines, run), cname, rt

    def make_body(self, f):
        r = self.r
        env_vars = list(zip(f.pnames, f.sig.params))
        stmts = []
        results = []  # (name, Ty) of calls' results
        # hash (and change) the parameters
        for n, p in env_vars:
            if p.kind == 'val':
                stmts.append(self.st_hash(n, p.t, False))
            elif p.kind == 'ptr':
                stmts.append(self.st_hash(n, p.t, True))
                if p.mut and r.random() < 0.7:
                    stmts.append(self.st_mut_ptr(n, p.t))
            elif p.kind == 'slice':
                stmts.append(self.st_hash_slice(n, p.t))
                if p.mut and r.random() < 0.7:
                    stmts.append(self.st_mut_slice(n, p.t))
            else:
                st, c, rt = self.call_stmt(f, n, lambda env, n=n: env[n], p.t, env_vars)
                stmts.append(st)
                if rt is not None:
                    results.append((c, rt))
        # calls of lower levels
        lower = [g for g in self.fns if g.level < f.level]
        for _ in range(r.randint(0, 2) if lower else 0):
            g = r.choice(lower)
            st, c, rt = self.call_stmt(f, g.name, lambda env, g=g: g, g.sig, env_vars)
            stmts.append(st)
            if rt is not None:
                results.append((c, rt))
        tables = [t for t in self.tables if t[2][0].level < f.level]
        if tables and r.random() < 0.5:
            name, sig, group = r.choice(tables)
            m = len(group)
            st, c, rt = self.call_stmt(f, f'{name}[@int_cast(usize, h % {m})]',
                                       lambda env, group=group: group[env['h'] % len(group)], sig, env_vars)
            stmts.append(st)
            if rt is not None:
                results.append((c, rt))
        # methods, generics, recursion
        for _ in range(r.randint(0, 3)):
            s = self.extra(f, env_vars)
            if s:
                stmts.append(s)
        if f.sig.ret is not None and isinstance(f.sig.ret, ErrT) and r.random() < 0.5:
            k, m, e = r.randrange(len(ERRS)), r.choice([3, 5, 7]), r.randrange(3)
            stmts.append(self.st_errret(m, e, k))
        r.shuffle(stmts)
        f.body = stmts
        f.ret = self.ret_expr(f, env_vars, results)

    def st_errret(self, m, e, k):
        def run(env):
            if env['h'] % m == e:
                raise Prop(k)
        return St([f'if (h % {m}) == {e} {{', f'    return error.{ERRS[k]};', '}'], run)

    def st_hash(self, n, t, ptr):
        if ptr:
            def run(env):
                env['h'] = t.hash(env['h'], env[n].get())
            return St([f'h = hs_{t.id}(h, {n}.*);'], run)

        def run(env):
            env['h'] = t.hash(env['h'], env[n])
        return St([f'h = hs_{t.id}(h, {n});'], run)

    def st_mut_ptr(self, n, t):
        def run(env):
            p = env[n]
            p.set(t.st(p.get()))
        return St([f'{n}.* = st_{t.id}({n}.*);'], run)

    def st_hash_slice(self, n, t):
        x = self.local('x')

        def run(env):
            s = env[n]
            h = mix(env['h'], len(s))
            for i in range(len(s)):
                h = t.hash(h, s.get(i))
            env['h'] = h
        return St([f'h = mix(h, @as(u64, {n}.len));', f'for {x} in {n} {{', f'    h = hs_{t.id}(h, {x});', '}'], run)

    def st_mut_slice(self, n, t):
        r = self.r
        if r.random() < 0.5:
            e = self.local('e')

            def run(env):
                s = env[n]
                for i in range(len(s)):
                    s.set(i, t.st(s.get(i)))
            return St([f'for &mut {e} in {n} {{', f'    {e}.* = st_{t.id}({e}.*);', '}'], run)
        k = r.randint(0, 4)

        def run(env):
            s = env[n]
            if len(s) > k:
                s.set(k, t.st(s.get(k)))
        return St([f'if {n}.len > {k} {{', f'    {n}[{k}] = st_{t.id}({n}[{k}]);', '}'], run)

    def value_of(self, t, env_vars, j):
        """A value of type t for a statement (from a parameter or made)."""
        return self.arg_for(PTy('val', t), env_vars, 'fn', j)

    def extra(self, f, env_vars):
        r = self.r
        kind = r.choice(['method', 'method', 'pick', 'swap', 'fold', 'mk', 'get', 'sum', 'max', 'rec', 'rec', 'two',
                         'ref', 'ref'])
        structs = [t for t in self.types if isinstance(t, StructT) and t.named]
        if kind == 'method':
            ptrs = [(n, p) for n, p in env_vars if p.kind == 'ptr' and isinstance(p.t, StructT) and p.t.named]
            vals = [(n, p) for n, p in env_vars if p.kind == 'val' and isinstance(p.t, StructT) and p.t.named]
            m = r.choice(['get', 'peek', 'bump', 'local', 'with'])
            if m in ('peek', 'bump') and ptrs:
                n, p = r.choice(ptrs)
                t = p.t
                if m == 'bump' and p.mut:
                    def run(env, n=n, t=t):
                        ref = env[n]
                        ref.set(t.bump(ref.get(), env['h']))
                    return St([f'{n}.bump(h);'], run)

                def run(env, n=n, t=t):
                    env['h'] = mix(env['h'], t.hash(13, env[n].get()))
                return St([f'h = mix(h, {n}.peek());'], run)
            if vals and r.random() < 0.6:
                n, p = r.choice(vals)
                t = p.t
                a = Arg(n, lambda env, n=n: env[n])
            else:
                if not structs:
                    return None
                t = r.choice(structs)
                a = self.value_of(t, env_vars, r.randrange(1000))
            if m == 'get':
                def run(env, a=a, t=t):
                    env['h'] = mix(env['h'], t.hash(mix(env['h'], 7), a.ev(env)))
                return St([f'h = mix(h, {a.text}.get(h));'] if a.text.isidentifier() else
                          [f'h = mix(h, {t.ax}.get({a.text}, h));'], run)
            l = self.local()
            if m == 'with':
                def run(env, a=a, t=t):
                    env['h'] = t.hash(env['h'], t.bump(a.ev(env), env['h']))
                return St([f'let {l} = {t.ax}.with({a.text}, h);' if not a.text.isidentifier() else
                           f'let {l} = {a.text}.with(h);', f'h = hs_{t.id}(h, {l});'], run)
            # a method of *mut Self on a local, and of *Self through &
            def run(env, a=a, t=t):
                v = t.bump(a.ev(env), env['h'])
                env['h'] = mix(t.hash(env['h'], v), t.hash(13, v))
            return St([f'let mut {l}: {t.ax} = {a.text};', f'{l}.bump(h);', f'h = hs_{t.id}(h, {l});',
                       f'h = mix(h, {l}.peek());'], run)
        if kind == 'pick':
            t = self.pick_type(0, vec=True)
            a, b = self.value_of(t, env_vars, r.randrange(1000)), self.value_of(t, env_vars, r.randrange(1000))
            g = self.local('g')
            explicit = r.random() < 0.5
            fn = f'gpick.[{t.ax}]' if explicit else 'gpick'

            def run(env, a=a, b=b, t=t):
                c = env['h'] & 1 == 1
                av, bv = a.ev(env), b.ev(env)
                env['h'] = t.hash(env['h'], av if c else bv)
            return St([f'let {g} = {fn}((h & 1) == 1, {a.text}, {b.text});', f'h = hs_{t.id}(h, {g});'], run)
        if kind == 'swap':
            t = self.pick_type(0, vec=True)
            a, b = self.value_of(t, env_vars, r.randrange(1000)), self.value_of(t, env_vars, r.randrange(1000))
            la, lb = self.local(), self.local()

            def run(env, a=a, b=b, t=t):
                av, bv = a.ev(env), b.ev(env)
                env['h'] = t.hash(t.hash(env['h'], bv), av)
            return St([f'let mut {la}: {t.ax} = {a.text};', f'let mut {lb}: {t.ax} = {b.text};',
                       f'gswap(&mut {la}, &mut {lb});', f'h = hs_{t.id}(hs_{t.id}(h, {la}), {lb});'], run)
        if kind == 'fold':
            sl = [(n, p) for n, p in env_vars if p.kind == 'slice']
            if sl:
                n, p = r.choice(sl)
                t = p.t
                a = Arg(n, lambda env, n=n: env[n])
            else:
                arrs = [g for g in self.gorder if isinstance(self.gtypes[g], ArrayT)]
                if not arrs:
                    return None
                g = r.choice(arrs)
                t = self.gtypes[g].elem
                a = Arg(f'{g}[1..]', lambda env, g=g: Sl(Ref(self.globals, g), 1, self.gtypes[g].n))

            def run(env, a=a, t=t):
                s = a.ev(env)
                h = env['h']
                for i in range(len(s)):
                    h = t.hash(h, s.get(i))
                env['h'] = h
            return St([f'h = gfold.[{t.ax}, hs_{t.id}](h, {a.text});'], run)
        if kind == 'ref':
            vals = [(n, p) for n, p in env_vars if p.kind == 'val']
            if not vals:
                return None
            n, p = r.choice(vals)
            t = p.t

            def run(env, n=n, t=t):
                env['h'] = t.hash(env['h'], env[n])
            return St([f'h = gderef.[{t.ax}, hs_{t.id}](h, &{n});'], run)
        if kind == 'mk':
            t = self.pick_type(0, vec=True)
            g = self.local('g')

            def run(env, t=t):
                env['h'] = t.hash(env['h'], t.mk(mix(env['h'], 3)))
            return St([f'let {g} = gmk.[{t.ax}, mk_{t.id}](mix(h, 3));', f'h = hs_{t.id}(h, {g});'], run)
        if kind == 'get':
            arrs = [t for t in self.types if isinstance(t, ArrayT) and t.id]
            if not arrs:
                return None
            t = r.choice(arrs)
            a = self.value_of(t, env_vars, r.randrange(1000))
            g = self.local('g')

            def run(env, a=a, t=t):
                av = a.ev(env)
                env['h'] = t.elem.hash(env['h'], av[env['h'] % 5 % t.n])
            return St([f'let {g} = gget({a.text}, @int_cast(usize, h % 5));', f'h = hs_{t.elem.id}(h, {g});'], run)
        if kind in ('sum', 'max'):
            ts = list(self.ints.values()) + (list(self.floats.values()) if kind == 'max' else [])
            t = r.choice(ts)
            vs = [self.value_of(t, env_vars, r.randrange(1000)) for _ in range(3 if kind == 'sum' else 2)]
            g = self.local('g')
            explicit = r.random() < 0.5
            fn = f'g{kind}.[{t.ax}]' if explicit else f'g{kind}'

            def run(env, vs=vs, t=t, kind=kind):
                xs = [v.ev(env) for v in vs]
                if kind == 'sum':
                    v = wrap_t(xs[0] + xs[1], t.ax) ^ xs[2]
                elif isinstance(t, FloatT):
                    v = f_minmax(xs[0], xs[1], True)
                else:
                    v = max(xs)
                env['h'] = t.hash(env['h'], v)
            return St([f'let {g} = {fn}(' + ', '.join(v.text for v in vs) + ');', f'h = hs_{t.id}(h, {g});'], run)
        if kind == 'rec':
            t = self.pick_type(0, vec=True)
            self.recs[t.id] = t
            a = self.value_of(t, env_vars, r.randrange(1000))
            k = r.randrange(8)

            def run(env, a=a, t=t, k=k):
                env['h'] = self.rec(t, self.depth[k], a.ev(env), env['h'])
            return St([f'h = rec_{t.id}(DEPTH[{k}], {a.text}, h);'], run)
        if kind == 'two':
            twos = [t for t in self.types if isinstance(t, StructT) and t.generic]
            if not twos:
                return None
            t = r.choice(twos)
            a = self.value_of(t, env_vars, r.randrange(1000))
            l, g, g2 = self.local(), self.local('g'), self.local('g')
            A, B = t.generic

            def run(env, a=a, t=t, A=A, B=B):
                v = a.ev(env)
                env['h'] = B.hash(A.hash(env['h'], v[0]), v[1])
            return St([f'let {l}: {t.ax} = {a.text};', f'let {g} = {l}.first();', f'let {g2} = {l}.second();',
                       f'h = hs_{B.id}(hs_{A.id}(h, {g}), {g2});'], run)
        return None

    def rec(self, t, n, x, h):
        stack = []
        while n:
            stack.append(n)
            h = mix(h, n)
            x = t.st(x)
            n -= 1
        h = t.hash(h, x)
        for n in reversed(stack):
            h = mix(h, n)
        return h

    def ret_expr(self, f, env_vars, results):
        r = self.r
        R = f.sig.ret
        if R is None:
            return None
        opts = []
        inner = R.inner if isinstance(R, (ErrT, OptT)) else None
        for n, p in env_vars:
            if p.kind == 'val' and p.t is R:
                opts.append((n, lambda env, n=n: env[n]))
                opts.append((f'st_{R.id}({n})', lambda env, n=n: R.st(env[n])))
            if p.kind == 'ptr' and p.t is R:
                opts.append((f'{n}.*', lambda env, n=n: env[n].get()))
            if p.kind == 'val' and isinstance(p.t, StructT):
                for i, (fname, ft) in enumerate(p.t.fields):
                    if ft is R:
                        opts.append((f'{n}.{fname}', lambda env, n=n, i=i: env[n][i]))
            if inner is not None and p.kind == 'val' and p.t is inner:
                w = 'O' if isinstance(R, ErrT) else 'S'
                opts.append((n, lambda env, n=n, w=w: (w, env[n])))
        for c, rt in results:
            if rt is R:
                opts.append((c, lambda env, c=c: env[c]))
        mk = (f'mk_{R.id}(h)', lambda env: R.mk(env['h']))
        if inner is not None and r.random() < 0.3:
            w = 'O' if isinstance(R, ErrT) else 'S'
            mk = (f'mk_{inner.id}(h)', lambda env: (w, inner.mk(env['h'])))
        if not opts or r.random() < 0.3:
            return mk
        return r.choice(opts)

    # -- the text --

    def fn_text(self, f):
        params = ', '.join(f'{n}: {p.ax()}' for n, p in zip(f.pnames, f.sig.params))
        ret = f.sig.ret.ax if f.sig.ret else 'void'
        out = [f'fn {f.name}({params}) {ret} {{', f'    let mut h: u64 = {f.seed};']
        for s in f.body:
            out += ['    ' + l for l in s.lines]
        if f.ret:
            out.append(f'    return {f.ret[0]};')
        else:
            out.append('    _ = h;')
        out += ['}', '']
        return out

    def write(self, fname, out_dir):
        r = self.r
        self.build()
        # the calls of run: every function once or more, the top level ones more
        cases = []
        for f in self.fns:
            reps = 3 if f.level == 3 else (2 if f.level == 2 else 1)
            for _ in range(reps):
                st, c, rt = self.call_stmt(None, f.name, lambda env, f=f: f, f.sig, [], ctx='top')
                cases.append(st)
        init = dict(self.globals)
        expects = []
        for st in cases:
            env = {'hr': 0, 'h': 0}
            self.steps = 0
            st.run(env)
            expects.append(env['hr'])
        gh = []
        names = [g for g in self.gorder]
        for i in range(0, len(names), 8):
            h = 0
            for g in names[i:i + 8]:
                h = self.gtypes[g].hash(h, self.globals[g])
            gh.append((names[i:i + 8], h))
        out = [
            '// Random functions taking and returning random types (integers, floats,',
            '// structs, arrays, unions, enums, optionals, error unions, vectors), by',
            '// value, by pointer and in slices, with methods, generic functions,',
            '// recursion and calls through function values: each line is a hash of one',
            "// call's results, or of the globals, computed by the rules of the spec",
            '// (§5, §7.14, §9 to §11). (Generated by',
            '// cmd/axiom/internal/tests/tools/gen_exact_calls.py, with the expectations',
            '// computed in Python.)',
            'import "../lib/io.ax" as io;',
            '',
            'error Errs { ' + ', '.join(ERRS) + ' }',
            '',
        ]
        for t in self.types:
            out += t.decl()
        for g in self.gorder:
            t = self.gtypes[g]
            if isinstance(t, VecT):
                out.append(f'let mut {g}: [{t.n}]{t.store} = [{", ".join(str(x) for x in t.raw(init[g]))}];')
            else:
                out.append(f'let mut {g}: {t.ax} = {t.lit(init[g])};')
        out.append(f'let mut SEEDS: [64]u64 = [{", ".join(str(x) for x in self.seeds)}];')
        out.append(f'let mut DEPTH: [8]u32 = [{", ".join(str(x) for x in self.depth)}];')
        for name, sig, group in self.tables:
            out.append(f'let mut {name}: [{len(group)}]{sig.ax()} = [{", ".join(g.name for g in group)}];')
        out += ['', 'fn mix(h: u64, x: u64) u64 {', '    let y = (h ^ x) *% 1099511628211;', '    return y ^ (y >> 32);',
                '}', '']
        for bits in (128, 256, 512):
            k = bits // 64
            out += [f'fn hb{bits}(h0: u64, v: u64x{k}) u64 {{', '    let mut h = h0;']
            out += [f'    h = mix(h, @extract_lane(v, {i}));' for i in range(k)]
            out += ['    return h;', '}', '']
        out += GENERICS.split('\n')
        for t in self.types:
            out += t.helpers()
        for tid, t in self.recs.items():
            out += [f'fn rec_{tid}(n: u32, x: {t.ax}, h: u64) u64 {{', '    if n == 0 {', f'        return hs_{tid}(h, x);',
                    '    }', f'    let r = rec_{tid}(n - 1, st_{tid}(x), mix(h, @as(u64, n)));',
                    '    return mix(r, @as(u64, n));', '}', '']
        for f in self.fns:
            out += self.fn_text(f)
        out.append('export fn run() void {')
        for st, e in zip(cases, expects):
            out.append('    {')
            out += ['        ' + l for l in st.lines]
            out.append(f'        io.uint(hr);  // expect: {e}')
            out.append('    }')
        for names, h in gh:
            out.append('    {')
            out.append('        let mut hg: u64 = 0;')
            for g in names:
                t = self.gtypes[g]
                v = t.load(g) if isinstance(t, VecT) else g
                out.append(f'        hg = hs_{t.id}(hg, {v});')
            out.append(f'        io.uint(hg);  // expect: {h}')
            out.append('    }')
        out.append('}')
        with open(os.path.join(out_dir, fname), 'w') as fh:
            fh.write('\n'.join(out) + '\n')


def main():
    args = sys.argv[1:]
    if len(args) == 2 and args[0].startswith('--seeds='):
        first, count = (int(x) for x in args[0][len('--seeds='):].split(':'))
        for seed in range(first, first + count):
            Prog(seed).write(f'exact_calls_s{seed}.ax', args[1])
        return
    for fname, seed in PROGRAMS:
        Prog(seed).write(fname, RUN)


if __name__ == '__main__':
    main()
