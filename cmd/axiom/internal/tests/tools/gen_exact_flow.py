#!/usr/bin/env python3
"""Writes tests of structured control flow whose expected outputs Python
computes from the specification's rules (spec §7.12, §7.13, §8, §11), not
from the compiler: random functions of nested `if`, `for`, `while` and
`loop` (labelled, with `break` and `continue` of any enclosing loop), `match`
on enums, unions (by value and through a pointer), integers (several
patterns to an arm), optionals, bools and errors, `if let` and `while let`,
`defer` and `errdefer` in every block, early returns, calls of each other
and errors returned with `return error.X`, propagated with `!` and handled
with every form of `catch` (a value, a binding, a block, `return`, `break`
and `continue`), and `and`/`or` whose operands log (short-circuit).

    python3 cmd/axiom/internal/tests/tools/gen_exact_flow.py
    cmd/axiom/build/axiom fmt cmd/axiom/internal/tests/run/exact_flow_*.ax

writes cmd/axiom/internal/tests/run/exact_flow_<n>.ax (the same files every
time: each has its own fixed seed). Every statement that runs, and every
deferred one, appends a number to a global log; each output line hashes one
call's result (a u64, an error union of one, or none) and the log it made,
which Python computes by running the same program in a model of the
statements. The arguments come from memory (global arrays of value sets),
so nothing folds. Integer arithmetic wraps only through the wrapping
operators.

    gen_exact_flow.py --seeds=<first>:<count> <dir>

writes more programs (exact_flow_s<seed>.ax, with other seeds) into <dir>,
for a longer search; copied into tests/run/, the suite runs them.
"""
import os
import random
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
RUN = os.path.join(HERE, '..', 'run')
M64 = (1 << 64) - 1
FNV = 1099511628211
ERRS = ['Bad', 'Worse', 'Ugly']
LOGCAP = 256

PROGRAMS = [('exact_flow_1.ax', 4101), ('exact_flow_2.ax', 4202), ('exact_flow_3.ax', 4303)]


def mix(h, x):
    y = ((h ^ (x & M64)) * FNV) & M64
    return y ^ (y >> 32)


class Brk(Exception):
    def __init__(self, label):
        self.label = label


class Cont(Exception):
    def __init__(self, label):
        self.label = label


class Ret(Exception):
    def __init__(self, v):
        self.v = v


def is_err(v):
    return isinstance(v, tuple) and v[0] == 'E'


class Ctx:
    def __init__(self, prog):
        self.prog, self.vars, self.log, self.steps = prog, {}, [], 0

    def emit(self, x):
        self.log.append(x & M64)
        self.steps += 1
        if self.steps > 200000:
            raise RuntimeError('runs too long')


# ---- expressions ------------------------------------------------------------

class E:
    """An expression: text and a function of the context."""

    def __init__(self, text, ev):
        self.text, self.ev = text, ev


class Fn:
    def __init__(self, name, kind, level):
        self.name, self.kind, self.level = name, kind, level  # kind: 'u64', 'err', 'void'
        self.body = []

    def ret_ax(self):
        return {'u64': 'u64', 'err': 'Errs!u64', 'void': 'void'}[self.kind]

    def call(self, args, prog, ctx):
        saved = ctx.vars
        ctx.vars = Env(a=args[0], b=args[1])
        try:
            try:
                run_block(self.body, ctx, self)
            except Ret as r:
                return r.v
            if self.kind == 'void':
                return None
            if self.kind == 'err':
                return ('O', 0)  # (not reached: every body ends in a return)
            raise AssertionError('no return')
        finally:
            ctx.vars = saved


def run_defers(defers, ctx, err):
    for d in reversed(defers):
        if d.errdefer and not err:
            continue
        run_block(d.body, ctx, None)


def run_block(stmts, ctx, fn):
    defers = []
    try:
        for s in stmts:
            if isinstance(s, Defer):
                defers.append(s)
            else:
                s.run(ctx)
    except Ret as r:
        run_defers(defers, ctx, is_err(r.v))
        raise
    except (Brk, Cont):
        run_defers(defers, ctx, False)
        raise
    run_defers(defers, ctx, False)


class S:
    """A statement: Axiom lines (`lines(indent)`) and `run(ctx)`."""
    jumps = False  # always leaves the block


class Simple(S):
    def __init__(self, text, run, jumps=False):
        self.text, self.fn_run, self.jumps = text, run, jumps

    def lines(self):
        return [self.text] if isinstance(self.text, str) else list(self.text)

    def run(self, ctx):
        self.fn_run(ctx)


