#!/usr/bin/env python3
"""Writes a random C program free of undefined behaviour, for differential
testing of the C front end against another implementation:

    gen_c_programs.py SEED [--no-long] [--loops] [--hosted] [--attrs] [--restrict]

The program is freestanding (it prints through host.h's print_u64) and
prints a checksum of its globals after each call from main. With --loops
it also has larger arrays and loops over them of the shapes the optimizer
turns into memory operations and vector code: copies within and between
arrays (overlapping either way, up and down, by index and by pointer),
fills, reductions (sums, products, xor, min and max, dot products, in
integers and in floating point, whose order is kept), searches and
histograms, with bounds computed at run time. Undefined
behaviour is excluded by construction: signed arithmetic is done in the
corresponding unsigned type and converted back, division and remainder are
guarded, shift counts are reduced modulo the width, floating values are
converted to integers only when in range, array indices are reduced modulo
the length, loops have fixed trip counts, recursion has a depth argument,
and a global fuel counter bounds the total number of calls. Expressions have
no side effects (functions called in them are pure), so the order of
evaluation does not matter. With --no-long the program does not use long, so
that it prints the same on ILP32 and LP64 targets. With --hosted the program
is a hosted one, for the C library's differential testing: it prints with
printf, and each checksum also goes through the library (snprintf's %llx,
%.17g and %a of the values, strtoull's and strtod's readings back, and the
string functions over them), which a mismatch with the other implementation
then shows. With --attrs (for a GNU dialect of C23: --c-std=gnu23) the
functions that write only their locals are declared with the attribute
pure, or, when their parameters are all scalars, C23's [[reproducible]],
for the optimizer to act on (they read globals; they always return).
With --restrict (and --loops) some of the loops over the arrays go through
restrict-qualified pointers, only in the uses C allows (6.7.3.1): block-scope
restricted pointers into different arrays (one written, one read), or into one
array only read, a block in a loop's body whose two restricted pointers
reach different elements in each iteration (the next iteration's overlap the
last's), a structure with restrict-qualified members into different arrays,
and calls of static inline functions with restrict parameters, given
different arrays.
"""
import random
import sys

# name: (spelling, bits, signed, rank); plain char's signedness is the target's
INTS = {
    'bool': ('_Bool', 1, False, 0),
    'char': ('char', 8, None, 1),
    'schar': ('signed char', 8, True, 1),
    'uchar': ('unsigned char', 8, False, 1),
    'short': ('short', 16, True, 2),
    'ushort': ('unsigned short', 16, False, 2),
    'int': ('int', 32, True, 3),
    'uint': ('unsigned', 32, False, 3),
    'long': ('long', 64, True, 4),
    'ulong': ('unsigned long', 64, False, 4),
    'llong': ('long long', 64, True, 5),
    'ullong': ('unsigned long long', 64, False, 5),
}
FLOATS = {'float': 'float', 'double': 'double'}
UNSIGNED_OF = {'int': 'uint', 'long': 'ulong', 'llong': 'ullong', 'uint': 'uint', 'ulong': 'ulong',
               'ullong': 'ullong'}
# the range of each type as doubles that bound a safe conversion: lo < x < hi
F2I = {
    'int': ('-2147483649.0', '2147483648.0'),
    'uint': ('-1.0', '4294967296.0'),
    'long': ('-9223372036854775808.0', '9223372036854775808.0'),
    'ulong': ('-1.0', '18446744073709551616.0'),
    'llong': ('-9223372036854775808.0', '9223372036854775808.0'),
    'ullong': ('-1.0', '18446744073709551616.0'),
}


def is_float(t):
    return t in FLOATS


def is_int(t):
    return t in INTS


def spell(t):
    if t in INTS:
        return INTS[t][0]
    if t in FLOATS:
        return FLOATS[t]
    return t


def promote(t):
    if is_float(t) or INTS[t][3] >= 3:
        return t
    return 'int'


def common(a, b):
    a, b = promote(a), promote(b)
    for f in ('double', 'float'):
        if f in (a, b):
            return f
    if a == b:
        return a
    sa, sb = INTS[a][2], INTS[b][2]
    if sa == sb:
        return a if INTS[a][3] >= INTS[b][3] else b
    s, u = (a, b) if sa else (b, a)
    if INTS[u][3] >= INTS[s][3]:
        return u
    if INTS[s][1] > INTS[u][1]:
        return s
    return UNSIGNED_OF[s]


