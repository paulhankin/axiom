#!/usr/bin/env python3
"""Writes tests of the atomic builtins (spec §12.8.3) and of threads that use
them (§12.8.4) whose expected outputs Python computes from the spec's rules,
not from the compiler.

    python3 cmd/axiom/internal/tests/tools/gen_exact_atomics.py
    cmd/axiom/build/axiom fmt cmd/axiom/internal/tests/run/exact_atomics_*.ax

writes into cmd/axiom/internal/tests/run/ (the same files every time; each
program has its own fixed seed):

- exact_atomics_1.ax: one thread, so every result is exact. Every atomic
  builtin (`@atomic_load`, `_store`, `_add`, `_sub`, `_and`, `_or`, `_xor`,
  `_xchg`, `_min`, `_max`, `_cas`; `@atomic_notify` with no waiter, `@fence`)
  on every integer type, at pairs of edge values from memory and with
  constant operands, on globals, fields of a struct whose other fields are of
  every width (so an 8- or 16-bit operation that writes its neighbours shows),
  elements of arrays (an index from memory), the same in locals, fields of
  structs in arrays of structs, through pointer parameters, through generic
  `[T: Int]` functions and through std/sync's `Atomic`. The results are used
  (named, or inline in an expression) or discarded, the memory orders are
  random among those each builtin allows, the initial store is plain or
  atomic and the value is read back plainly or atomically. `add` and `sub`
  wrap, `min` and `max` compare by the type's signedness, `cas` stores iff
  the old value equals the expected one (both succeeding and failing), and
  each returns the previous value. usize and isize keep values of 32 bits
  (and `add` and `sub` on them skip the pairs that would wrap at 32 bits),
  so the results are the same on every target.
- exact_atomics_2.ax: the same, random: other value sets, a random choice of
  types, locations, forms and orders.
- exact_atomics_size64_1.ax (native-only): the same on usize and isize with
  64-bit values, for the 64-bit targets.
- exact_atomics_threads_1.ax, _2.ax: several threads (std/thread) on shared
  memory whose results do not depend on the interleaving: `add` `sub` `and`
  `or` `xor` `min` `max` of values from memory on shared counters of random
  types (the final value is exact); tickets (`add` of one constant: the old
  values are a known set, so their sum is exact); `xchg` chains (the old
  values and the final one are the initial value and every value written);
  compare-exchange retry loops computing a sum, a product, a minimum and a
  bitwise fold on 8- to 64-bit words; std/sync's `Atomic`; plain stores by
  each thread to its own bytes of a shared array and to its own slot of
  sub-word fields (joined, then hashed); thread-local accumulators; a sum,
  count and hash folded under a `Mutex`; turns, where each thread waits
  (`@atomic_wait`) for a word to name it, appends to a plain log and passes
  the turn on with a releasing store and `@atomic_notify`, so that the log
  is in a known order; and `parallel_for`. The threads start together
  (a `Barrier`) and iterate a few hundred times, so that they contend.

Each output line is a hash (the `mix` of gen_exact_ops.py) of one group's
results in a fixed order.

    gen_exact_atomics.py --seeds=<first>:<count> <dir>
    gen_exact_atomics.py --threads --seeds=<first>:<count> <dir>
    gen_exact_atomics.py --size64 --seeds=<first>:<count> <dir>

write more programs (exact_atomics_s<seed>.ax, exact_atomics_threads_s<seed>.ax,
exact_atomics_size64_s<seed>.ax) into <dir>, for a longer search; copied
into tests/run/, the suite runs them.
"""
import os
import random
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from gen_exact_ops import HELPERS, M64, mix  # noqa: E402

HERE = os.path.dirname(os.path.abspath(__file__))
RUN = os.path.join(HERE, '..', 'run')

FIXED = ['u8', 'i8', 'u16', 'i16', 'u32', 'i32', 'u64', 'i64']
ALL = FIXED + ['usize', 'isize']
RMW = ['add', 'sub', 'and', 'or', 'xor', 'xchg', 'min', 'max']
OPS = ['load', 'store'] + RMW + ['cas']
ORDERS = {
    'load': ['relaxed', 'acquire', 'seq_cst'],
    'store': ['relaxed', 'release', 'seq_cst'],
    'rmw': ['relaxed', 'acquire', 'release', 'acq_rel', 'seq_cst'],
}
LOCS = ['glob', 'field', 'elem', 'local', 'lfield', 'lelem', 'nested', 'ptr', 'gen', 'sync']
SYNC_OPS = {'load': 'load', 'store': 'store', 'add': 'add', 'sub': 'sub', 'xchg': 'swap', 'cas': 'compare_exchange'}
ALEN = 8  # elements of the arrays GA_T and la_T