class Defer(S):
    def __init__(self, body, errdefer):
        self.body, self.errdefer = body, errdefer

    def lines(self):
        kw = 'errdefer' if self.errdefer else 'defer'
        if len(self.body) == 1 and isinstance(self.body[0], Simple) and isinstance(self.body[0].text, str):
            return [f'{kw} {self.body[0].text}']
        return [f'{kw} {{'] + indent(self.body) + ['}']


def indent(stmts):
    out = []
    for s in stmts:
        out += ['    ' + l for l in s.lines()]
    return out


class Block(S):
    def __init__(self, body):
        self.body = body
        self.jumps = any(s.jumps for s in body)

    def lines(self):
        return ['{'] + indent(self.body) + ['}']

    def run(self, ctx):
        run_block(self.body, ctx, None)


class If(S):
    def __init__(self, cond, then, els):
        self.cond, self.then, self.els = cond, then, els  # els: None, [stmts] or an If
        self.jumps = any(s.jumps for s in then) and els is not None and (
            els.jumps if isinstance(els, If) else any(s.jumps for s in els))

    def lines(self, head='if'):
        out = [f'{head} {self.cond.text} {{'] + indent(self.then)
        if self.els is None:
            return out + ['}']
        if isinstance(self.els, If):
            sub = self.els.lines('} else if')
            return out + sub
        return out + ['} else {'] + indent(self.els) + ['}']

    def run(self, ctx):
        if self.cond.ev(ctx):
            run_block(self.then, ctx, None)
        elif isinstance(self.els, If):
            self.els.run(ctx)
        elif self.els is not None:
            run_block(self.els, ctx, None)


class Loop(S):
    """for (range), while (counter) and loop (counter): `head` is the text
    before the body, `pre` a statement before the loop."""

    def __init__(self, kind, label, var, lo, hi, limit, body):
        self.kind, self.label, self.var, self.lo, self.hi, self.limit, self.body = \
            kind, label, var, lo, hi, limit, body

    def lines(self):
        lab = f'{self.label}: ' if self.label else ''
        if self.kind == 'for':
            return [f'{lab}for {self.var} in {self.lo.text}..{self.hi.text} {{'] + indent(self.body) + ['}']
        if self.kind == 'while':
            return [f'let mut {self.var}: u64 = 0;', f'{lab}while {self.var} < {self.limit} {{',
                    f'    {self.var} += 1;'] + indent(self.body) + ['}']
        return [f'let mut {self.var}: u64 = 0;', f'{lab}loop {{', f'    {self.var} += 1;',
                f'    if {self.var} > {self.limit} {{', '        break;', '    }'] + indent(self.body) + ['}']

    def iterate(self, ctx):
        if self.kind == 'for':
            lo, hi = self.lo.ev(ctx), self.hi.ev(ctx)
            for i in range(lo, hi):
                ctx.vars[self.var] = i
                yield
            return
        ctx.vars[self.var] = 0
        while True:
            if self.kind == 'while' and not ctx.vars[self.var] < self.limit:
                return
            ctx.vars[self.var] += 1
            if self.kind == 'loop' and ctx.vars[self.var] > self.limit:
                return
            yield

    def run(self, ctx):
        for _ in self.iterate(ctx):
            ctx.steps += 1
            try:
                run_block(self.body, ctx, None)
            except Brk as b:
                if b.label is None or b.label == self.label:
                    break
                raise
            except Cont as c:
                if c.label is None or c.label == self.label:
                    continue
                raise


class Match(S):
    """arms: [(patterns or None for else, binding or None, [stmts])];
    `scrut` an expression; `key(v)` gives the pattern key of a value and
    `bind(v)` the binding's value."""

    def __init__(self, scrut, arms, key, bind):
        self.scrut, self.arms, self.key, self.bind = scrut, arms, key, bind

    def lines(self):
        out = [f'match {self.scrut.text} {{']
        for pats, b, body in self.arms:
            if pats is None:
                p = 'else'
            else:
                p = ', '.join(pt + (f'({b})' if b else '') for pt in pats)
            out += [f'    {p} => {{'] + ['    ' + l for l in indent(body)] + ['    }']
        return out + ['}']

    @property
    def jumps(self):
        return all(any(s.jumps for s in body) for _, _, body in self.arms)

    def run(self, ctx):
        v = self.scrut.ev(ctx)
        k = self.key(v)
        for pats, b, body in self.arms:
            if pats is None or k in pats:
                if b:
                    ctx.vars[b] = self.bind(v)
                run_block(body, ctx, None)
                return
        raise AssertionError('no arm')