class Gen:
    def __init__(self, seed, no_long, loops=False):
        self.attrs = False
        self.restrict = False
        self.r = random.Random(seed)
        self.no_long = no_long
        self.loops = loops
        self.ints = [t for t in INTS if not (no_long and t in ('long', 'ulong'))]
        self.scalars = self.ints + list(FLOATS)
        self.out = []
        self.structs = []     # (name, [(field, type, bits or None, array len or None)])
        self.globals = []     # (name, kind, info)
        self.funcs = []       # dicts
        self.label = 0

    def pick(self, xs):
        return self.r.choice(xs)

    def chance(self, p):
        return self.r.random() < p

    # ---- constants ----

    def const(self, t):
        r = self.r
        if t == 'bool':
            return self.pick(['0', '1'])
        if is_float(t):
            v = self.pick([0.0, 1.0, -1.0, 0.5, 1e10, -3.25, 1e-5, 123456.789, 2.0 ** 53, 1e300 if t == 'double' else 3e38,
                           r.uniform(-1000, 1000), float(r.randint(-100, 100))])
            s = repr(v)
            if 'e' not in s and '.' not in s:
                s += '.0'
            s += 'f' if t == 'float' else ''
            return '(%s)' % s if s.startswith('-') else s
        _, bits, signed, _ = INTS[t]
        if signed is None:
            signed = False
            bits = 7
        if signed:
            v = self.pick([0, 1, -1, 2, r.randint(-100, 100), r.randint(-(1 << (bits - 1)), (1 << (bits - 1)) - 1),
                           (1 << (bits - 1)) - 1, -(1 << (bits - 1))])
        else:
            v = self.pick([0, 1, 2, r.randint(0, 255), r.randint(0, (1 << bits) - 1), (1 << bits) - 1])
        if t in ('int',):
            return str(v) if v != -(1 << 31) else '(-2147483647 - 1)'
        if t in ('llong', 'long'):
            suf = 'LL' if t == 'llong' else 'L'
            return ('(%d%s)' % (v, suf)) if v != -(1 << 63) else '(-9223372036854775807%s - 1)' % suf
        if t == 'uint':
            return '%du' % v if self.chance(0.5) else hex(v) + 'u'
        if t == 'ulong':
            return '%dUL' % v
        if t == 'ullong':
            return '%dULL' % v if self.chance(0.5) else hex(v) + 'ULL'
        return '(%s)%d' % (spell(t), v)

    # ---- expressions: each returns (code, type) ----

    def conv_to(self, e, et, t):
        """e converted to t without undefined behaviour (as an explicit cast)."""
        if t == et:
            return e
        if is_float(et) and is_int(t) and t != 'bool':
            pt = t if t in F2I else 'int'
            lo, hi = F2I[pt]
            inner = '((%s > %s && %s < %s) ? (%s)%s : 0)' % (e, lo, e, hi, spell(pt), e)
            return inner if pt == t else '((%s)%s)' % (spell(t), inner)
        return '((%s)%s)' % (spell(t), e)

    def leaf(self, t, env):
        cands = [v for v in env['vars'] if v[1] == t]
        if cands and self.chance(0.7):
            return self.pick(cands)[0], t
        if self.chance(0.3):
            arrs = [a for a in env['arrays'] if a[1] == t]
            if arrs:
                name, _, n = self.pick(arrs)
                return '%s[%s]' % (name, self.index(n, env)), t
        if self.chance(0.3):
            fs = [f for f in env['fields'] if f[1] == t]
            if fs:
                f = self.pick(fs)
                return f[0], f[2]
        if self.chance(0.2):
            ps = [p for p in env['ptrs'] if p[1] == t]
            if ps:
                name, _, n, _b = self.pick(ps)
                if self.chance(0.5):
                    return '*%s' % name, t
                return '%s[%s]' % (name, self.index(n, env)), t
        if self.chance(0.15):
            a2 = [a for a in env['arrays2'] if a[1] == t]
            if a2:
                name, _, a, b = self.pick(a2)
                return '%s[%s][%s]' % (name, self.index(a, env), self.index(b, env)), t
        if t == 'char' and self.chance(0.2):
            return '"abcdefghij"[%s]' % self.index(10, env), 'char'
        if is_int(t) and t != 'bool' and self.chance(0.05) and getattr(self, 'enums', None):
            return self.pick(self.enums), 'int'
        if self.chance(0.05) and env['depth_left'] > 0:
            env['depth_left'] -= 1
            elems = ', '.join(self.expr(t, env, 1)[0] for _ in range(3))
            env['depth_left'] += 1
            return '((%s[3]){ %s })[%s]' % (spell(t), elems, self.index(3, env)), t
        if self.chance(0.1) and env['depth_left'] > 0:
            sf = [f for f in env['pure'] if f['ret'].startswith('struct ')]
            if sf:
                f = self.pick(sf)
                fl = [x for x in self.struct_fields('X', f['ret']) if is_int(x[2]) or is_float(x[2])]
                if fl:
                    x = self.pick(fl)
                    env['depth_left'] -= 1
                    c = self.call(f, env, 1)
                    env['depth_left'] += 1
                    return c + x[0][1:], x[2]
        anyv = env['vars']
        if anyv and self.chance(0.5):
            v = self.pick(anyv)
            return v[0], v[1]
        return self.const(t), t

    def index(self, n, env):
        e, et = self.expr(self.pick(['int', 'uint', 'uchar', 'ullong']), env, 2)
        e = self.conv_to(e, et, 'uint')
        return '(%s %% %du)' % (e, n)

    def expr(self, t, env, depth):
        """An expression whose value is converted to type t."""
        e, et = self.expr1(t, env, depth)
        if len(e) > 2000:
            # (the guards repeat their operands: keep the program's size in bounds)
            e, et = self.leaf(t, env)
        return self.conv_to(e, et, t), t

    def int_expr(self, env, depth):
        t = self.pick(self.ints)
        return self.expr(t, env, depth)

    def expr1(self, t, env, depth):
        if depth <= 0 or self.chance(0.2):
            return self.leaf(t, env)
        k = self.r.randint(0, 13)
        if k <= 3:
            a, at = self.expr1(self.pick(self.scalars) if self.chance(0.3) else t, env, depth - 1)
            b, bt = self.expr1(self.pick(self.scalars) if self.chance(0.3) else t, env, depth - 1)
            return self.arith(a, at, b, bt, self.pick(['+', '-', '*', '+', '-']))
        if k == 4:
            a, at = self.expr1(t, env, depth - 1)
            b, bt = self.expr1(t, env, depth - 1)
            return self.divmod(a, at, b, bt, self.pick(['/', '%']))
        if k == 5 and is_int(t):
            a, at = self.expr1(t if is_int(t) else 'int', env, depth - 1)
            if is_float(at):
                a, at = self.conv_to(a, at, 'int'), 'int'
            b, bt = self.int_expr(env, depth - 1)
            return self.shift(a, at, b, bt, self.pick(['<<', '>>']))
        if k == 6 and is_int(t):
            a, at = self.expr1(t, env, depth - 1)
            b, bt = self.expr1(t, env, depth - 1)
            if is_float(at):
                a, at = self.conv_to(a, at, 'int'), 'int'
            if is_float(bt):
                b, bt = self.conv_to(b, bt, 'int'), 'int'
            return '(%s %s %s)' % (a, self.pick(['&', '|', '^']), b), common(at, bt)
        if k == 7:
            ct = self.pick(self.scalars)
            a, at = self.expr1(ct, env, depth - 1)
            b, bt = self.expr1(ct, env, depth - 1)
            return '(%s %s %s)' % (a, self.pick(['<', '<=', '>', '>=', '==', '!=']), b), 'int'
        if k == 8:
            a, at = self.expr1(self.pick(self.scalars), env, depth - 1)
            b, bt = self.expr1(self.pick(self.scalars), env, depth - 1)
            return '(%s %s %s)' % (a, self.pick(['&&', '||']), b), 'int'
        if k == 9:
            c, _ = self.expr1(self.pick(self.scalars), env, depth - 1)
            a, at = self.expr(t, env, depth - 1)
            b, bt = self.expr(t, env, depth - 1)
            return '(%s ? %s : %s)' % (c, a, b), t
        if k == 10:
            a, at = self.expr1(t, env, depth - 1)
            op = self.pick(['-', '~', '!', '+'])
            if op == '!':
                return '(!%s)' % a, 'int'
            if op == '~':
                if is_float(at):
                    a, at = self.conv_to(a, at, 'uint'), 'uint'
                return '(~%s)' % a, promote(at)
            if op == '-':
                pt = promote(at)
                if is_int(pt) and INTS[pt][2]:
                    u = UNSIGNED_OF[pt]
                    return '((%s)(0%s - (%s)%s))' % (spell(pt), 'u' if u == 'uint' else 'ULL' if u == 'ullong' else 'UL',
                                                     spell(u), a), pt
                return '(-%s)' % a, pt
            return '(+%s)' % a, promote(at)
        if k == 11:
            st = self.pick(self.scalars)
            a, at = self.expr1(st, env, depth - 1)
            return self.conv_to(a, at, t), t
        if k == 12:
            pure = [f for f in env['pure'] if not f['ret'].startswith('struct ')]
            calls = [f for f in pure if f['ret'] == t] or pure
            if calls:
                f = self.pick(calls)
                return self.call(f, env, depth - 1), f['ret']
        if k == 13 and self.chance(0.3):
            a, at = self.expr1(self.pick(self.scalars), env, depth - 1)
            b, bt = self.expr1(t, env, depth - 1)
            return '(%s, %s)' % (a, b), bt
        if k == 13 and env['ptrs'] and self.chance(0.5):
            p = self.pick(env['ptrs'])
            q = [x for x in env['ptrs'] if x[3] == p[3]]
            if q:
                qq = self.pick(q)
                return '(%s %s %s)' % (p[0], self.pick(['==', '!=', '<', '>=']), qq[0]), 'int'
        return self.leaf(t, env)

    def arith(self, a, at, b, bt, op):
        ct = common(at, bt)
        if is_int(ct) and INTS[ct][2]:
            u = UNSIGNED_OF[ct]
            return '((%s)((%s)%s %s (%s)%s))' % (spell(ct), spell(u), a, op, spell(u), b), ct
        return '(%s %s %s)' % (a, op, b), ct

    def divmod(self, a, at, b, bt, op):
        ct = common(at, bt)
        if is_float(ct):
            if op == '%':
                op = '/'
            return '(%s %s %s)' % (a, op, b), ct
        if INTS[ct][2]:
            mn = '(-2147483647 - 1)' if ct == 'int' else '(-9223372036854775807LL - 1)'
            if ct == 'long':
                mn = '(-9223372036854775807L - 1)'
            return '(((%s)%s == 0 || ((%s)%s == %s && (%s)%s == -1)) ? (%s)%s : (%s)%s %s (%s)%s)' % (
                spell(ct), b, spell(ct), a, mn, spell(ct), b, spell(ct), a, spell(ct), a, op, spell(ct), b), ct
        return '(((%s)%s == 0) ? (%s)%s : (%s)%s %s (%s)%s)' % (spell(ct), b, spell(ct), a, spell(ct), a, op, spell(ct), b), ct

    def shift(self, a, at, b, bt, op):
        pt = promote(at)
        w = INTS[pt][1]
        cnt = '((unsigned)%s %% %du)' % (b, w) if INTS[bt][1] <= 32 else '((unsigned)(%s %% %du))' % (
            self.conv_to(b, bt, 'ullong'), w)
        if op == '<<' and INTS[pt][2]:
            u = UNSIGNED_OF[pt]
            return '((%s)((%s)%s << %s))' % (spell(pt), spell(u), a, cnt), pt
        return '(%s %s %s)' % (a, op, cnt), pt

    def call(self, f, env, depth):
        args = []
        for (pn, pt) in f['params']:
            if pt.startswith('struct '):
                args.append(self.struct_value(pt, env))
            elif pt == 'depth':
                args.append(str(self.r.randint(0, 3)))
            else:
                e, et = self.expr(pt, env, depth)
                args.append(e)
        if f['variadic']:
            args += self.var_args(f['vtypes'], env)
        return '%s(%s)' % (f['name'], ', '.join(args))

    def struct_value(self, st, env):
        svs = [v for v in env['svars'] if v[1] == st]
        if svs:
            return self.pick(svs)[0]
        return env['struct_globals'][st]

    # ---- types and globals ----

    def gen_struct(self, i):
        name = 'S%d' % i
        fields = []
        for k in range(self.r.randint(1, 6)):
            r = self.r.random()
            if r < 0.3:
                bt = self.pick(['unsigned int', 'signed int', '_Bool', 'unsigned int'])
                w = 1 if bt == '_Bool' else self.r.randint(1, 32)
                fields.append(('f%d' % k, bt, w, None))
            elif r < 0.45:
                fields.append(('f%d' % k, self.pick(self.scalars), None, self.pick([2, 4, 3])))
            elif r < 0.55 and self.structs:
                fields.append(('f%d' % k, 'struct ' + self.pick(self.structs)[0], None, None))
            else:
                fields.append(('f%d' % k, self.pick(self.scalars), None, None))
        if self.chance(0.2):
            fields.append(('fz', 'unsigned int', 0, None))
            fields.append(('fy', 'unsigned int', self.r.randint(1, 8), None))
        self.structs.append((name, fields))
        lines = ['struct %s {' % name]
        for (fn, ft, w, n) in fields:
            if w is not None:
                lines.append('    %s %s : %d;' % (ft, fn if w else '', w) if w else '    %s : 0;' % ft)
            elif n:
                lines.append('    %s %s[%d];' % (spell(ft), fn, n))
            else:
                lines.append('    %s %s;' % (spell(ft), fn))
        lines.append('};')
        self.out.extend(lines)

    def bf_type(self, bt, w):
        if bt == '_Bool':
            return 'bool'
        if bt == 'unsigned int':
            return 'uint' if w == 32 else 'int'
        return 'int'

    def struct_fields(self, base, st, depth=0):
        """Accessible scalar fields: (code, storage type, read type, is bitfield, bitwidth)."""
        fields = dict(self.structs)[st[len('struct '):]]
        res = []
        for (fn, ft, w, n) in fields:
            if w == 0:
                continue
            if w is not None:
                bt = 'bool' if ft == '_Bool' else ('uint' if ft == 'unsigned int' else 'int')
                res.append(('%s.%s' % (base, fn), bt, self.bf_type(ft, w), True, w))
            elif ft.startswith('struct '):
                if depth < 2:
                    res.extend(self.struct_fields('%s.%s' % (base, fn), ft, depth + 1))
            elif n:
                for j in range(n):
                    res.append(('%s.%s[%d]' % (base, fn, j), ft, ft, False, None))
            else:
                res.append(('%s.%s' % (base, fn), ft, ft, False, None))
        return res

    def struct_init(self, st, depth=0):
        fields = dict(self.structs)[st[len('struct '):]]
        parts = []
        designated = self.chance(0.3)
        for (fn, ft, w, n) in fields:
            if w == 0:
                continue
            if w is not None:
                v = self.r.randint(0, 1) if ft == '_Bool' else (
                    self.r.randint(0, (1 << w) - 1) if ft == 'unsigned int' else self.r.randint(-(1 << (w - 1)), (1 << (w - 1)) - 1))
                p = str(v) if v >= 0 or w < 32 else '(-2147483647 - 1)' if v == -(1 << 31) else str(v)
            elif ft.startswith('struct '):
                p = self.struct_init(ft, depth + 1)
            elif n:
                p = '{ %s }' % ', '.join(self.const(ft) for _ in range(self.r.randint(1, n)))
            else:
                p = self.const(ft)
            if designated and self.chance(0.5):
                parts.append('.%s = %s' % (fn, p))
            elif not designated:
                parts.append(p)
        return '{ %s }' % ', '.join(parts) if parts else '{ 0 }'

    def gen_globals(self, env):
        n = self.r.randint(3, 10)
        for i in range(n):
            t = self.pick(self.scalars)
            name = 'g%d' % i
            q = 'volatile ' if self.chance(0.1) else ''
            self.out.append('static %s%s %s = %s;' % (q, spell(t), name, self.const(t)))
            env['vars'].append((name, t))
            self.globals.append((name, t))
        for i in range(self.r.randint(1, 4)):
            t = self.pick(self.scalars)
            ln = self.pick([2, 4, 5, 8, 16])
            name = 'ga%d' % i
            init = ', '.join(self.const(t) for _ in range(self.r.randint(0, ln)))
            self.out.append('static %s %s[%d] = { %s };' % (spell(t), name, ln, init or '0'))
            env['arrays'].append((name, t, ln))
            self.globals.append((name, 'array', (t, ln)))
            if self.chance(0.6):
                off = self.r.randint(0, ln // 2)
                pn = 'gp%d' % i
                self.out.append('static %s *%s = &%s[%d];' % (spell(t), pn, name, off) if self.chance(0.5) else
                                'static %s *%s = %s + %d;' % (spell(t), pn, name, off))
                env['ptrs'].append((pn, t, ln - off, name))
        if self.loops:
            env['big'] = []
            for i in range(self.r.randint(2, 4)):
                t = self.pick(self.scalars)
                ln = self.pick([17, 33, 64, 100, 257])
                name = 'gb%d' % i
                init = ', '.join(self.const(t) for _ in range(self.r.randint(0, min(ln, 40))))
                self.out.append('static %s %s[%d] = { %s };' % (spell(t), name, ln, init or '0'))
                env['big'].append((name, t, ln))
                self.globals.append((name, 'array', (t, ln)))
        for i, (sn, _) in enumerate(self.structs):
            st = 'struct ' + sn
            name = 'gs%d' % i
            self.out.append('static %s %s = %s;' % (st, name, self.struct_init(st)))
            env['svars'].append((name, st))
            env['struct_globals'][st] = name
            for f in self.struct_fields(name, st):
                env['fields'].append((f[0], f[1], f[2], f[3], f[4]))
            self.globals.append((name, 'struct', st))
        for i, (sn, _) in enumerate(self.structs):
            if not self.chance(0.5):
                continue
            st = 'struct ' + sn
            name = 'gsa%d' % i
            n = self.pick([2, 3, 4])
            self.out.append('static %s %s[%d] = { %s };' % (st, name, n, ', '.join(self.struct_init(st) for _ in range(self.r.randint(1, n)))))
            self.globals.append((name, 'sarray', (st, n)))
            for k in range(n):
                env['svars'].append(('%s[%d]' % (name, k), st))
                for f in self.struct_fields('%s[%d]' % (name, k), st):
                    env['fields'].append(f)
            k = self.r.randrange(n)
            pn = 'gps%d' % i
            self.out.append('static %s *%s = &%s[%d];' % (st, pn, name, k) if self.chance(0.5) else
                            'static %s *%s = %s + %d;' % (st, pn, name, k))
            for f in self.struct_fields('%s->' % pn, st):
                env['fields'].append((f[0].replace('->.', '->'),) + tuple(f[1:]))
            env['sarrays'].append((name, st, n))
        for i in range(self.r.randint(0, 2)):
            t = self.pick(self.scalars)
            a, b = self.pick([2, 3, 4]), self.pick([2, 3, 5])
            name = 'gm%d' % i
            rows = ', '.join('{ %s }' % ', '.join(self.const(t) for _ in range(self.r.randint(1, b))) for _ in range(self.r.randint(1, a)))
            self.out.append('static %s %s[%d][%d] = { %s };' % (spell(t), name, a, b, rows))
            env['arrays2'].append((name, t, a, b))
            self.globals.append((name, 'array2', (t, a, b)))
        if self.chance(0.5):
            vals = sorted(self.r.sample(range(-1000, 1000), self.r.randint(1, 5)))
            self.enums = ['E%d' % k for k in range(len(vals))]
            self.out.append('enum en { %s };' % ', '.join('E%d = %d' % (k, v) for k, v in enumerate(vals)))
        if self.chance(0.7):
            self.out.append('static union { unsigned int u; float f; unsigned char b[4]; short s[2]; } gu = { %du };'
                            % self.r.randint(0, (1 << 32) - 1))
            # (not gu.f: which NaN's payload an operation propagates is not specified, and the bits are seen)
            env['fields'].extend([('gu.u', 'uint', 'uint', False, None), ('gu.b[1]', 'uchar', 'uchar', False, None),
                                  ('gu.s[1]', 'short', 'short', False, None)])
            self.globals.append(('gu', 'union', None))

    # ---- statements ----

    def lvalues(self, env, local_only=False):
        lv = [(v[0], v[1], v[1]) for v in env['vars'] if (v[0] not in env['readonly'])]
        if local_only:
            return [x for x in lv if x[0].startswith('l') or x[0].startswith('p')]
        for f in env['fields']:
            lv.append((f[0], f[1], f[2]))
        return lv

    def assign(self, env, depth, ind):
        lv = self.lvalues(env, env['pure_mode'])
        r = self.r.random()
        if not env['pure_mode'] and r < 0.2 and env['arrays']:
            name, t, n = self.pick(env['arrays'])
            e, _ = self.expr(t, env, depth)
            return ['%s%s[%s] = %s;' % (ind, name, self.index(n, env), e)]
        if not env['pure_mode'] and r < 0.25 and env['sarrays'] and self.chance(0.5):
            name, st, n = self.pick(env['sarrays'])
            src = self.pick([v for v in env['svars'] if v[1] == st])[0]
            return ['%s%s[%s] = %s;' % (ind, name, self.index(n, env), src)]
        if not env['pure_mode'] and r < 0.25 and env['arrays2']:
            name, t, a, b = self.pick(env['arrays2'])
            e, _ = self.expr(t, env, depth)
            return ['%s%s[%s][%s] = %s;' % (ind, name, self.index(a, env), self.index(b, env), e)]
        if not env['pure_mode'] and r < 0.3 and env['ptrs']:
            name, t, n, _ = self.pick(env['ptrs'])
            e, _ = self.expr(t, env, depth)
            return ['%s%s[%s] = %s;' % (ind, name, self.index(n, env), e)]
        if not lv:
            return []
        name, t, rt = self.pick(lv)
        if is_float(t) and self.chance(0.3):
            e, _ = self.expr(t, env, depth)
            return ['%s%s %s %s;' % (ind, name, self.pick(['+=', '-=', '*=', '/=']), e)]
        if is_int(t) and t != 'bool' and self.chance(0.15):
            u = 'ullong' if INTS[t][1] == 64 else 'uint'
            e, _ = self.expr(u, env, depth)
            op = self.pick(['+=', '-=', '*='])
            if not INTS[rt][2] and INTS[rt][3] >= 3 and self.chance(0.5):
                op = self.pick(['/=', '%='])
                e = '(%s | 1u)' % e
            return ['%s%s %s %s;' % (ind, name, op, e)]
        if is_int(t) and self.chance(0.3):
            t = rt
            if not INTS[t][2] and INTS[t][3] >= 3 and t != 'char':
                op = self.pick(['+=', '-=', '*=', '&=', '|=', '^=', '<<=', '>>='])
            else:
                op = self.pick(['&=', '|=', '^='])
            if op in ('<<=', '>>='):
                b, bt = self.int_expr(env, depth)
                return ['%s%s %s (%s %% %du);' % (ind, name, op, self.conv_to(b, bt, 'uint'), INTS[t][1])]
            e, _ = self.expr(t, env, depth)
            return ['%s%s %s %s;' % (ind, name, op, e)]
        if is_int(rt) and not INTS[rt][2] and INTS[rt][3] >= 3 and self.chance(0.15):
            return ['%s%s%s;' % (ind, name, self.pick(['++', '--'])) if self.chance(0.5) else
                    '%s%s%s;' % (ind, self.pick(['++', '--']), name)]
        e, _ = self.expr(t, env, depth)
        return ['%s%s = %s;' % (ind, name, e)]

    def block(self, env, n, depth, ind):
        lines = []
        for _ in range(n):
            lines.extend(self.stmt(env, depth, ind))
        return lines

    def stmt(self, env, depth, ind):
        r = self.r.random()
        if depth <= 0 or r < 0.4:
            if not env['pure_mode'] and self.chance(0.2) and env['impure']:
                f = self.pick(env['impure'])
                lv = self.lvalues(env)
                lv = [x for x in lv if x[1] == f['ret']] or [None]
                tgt = self.pick(lv)
                c = self.call(f, env, 2)
                if tgt:
                    return ['%s%s = %s;' % (ind, tgt[0], c)]
                return ['%s(void)%s;' % (ind, c)]
            return self.assign(env, 3, ind)
        if r < 0.55:
            c, _ = self.expr1(self.pick(self.scalars), env, 3)
            lines = ['%sif (%s) {' % (ind, c)] + self.block(env, self.r.randint(1, 3), depth - 1, ind + '    ')
            if self.chance(0.5):
                lines += ['%s} else {' % ind] + self.block(env, self.r.randint(1, 3), depth - 1, ind + '    ')
            return lines + ['%s}' % ind]
        if self.loops and env.get('big') and r < 0.62:
            return self.array_loop(env, ind)
        if r < 0.7 and env['loopvars']:
            v = env['loopvars'].pop()
            k = self.r.randint(1, 6)
            env['readonly'].add(v)
            kind = self.r.randint(0, 2)
            body = self.block(env, self.r.randint(1, 3), depth - 1, ind + '    ')
            if self.chance(0.3):
                body.append('%s    if (%s) %s;' % (ind, self.expr1('int', env, 2)[0], self.pick(['break', 'continue'])))
            env['readonly'].discard(v)
            env['loopvars'].append(v)
            if kind == 0:
                return ['%sfor (%s = 0; %s < %d; %s++) {' % (ind, v, v, k, v)] + body + ['%s}' % ind]
            if kind == 1:
                return ['%s%s = %d;' % (ind, v, k), '%swhile (%s-- > 0) {' % (ind, v)] + body + ['%s}' % ind]
            return ['%s%s = 0;' % (ind, v), '%sdo {' % ind] + body + ['%s} while (++%s < %d);' % (ind, v, k)]
        if r < 0.8:
            e, et = self.int_expr(env, 2)
            # few cases, or more of them spread out and shifted (code
            # generation makes tables, bit tests and searches of those)
            n = self.r.randint(2, 6)
            scale, off = 1, 0
            if self.chance(0.3):
                n = self.r.randint(7, 40)
                scale = self.r.choice([1, 1, 2, 3, 7, 1000])
                off = self.r.choice([0, -5, 100, -2147000000, 2147000000])
            lines = ['%sswitch ((int)(%s %% %d) + %d) {' % (ind, self.conv_to(e, et, 'ullong'), n * scale, off)]
            cases = self.r.sample(range(n), self.r.randint(1, n))
            for c in cases:
                lines.append('%scase %d:;' % (ind, c * scale + off))
                lines += self.block(env, self.r.randint(0, 2), depth - 1, ind + '    ')
                if self.chance(0.7):
                    lines.append('%s    break;' % ind)
            if self.chance(0.5):
                lines.append('%sdefault:;' % ind)
                lines += self.block(env, self.r.randint(1, 2), depth - 1, ind + '    ')
            return lines + ['%s}' % ind]
        if r < 0.9:
            self.label += 1
            lb = 'L%d' % self.label
            if self.chance(0.5):
                c, _ = self.expr1('int', env, 2)
                return (['%sif (%s) goto %s;' % (ind, c, lb)] + self.block(env, self.r.randint(1, 3), depth - 1, ind)
                        + ['%s%s:;' % (ind, lb)])
            cnt = 'c%d' % self.label
            return (['%s{' % ind, '%s    int %s = 0;' % (ind, cnt), '%s%s:' % (ind, lb)]
                    + self.block(env, self.r.randint(1, 3), depth - 1, ind + '    ')
                    + ['%s    if (++%s < %d) goto %s;' % (ind, cnt, self.r.randint(1, 4), lb), '%s}' % ind])
        return self.assign(env, 3, ind)

    # ---- functions ----

    def array_loop(self, env, ind):
        # a loop over the big arrays (--loops): indices kept in bounds by
        # construction (lo and hi reduced to 0..n - m, the offsets < m)
        name, t, ln = self.pick(env['big'])
        others = [a for a in env['big'] if a[1] == t]
        src, _, sln = self.pick(others)
        m = self.r.randint(0, 3)
        n = min(ln, sln) - m
        e1, t1 = self.int_expr(env, 2)
        e2, t2 = self.int_expr(env, 2)
        lo = '(%s %% %du)' % (self.conv_to(e1, t1, 'uint'), n + 1)
        hi = '(%s %% %du)' % (self.conv_to(e2, t2, 'uint'), n + 1)
        if self.chance(0.3):
            lo = '0'
        if self.chance(0.3):
            hi = '%du' % n
        dx, dy = self.r.randint(0, m), self.r.randint(0, m)
        T = spell(t)
        kind = self.r.randint(0, 9)
        if self.restrict and self.chance(0.5):
            kind = self.r.randint(10, 14)
        b = ['%s{' % ind, '%s    unsigned lo_ = %s, hi_ = %s, ii;' % (ind, lo, hi)]
        i2 = ind + '    '
        # (the arithmetic of an element: unsigned for integers)
        if is_float(t):
            mix = lambda x, y: '%s * (%s)0.5 + %s' % (x, T, y)
        else:
            mix = lambda x, y: '(%s)((unsigned long long)%s * 3u + (unsigned long long)%s)' % (T, x, y)
        if kind >= 10 and kind != 11 and kind != 12 and src == name:
            kind = 11  # (one array: through restricted pointers only read, or apart each iteration)
        if kind == 10:
            # block-scope restricted pointers into two arrays
            b.append('%s{ %s *restrict d_ = %s + %d; const %s *restrict s_ = %s + %d;' % (i2, T, name, dx, T, src, dy))
            b.append('%s  for (ii = lo_; ii < hi_; ii++) d_[ii] = %s; }' % (i2, mix('s_[ii]', 'd_[ii]')))
        elif kind == 11:
            # two restricted pointers into one array, both only read
            b.append('%s{ const %s *restrict p_ = %s + %d; const %s *restrict q_ = %s + %d; unsigned long long a_ = 0;'
                     % (i2, T, name, dx, T, name, dy))
            if is_float(t):
                b.append('%s  for (ii = lo_; ii < hi_; ii++) a_ = a_ * 3u + (p_[ii] < q_[ii]); ck(a_); }' % i2)
            else:
                b.append('%s  for (ii = lo_; ii < hi_; ii++) a_ = a_ * 3u + (unsigned long long)p_[ii] * (unsigned long long)q_[ii]; ck(a_); }' % i2)
        elif kind == 12:
            # a block in the loop's body: its pointers reach two elements,
            # the next iteration's the last one's
            b.append('%sfor (ii = lo_; ii + 1 < hi_; ii++) {' % i2)
            b.append('%s    %s *restrict a_ = %s + ii + 1; const %s *restrict b_ = %s + ii;' % (i2, T, name, T, name))
            b.append('%s    a_[0] = %s;' % (i2, mix('b_[0]', 'a_[0]')))
            b.append('%s}' % i2)
        elif kind == 13:
            # restrict-qualified members into two arrays
            b.append('%s{ struct { %s *restrict d; const %s *restrict s; } r_ = { %s + %d, %s + %d };'
                     % (i2, T, T, name, dx, src, dy))
            b.append('%s  for (ii = lo_; ii < hi_; ii++) r_.d[ii] = %s; }' % (i2, mix('r_.s[ii]', 'r_.d[ii]')))
        elif kind == 14:
            # an inlined function's restrict parameters, two arrays
            b.append('%sif (hi_ > lo_) rmix_%s(%s + lo_ + %d, %s + lo_ + %d, hi_ - lo_);' % (i2, t, name, dx, src, dy))
        elif kind == 0:
            b.append('%sfor (ii = lo_; ii < hi_; ii++) %s[ii + %d] = %s[ii + %d];' % (i2, name, dx, src, dy))
        elif kind == 1:
            b.append('%sfor (ii = hi_; ii > lo_; ii--) %s[ii - 1 + %d] = %s[ii - 1 + %d];' % (i2, name, dx, src, dy))
        elif kind == 2:
            b.append('%s{ %s *d_ = %s + lo_ + %d; const %s *s_ = %s + lo_ + %d; unsigned k_ = hi_ > lo_ ? hi_ - lo_ : 0;'
                     % (i2, T, name, dx, T, src, dy))
            b.append('%s  while (k_--) *d_++ = *s_++; }' % i2)
        elif kind == 3:
            v, vt = self.expr(t, env, 1)
            b.append('%s{ %s v_ = %s; for (ii = lo_; ii < hi_; ii++) %s[ii] = v_; }' % (i2, T, self.conv_to(v, vt, t), name))
        elif kind == 4:
            c = self.const(t)
            b.append('%sfor (ii = lo_; ii != hi_ && ii < %du; ii++) %s[ii] = %s;' % (i2, n, name, c))
        elif kind in (5, 6):
            # a reduction: integers in unsigned long long, floating in order
            acc = 'double' if (is_float(t) and kind == 6) else 'unsigned long long'
            if is_float(t):
                acc = T if self.chance(0.5) else 'double'
                op = self.pick(['+', '*', '-'])
                b.append('%s{ %s a_ = %s; for (ii = lo_; ii < hi_; ii++) a_ = a_ %s %s[ii]; ckf((double)a_); }'
                         % (i2, acc, self.pick(['0', '1', '0.5']), op, name))
            else:
                op = self.pick(['+', '^', '|', '&', '*', 'dot', 'min', 'max'])
                if op == 'dot':
                    b.append('%s{ unsigned long long a_ = 0; for (ii = lo_; ii < hi_; ii++) a_ += (unsigned long long)%s[ii] * (unsigned long long)%s[ii]; ck(a_); }'
                             % (i2, name, src))
                elif op in ('min', 'max'):
                    cmp = '<' if op == 'min' else '>'
                    b.append('%s{ %s a_ = %s[0]; for (ii = lo_; ii < hi_; ii++) if (%s[ii] %s a_) a_ = %s[ii]; ck((unsigned long long)a_); }'
                             % (i2, T, name, name, cmp, name))
                else:
                    init = '1' if op in ('*', '&') else '0'
                    if op == '&':
                        init = '~0ULL'
                    b.append('%s{ unsigned long long a_ = %s; for (ii = lo_; ii < hi_; ii++) a_ = a_ %s (unsigned long long)%s[ii]; ck(a_); }'
                             % (i2, init, op, name))
        elif kind == 7:
            v, vt = self.expr(t, env, 1)
            b.append('%s{ %s c_ = %s; for (ii = lo_; ii < hi_; ii++) if (%s[ii] == c_) break; ck(ii); }'
                     % (i2, T, self.conv_to(v, vt, t), name))
        elif kind == 8 and is_int(t):
            b.append('%s{ static unsigned hist_[17]; for (ii = lo_; ii < hi_; ii++) hist_[(unsigned long long)%s[ii] %% 17u]++;'
                     % (i2, name))
            b.append('%s  for (ii = 0; ii < 17; ii++) ck(hist_[ii]); }' % i2)
        else:
            # element-wise arithmetic in unsigned types (or floating), into another array
            if is_float(t):
                b.append('%sfor (ii = lo_; ii < hi_; ii++) %s[ii + %d] = %s[ii] * (%s)0.5 + (%s)%s;'
                         % (i2, name, dx, src, T, T, self.const(t)))
            else:
                ut = 'unsigned long long'
                b.append('%sfor (ii = lo_; ii < hi_; ii++) %s[ii + %d] = (%s)((%s)%s[ii] * 3u + (%s)%s[ii + %d]);'
                         % (i2, name, dx, T, ut, src, ut, name, dy))
        b.append('%s}' % ind)
        return b

    def gen_func(self, idx, genv):
        pure = self.chance(0.5)
        ret = self.pick(self.scalars + (['struct ' + s[0] for s in self.structs] if self.structs and self.chance(0.2) else []))
        name = 'f%d' % idx
        params = []
        recursive = self.chance(0.25)
        if recursive:
            params.append(('depth', 'depth'))
        for i in range(self.r.randint(0, 9)):
            if self.structs and self.chance(0.15):
                params.append(('p%d' % i, 'struct ' + self.pick(self.structs)[0]))
            else:
                params.append(('p%d' % i, self.pick(self.scalars)))
        variadic = not pure and self.chance(0.2)
        if variadic:
            params.append(('pv', 'int'))
        vtypes = [self.pick(['int', 'uint', 'llong', 'double'] + (['struct ' + self.structs[0][0]] if self.structs else []))
                  for _ in range(self.r.randint(1, 5))] if variadic else []
        f = {'name': name, 'ret': ret, 'params': params, 'pure': pure, 'variadic': variadic, 'vtypes': vtypes}
        env = {
            'big': [] if pure else list(genv.get('big', [])),
            'vars': [], 'arrays': list(genv['arrays']) if not pure else [], 'fields': [], 'ptrs': [],
            'svars': [], 'struct_globals': genv['struct_globals'], 'pure': [] if pure else list(genv['pure']),
            'impure': [] if pure else list(genv['impure']), 'readonly': set(), 'loopvars': [], 'pure_mode': pure,
            'arrays2': [] if pure else list(genv['arrays2']), 'sarrays': [] if pure else list(genv['sarrays']),
            'depth_left': 2,
        }
        # globals are readable everywhere, writable only from impure functions
        env['vars'] = list(genv['vars'])
        env['fields'] = list(genv['fields'])
        env['ptrs'] = list(genv['ptrs']) if not pure else []
        env['svars'] = list(genv['svars'])
        if pure:
            env['readonly'] |= {v[0] for v in genv['vars']}
        ps = []
        for (pn, pt) in params:
            if pt == 'depth':
                ps.append('int depth')
                env['readonly'].add('depth')
            elif pt.startswith('struct '):
                ps.append('%s %s' % (pt, pn))
                env['svars'].append((pn, pt))
                for fl in self.struct_fields(pn, pt):
                    if not pure:
                        env['fields'].append(fl)
            else:
                ps.append('%s %s' % (spell(pt), pn))
                env['vars'].append((pn, pt))
        if variadic:
            ps.append('...')
        sig = '%s %s(%s)' % (spell(ret), name, ', '.join(ps) or 'void')
        body = []
        locs = []
        for i in range(self.r.randint(1, 6)):
            t = self.pick(self.scalars)
            locs.append(('l%d' % i, t))
            body.append('    %s l%d = %s;' % (spell(t), i, self.const(t)))
        if not pure and self.chance(0.4):
            t = self.pick(self.scalars)
            locs.append(('ls%d' % idx, t))
            body.append('    static %s ls%d = %s;' % (spell(t), idx, self.const(t)))
        for i in range(3):
            body.append('    int i%d = 0;' % i)
            env['loopvars'].append('i%d' % i)
        if self.chance(0.3):
            t = self.pick(self.scalars)
            n = self.pick([3, 4, 8])
            body.append('    %s la[%d] = { %s };' % (spell(t), n, ', '.join(self.const(t) for _ in range(n))))
            env['arrays'] = env['arrays'] + [('la', t, n)]
            if pure:
                env['pure_mode'] = True
        env['vars'] = env['vars'] + locs
        if not pure:
            body.append('    if (fuel-- <= 0) return %s;' % self.ret_default(ret, env))
        if variadic:
            body.append('    va_list ap;')
            body.append('    va_start(ap, pv);')
            for j, vt in enumerate(vtypes):
                if vt.startswith('struct '):
                    body.append('    { %s s = va_arg(ap, %s); %s }' % (vt, vt, self.fold_struct('s', vt, 'l0', locs)))
                else:
                    body.append('    { %s t = va_arg(ap, %s); vacc += %s; }' % (spell(vt), spell(vt), self.conv_to('t', vt, 'ullong')))
            body.append('    va_end(ap);')
            body.insert(0, '    unsigned long long vacc = 0;')
        if pure:
            # a pure function writes only its locals; array writes to its own la are fine
            env['arrays'] = [a for a in env['arrays'] if a[0] == 'la']
        body += self.block(env, self.r.randint(2, 7), 3, '    ')
        if recursive:
            body.append('    if (depth > 0) {')
            args = []
            for (pn, pt) in params:
                if pt == 'depth':
                    args.append('depth - 1')
                elif pt.startswith('struct '):
                    args.append(pn)
                else:
                    args.append(self.expr(pt, env, 2)[0])
            if variadic:
                args += self.var_args(vtypes, env)
            tgt = [l for l in locs if l[1] == ret]
            call = '%s(%s)' % (name, ', '.join(args))
            if tgt:
                body.append('        %s = %s;' % (tgt[0][0], call))
            else:
                body.append('        (void)%s;' % call)
            body.append('    }')
        body.append('    return %s;' % self.ret_value(ret, env, variadic))
        if pure and self.attrs and self.chance(0.7):
            if self.chance(0.5) and not any(pt.startswith('struct ') for (_, pt) in params):
                sig += ' [[reproducible]]'
            else:
                sig = '__attribute__((pure)) ' + sig
        self.out.append('static ' + sig + ' {')
        self.out.extend(body)
        self.out.append('}')
        self.out.append('')
        self.funcs.append(f)
        (genv['pure'] if pure else genv['impure']).append(f)

    def fold_struct(self, base, st, into, locs):
        fl = self.struct_fields(base, st)
        if not fl:
            return ''
        f = fl[0]
        return 'vacc += %s;' % self.conv_to(f[0], f[2], 'ullong')

    def var_args(self, vtypes, env):
        args = []
        for vt in vtypes:
            if vt.startswith('struct '):
                args.append(self.struct_value(vt, env))
            else:
                args.append(self.expr(vt, env, 2)[0])
        return args

    def ret_default(self, ret, env):
        if ret.startswith('struct '):
            return env['struct_globals'][ret]
        return self.const(ret)

    def ret_value(self, ret, env, variadic):
        if ret.startswith('struct '):
            svs = [v for v in env['svars'] if v[1] == ret]
            return self.pick(svs)[0] if svs else env['struct_globals'][ret]
        e, _ = self.expr(ret, env, 3)
        if variadic:
            e = self.conv_to('(%s + %s)' % (self.conv_to(e, ret, 'ullong'), 'vacc'), 'ullong', ret) if is_int(ret) else e
        return e

    # ---- the checksum ----

    def hash_lines(self, genv):
        lines = []
        for g in self.globals:
            name, kind = g[0], g[1]
            if kind == 'array':
                t, n = g[2]
                for j in range(n):
                    lines.append(self.hash_value('%s[%d]' % (name, j), t))
            elif kind == 'struct':
                for f in self.struct_fields(name, g[2]):
                    lines.append(self.hash_value(f[0], f[2]))
            elif kind == 'sarray':
                st, n = g[2]
                for k in range(n):
                    for f in self.struct_fields('%s[%d]' % (name, k), st):
                        lines.append(self.hash_value(f[0], f[2]))
            elif kind == 'array2':
                t, a, b = g[2]
                for i in range(a):
                    for j in range(b):
                        lines.append(self.hash_value('%s[%d][%d]' % (name, i, j), t))
            elif kind == 'union':
                lines.append(self.hash_value('gu.u', 'uint'))
            else:
                lines.append(self.hash_value(name, kind))
        return lines

    def layout_lines(self):
        lines = []
        for (sn, fields) in self.structs:
            lines.append('    ck(sizeof(struct %s));' % sn)
            # (the alignment, as C99 can say it)
            lines.append('    ck(sizeof(struct { char c; struct %s s; }));' % sn)
            for (fn, ft, w, n) in fields:
                if w is None:
                    lines.append('    ck(offsetof(struct %s, %s));' % (sn, fn))
        return lines

    def hash_value(self, e, t):
        if is_float(t):
            return '    ckf(%s);' % e
        return '    ck((unsigned long long)%s);' % e

    def program(self):
        o = self.out
        o.append('/* generated program */')
        if self.hosted:
            o.append('#include <stdio.h>')
            o.append('#include <stdlib.h>')
            o.append('#include <string.h>')
        else:
            o.append('#include "host.h"')
        o.append('#include <stdarg.h>')
        o.append('#include <stddef.h>')
        o.append('static int fuel = %d;' % self.r.randint(200, 3000))
        o.append('static unsigned long long h = 1469598103934665603ULL;')
        o.append('static void ck(unsigned long long v) { h = (h ^ v) * 1099511628211ULL; h ^= h >> 31; }')
        o.append('static void ckf(double d) {')
        o.append('    union { double d; unsigned long long u; } x;')
        o.append('    x.d = d;')
        o.append('    if (d != d) x.u = 0x7ff8000000000000ULL;')
        o.append('    ck(x.u);')
        if self.hosted:
            # (the value as text and back: the formatting and conversion functions)
            o.append('    if (d == d) {')
            o.append('        char b[64], c[64];')
            o.append('        snprintf(b, sizeof b, "%.17g", d);')
            o.append('        snprintf(c, sizeof c, "%a", d);')
            o.append('        ck(strtod(b, NULL) == d && strtod(c, NULL) == d);')
            o.append('        ck(strlen(b) * 131 + (unsigned long long)(strchr(b, \'e\') != NULL));')
            o.append('    }')
        o.append('}')
        if self.hosted:
            o.append('static void out(unsigned long long v) {')
            o.append('    char b[32];')
            o.append('    snprintf(b, sizeof b, "%llx", v);')
            o.append('    printf("%llu %d\\n", strtoull(b, NULL, 16), (int)strlen(b));')
            o.append('}')
        if self.restrict:
            # (for array_loop: functions with restrict parameters to inline)
            for t in self.scalars:
                T = spell(t)
                if is_float(t):
                    e = 's[i] * (%s)0.5 + d[i]' % T
                else:
                    e = '(%s)((unsigned long long)s[i] * 3u + (unsigned long long)d[i])' % T
                o.append('static inline void rmix_%s(%s *restrict d, const %s *restrict s, unsigned n) {' % (t, T, T))
                o.append('    for (unsigned i = 0; i < n; i++) d[i] = %s;' % e)
                o.append('}')
        for i in range(self.r.randint(0, 4)):
            self.gen_struct(i)
        genv = {'vars': [], 'arrays': [], 'fields': [], 'ptrs': [], 'svars': [], 'struct_globals': {}, 'pure': [],
                'impure': [], 'arrays2': [], 'sarrays': [], 'depth_left': 2}
        self.gen_globals(genv)
        o.append('')
        nf = self.r.randint(2, 9)
        for i in range(nf):
            self.gen_func(i, genv)
        o.append('static void layout(void) {')
        o.extend(self.layout_lines())
        o.append('    %s(h);' % ('out' if self.hosted else 'print_u64'))
        o.append('}')
        o.append('static void checksum(void) {')
        o.extend(self.hash_lines(genv))
        o.append('    %s(h);' % ('out' if self.hosted else 'print_u64'))
        o.append('}')
        o.append('')
        o.append('int main(void) {')
        env = dict(genv)
        env['readonly'] = set()
        env['loopvars'] = []
        env['pure_mode'] = False
        o.append('    layout();')
        for f in self.funcs[-4:] + self.r.sample(self.funcs, min(2, len(self.funcs))):
            c = self.call(f, env, 2)
            if f['ret'].startswith('struct '):
                o.append('    { %s r = %s; ' % (f['ret'], c) + ' '.join(
                    self.hash_value(x[0], x[2]).strip() for x in self.struct_fields('r', f['ret'])) + ' }')
            else:
                o.append('    %s' % self.hash_value(c, f['ret']).strip())
            o.append('    checksum();')
        o.append('    return 0;')
        o.append('}')
        return '\n'.join(o) + '\n'


def main():
    seed = int(sys.argv[1])
    no_long = '--no-long' in sys.argv
    g = Gen(seed, no_long, '--loops' in sys.argv)
    g.hosted = '--hosted' in sys.argv
    g.attrs = '--attrs' in sys.argv
    g.restrict = '--restrict' in sys.argv
    sys.stdout.write(g.program())


if __name__ == '__main__':
    main()