class Types:
    """Integer types: name -> (bits, signed); usize and isize have 32 bits
    (values that give the same results on every target) or 64."""

    def __init__(self, size64):
        self.size64 = size64
        self.t = {t: (int(t[1:]), t[0] == 'i') for t in FIXED}
        w = 64 if size64 else 32
        self.t['usize'] = (w, False)
        self.t['isize'] = (w, True)

    def bits(self, t):
        return self.t[t][0]

    def signed(self, t):
        return self.t[t][1]

    def rng(self, t):
        bits, signed = self.t[t]
        if t == 'usize' and not self.size64:
            return 0, (1 << 31) - 1
        if signed:
            return -(1 << (bits - 1)), (1 << (bits - 1)) - 1
        return 0, (1 << bits) - 1

    def wrap(self, v, t):
        bits, signed = self.t[t]
        v &= (1 << bits) - 1
        if signed and v >> (bits - 1):
            v -= 1 << bits
        return v

    def narrow32(self, t):
        """add and sub on this type must not wrap (the result would depend
        on the target's width)."""
        return t in ('usize', 'isize') and not self.size64

    def edges(self, t, r, n):
        """n values: MIN, MAX, 0, +-1, their neighbours, halves, powers of
        two and their neighbours, random ones."""
        lo, hi = self.rng(t)
        bits = self.bits(t)
        must = [0, 1, hi, hi - 1, hi // 2, hi // 2 + 1, 2]
        if self.signed(t):
            must += [-1, lo, lo + 1, lo // 2, -2]
        more = []
        for k in range(1, bits):
            p = 1 << k
            more += [p - 1, p, p + 1, -p, -p + 1]
        more += [r.randint(lo, hi) for _ in range(8)]
        r.shuffle(more)
        vs = []
        for v in must + more:
            if lo <= v <= hi and v not in vs:
                vs.append(v)
        return vs[:n]

    def hbits(self, v, t):
        return v & M64


def lit(v):
    return str(v)


def fmt_list(vals):
    return ', '.join(lit(v) for v in vals)


# ---- one thread -----------------------------------------------------------

class Stmt:
    """One atomic operation on one location, with the Axiom lines and a
    Python function that runs it on the model of memory."""

    def __init__(self, lines, sim):
        self.lines, self.sim = lines, sim


class Single:
    def __init__(self, seed, full, size64):
        self.r = random.Random(seed)
        self.full = full
        self.T = Types(size64)
        self.size64 = size64
        r = self.r
        if size64:
            self.types = ['usize', 'isize']
        elif full:
            self.types = list(ALL)
        else:
            self.types = r.sample(ALL, r.randint(4, len(ALL)))
        self.mtypes = list(ALL)
        r.shuffle(self.mtypes)  # the fields of Mixed, in this order
        nv = 16 if full else r.randint(6, 18)
        self.vals = {t: self.T.edges(t, r, nv) for t in ALL}
        for t in ALL:
            r.shuffle(self.vals[t])
        self.idx = [r.randrange(ALEN) for _ in range(4)] + [0, ALEN - 1]
        self.nidx = [r.randrange(3) for _ in range(4)]
        self.ptr_helpers = {}  # (op, t) -> (name, order)
        self.gen_helpers = {}  # op -> (name, order)
        self.nreg = 0
        self.fns = []
        self.expects = []
        # the model of memory: location -> value
        self.mem = {}
        self.init_mixed = {t: self.rand(t) for t in ALL}
        for t in ALL:
            self.mem[('G', t)] = self.rand(t)
            self.mem[('GS', t)] = self.init_mixed[t]
            for k in range(ALEN):
                self.mem[('GA', t, k)] = self.rand(t)
            for n in range(3):
                self.mem[('GN', n, t)] = self.init_mixed[t]
        for n in range(3):
            self.mem[('GN', n, 'x')] = self.r.randint(0, 255)
            self.mem[('GN', n, 'y')] = self.r.randint(0, 65535)
        self.glob_init = dict(self.mem)

    def rand(self, t):
        lo, hi = self.T.rng(t)
        return self.r.choice(self.vals[t]) if self.r.random() < 0.5 else self.r.randint(lo, hi)

    def order(self, kind):
        if self.r.random() < 0.3:
            return None
        return self.r.choice(ORDERS[kind])

    def reg(self):
        self.nreg += 1
        return f'r{self.nreg}'

    # -- places: (Axiom place, model key, neighbour hash lines, neighbour sim)

    def place(self, loc, t):
        T = self.T
        if loc == 'glob':
            return f'G_{t.upper()}', ('G', t), [], None
        if loc in ('field', 'lfield'):
            base = 'GS' if loc == 'field' else 'ls'
            key = 'GS' if loc == 'field' else 'LS'
            arg = '&GS' if loc == 'field' else '&ls'

            def nb(mem, h, key=key):
                for f in self.mtypes:
                    h = mix(h, T.hbits(mem[(key, f)], f))
                return h
            return f'{base}.f_{t}', (key, t), [f'hs_mixed({arg})'], nb
        if loc in ('elem', 'lelem'):
            c = self.r.randrange(len(self.idx))
            k = self.idx[c]
            arr = f'GA_{t.upper()}' if loc == 'elem' else f'la_{t}'
            key = 'GA' if loc == 'elem' else 'LA'

            def nb(mem, h, key=key):
                for i in range(ALEN):
                    h = mix(h, T.hbits(mem[(key, t, i)], t))
                return h
            return f'{arr}[IDX[{c}]]', (key, t, k), [f'hs_arr_{t}(&{arr})'], nb
        if loc == 'nested':
            c = self.r.randrange(len(self.nidx))
            n = self.nidx[c]

            def nb(mem, h):
                h = mix(h, mem[('GN', n, 'x')])
                for f in self.mtypes:
                    h = mix(h, T.hbits(mem[('GN', n, f)], f))
                return mix(h, mem[('GN', n, 'y')])
            return f'GN[NIDX[{c}]].m.f_{t}', ('GN', n, t), [f'hs_outer(&GN[NIDX[{c}]])'], nb
        if loc == 'local':
            return f'l_{t}', ('L', t), [], None
        raise ValueError(loc)

    def ptr_helper(self, op, t):
        if (op, t) not in self.ptr_helpers:
            self.ptr_helpers[(op, t)] = (f'ap_{op}_{t}', self.order('load' if op == 'load' else 'store' if op == 'store'
                                                                    else 'rmw'))
        return self.ptr_helpers[(op, t)][0]

    def gen_helper(self, op):
        if op not in self.gen_helpers:
            self.gen_helpers[op] = (f'ag_{op}', self.order('load' if op == 'load' else 'store' if op == 'store'
                                                           else 'rmw'))
        return self.gen_helpers[op][0]

    def stmt(self, t, op, loc, hv, src):
        """An operation `op` on a location `loc` of type t; hv is the hash
        variable; src is 'mem' (b from the inner loop) or a constant."""
        r, T = self.r, self.T
        via = None
        if loc in ('ptr', 'gen'):
            via = loc
            loc = r.choice(['glob', 'field', 'elem', 'local', 'lfield', 'lelem', 'nested'])
        if loc == 'sync' and op not in SYNC_OPS:
            via, loc = 'ptr', 'glob'
        if loc == 'sync':
            return self.sync_stmt(t, op, hv, src)
        P, key, nb_lines, nb = self.place(loc, t)
        bx = 'b' if src == 'mem' else lit(src)
        lines = []
        # the initial store
        init_atomic = r.random() < 0.4
        if init_atomic:
            o = self.order('store')
            lines.append(f'@atomic_store(&mut {P}, a{", " + q(o) if o else ""});')
        else:
            lines.append(f'{P} = a;')
        form = r.choice(['let', 'inline', 'discard']) if op not in ('store',) else 'none'
        kind = 'load' if op == 'load' else 'store' if op == 'store' else 'rmw'
        o = self.order(kind)
        ox = f', {q(o)}' if o else ''
        # the operands of cas: expected (a: succeeds; b or the constant:
        # fails unless equal) and desired
        cas_e = cas_d = None
        if op == 'cas':
            cas_e = r.choice(['a', 'b'])
            cas_d = r.choice(['b', 'x']) if cas_e == 'a' else r.choice(['a', 'x'])
        if via == 'ptr':
            fn = self.ptr_helper(op, t)
        elif via == 'gen':
            fn = self.gen_helper(op)
        if op == 'load':
            call = f'{fn}(&{P})' if via else f'@atomic_load(&{P}{ox})'
        elif op == 'store':
            call = f'{fn}(&mut {P}, {bx})' if via else f'@atomic_store(&mut {P}, {bx}{ox})'
        elif op == 'cas':
            ex = {'a': 'a', 'b': bx, 'x': f'(a ^ {bx})'}
            args = f'{ex[cas_e]}, {ex[cas_d]}'
            call = f'{fn}(&mut {P}, {args})' if via else f'@atomic_cas(&mut {P}, {args}{ox})'
        else:
            call = f'{fn}(&mut {P}, {bx})' if via else f'@atomic_{op}(&mut {P}, {bx}{ox})'
        if op == 'load':
            # a load reads the value of the initial store; store b first
            # sometimes, so that the value loaded is not the one in a
            lines[0] = lines[0].replace(', a', f', {bx}').replace('= a;', f'= {bx};')
        if form == 'let':
            rv = self.reg()
            lines.append(f'let {rv} = {call};')
            lines.append(f'{hv} = h_{t}({hv}, {rv});')
        elif form == 'inline':
            lines.append(f'{hv} = h_{t}({hv}, {call});')
        elif form == 'discard':
            lines.append(f'_ = {call};')
        else:
            lines.append(f'{call};')
        read_atomic = r.random() < 0.4
        if read_atomic:
            o2 = self.order('load')
            lines.append(f'{hv} = h_{t}({hv}, @atomic_load(&{P}{", " + q(o2) if o2 else ""}));')
        else:
            lines.append(f'{hv} = h_{t}({hv}, {P});')
        for nl in nb_lines:
            lines.append(f'{hv} = mix({hv}, {nl});')
        if r.random() < 0.08:
            o3 = self.order('rmw')
            lines.append(f'@fence({q(o3) if o3 else ""});')

        def sim(mem, env, a, b, t=t, op=op, key=key, src=src, form=form, nb=nb, cas_e=cas_e, cas_d=cas_d):
            bv = b if src == 'mem' else src
            mem[key] = bv if op == 'load' else a
            old = mem[key]
            res = old
            if op == 'store':
                mem[key] = bv
            elif op == 'cas':
                vals = {'a': a, 'b': bv, 'x': T.wrap(a ^ bv, t)}
                if old == vals[cas_e]:
                    mem[key] = vals[cas_d]
            elif op != 'load':
                mem[key] = rmw(T, t, op, old, bv)
            h = env[hv]
            if form in ('let', 'inline'):
                h = mix(h, T.hbits(res, t))
            h = mix(h, T.hbits(mem[key], t))
            if nb:
                h = mix(h, nb(mem, 0))
            env[hv] = h
        return Stmt(lines, sim)

    def sync_stmt(self, t, op, hv, src):
        r, T = self.r, self.T
        P = 'sy'
        key = ('SY', t)
        bx = 'b' if src == 'mem' else lit(src)
        lines = [f'{P}.store({bx if op == "load" else "a"});']
        m = SYNC_OPS[op]
        cas_e = cas_d = None
        if op == 'cas':
            cas_e = r.choice(['a', 'b'])
            cas_d = r.choice(['b', 'x']) if cas_e == 'a' else r.choice(['a', 'x'])
            ex = {'a': 'a', 'b': bx, 'x': f'(a ^ {bx})'}
            call = f'{P}.{m}({ex[cas_e]}, {ex[cas_d]})'
        elif op == 'load':
            call = f'{P}.load()'
        else:
            call = f'{P}.{m}({bx})'
        if op == 'store':
            lines.append(f'{call};')
        else:
            lines.append(f'{hv} = h_{t}({hv}, {call});')
        lines.append(f'{hv} = h_{t}({hv}, {P}.load());')

        def sim(mem, env, a, b, t=t, op=op, src=src):
            bv = b if src == 'mem' else src
            mem[key] = bv if op == 'load' else a
            old = mem[key]
            if op == 'store':
                mem[key] = bv
            elif op == 'cas':
                vals = {'a': a, 'b': bv, 'x': T.wrap(a ^ bv, t)}
                if old == vals[cas_e]:
                    mem[key] = vals[cas_d]
            elif op != 'load':
                mem[key] = rmw(T, t, op, old, bv)
            h = env[hv]
            if op != 'store':
                h = mix(h, T.hbits(old, t))
            env[hv] = mix(h, T.hbits(mem[key], t))
        return Stmt(lines, sim)

    def consts(self, t):
        lo, hi = self.T.rng(t)
        vs = [0, 1, 3, hi, hi - 1, 0x55 & hi, hi // 2 + 1]
        if self.T.signed(t):
            vs += [-1, lo, lo + 1, -100]
        return [v for v in vs if lo <= v <= hi]

    def gen_fn(self, t):
        r = self.r
        name = f't_{t}'
        hvs = {op: f'h_{op}' for op in OPS}
        blocks = []  # (guard lines, guard predicate, statements)
        plain = []
        guarded = {'add': [], 'sub': []}
        for op in OPS:
            if self.full:
                rot = ['local', 'lfield', 'lelem', 'nested', 'ptr', 'gen', 'sync']
                locs = ['glob', 'field', 'elem', rot[(OPS.index(op) + ALL.index(t)) % len(rot)]]
                if op in SYNC_OPS and ALL.index(t) % 2 == 0:
                    locs.append('sync')
                if ALL.index(t) % 3 == 0:
                    locs.append(rot[(OPS.index(op) + ALL.index(t) + 3) % len(rot)])
            else:
                locs = [r.choice(LOCS) for _ in range(r.randint(1, 4))]
            for i, loc in enumerate(locs):
                use_const = (i == 1) if self.full else r.random() < 0.3
                narrow = self.T.narrow32(t) and op in ('add', 'sub')
                src = r.choice(self.consts(t)) if use_const and not narrow else 'mem'
                s = self.stmt(t, op, loc, hvs[op], src)
                (guarded[op] if narrow else plain).append(s)
        r.shuffle(plain)
        blocks.append(([], None, plain))
        if self.T.narrow32(t):
            if t == 'usize':
                ga = (['if @as(u64, a) + @as(u64, b) > 2147483647 {', '    continue;', '}'],
                      lambda a, b: a + b > (1 << 31) - 1)
                gs = (['if a < b {', '    continue;', '}'], lambda a, b: a < b)
            else:
                ga = (['let s = @as(i64, a) + @as(i64, b);', 'if s < -2147483648 or s > 2147483647 {', '    continue;',
                       '}'], lambda a, b: not -(1 << 31) <= a + b < (1 << 31))
                gs = (['let s = @as(i64, a) - @as(i64, b);', 'if s < -2147483648 or s > 2147483647 {', '    continue;',
                       '}'], lambda a, b: not -(1 << 31) <= a - b < (1 << 31))
            for op, g in (('add', ga), ('sub', gs)):
                if guarded[op]:
                    blocks.append((g[0], g[1], guarded[op]))
        V = f'V_{t.upper()}'
        lines = [f'fn {name}() void {{']
        for op in OPS:
            lines.append(f'    let mut {hvs[op]}: u64 = 0;')
        lines.append(f'    let mut l_{t}: {t} = 0;')
        lines.append('    let mut ls: Mixed = ' + self.mixed_lit(self.init_mixed) + ';')
        la = [self.rand(t) for _ in range(ALEN)]
        lines.append(f'    let mut la_{t}: [{ALEN}]{t} = [{fmt_list(la)}];')
        sy0 = self.rand(t)
        lines.append(f'    let mut sy = sync.Atomic.[{t}].init({lit(sy0)});')
        lines.append(f'    _ = &mut l_{t};')
        lines.append('    _ = &mut ls;')
        lines.append(f'    _ = &mut la_{t};')
        lines.append('    _ = &mut sy;')
        for g_lines, _, stmts in blocks:
            lines += [f'    for i in 0..{V}.len {{', f'        for j in 0..{V}.len {{', f'            let a = {V}[i];',
                      f'            let b = {V}[j];']
            lines += ['            ' + g for g in g_lines]
            for s in stmts:
                lines += ['            ' + x for x in s.lines]
            lines += ['        }', '    }']
        if t in ('u32', 'i32'):
            lines.append(f'    h_load = h_u32(h_load, @atomic_notify(&G_{t.upper()}, 1));')
            lines.append(f'    h_load = h_u32(h_load, @atomic_notify(&l_{t}, 0xFFFF_FFFF));')
        used = [op for op in OPS if any(hvs[op] in x for _, _, ss in blocks for s in ss for x in s.lines)]
        for op in used:
            lines.append(f'    io.uint({hvs[op]});')
        lines.append('}')
        # the model
        mem = self.mem
        mem[('L', t)] = 0
        for f in ALL:
            mem[('LS', f)] = self.init_mixed[f]
        for k in range(ALEN):
            mem[('LA', t, k)] = la[k]
        mem[('SY', t)] = sy0
        env = {hvs[op]: 0 for op in OPS}
        vals = self.vals[t]
        for _, pred, stmts in blocks:
            for a in vals:
                for b in vals:
                    if pred and pred(a, b):
                        continue
                    for s in stmts:
                        s.sim(mem, env, a, b)
        if t in ('u32', 'i32'):
            env['h_load'] = mix(mix(env['h_load'], 0), 0)
        self.fns.append(lines)
        self.expects.append((name, [env[hvs[op]] for op in used]))

    def mixed_lit(self, vals):
        return 'Mixed.{ ' + ', '.join(f'f_{f}: {lit(vals[f])}' for f in self.mtypes) + ' }'

    def write(self, fname, out_dir):
        for t in self.types:
            self.gen_fn(t)
        mem = self.glob_init
        out = []
        if self.size64:
            out += ['// The atomic builtins on usize and isize with 64-bit values (native-only:',
                    '// the 64-bit targets), as exact_atomics_1.ax does for every type.']
        elif self.full:
            out += ['// Every atomic builtin (spec §12.8.3) on every integer type, at pairs of',
                    '// edge values from memory and with constant operands, on globals, struct',
                    '// fields (next to fields of every width), array elements, locals, fields',
                    '// of structs in arrays, through pointer and generic parameters and through',
                    "// std/sync's Atomic; results used or discarded, in random memory orders.",
                    '// The old value is returned; add and sub wrap; min and max compare by the',
                    '// signedness of the type; cas stores iff the old value is the expected one.',
                    '// usize and isize keep 32-bit values (add and sub skip the pairs that wrap',
                    '// at 32 bits), so that every target gives these results.']
        else:
            out += ['// Random atomic operations of random types on random locations, as in',
                    '// exact_atomics_1.ax.']
        out += ['// (Generated by cmd/axiom/internal/tests/tools/gen_exact_atomics.py, with the',
                '// expectations computed in Python from the rules of the spec.)']
        if self.size64:
            out.append('// native-only')
        out += ['import "../lib/io.ax" as io;', 'import "std/sync" as sync;', '']
        out.append('struct Mixed {')
        out += [f'    f_{f}: {f},' for f in self.mtypes]
        out += ['}', '', 'struct Outer {', '    x: u8,', '    m: Mixed,', '    y: u16,', '}', '']
        for t in ALL:
            out.append(f'let mut G_{t.upper()}: {t} = {lit(mem[("G", t)])};')
        for t in ALL:
            out.append(f'let mut V_{t.upper()}: [{len(self.vals[t])}]{t} = [{fmt_list(self.vals[t])}];')
        for t in ALL:
            out.append(f'let mut GA_{t.upper()}: [{ALEN}]{t} = [{fmt_list(mem[("GA", t, k)] for k in range(ALEN))}];')
        out.append(f'let mut IDX: [{len(self.idx)}]usize = [{fmt_list(self.idx)}];')
        out.append(f'let mut NIDX: [{len(self.nidx)}]usize = [{fmt_list(self.nidx)}];')
        out.append('let mut GS: Mixed = ' + self.mixed_lit(self.init_mixed) + ';')
        outers = [f'Outer.{{ x: {mem[("GN", n, "x")]}, m: {self.mixed_lit(self.init_mixed)}, y: {mem[("GN", n, "y")]} }}'
                  for n in range(3)]
        out.append(f'let mut GN: [3]Outer = [{", ".join(outers)}];')
        out.append('')
        out += HELPERS.split('\n')
        out += ['fn hs_mixed(s: *Mixed) u64 {', '    let mut h: u64 = 0;']
        out += [f'    h = h_{f}(h, s.f_{f});' for f in self.mtypes]
        out += ['    return h;', '}', '']
        out += ['fn hs_outer(o: *Outer) u64 {', '    let mut h = h_u8(0, o.x);']
        out += [f'    h = h_{f}(h, o.m.f_{f});' for f in self.mtypes]
        out += ['    return h_u16(h, o.y);', '}', '']
        for t in ALL:
            out += [f'fn hs_arr_{t}(p: *[{ALEN}]{t}) u64 {{', '    let mut h: u64 = 0;', '    for x in p.* {',
                    f'        h = h_{t}(h, x);', '    }', '    return h;', '}', '']
        for (op, t), (name, o) in sorted(self.ptr_helpers.items()):
            ox = f', {q(o)}' if o else ''
            if op == 'load':
                out += [f'fn {name}(p: *{t}) {t} {{', f'    return @atomic_load(p{ox});', '}', '']
            elif op == 'store':
                out += [f'fn {name}(p: *mut {t}, v: {t}) void {{', f'    @atomic_store(p, v{ox});', '}', '']
            elif op == 'cas':
                out += [f'fn {name}(p: *mut {t}, e: {t}, d: {t}) {t} {{', f'    return @atomic_cas(p, e, d{ox});', '}', '']
            else:
                out += [f'fn {name}(p: *mut {t}, v: {t}) {t} {{', f'    return @atomic_{op}(p, v{ox});', '}', '']
        for op, (name, o) in sorted(self.gen_helpers.items()):
            ox = f', {q(o)}' if o else ''
            if op == 'load':
                out += [f'fn {name}[T: Int](p: *T) T {{', f'    return @atomic_load(p{ox});', '}', '']
            elif op == 'store':
                out += [f'fn {name}[T: Int](p: *mut T, v: T) void {{', f'    @atomic_store(p, v{ox});', '}', '']
            elif op == 'cas':
                out += [f'fn {name}[T: Int](p: *mut T, e: T, d: T) T {{', f'    return @atomic_cas(p, e, d{ox});', '}', '']
            else:
                out += [f'fn {name}[T: Int](p: *mut T, v: T) T {{', f'    return @atomic_{op}(p, v{ox});', '}', '']
        for f in self.fns:
            out += f
            out.append('')
        # the globals at the end
        gl = ['    let mut hg: u64 = 0;']
        hg = 0
        fm = self.mem
        for t in ALL:
            gl.append(f'    hg = h_{t}(hg, G_{t.upper()});')
            hg = mix(hg, fm[('G', t)] & M64)
            gl.append(f'    hg = mix(hg, hs_arr_{t}(&GA_{t.upper()}));')
            ha = 0
            for k in range(ALEN):
                ha = mix(ha, fm[('GA', t, k)] & M64)
            hg = mix(hg, ha)
        gl.append('    hg = mix(hg, hs_mixed(&GS));')
        hm = 0
        for f in self.mtypes:
            hm = mix(hm, fm[('GS', f)] & M64)
        hg = mix(hg, hm)
        for n in range(3):
            gl.append(f'    hg = mix(hg, hs_outer(&GN[{n}]));')
            ho = mix(0, fm[('GN', n, 'x')])
            for f in self.mtypes:
                ho = mix(ho, fm[('GN', n, f)] & M64)
            hg = mix(hg, mix(ho, fm[('GN', n, 'y')]))
        out.append('fn globals() void {')
        out += gl
        out.append('    io.uint(hg);')
        out += ['}', '']
        out.append('export fn run() void {')
        for name, hs in self.expects:
            if not hs:
                out.append(f'    {name}();')
                continue
            out.append(f'    {name}();  // expect: {hs[0]}')
            out += [f'    // expect: {h}' for h in hs[1:]]
        out.append(f'    globals();  // expect: {hg}')
        out.append('}')
        with open(os.path.join(out_dir, fname), 'w') as fh:
            fh.write('\n'.join(out) + '\n')


def q(o):
    return f'"{o}"'


def rmw(T, t, op, old, b):
    if op == 'add':
        return T.wrap(old + b, t)
    if op == 'sub':
        return T.wrap(old - b, t)
    if op == 'and':
        return T.wrap(old & b, t)
    if op == 'or':
        return T.wrap(old | b, t)
    if op == 'xor':
        return T.wrap(old ^ b, t)
    if op == 'xchg':
        return b
    if op == 'min':
        return min(old, b)
    if op == 'max':
        return max(old, b)
    raise ValueError(op)


# ---- threads --------------------------------------------------------------

FOLD_OPS = ['add', 'sub', 'and', 'or', 'xor', 'min', 'max']
CAS_OPS = ['add', 'mul', 'and', 'or', 'xor', 'min', 'max']
WIDTH_FREE = ['and', 'or', 'xor', 'min', 'max']  # the ops usize/isize may use


def cas_new(op, old, v):
    """The Axiom expression of a compare-exchange loop's new value."""
    return {'add': f'{old} +% {v}', 'mul': f'{old} *% {v}', 'and': f'{old} & {v}', 'or': f'{old} | {v}',
            'xor': f'{old} ^ {v}', 'min': f'@min({old}, {v})', 'max': f'@max({old}, {v})'}[op]


def fold(T, t, op, x, v):
    if op == 'mul':
        return T.wrap(x * v, t)
    return rmw(T, t, op, x, v)


def wide(T, v):
    return v & M64


class Threads:
    """Threads on shared memory whose results do not depend on how they
    interleave."""

    def __init__(self, seed):
        self.r = r = random.Random(seed)
        self.T = Types(False)
        self.nt = r.randint(2, 4)
        self.iters = r.randint(100, 400)
        self.m = r.randint(3, 9)  # bytes per thread in the shared byte arrays
        self.arrays = []  # (name, type, values)
        # counters folded by one atomic operation: (name, type, op, init, values array, used result)
        self.folds = []
        for i in range(r.randint(6, 12)):
            t = r.choice(ALL)
            op = r.choice(WIDTH_FREE if t in ('usize', 'isize') else FOLD_OPS)
            self.folds.append((f'c{i}', t, op, self.val(t), self.arr(t), r.random() < 0.5, self.order('rmw')))
        # tickets: add of one constant; the old values are known
        self.tickets = []
        for i in range(r.randint(1, 3)):
            t = r.choice(FIXED)
            lo, hi = self.T.rng(t)
            d = r.choice([1, 1, 3, r.randint(lo, hi)])
            self.tickets.append((f'k{i}', t, self.val(t), d, r.random() < 0.5, self.order('rmw')))
        # exchanges
        self.xchgs = []
        for i in range(r.randint(1, 3)):
            t = r.choice(ALL)
            self.xchgs.append((f'x{i}', t, self.val(t), self.arr(t), self.order('rmw')))
        # compare-exchange loops: (name, type, op, init, values, form)
        self.cases = []
        for i in range(r.randint(2, 5)):
            t = r.choice(ALL)
            op = r.choice(WIDTH_FREE if t in ('usize', 'isize') else CAS_OPS)
            vals = self.arr(t, odd=op == 'mul')
            self.cases.append((f'q{i}', t, op, self.val(t, odd=op == 'mul'), vals, r.choice(['load', 'seen']),
                               self.order('rmw')))
        # std/sync's Atomic: add
        self.syncs = []
        for i in range(r.randint(1, 2)):
            t = r.choice(FIXED)
            self.syncs.append((f's{i}', t, self.val(t), self.arr(t)))
        self.mvals = self.arr('u32')
        self.tl0 = r.getrandbits(64)
        self.pf_n = r.randint(10, 200)
        # turns: the threads take turns (waiting on a word), each appending
        # to a plain log and publishing it by passing the turn on
        self.rounds = r.randint(3, 20)
        # (orders that publish: the log is plain memory)
        self.turn_orders = (r.choice([None, 'acquire', 'seq_cst']), r.choice([None, 'release', 'seq_cst']))

    def val(self, t, odd=False):
        lo, hi = self.T.rng(t)
        v = self.r.choice(self.T.edges(t, self.r, 20)) if self.r.random() < 0.6 else self.r.randint(lo, hi)
        if odd:
            v |= 1
            if v > hi:
                v -= 2
        return v

    def order(self, kind):
        return None if self.r.random() < 0.5 else self.r.choice(ORDERS[kind])

    def arr(self, t, odd=False):
        n = self.r.randint(3, 13)
        vals = [self.val(t, odd) for _ in range(n)]
        name = f'V{len(self.arrays)}'
        self.arrays.append((name, t, vals))
        return name

    def values(self, name):
        for n, t, vs in self.arrays:
            if n == name:
                return vs
        raise KeyError(name)

    def stream(self, name):
        """Every value the threads take from array `name`."""
        vs = self.values(name)
        out = []
        for it in range(self.iters):
            for i in range(self.nt):
                out.append(vs[(it * self.nt + i) % len(vs)])
        return out

    def write(self, fname, out_dir):
        T, nt, iters, m = self.T, self.nt, self.iters, self.m
        out = ['// Threads on shared memory (spec §12.8) whose results do not depend on how',
               '// they interleave: atomic folds on shared counters, tickets, exchanges,',
               '// compare-exchange loops, std/sync\'s Atomic and a Mutex, plain stores to',
               '// each thread\'s own bytes and fields, thread-local accumulators and',
               '// parallel_for. (Generated by cmd/axiom/internal/tests/tools/gen_exact_atomics.py,',
               '// with the expectations computed in Python from the rules of the spec.)',
               'import "../lib/io.ax" as io;', 'import "std/sync" as sync;', 'import "std/thread" as thread;', '']
        for name, t, vals in self.arrays:
            out.append(f'let mut {name}: [{len(vals)}]{t} = [{fmt_list(vals)}];')
        out.append(f'let mut MV: [{len(self.values(self.mvals))}]u32 = [{fmt_list(self.values(self.mvals))}];')
        for name, t, init, d, mem_d, o in self.tickets:
            if mem_d:
                out.append(f'let mut D_{name.upper()}: {t} = {lit(d)};')
        out.append(f'threadlocal let mut TL: u64 = {self.tl0};')
        out.append('')
        out += HELPERS.split('\n')
        out += ['struct Slot {', '    b8: u8,', '    tsum: u64,', '    b16: i16,', '    xsum: u64,', '    b8b: u8,',
                '    xxor: u64,', '    err: u32,', '    tl: u64,', '}', '']
        out.append('struct Shared {')
        out += ['    bar: sync.Barrier,', '    lock: sync.Mutex,']
        fields = []
        for c in self.folds:
            fields.append((c[0], c[1]))
        for c in self.tickets:
            fields.append((c[0], c[1]))
        for c in self.xchgs:
            fields.append((c[0], c[1]))
        for c in self.cases:
            fields.append((c[0], c[1]))
        self.r.shuffle(fields)
        out += [f'    {n}: {t},' for n, t in fields]
        out += [f'    {c[0]}: sync.Atomic.[{c[1]}],' for c in self.syncs]
        out += ['    turn: u32,', f'    tlog: [{self.rounds * nt}]u64,', '    tn: usize,']
        out += ['    m_sum: u64,', '    m_b: u8,', '    m_cnt: u32,', '    m_h: u64,', f'    slots: [{nt}]Slot,',
                f'    bytes: [{m * nt}]u8,', f'    halves: [{m * nt}]u16,', '}', '']
        out += ['struct Arg {', '    sh: *mut Shared,', '    id: usize,', '}', '']
        # the worker
        w = ['fn worker(arg: Arg) void {', '    let sh = arg.sh;', '    let id = arg.id;', '    let slot = &mut sh.slots[id];',
             '    _ = sh.bar.wait();', f'    for it in @as(usize, 0)..{iters} {{', f'        let k = it * {nt} + id;']
        body = []
        for name, t, op, init, arr, used, o in self.folds:
            ox = f', {q(o)}' if o else ''
            call = f'@atomic_{op}(&mut sh.{name}, {arr}[k % {arr}.len]{ox})'
            if used and op in ('and', 'or', 'min', 'max'):
                # the old value is not known, but it is a subset (or) or a
                # superset (and) of the initial bits, or on one side of it
                cond = {'or': f'(o & {lit(init)}) != {lit(init)}', 'and': f'(o | {lit(init)}) != {lit(init)}',
                        'min': f'o > {lit(init)}', 'max': f'o < {lit(init)}'}[op]
                body.append([f'{{', f'    let o = {call};', f'    if {cond} {{', '        slot.err += 1;', '    }', '}'])
            else:
                body.append([f'_ = {call};'])
        for name, t, init, d, mem_d, o in self.tickets:
            ox = f', {q(o)}' if o else ''
            dx = f'D_{name.upper()}' if mem_d else lit(d)
            body.append([f'slot.tsum +%= {self.u64_of(t, f"@atomic_add(&mut sh.{name}, {dx}{ox})")};'])
        for name, t, init, arr, o in self.xchgs:
            ox = f', {q(o)}' if o else ''
            body.append(['{', f'    let o = {self.u64_of(t, f"@atomic_xchg(&mut sh.{name}, {arr}[k % {arr}.len]{ox})")};',
                         '    slot.xsum +%= o;', '    slot.xxor ^= o;', '}'])
        for name, t, op, init, arr, form, o in self.cases:
            ox = f', {q(o)}' if o else ''
            v = f'{arr}[k % {arr}.len]'
            if form == 'load':
                body.append(['loop {', f'    let old = @atomic_load(&sh.{name});',
                             f'    if @atomic_cas(&mut sh.{name}, old, {cas_new(op, "old", v)}{ox}) == old {{',
                             '        break;', '    }', '}'])
            else:
                body.append(['{', f'    let v = {v};', f'    let mut old = sh.{name};', '    loop {',
                             f'        let seen = @atomic_cas(&mut sh.{name}, old, {cas_new(op, "old", "v")}{ox});',
                             '        if seen == old {', '            break;', '        }', '        old = seen;', '    }',
                             '}'])
        for name, t, init, arr in self.syncs:
            body.append([f'_ = sh.{name}.add({arr}[k % {arr}.len]);'])
        body.append(['{', '    let v = MV[k % MV.len];', '    sh.lock.lock();', '    sh.m_sum +%= @as(u64, v);',
                     '    sh.m_b +%= @truncate(u8, v);', '    sh.m_cnt += 1;', '    sh.m_h +%= mix(0, @as(u64, v));',
                     '    sh.lock.unlock();', '}'])
        body.append([f'sh.bytes[(it % {m}) * {nt} + id] = @truncate(u8, it *% 7 +% id *% 13 +% 1);',
                     f'sh.halves[(it % {m}) * {nt} + id] = @truncate(u16, it *% 331 +% id);',
                     'slot.b8 = @truncate(u8, k);', 'slot.b16 = @bit_cast(i16, @truncate(u16, k *% 3));',
                     'slot.b8b = @truncate(u8, it +% 5);', 'TL = mix(TL, @as(u64, k));'])
        self.r.shuffle(body)
        for b in body:
            w += ['        ' + x for x in b]
        w += ['    }', '    slot.tl = TL;']
        lo, so = self.turn_orders
        lx = f', {q(lo)}' if lo else ''
        sx = f', {q(so)}' if so else ''
        w += [f'    for round in @as(u64, 0)..{self.rounds} {{', '        loop {',
              f'            let t = @atomic_load(&sh.turn{lx});', '            if t == @truncate(u32, id) {',
              '                break;', '            }', '            _ = @atomic_wait(&sh.turn, t, -1);', '        }',
              '        sh.tlog[sh.tn] = mix(round, @as(u64, id));', '        sh.tn += 1;',
              f'        @atomic_store(&mut sh.turn, @truncate(u32, (id + 1) % {nt}){sx});',
              '        _ = @atomic_notify(&sh.turn, 0xFFFF_FFFF);', '    }', '}', '']
        out += w
        # parallel_for
        out += ['struct Pf {', '    out: []mut u32,', '    sum: u64,', '}', '',
                'fn pf_body(ctx: *mut Pf, lo: usize, hi: usize) void {', '    for i in lo..hi {',
                '        let x = @truncate(u32, i *% 2654435761);', '        ctx.out[i] = x;',
                '        _ = @atomic_add(&mut ctx.sum, @as(u64, x));', '    }', '}', '']
        # demo
        d = ['fn demo() !void {']
        inits = ['bar: sync.Barrier.init(' + str(nt) + ')', 'lock: sync.Mutex.init()']
        for n, t in fields:
            init = self.init_of(n)
            inits.append(f'{n}: {lit(init)}')
        for name, t, init, arr in self.syncs:
            inits.append(f'{name}: sync.Atomic.[{t}].init({lit(init)})')
        slot0 = 'Slot.{ b8: 0, tsum: 0, b16: 0, xsum: 0, b8b: 0, xxor: 0, err: 0, tl: 0 }'
        inits += ['turn: 0', f'tlog: [0; {self.rounds * nt}]', 'tn: 0']
        inits += ['m_sum: 0', 'm_b: 0', 'm_cnt: 0', 'm_h: 0', f'slots: [{slot0}; {nt}]', f'bytes: [0; {m * nt}]',
                  f'halves: [0; {m * nt}]']
        d.append('    let mut sh = Shared.{ ' + ', '.join(inits) + ' };')
        spawns = ', '.join(f'thread.spawn(worker, Arg.{{ sh: &mut sh, id: {i} }})!' for i in range(nt))
        d.append(f'    let mut ts: [{nt}]thread.Thread.[Arg] = [{spawns}];')
        d += ['    for &mut t in ts {', '        t.join();', '    }']
        exp = []
        # folds
        d.append('    let mut h: u64 = 0;')
        h = 0
        for name, t, op, init, arr, used, o in self.folds:
            x = init
            for v in self.stream(arr):
                x = fold(T, t, op, x, v)
            d.append(f'    h = h_{t}(h, sh.{name});')
            h = mix(h, wide(T, x))
        d.append(f'    io.uint(h);  // expect: {h}')
        # tickets: the sum of the old values, and the final value
        d.append('    h = 0;')
        h = 0
        d.append('    let mut tsum: u64 = 0;')
        d += [f'    for &s in sh.slots {{', '        tsum +%= s.tsum;', '    }']
        tsum = 0
        for name, t, init, dd, mem_d, o in self.tickets:
            x = init
            for _ in range(nt * iters):
                tsum = (tsum + wide(T, x)) & M64
                x = T.wrap(x + dd, t)
            d.append(f'    h = h_{t}(h, sh.{name});')
            h = mix(h, wide(T, x))
        d.append('    h = mix(h, tsum);')
        h = mix(h, tsum)
        d.append(f'    io.uint(h);  // expect: {h}')
        # exchanges: the old values with the final one are the initial value
        # and every value written
        d += ['    h = 0;', '    let mut xsum: u64 = 0;', '    let mut xxor: u64 = 0;', '    for &s in sh.slots {',
              '        xsum +%= s.xsum;', '        xxor ^= s.xxor;', '    }']
        xsum = xxor = 0
        for name, t, init, arr, o in self.xchgs:
            d.append(f'    xsum +%= {self.u64_of(t, f"sh.{name}")};')
            d.append(f'    xxor ^= {self.u64_of(t, f"sh.{name}")};')
            for v in [init] + self.stream(arr):
                xsum = (xsum + wide(T, v)) & M64
                xxor ^= wide(T, v)
        d += ['    h = mix(mix(h, xsum), xxor);']
        h = mix(mix(0, xsum), xxor)
        d.append(f'    io.uint(h);  // expect: {h}')
        # compare-exchange loops
        d.append('    h = 0;')
        h = 0
        for name, t, op, init, arr, form, o in self.cases:
            x = init
            for v in self.stream(arr):
                x = fold(T, t, op, x, v)
            d.append(f'    h = h_{t}(h, sh.{name});')
            h = mix(h, wide(T, x))
        d.append(f'    io.uint(h);  // expect: {h}')
        # std/sync's Atomic and the mutex
        d.append('    h = 0;')
        h = 0
        for name, t, init, arr in self.syncs:
            x = init
            for v in self.stream(arr):
                x = T.wrap(x + v, t)
            d.append(f'    h = h_{t}(h, sh.{name}.load());')
            h = mix(h, wide(T, x))
        mv = self.stream(self.mvals)
        msum = sum(mv) & M64
        mb = sum(mv) & 255
        mh = 0
        for v in mv:
            mh = (mh + mix(0, v)) & M64
        d += ['    h = h_u64(h, sh.m_sum);', '    h = h_u8(h, sh.m_b);', '    h = h_u32(h, sh.m_cnt);',
              '    h = h_u64(h, sh.m_h);']
        for x in (msum, mb, nt * iters, mh):
            h = mix(h, x)
        d.append(f'    io.uint(h);  // expect: {h}')
        # each thread's own memory
        d.append('    h = 0;')
        h = 0
        d += ['    for &s in sh.slots {', '        h = h_u8(h, s.b8);', '        h = h_i16(h, s.b16);',
              '        h = h_u8(h, s.b8b);', '        h = h_u32(h, s.err);', '        h = h_u64(h, s.tl);', '    }']
        for i in range(nt):
            k = (iters - 1) * nt + i
            tl = self.tl0
            for it in range(iters):
                tl = mix(tl, it * nt + i)
            b16 = T.wrap((k * 3) & 0xffff, 'i16')
            for x in (k & 255, b16 & M64, (iters - 1 + 5) & 255, 0, tl):
                h = mix(h, x)
        d += ['    for b in sh.bytes {', '        h = h_u8(h, b);', '    }', '    for x in sh.halves {', '        h = h_u16(h, x);',
              '    }', '    h = h_u64(h, TL);']
        last = {}
        for it in range(iters):
            last[it % m] = it
        for j in range(m):
            for i in range(nt):
                it = last.get(j)
                h = mix(h, 0 if it is None else (it * 7 + i * 13 + 1) & 255)
        for j in range(m):
            for i in range(nt):
                it = last.get(j)
                h = mix(h, 0 if it is None else (it * 331 + i) & 0xffff)
        h = mix(h, self.tl0)
        d.append(f'    io.uint(h);  // expect: {h}')
        # the turns
        d += ['    h = h_usize(0, sh.tn);', '    for x in sh.tlog {', '        h = h_u64(h, x);', '    }']
        h = mix(0, self.rounds * nt)
        for rd in range(self.rounds):
            for i in range(nt):
                h = mix(h, mix(rd, i))
        d.append(f'    io.uint(h);  // expect: {h}')
        # parallel_for
        n = self.pf_n
        d += [f'    let mut pf_out: [{n}]u32 = [0; {n}];', '    let mut pf = Pf.{ out: pf_out[..], sum: 0 };',
              f'    thread.parallel_for({n}, {nt}, pf_body, &mut pf)!;', '    h = h_u64(0, pf.sum);', '    for x in pf_out {',
              '        h = h_u32(h, x);', '    }']
        xs = [(i * 2654435761) & 0xffffffff for i in range(n)]
        h = mix(0, sum(xs) & M64)
        for x in xs:
            h = mix(h, x)
        d.append(f'    io.uint(h);  // expect: {h}')
        d += ['}', '', 'export fn run() void {', '    demo() catch @trap();', '}']
        out += d
        with open(os.path.join(out_dir, fname), 'w') as fh:
            fh.write('\n'.join(out) + '\n')

    def init_of(self, n):
        for c in self.folds + self.tickets + self.xchgs + self.cases:
            if c[0] == n:
                return c[3] if c in self.folds or c in self.cases else c[2]
        raise KeyError(n)

    def u64_of(self, t, e):
        if t == 'u64':
            return e
        if t == 'i64':
            return f'@bit_cast(u64, {e})'
        if self.T.signed(t):
            return f'@bit_cast(u64, @as(i64, {e}))'
        return f'@as(u64, {e})'


PROGRAMS = [('exact_atomics_1.ax', 4101, True), ('exact_atomics_2.ax', 4202, False)]
SIZE64_PROGRAMS = [('exact_atomics_size64_1.ax', 4303)]
THREAD_PROGRAMS = [('exact_atomics_threads_1.ax', 4404), ('exact_atomics_threads_2.ax', 4505)]


def main():
    args = sys.argv[1:]
    mode = None
    if args and args[0] in ('--threads', '--size64'):
        mode = args.pop(0)
    if len(args) == 2 and args[0].startswith('--seeds='):
        first, count = (int(x) for x in args[0][len('--seeds='):].split(':'))
        for seed in range(first, first + count):
            if mode == '--threads':
                Threads(seed).write(f'exact_atomics_threads_s{seed}.ax', args[1])
            elif mode == '--size64':
                Single(seed, False, True).write(f'exact_atomics_size64_s{seed}.ax', args[1])
            else:
                Single(seed, False, False).write(f'exact_atomics_s{seed}.ax', args[1])
        return
    for fname, seed, full in PROGRAMS:
        Single(seed, full, False).write(fname, RUN)
    for fname, seed in SIZE64_PROGRAMS:
        Single(seed, True, True).write(fname, RUN)
    for fname, seed in THREAD_PROGRAMS:
        Threads(seed).write(fname, RUN)


if __name__ == '__main__':
    main()