class IfLet(S):
    def __init__(self, pat, b, scrut, key, bind, then, els, loop=False, label=None, pre=None):
        self.pat, self.b, self.scrut, self.key, self.bind = pat, b, scrut, key, bind
        self.then, self.els, self.loop, self.label, self.pre = then, els, loop, label, pre

    @property
    def jumps(self):
        return not self.loop and self.els is not None and any(s.jumps for s in self.then) and \
            any(s.jumps for s in self.els)

    def lines(self):
        p = self.pat + (f'({self.b})' if self.b else '')
        pre = [self.pre[0]] if self.pre else []
        if self.loop:
            lab = f'{self.label}: ' if self.label else ''
            return pre + [f'{lab}while let {p} = {self.scrut.text} {{'] + indent(self.then) + ['}']
        out = pre + [f'if let {p} = {self.scrut.text} {{'] + indent(self.then)
        if self.els is None:
            return out + ['}']
        return out + ['} else {'] + indent(self.els) + ['}']

    def run(self, ctx):
        if self.pre:
            self.pre[1](ctx)
        if not self.loop:
            v = self.scrut.ev(ctx)
            if self.key(v) == self.pat:
                if self.b:
                    ctx.vars[self.b] = self.bind(v)
                run_block(self.then, ctx, None)
            elif self.els is not None:
                run_block(self.els, ctx, None)
            return
        while True:
            v = self.scrut.ev(ctx)
            if self.key(v) != self.pat:
                return
            ctx.steps += 1
            if self.b:
                ctx.vars[self.b] = self.bind(v)
            try:
                run_block(self.then, ctx, None)
            except Brk as e:
                if e.label is None or e.label == self.label:
                    return
                raise
            except Cont as e:
                if e.label is None or e.label == self.label:
                    continue
                raise


# ---- the generator ------------------------------------------------------------

HELPERS = '''fn mix(h: u64, x: u64) u64 {
    let y = (h ^ x) *% 1099511628211;
    return y ^ (y >> 32);
}

fn log(x: u64) void {
    if LOGN < LOG.len {
        LOG[LOGN] = x;
    }
    LOGN += 1;
}

fn lb(k: u64, c: bool) bool {
    log(k);
    return c;
}

fn trace(h0: u64) u64 {
    let mut h = h0;
    let n = @min(LOGN, LOG.len);
    for i in 0..n {
        h = mix(h, LOG[i]);
    }
    LOGN = 0;
    return mix(h, @as(u64, n));
}

error Errs { Bad, Worse, Ugly }

fn herr(r: Errs!u64) u64 {
    let v = r catch |e| {
        return mix(0, 100 + eidx(e));
    };
    return mix(mix(0, 0), v);
}

fn eidx(e: Errs) u64 {
    match e {
        error.Bad => { return 1; }
        error.Worse => { return 2; }
        error.Ugly => { return 3; }
    }
}

enum Col: u8 {
    Red,
    Green,
    Blue,
    White,
}

struct Pt {
    x: u64,
    y: u64,
}

union Sh {
    A: u64,
    B: Pt,
    C,
}

fn sh_of(a: u64) Sh {
    match a % 3 {
        0 => { return Sh.A(a >> 3); }
        1 => { return Sh.B(.{ x: a ^ 5, y: a >> 7 }); }
        else => { return Sh.C; }
    }
}

fn opt_of(a: u64) ?u64 {
    if (a % 3) == 0 {
        return .none;
    }
    return .some(a >> 2);
}

struct It {
    i: u64,
    n: u64,

    fn next(self: *mut Self) ?u64 {
        if self.i >= self.n {
            return .none;
        }
        self.i += 1;
        return .some(self.i * 3);
    }
}
'''


def sh_of(a):
    if a % 3 == 0:
        return ('A', a >> 3)
    if a % 3 == 1:
        return ('B', (a ^ 5, a >> 7))
    return ('C', None)


def opt_of(a):
    return None if a % 3 == 0 else a >> 2


class Gen:
    def __init__(self, seed):
        self.r = random.Random(seed)
        self.n = 0
        self.ids = 0
        self.fns = []

    def fresh(self, p):
        self.n += 1
        return f'{p}{self.n}'

    def lid(self):
        self.ids += 1
        return self.ids * 1000 + self.r.randrange(1000)

    # -- expressions over the variables in scope --

    def expr(self, sc, d=2):
        r = self.r
        if d == 0 or r.random() < 0.3:
            if r.random() < 0.8 and sc['vars']:
                v = r.choice(sc['vars'])
                return E(v, lambda ctx, v=v: ctx.vars[v])
            c = r.choice([0, 1, 2, 3, 7, 100, r.getrandbits(64), r.getrandbits(16)])
            return E(f'@as(u64, {c})', lambda ctx, c=c: c)
        a = self.expr(sc, d - 1)
        op = r.choice(['+%', '-%', '*%', '^', '&', '|', '>>', '<<', '%', '/'])
        if op in ('>>', '<<'):
            k = r.randrange(64)
            f = (lambda x: x >> k) if op == '>>' else (lambda x: (x << k) & M64)
            return E(f'({a.text} {op} {k})', lambda ctx, a=a, f=f: f(a.ev(ctx)))
        if op in ('%', '/'):
            k = r.choice([2, 3, 5, 7, 10, 255, r.getrandbits(32) | 1])
            f = (lambda x: x % k) if op == '%' else (lambda x: x // k)
            return E(f'({a.text} {op} {k})', lambda ctx, a=a, f=f: f(a.ev(ctx)))
        b = self.expr(sc, d - 1)
        f = {'+%': lambda x, y: (x + y) & M64, '-%': lambda x, y: (x - y) & M64, '*%': lambda x, y: (x * y) & M64,
             '^': lambda x, y: x ^ y, '&': lambda x, y: x & y, '|': lambda x, y: x | y}[op]
        return E(f'({a.text} {op} {b.text})', lambda ctx, a=a, b=b, f=f: f(a.ev(ctx), b.ev(ctx)))

    def cond(self, sc, d=1):
        r = self.r
        x = r.random()
        if d > 0 and x < 0.2:
            a, b = self.cond(sc, d - 1), self.cond(sc, d - 1)
            op = r.choice(['and', 'or'])
            if op == 'and':
                return E(f'({a.text} and {b.text})', lambda ctx: a.ev(ctx) and b.ev(ctx))
            return E(f'({a.text} or {b.text})', lambda ctx: a.ev(ctx) or b.ev(ctx))
        if d > 0 and x < 0.28:
            a = self.cond(sc, d - 1)
            return E(f'(not {a.text})', lambda ctx: not a.ev(ctx))
        if d > 0 and x < 0.4:
            # an operand that logs: the short-circuit is visible
            k = self.lid()
            a = self.cond(sc, 0)

            def ev(ctx, a=a, k=k):
                ctx.emit(k)
                return a.ev(ctx)
            return E(f'lb({k}, {a.text})', ev)
        e = self.expr(sc, 1)
        m = r.choice([2, 3, 4, 5, 7, 16])
        k = r.randrange(m)
        op = r.choice(['==', '!=', '<', '>='])
        f = {'==': lambda x, y: x == y, '!=': lambda x, y: x != y, '<': lambda x, y: x < y,
             '>=': lambda x, y: x >= y}[op]
        return E(f'(({e.text} % {m}) {op} {k})', lambda ctx, e=e, f=f: f(e.ev(ctx) % m, k))

    # -- statements --

    def log_stmt(self, sc):
        k = self.lid()
        e = self.expr(sc, 1)
        return Simple(f'log({k} +% {e.text});', lambda ctx, e=e: ctx.emit(k + e.ev(ctx)))

    def defer_body(self, sc):
        r = self.r
        if r.random() < 0.6:
            return [self.log_stmt(sc)]
        body = [self.log_stmt(sc)]
        if r.random() < 0.5:
            c = self.cond(sc, 0)
            body.append(If(c, [self.log_stmt(sc)], None))
        body.append(self.log_stmt(sc))
        return body

    def jump(self, sc, fn):
        """A statement that leaves: break, continue, return."""
        r = self.r
        opts = []
        if sc['loops']:
            opts += ['break', 'continue']
        opts += ['return']
        k = r.choice(opts)
        if k in ('break', 'continue'):
            lab = None
            labs = [l for l in sc['loops'] if l]
            if labs and r.random() < 0.7:
                lab = r.choice(labs)
            text = f'{k} {lab};' if lab else f'{k};'
            exc = Brk if k == 'break' else Cont

            def run(ctx, lab=lab, exc=exc):
                raise exc(lab)
            return Simple(text, run, True)
        return self.ret_stmt(sc, fn)

    def ret_stmt(self, sc, fn):
        r = self.r
        if fn.kind == 'void':
            def run(ctx):
                raise Ret(None)
            return Simple('return;', run, True)
        if fn.kind == 'err' and r.random() < 0.4:
            k = r.randrange(3)

            def run(ctx, k=k):
                raise Ret(('E', k))
            return Simple(f'return error.{ERRS[k]};', run, True)
        e = self.expr(sc, 2)
        wrap = (lambda v: ('O', v)) if fn.kind == 'err' else (lambda v: v)

        def run(ctx, e=e):
            raise Ret(wrap(e.ev(ctx)))
        return Simple(f'return {e.text};', run, True)

    def call_stmt(self, sc, fn):
        r = self.r
        lower = [g for g in self.fns if g.level < fn.level]
        if not lower or len(sc['loops']) > 1:
            return None
        g = r.choice(lower)
        a, b = self.expr(sc, 1), self.expr(sc, 1)
        call = f'{g.name}({a.text}, {b.text})'

        def args(ctx):
            return [a.ev(ctx), b.ev(ctx)]
        muts = sc['muts']
        if g.kind == 'void':
            return Simple(f'{call};', lambda ctx: g.call(args(ctx), self, ctx))
        if g.kind == 'u64':
            if not muts:
                return Simple(f'log({call});', lambda ctx: ctx.emit(g.call(args(ctx), self, ctx)))
            v = r.choice(muts)

            def run(ctx, v=v):
                ctx.vars[v] = g.call(args(ctx), self, ctx)
            return Simple(f'{v} = {call};', run)
        # an error union
        forms = ['value', 'bind', 'block_ret', 'ret']
        if fn.kind == 'err':
            forms += ['try', 'try', 'ret_err']
        if sc['loops']:
            forms += ['break', 'continue', 'block_break']
        form = r.choice(forms)
        target = r.choice(muts) if muts else None
        k = self.lid()
        if form == 'value':
            c = self.expr(sc, 1)
            text, h = f'{call} catch {c.text}', lambda ctx, e: c.ev(ctx)
        elif form == 'bind':
            ev = self.fresh('e')
            text, h = f'{call} catch |{ev}| eidx({ev}) +% {k}', lambda ctx, e: e + 1 + k
        elif form == 'try':
            text = f'{call}!'

            def h(ctx, e):
                raise Ret(('E', e))
        elif form == 'ret':
            ret = self.ret_stmt(sc, fn)
            body = ret.text[len('return'):].rstrip(';').strip()
            text = f'{call} catch return' + (f' {body}' if body else '')

            def h(ctx, e, ret=ret):
                ret.run(ctx)
        elif form == 'ret_err':
            ev = self.fresh('e')
            text = f'{call} catch |{ev}| return {ev}'

            def h(ctx, e):
                raise Ret(('E', e))
        elif form in ('break', 'continue'):
            text = f'{call} catch {form}'

            def h(ctx, e, form=form):
                raise (Brk if form == 'break' else Cont)(None)
        else:
            ev = self.fresh('e')
            inner = self.log_stmt(sc)
            leave = self.ret_stmt(sc, fn) if form == 'block_ret' else Simple('break;', None, True)
            text = f'{call} catch |{ev}| {{ log(eidx({ev}) +% {k}); ' + inner.text + ' ' + leave.text + ' }'

            def h(ctx, e, inner=inner, leave=leave, form=form):
                ctx.emit(e + 1 + k)
                inner.run(ctx)
                if form == 'block_ret':
                    leave.run(ctx)
                raise Brk(None)

        def value(ctx):
            res = g.call(args(ctx), self, ctx)
            if res[0] == 'O':
                return res[1]
            return h(ctx, res[1])
        if target is None:
            return Simple(f'log({text});', lambda ctx: ctx.emit(value(ctx)))

        def run(ctx, target=target):
            ctx.vars[target] = value(ctx)
        return Simple(f'{target} = {text};', run)

    def block(self, sc, fn, depth, n=None):
        """A list of statements; sc is copied (the names declared here are
        not seen outside)."""
        r = self.r
        sc = {'vars': list(sc['vars']), 'muts': list(sc['muts']), 'loops': list(sc['loops'])}
        out = []
        if n is None:
            n = r.randint(1, 4)
        for _ in range(n):
            s = self.stmt(sc, fn, depth)
            if s is not None:
                out.append(s)
                if s.jumps:
                    return out
        if r.random() < 0.15 and depth > 0:
            out.append(self.jump(sc, fn))
        return out or [self.log_stmt(sc)]

    def stmt(self, sc, fn, depth):
        r = self.r
        kinds = ['log', 'log', 'assign', 'let', 'defer', 'call']
        if depth > 0:
            kinds += ['if', 'if', 'for', 'while', 'loop', 'match_int', 'match_enum', 'match_union', 'match_opt',
                      'match_bool', 'match_ptr', 'iflet', 'whilelet', 'block', 'jumpif', 'jumpif', 'retif']
        if depth > 0 and sc['loops']:
            kinds += ['jumpif', 'jumpif']
        if fn.kind == 'err':
            kinds += ['errdefer']
        k = r.choice(kinds)
        if k == 'log':
            return self.log_stmt(sc)
        if k == 'assign':
            if not sc['muts']:
                return self.log_stmt(sc)
            v = r.choice(sc['muts'])
            e = self.expr(sc, 2)

            def run(ctx, v=v, e=e):
                ctx.vars[v] = e.ev(ctx)
            return Simple(f'{v} = {e.text};', run)
        if k == 'let':
            v = self.fresh('w')
            e = self.expr(sc, 2)
            sc['vars'].append(v)
            sc['muts'].append(v)

            def run(ctx, v=v, e=e):
                ctx.vars[v] = e.ev(ctx)
            return Simple(f'let mut {v}: u64 = {e.text};', run)
        if k in ('defer', 'errdefer'):
            return Defer(self.defer_body(sc), k == 'errdefer')
        if k == 'call':
            return self.call_stmt(sc, fn) or self.log_stmt(sc)
        if k == 'if':
            c = self.cond(sc)
            then = self.block(sc, fn, depth - 1)
            els = None
            x = r.random()
            if x < 0.3:
                els = self.block(sc, fn, depth - 1)
            elif x < 0.45:
                c2 = self.cond(sc)
                els = If(c2, self.block(sc, fn, depth - 1), self.block(sc, fn, depth - 1) if r.random() < 0.5 else None)
            return If(c, then, els)
        if k == 'block':
            return Block(self.block(sc, fn, depth - 1))
        if k == 'jumpif':
            return If(self.cond(sc), [self.log_stmt(sc), self.jump(sc, fn)], None)
        if k == 'retif':
            return If(self.cond(sc), [self.ret_stmt(sc, fn)], None)
        if k in ('for', 'while', 'loop'):
            label = self.fresh('L') if r.random() < 0.5 else None
            var = self.fresh('k' if k == 'for' else 'c')
            inner = {'vars': sc['vars'] + [var], 'muts': list(sc['muts']), 'loops': sc['loops'] + [label]}
            lo = hi = None
            limit = r.randint(0, 5)
            if k == 'for':
                a = self.expr(sc, 1)
                n = r.randint(1, 5)
                lo = E(f'({a.text} % 3)', lambda ctx, a=a: a.ev(ctx) % 3)
                hi = E(f'(({a.text} % 3) + {n})' if r.random() < 0.7 else f'{r.randint(0, 3)}',
                       None)
                if hi.text.startswith('(('):
                    hi.ev = lambda ctx, a=a, n=n: a.ev(ctx) % 3 + n
                else:
                    c = int(hi.text)
                    hi.ev = lambda ctx, c=c: c
            body = self.block(inner, fn, depth - 1)
            return Loop(k, label, var, lo, hi, limit, body)
        if k == 'match_int':
            e = self.expr(sc, 1)
            m = r.choice([3, 4, 5, 8])
            pool = list(range(m))
            r.shuffle(pool)
            arms = []
            while pool and r.random() < 0.8:
                c = r.randint(1, 2)
                pats, pool = pool[:c], pool[c:]
                arms.append(([str(p) for p in pats], None, self.block(sc, fn, depth - 1)))
            arms.append((None, None, self.block(sc, fn, depth - 1)))
            return Match(E(f'{e.text} % {m}', lambda ctx, e=e: e.ev(ctx) % m), arms, lambda v: str(v), None)
        if k == 'match_bool':
            c = self.cond(sc)
            if r.random() < 0.5:
                arms = [(['true'], None, self.block(sc, fn, depth - 1)), (['false'], None, self.block(sc, fn, depth - 1))]
            else:
                arms = [([r.choice(['true', 'false'])], None, self.block(sc, fn, depth - 1)),
                        (None, None, self.block(sc, fn, depth - 1))]
            return Match(c, arms, lambda v: 'true' if v else 'false', None)
        if k == 'match_enum':
            e = self.expr(sc, 1)
            mem = ['.Red', '.Green', '.Blue', '.White']
            pool = list(mem)
            r.shuffle(pool)
            arms = []
            while pool:
                if len(pool) < 4 and r.random() < 0.3:
                    arms.append((None, None, self.block(sc, fn, depth - 1)))
                    break
                c = r.randint(1, 2)
                pats, pool = pool[:c], pool[c:]
                arms.append((pats, None, self.block(sc, fn, depth - 1)))
            return Match(E(f'@enum_from_int(Col, {e.text} % 4)', lambda ctx, e=e: mem[e.ev(ctx) % 4]), arms,
                         lambda v: v, None)
        if k in ('match_union', 'match_ptr'):
            e = self.expr(sc, 1)
            arms = []
            ptr = k == 'match_ptr'
            pre = None
            if ptr:
                u = self.fresh('sv')
                scrut = E(f'&{u}', lambda ctx, u=u: ctx.vars[u])
                pre_e = e
            else:
                scrut = E(f'sh_of({e.text})', lambda ctx, e=e: sh_of(e.ev(ctx)))
            pool = ['.A', '.B', '.C']
            r.shuffle(pool)
            for p in pool:
                if len(arms) > 0 and r.random() < 0.3:
                    arms.append((None, None, self.block(sc, fn, depth - 1)))
                    break
                if p == '.C':
                    arms.append(([p], None, self.block(sc, fn, depth - 1)))
                    continue
                b = self.fresh('x')
                inner = {'vars': list(sc['vars']), 'muts': list(sc['muts']), 'loops': sc['loops']}
                if p == '.A':
                    inner['vars'].append(f'{b}.*' if ptr else b)
                else:
                    inner['vars'].append(f'{b}.x')
                    inner['vars'].append(f'{b}.y')
                arms.append(([p], b, self.block(inner, fn, depth - 1)))
            m = Match(scrut, arms, lambda v: '.' + v[0], lambda v: v[1])
            if ptr:
                return Block([Simple(f'let {u} = sh_of({pre_e.text});',
                                     lambda ctx, u=u, e=pre_e: ctx.vars.__setitem__(u, sh_of(e.ev(ctx)))), m])
            return m
        if k == 'match_opt':
            e = self.expr(sc, 1)
            b = self.fresh('x')
            inner = {'vars': sc['vars'] + [b], 'muts': list(sc['muts']), 'loops': sc['loops']}
            some = (['.some'], b, self.block(inner, fn, depth - 1))
            none = (['.none'], None, self.block(sc, fn, depth - 1))
            arms = [some, none] if r.random() < 0.5 else [none, some]
            if r.random() < 0.3:
                arms[1] = (None, None, arms[1][2]) if arms[1][1] is None else arms[1]
            return Match(E(f'opt_of({e.text})', lambda ctx, e=e: opt_of(e.ev(ctx))), arms,
                         lambda v: '.none' if v is None else '.some', lambda v: v)
        if k == 'iflet':
            e = self.expr(sc, 1)
            b = self.fresh('x')
            if r.random() < 0.5:
                inner = {'vars': sc['vars'] + [b], 'muts': list(sc['muts']), 'loops': sc['loops']}
                return IfLet('.some', b, E(f'opt_of({e.text})', lambda ctx, e=e: opt_of(e.ev(ctx))),
                             lambda v: '.none' if v is None else '.some', lambda v: v, self.block(inner, fn, depth - 1),
                             self.block(sc, fn, depth - 1) if r.random() < 0.5 else None)
            inner = {'vars': sc['vars'] + [b], 'muts': list(sc['muts']), 'loops': sc['loops']}
            return IfLet('.A', b, E(f'sh_of({e.text})', lambda ctx, e=e: sh_of(e.ev(ctx))), lambda v: '.' + v[0],
                         lambda v: v[1], self.block(inner, fn, depth - 1),
                         self.block(sc, fn, depth - 1) if r.random() < 0.5 else None)
        if k == 'whilelet':
            e = self.expr(sc, 1)
            it = self.fresh('it')
            b = self.fresh('x')
            label = self.fresh('L') if r.random() < 0.5 else None
            inner = {'vars': sc['vars'] + [b], 'muts': list(sc['muts']), 'loops': sc['loops'] + [label]}
            body = self.block(inner, fn, depth - 1)

            def pre(ctx, it=it, e=e):
                ctx.vars[it] = [0, e.ev(ctx) % 5]

            def nxt(ctx, it=it):
                s = ctx.vars[it]
                if s[0] >= s[1]:
                    return None
                s[0] += 1
                return s[0] * 3
            return IfLet('.some', b, E(f'{it}.next()', nxt), lambda v: '.none' if v is None else '.some', lambda v: v,
                         body, None, loop=True, label=label,
                         pre=(f'let mut {it} = It.{{ i: 0, n: {e.text} % 5 }};', pre))
        raise AssertionError(k)

    def make_fn(self, name, kind, level):
        f = Fn(name, kind, level)
        sc = {'vars': ['a', 'b'], 'muts': [], 'loops': []}
        body = self.block(sc, f, 3, n=self.r.randint(3, 7))
        if not (body and body[-1].jumps):
            if kind != 'void':
                body.append(self.ret_stmt({'vars': ['a', 'b'], 'muts': [], 'loops': []}, f))
        f.body = body
        return f


class Env(dict):
    """The variables: names, and the forms `x.*`, `x.x`, `x.y` of the
    bindings of union payloads."""

    def __missing__(self, k):
        if k.endswith('.*'):
            return dict.__getitem__(self, k[:-2])
        if k.endswith('.x'):
            return dict.__getitem__(self, k[:-2])[0]
        if k.endswith('.y'):
            return dict.__getitem__(self, k[:-2])[1]
        raise KeyError(k)


def write(fname, seed, out_dir):
    g = Gen(seed)
    r = g.r
    for level in range(3):
        for _ in range(r.randint(3, 5)):
            kind = r.choice(['u64', 'u64', 'err', 'err', 'void'])
            g.fns.append(g.make_fn(f'f{len(g.fns)}', kind, level))
    nsets = 8
    A = [r.choice([0, 1, 2, 3, 5, 7, 9, 12, 100, M64, r.getrandbits(64), r.getrandbits(8)]) for _ in range(nsets)]
    B = [r.choice([0, 1, 2, 3, 5, 7, 9, 12, 100, M64, r.getrandbits(64), r.getrandbits(8)]) for _ in range(nsets)]
    lines = []
    for f in g.fns:
        lines += [f'fn {f.name}(a: u64, b: u64) {f.ret_ax()} {{'] + indent(f.body) + ['}', '']
    run = ['export fn run() void {']
    for f in g.fns:
        exp = 0
        for s in range(nsets):
            ctx = Ctx(g)
            ctx.vars = Env()
            res = f.call([A[s], B[s]], g, ctx)
            if f.kind == 'u64':
                h = mix(0, res)
            elif f.kind == 'err':
                h = mix(0, 100 + res[1] + 1) if res[0] == 'E' else mix(mix(0, 0), res[1])
            else:
                h = 0
            n = min(len(ctx.log), LOGCAP)
            for x in ctx.log[:n]:
                h = mix(h, x)
            h = mix(h, n)
            exp = mix(exp, h)
        run += ['    {', '        let mut hr: u64 = 0;', '        for s in 0..A.len {']
        if f.kind == 'u64':
            run.append(f'            let h = mix(0, {f.name}(A[s], B[s]));')
        elif f.kind == 'err':
            run.append(f'            let h = herr({f.name}(A[s], B[s]));')
        else:
            run += [f'            {f.name}(A[s], B[s]);', '            let h: u64 = 0;']
        run += ['            hr = mix(hr, trace(h));', '        }', f'        io.uint(hr);  // expect: {exp}', '    }']
    run.append('}')
    out = [
        '// Random structured control flow: nested loops with labelled break and',
        '// continue, match, if let, while let, defer and errdefer, early returns,',
        '// errors returned, propagated with ! and handled with catch. Every',
        '// statement that runs appends to a global log; each line hashes the results',
        "// and logs of one function's calls, computed by the rules of the spec (§7.12,",
        '// §7.13, §8, §11). (Generated by cmd/axiom/internal/tests/tools/gen_exact_flow.py,',
        '// with the expectations computed in Python.)',
        'import "../lib/io.ax" as io;',
        '',
        f'let mut LOG: [{LOGCAP}]u64 = [0; {LOGCAP}];',
        'let mut LOGN: usize = 0;',
        f'let mut A: [{nsets}]u64 = [{", ".join(str(x) for x in A)}];',
        f'let mut B: [{nsets}]u64 = [{", ".join(str(x) for x in B)}];',
        '',
    ]
    out += HELPERS.split('\n')
    out += lines + run
    with open(os.path.join(out_dir, fname), 'w') as fh:
        fh.write('\n'.join(out) + '\n')


def main():
    args = sys.argv[1:]
    if len(args) == 2 and args[0].startswith('--seeds='):
        first, count = (int(x) for x in args[0][len('--seeds='):].split(':'))
        for seed in range(first, first + count):
            write(f'exact_flow_s{seed}.ax', seed, args[1])
        return
    for fname, seed in PROGRAMS:
        write(fname, seed, RUN)


if __name__ == '__main__':
    main()
