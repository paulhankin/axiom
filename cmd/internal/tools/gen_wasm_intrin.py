#!/usr/bin/env python3
"""Generates the WebAssembly intrinsics header of the C compiler
(std/c/include/wasm_simd128.h), the WebAssembly part of its table of machine
operations (cmd/axiom/src/c/csimd_tab.ax) and the tests of them
(cmd/axiom/internal/tests/c/run/intrin_wasm_*.c), from the tables below.

    python3 cmd/internal/tools/gen_wasm_intrin.py

The operations are those of the WebAssembly specification's vector
instructions (simd128) and of its relaxed-simd extension, with their
semantics; the intrinsics' names (wasm_i8x16_add ...) are the conventional
ones of the C interface to them, one per instruction (and the unsigned
names of the instructions that do not depend on the sign). Each is a static
function (always inlined) or, where an operand must be a constant (a lane,
a shuffle's indices), a macro. Their bodies are plain C on vector types
where C or the IR says the operation (csema_simd.ax gives the design: add,
mul, the bit operations, compares, shifts, saturating add, min and max,
abs, sqrt, rounding, widening, the lanes, loads and stores, shuffles,
conversions to floating), else the instruction itself: an asm statement in
the text format with its operands in locals (local.get %1 ... local.set %0).
The lane values of the tests' reference results are computed with the
helpers of gen_neon_intrin.py (the same IEEE arithmetic).
"""
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gen_neon_intrin as N  # noqa: E402
from gen_neon_intrin import (fadd, fsub, fmul, fdiv, fmin, fmax, fsqrt, fround, f2i, i2f, rnd, sat, wrap, ubits,  # noqa: E402
                             isnan, fabs_, fenc, lo_hi, cstr)

ROOT = N.ROOT
INC = N.INC
TAB = N.TAB
TESTS = N.TESTS

# the lane kinds: (C element, bits, lanes)
SH = {'i8': ('signed char', 8, 16), 'u8': ('unsigned char', 8, 16), 'i16': ('short', 16, 8), 'u16': ('unsigned short', 16, 8),
      'i32': ('int', 32, 4), 'u32': ('unsigned int', 32, 4), 'i64': ('long long', 64, 2),
      'u64': ('unsigned long long', 64, 2), 'f32': ('float', 32, 4), 'f64': ('double', 64, 2)}
CT = {'i8': 'int8_t', 'u8': 'uint8_t', 'i16': 'int16_t', 'u16': 'uint16_t', 'i32': 'int32_t', 'u32': 'uint32_t',
      'i64': 'int64_t', 'u64': 'uint64_t', 'f32': 'float', 'f64': 'double'}


def nk(k):
    """the gen_neon kind of a lane kind"""
    return {'i': 's', 'u': 'u', 'f': 'f'}[k[0]] + k[1:]


def lanes(k):
    return SH[k][2]


def shape(k):
    return '%sx%d' % ('i' if k[0] in 'iu' else 'f', 0) if False else ('i' if k[0] in 'iu' else 'f') + '%dx%d' % (SH[k][1], SH[k][2])


def T(k):
    return '__axiom_w%s' % k


OPLIST = []
OPS = {}


def mop(tmpl, cons, sig, res, ext):
    key = (tmpl, cons, sig, res, ext)
    if key in OPS:
        return OPS[key]
    ins = [w for w in tmpl.split() if '.' in w and not w.startswith('local.')][0]
    name = 'w.' + ins
    n = 1
    while name in OPS.values():
        n += 1
        name = 'w.%s.%d' % (ins, n)
    OPS[key] = name
    OPLIST.append((name, ext, sig, res, tmpl, cons))
    return name


def M(ins, args, res='a', ext='simd128'):
    """the instruction ins on operands args (C expressions of vector types, or
    scalars: (expr, 'n')), its result a vector (res a: of the first operand's
    type) or a scalar (i: int)"""
    n = len(args)
    gets = ' '.join('local.get %%%d' % (i + 1) for i in range(n))
    tmpl = '%s %s local.set %%0' % (gets, ins)
    cons = ';'.join(['=r'] + ['r'] * n)
    sig = ''.join(a[1] if isinstance(a, tuple) else 'v' for a in args)
    exprs = [a[0] if isinstance(a, tuple) else a for a in args]
    name = mop(tmpl, cons, sig, res, ext)
    return '__builtin_axiom_simd("%s", %s)' % (name, ', '.join(exprs))


INTRIN = []


def fn(name, ret, params, body, model=None, ext='simd128', imms=None, ins=None, out=None, test=None, cat='', mode=None):
    """params: [(C type, name)]; ins: the lane kind of each v128_t operand
    (or scalar kind), for the tests; out: the result's lane kind"""
    INTRIN.append(dict(name=name, ret=ret, params=params, body=body, model=model, ext=ext, imms=imms or {}, ins=ins,
                       out=out, test=test, cat=cat, mode=mode))


def c(k, x):
    return '((%s)(%s))' % (T(k), x)


def v(x):
    return '((v128_t)(%s))' % x


def L1(f):
    return lambda a: [f(x) for x in a]


def L2(f):
    return lambda a, b: [f(x, y) for (x, y) in zip(a, b)]


VV = [('v128_t', 'a'), ('v128_t', 'b')]
V1 = [('v128_t', 'a')]
INTK = ['i8', 'i16', 'i32', 'i64']


def build():
    # ---- loads and stores (plain C: the unaligned forms of the types)
    fn('wasm_v128_load', 'v128_t', [('const void *', 'p')], '*(const __axiom_wu128 *)p', model=lambda m: m[:16], ins=['u8'],
       out='u8', cat='mem')
    fn('wasm_v128_store', 'void', [('void *', 'p'), ('v128_t', 'a')], '{ *(__axiom_wu128 *)p = a; }',
       model=lambda m, a: ('mem', a), ins=['u8', 'u8'], out='u8', cat='mem')
    for k in ('u8', 'u16', 'u32', 'u64'):
        b = SH[k][1]
        fn('wasm_v128_load%d_splat' % b, 'v128_t', [('const void *', 'p')],
           v('__builtin_axiom_simd("splat", (%s)__axiom_wz, *(const __axiom_wu%s *)p)' % (T(k), k)),
           model=lambda m, k=k: [m[0]] * lanes(k), ins=[k], out=k, cat='mem')
        fn('wasm_v128_load%d_lane' % b, 'v128_t', [('const void *', 'p'), ('v128_t', 'a'), ('const int', 'l')],
           '__axiom_wset_%s(a, l, *(const __axiom_wu%s *)p)' % (k, k),
           model=lambda m, a, l: a[:l] + [m[0]] + a[l + 1:], imms={'l': (0, lanes(k) - 1)}, ins=[k, k], out=k, cat='mem')
        fn('wasm_v128_store%d_lane' % b, 'void', [('void *', 'p'), ('v128_t', 'a'), ('const int', 'l')],
           '(void)(*(__axiom_wu%s *)p = (%s)%s[l])' % (k, CT[k], c(k, 'a')),
           model=lambda m, a, l: ('mem', [a[l]]), imms={'l': (0, lanes(k) - 1)}, ins=[k, k], out=k, cat='mem')
    for k in ('u32', 'u64'):
        b = SH[k][1]
        fn('wasm_v128_load%d_zero' % b, 'v128_t', [('const void *', 'p')],
           '__axiom_wset_%s((v128_t)__axiom_wz, 0, *(const __axiom_wu%s *)p)' % (k, k),
           model=lambda m, k=k: [m[0]] + [0] * (lanes(k) - 1), ins=[k], out=k, cat='mem')
    for (w, src) in (('i16x8', 'i8'), ('u16x8', 'u8'), ('i32x4', 'i16'), ('u32x4', 'u16'), ('i64x2', 'i32'), ('u64x2', 'u32')):
        n = 64 // SH[src][1]
        fn('wasm_%s_load%dx%d' % (w, SH[src][1], n), 'v128_t', [('const void *', 'p')],
           M('%s.extend_low_i%sx%d_%s' % ('i' + w[1:], SH[src][1], 128 // SH[src][1], 's' if src[0] == 'i' else 'u'),
             ['__axiom_wset_u64((v128_t)__axiom_wz, 0, *(const __axiom_wuu64 *)p)']),
           model=lambda m, n=n: m[:n], ins=[src], out=w[0] + w[1:3].rstrip('x') if False else
           {'i16x8': 'i16', 'u16x8': 'u16', 'i32x4': 'i32', 'u32x4': 'u32', 'i64x2': 'i64', 'u64x2': 'u64'}[w], cat='mem')
    # ---- make, const, splat, lanes
    for k in SH:
        n = lanes(k)
        sh = shape(k) if k[0] != 'u' else 'u%dx%d' % (SH[k][1], n)
        ps = [(CT[k], 'c%d' % i) for i in range(n)]
        body = '{ %s __r = __axiom_wz_%s; %s return (v128_t)__r; }' % (
            T(k), k, ' '.join('__r[%d] = c%d;' % (i, i) for i in range(n)))
        fn('wasm_%s_make' % sh, 'v128_t', ps, body, model=lambda *xs: list(xs), ins=[k] * n, out=k, cat='data')
        fn('wasm_%s_const' % sh, 'v128_t', ps, 'wasm_%s_make(%s)' % (sh, ', '.join('c%d' % i for i in range(n))),
           model=lambda *xs: list(xs), ins=[k] * n, out=k, cat='data', imms={'c%d' % i: None for i in range(n)})
        fn('wasm_%s_const_splat' % sh, 'v128_t', [(CT[k], 'x')], 'wasm_%s_splat(x)' % sh, model=lambda x, n=n: [x] * n,
           ins=[k], out=k, cat='data', imms={'x': None})
        fn('wasm_%s_splat' % sh, 'v128_t', [(CT[k], 'x')], v('__builtin_axiom_simd("splat", (%s)__axiom_wz, x)' % T(k)),
           model=lambda x, n=n: [x] * n, ins=[k], out=k, cat='data')
        fn('wasm_%s_extract_lane' % sh, CT[k], [('v128_t', 'a'), ('const int', 'l')], '(%s)%s[l]' % (CT[k], c(k, 'a')),
           model=lambda a, l: a[l], imms={'l': (0, n - 1)}, ins=[k], out=k, cat='data')
        fn('wasm_%s_replace_lane' % sh, 'v128_t', [('v128_t', 'a'), ('const int', 'l'), (CT[k], 'x')],
           '__axiom_wset_%s(a, l, x)' % ('u' + k[1:] if k[0] == 'i' else k),
           model=lambda a, l, x: a[:l] + [x] + a[l + 1:], imms={'l': (0, n - 1)}, ins=[k, k], out=k, cat='data')
    # ---- shuffles, swizzles
    for (sh, k) in (('i8x16', 'u8'), ('i16x8', 'u16'), ('i32x4', 'u32'), ('i64x2', 'u64')):
        n = lanes(k)
        ps = [('v128_t', 'a'), ('v128_t', 'b')] + [('const int', 'c%d' % i) for i in range(n)]
        fn('wasm_%s_shuffle' % sh, 'v128_t', ps,
           v('__builtin_axiom_shufflevector(%s, %s, %s)' % (c(k, 'a'), c(k, 'b'), ', '.join('c%d' % i for i in range(n)))),
           model=lambda a, b, *ix: [(a + b)[i] for i in ix], imms={'c%d' % i: (0, 2 * n - 1) for i in range(n)}, ins=[k, k],
           out=k, cat='data')
    fn('wasm_i8x16_swizzle', 'v128_t', VV, M('i8x16.swizzle', ['a', 'b']),
       model=lambda a, b: [a[i] if i < 16 else 0 for i in b], ins=['u8', 'u8'], out='u8', cat='data')
    fn('wasm_i8x16_relaxed_swizzle', 'v128_t', VV, M('i8x16.relaxed_swizzle', ['a', 'b'], ext='relaxed-simd'),
       model=lambda a, b: [a[i] if i < 16 else 0 for i in b], ext='relaxed-simd', ins=['u8', 'swz'], out='u8', cat='relaxed')
    # ---- the bit operations
    fn('wasm_v128_not', 'v128_t', V1, '~a', model=L1(lambda x: ~x), ins=['u8'], out='u8', cat='int')
    for (op, e, f) in (('and', 'a & b', lambda x, y: x & y), ('or', 'a | b', lambda x, y: x | y),
                       ('xor', 'a ^ b', lambda x, y: x ^ y), ('andnot', 'a & ~b', lambda x, y: x & ~y)):
        fn('wasm_v128_%s' % op, 'v128_t', VV, e, model=L2(f), ins=['u8', 'u8'], out='u8', cat='int')
    fn('wasm_v128_bitselect', 'v128_t', VV + [('v128_t', 'm')], '(a & m) | (b & ~m)',
       model=lambda a, b, m: [(x & z) | (y & ~z) for (x, y, z) in zip(a, b, m)], ins=['u8'] * 3, out='u8', cat='int')
    fn('wasm_v128_any_true', 'bool', V1, '(bool)%s' % M('v128.any_true', ['a'], 'i'), model=lambda a: int(any(a)),
       ins=['u8'], out='b', cat='int')
    # ---- integers
    for k in INTK:
        u = 'u' + k[1:]
        b = SH[k][1]
        n = lanes(k)
        sh = shape(k)
        ush = 'u%dx%d' % (b, n)
        both = [(sh, k), (ush, u)]
        for (s, kk) in both:
            fn('wasm_%s_add' % s, 'v128_t', VV, v('%s + %s' % (c(kk, 'a'), c(kk, 'b'))), model=L2(lambda x, y: x + y),
               ins=[kk, kk], out=kk, cat='int')
            fn('wasm_%s_sub' % s, 'v128_t', VV, v('%s - %s' % (c(kk, 'a'), c(kk, 'b'))), model=L2(lambda x, y: x - y),
               ins=[kk, kk], out=kk, cat='int')
            if b > 8:
                fn('wasm_%s_mul' % s, 'v128_t', VV, v('%s * %s' % (c(kk, 'a'), c(kk, 'b'))), model=L2(lambda x, y: x * y),
                   ins=[kk, kk], out=kk, cat='int')
            fn('wasm_%s_eq' % s, 'v128_t', VV, v('%s == %s' % (c(kk, 'a'), c(kk, 'b'))),
               model=L2(lambda x, y: -1 if x == y else 0), ins=[kk, kk], out=k, cat='int')
            fn('wasm_%s_ne' % s, 'v128_t', VV, v('%s != %s' % (c(kk, 'a'), c(kk, 'b'))),
               model=L2(lambda x, y: -1 if x != y else 0), ins=[kk, kk], out=k, cat='int')
            fn('wasm_%s_shl' % s, 'v128_t', [('v128_t', 'a'), ('uint32_t', 'n')],
               v('__builtin_axiom_simd("shli", %s, n & %d)' % (c(kk, 'a'), b - 1)),
               model=lambda a, x, b=b: [y << (x % b) for y in a], ins=[kk, 'u32'], out=kk, cat='int')
            fn('wasm_%s_shr' % s, 'v128_t', [('v128_t', 'a'), ('uint32_t', 'n')],
               v('__builtin_axiom_simd("%s", %s, n & %d)' % ('srai' if kk[0] == 'i' else 'srli', c(kk, 'a'), b - 1)),
               model=lambda a, x, b=b: [y >> (x % b) for y in a], ins=[kk, 'u32'], out=kk, cat='int')
            fn('wasm_%s_all_true' % s, 'bool', V1, '(bool)%s' % M('%s.all_true' % sh, ['a'], 'i'),
               model=lambda a: int(all(a)), ins=[kk], out='b', cat='int')
            fn('wasm_%s_bitmask' % s, 'uint32_t', V1, '(uint32_t)__builtin_axiom_simd("bitmask", %s)' % c(k, 'a'),
               model=lambda a, b=b: sum(1 << i for (i, x) in enumerate(a) if x >> (b - 1) & 1), ins=[u], out='u32', cat='int')
        fn('wasm_%s_neg' % sh, 'v128_t', V1, v('-%s' % c(k, 'a')), model=L1(lambda x: -x), ins=[k], out=k, cat='int')
        fn('wasm_%s_abs' % sh, 'v128_t', V1, v('__builtin_axiom_simd("abs", %s)' % c(k, 'a')), model=L1(abs), ins=[k], out=k,
           cat='int')
        if b == 64:
            for (r, op) in (('lt', '<'), ('gt', '>'), ('le', '<='), ('ge', '>=')):
                fn('wasm_%s_%s' % (sh, r), 'v128_t', VV, v('%s %s %s' % (c(k, 'a'), op, c(k, 'b'))),
                   model=L2(lambda x, y, op=op: -1 if eval('x %s y' % op) else 0), ins=[k, k], out=k, cat='int')
            continue
        for (s, kk) in both:
            for (r, op) in (('lt', '<'), ('gt', '>'), ('le', '<='), ('ge', '>=')):
                fn('wasm_%s_%s' % (s, r), 'v128_t', VV, v('%s %s %s' % (c(kk, 'a'), op, c(kk, 'b'))),
                   model=L2(lambda x, y, op=op: -1 if eval('x %s y' % op) else 0), ins=[kk, kk], out=k, cat='int')
            for (r, g, f) in (('min', 'min', min), ('max', 'max', max)):
                fn('wasm_%s_%s' % (s, r), 'v128_t', VV, v('__builtin_axiom_simd("%s", %s, %s)' % (g, c(kk, 'a'), c(kk, 'b'))),
                   model=L2(f), ins=[kk, kk], out=kk, cat='int')
            if b <= 16:
                for (r, g, f) in (('add_sat', 'adds', lambda x, y: x + y), ('sub_sat', 'subs', lambda x, y: x - y)):
                    gg = g if kk[0] == 'i' else g[:-1] + 'us'
                    fn('wasm_%s_%s' % (s, r), 'v128_t', VV,
                       v('__builtin_axiom_simd("%s", %s, %s)' % (gg, c(kk, 'a'), c(kk, 'b'))),
                       model=L2(lambda x, y, f=f, kk=kk: sat(f(x, y), nk(kk))), ins=[kk, kk], out=kk, cat='int')
                if kk[0] == 'u':
                    fn('wasm_%s_avgr' % s, 'v128_t', VV, M('%s.avgr_u' % sh, ['a', 'b']), model=L2(lambda x, y: (x + y + 1) >> 1),
                       ins=[kk, kk], out=kk, cat='int')
    for (s, k) in (('i8x16', 'i8'), ('u8x16', 'u8')):
        fn('wasm_%s_popcnt' % s, 'v128_t', V1, M('i8x16.popcnt', ['a']), model=L1(lambda x: bin(ubits(x, 8)).count('1')),
           ins=[k], out=k, cat='int')
    fn('wasm_i16x8_q15mulr_sat', 'v128_t', VV, M('i16x8.q15mulr_sat_s', ['a', 'b']),
       model=L2(lambda x, y: sat((x * y + 0x4000) >> 15, 's16')), ins=['i16', 'i16'], out='i16', cat='int')
    fn('wasm_i16x8_relaxed_q15mulr', 'v128_t', VV, M('i16x8.relaxed_q15mulr_s', ['a', 'b'], ext='relaxed-simd'),
       model=L2(lambda x, y: sat((x * y + 0x4000) >> 15, 's16')), ext='relaxed-simd', ins=['i16', 'q15'], out='i16',
       cat='relaxed')
    # (narrowing, widening, extended multiplies, pairwise sums, dot)
    for (d, s, sg) in (('i8x16', 'i16x8', 's'), ('u8x16', 'i16x8', 'u'), ('i16x8', 'i32x4', 's'), ('u16x8', 'i32x4', 'u')):
        dk = d[0] + d[1:].split('x')[0]
        sk_ = 'i' + s[1:].split('x')[0]
        fn('wasm_%s_narrow_%s' % (d, s), 'v128_t', VV, M('%s.narrow_%s_%s' % ('i' + d[1:], s, sg), ['a', 'b']),
           model=lambda a, b, dk=dk: [sat(x, nk(dk)) for x in a + b], ins=[sk_, sk_], out=dk, cat='int')
    for (d, s) in (('i16x8', 'i8x16'), ('u16x8', 'u8x16'), ('i32x4', 'i16x8'), ('u32x4', 'u16x8'), ('i64x2', 'i32x4'),
                   ('u64x2', 'u32x4')):
        dk = d[0] + d[1:].split('x')[0]
        sk_ = s[0] + s[1:].split('x')[0]
        sg = 's' if s[0] == 'i' else 'u'
        for (h, g) in (('low', 'widen_lo'), ('high', 'widen_hi')):
            fn('wasm_%s_extend_%s_%s' % (d, h, s), 'v128_t', V1, M('%s.extend_%s_%s_%s' % ('i' + d[1:], h, 'i' + s[1:], sg), ['a']),
               model=lambda a, h=h: a[:len(a) // 2] if h == 'low' else a[len(a) // 2:], ins=[sk_], out=dk, cat='int')
            fn('wasm_%s_extmul_%s_%s' % (d, h, s), 'v128_t', VV,
               M('%s.extmul_%s_%s_%s' % ('i' + d[1:], h, 'i' + s[1:], sg), ['a', 'b']),
               model=lambda a, b, h=h: [x * y for (x, y) in zip(*((a[:len(a) // 2], b[:len(b) // 2]) if h == 'low' else
                                                                  (a[len(a) // 2:], b[len(b) // 2:])))],
               ins=[sk_, sk_], out=dk, cat='int')
        if d[1:3] in ('16', '32'):
            fn('wasm_%s_extadd_pairwise_%s' % (d, s), 'v128_t', V1,
               M('%s.extadd_pairwise_%s_%s' % ('i' + d[1:], 'i' + s[1:], sg), ['a']),
               model=lambda a: [a[i] + a[i + 1] for i in range(0, len(a), 2)], ins=[sk_], out=dk, cat='int')
    fn('wasm_i32x4_dot_i16x8', 'v128_t', VV, M('i32x4.dot_i16x8_s', ['a', 'b']),
       model=lambda a, b: [a[2 * i] * b[2 * i] + a[2 * i + 1] * b[2 * i + 1] for i in range(4)], ins=['i16', 'i16'], out='i32',
       cat='int')
    fn('wasm_i16x8_relaxed_dot_i8x16_i7x16', 'v128_t', VV, M('i16x8.relaxed_dot_i8x16_i7x16_s', ['a', 'b'], ext='relaxed-simd'),
       model=lambda a, b: [a[2 * i] * b[2 * i] + a[2 * i + 1] * b[2 * i + 1] for i in range(8)], ext='relaxed-simd',
       ins=['i8', 'i7'], out='i16', cat='relaxed')
    fn('wasm_i32x4_relaxed_dot_i8x16_i7x16_add', 'v128_t', VV + [('v128_t', 'c')],
       M('i32x4.relaxed_dot_i8x16_i7x16_add_s', ['a', 'b', 'c'], ext='relaxed-simd'),
       model=lambda a, b, cc: [cc[i] + sum(a[4 * i + j] * b[4 * i + j] for j in range(4)) for i in range(4)],
       ext='relaxed-simd', ins=['i8', 'i7', 'i32'], out='i32', cat='relaxed')
    for (k, sh) in (('i8', 'i8x16'), ('i16', 'i16x8'), ('i32', 'i32x4'), ('i64', 'i64x2')):
        fn('wasm_%s_relaxed_laneselect' % sh, 'v128_t', VV + [('v128_t', 'm')],
           M('%s.relaxed_laneselect' % sh, ['a', 'b', 'm'], ext='relaxed-simd'),
           model=lambda a, b, m: [x if z else y for (x, y, z) in zip(a, b, m)], ext='relaxed-simd', ins=[k, k, 'mask' + k],
           out=k, cat='relaxed')
    # ---- floating
    for k in ('f32', 'f64'):
        sh = shape(k)
        ik = 'i' + k[1:]
        fk = N.FLT[0] if k == 'f32' else 'f64'
        for (op, e, f) in (('add', '+', fadd), ('sub', '-', fsub), ('mul', '*', fmul), ('div', '/', fdiv)):
            fn('wasm_%s_%s' % (sh, op), 'v128_t', VV, v('%s %s %s' % (c(k, 'a'), e, c(k, 'b'))), model=L2(f), ins=[k, k], out=k,
               cat='float')
        for (r, op, f) in (('eq', '==', lambda x, y: x == y), ('ne', '!=', lambda x, y: x != y), ('lt', '<', lambda x, y: x < y),
                           ('gt', '>', lambda x, y: x > y), ('le', '<=', lambda x, y: x <= y), ('ge', '>=', lambda x, y: x >= y)):
            fn('wasm_%s_%s' % (sh, r), 'v128_t', VV, v('%s %s %s' % (c(k, 'a'), op, c(k, 'b'))),
               model=L2(lambda x, y, f=f, r=r: -1 if ((isnan(x) or isnan(y)) and r == 'ne') or
                        (not isnan(x) and not isnan(y) and f(x, y)) else 0), ins=[k, k], out=ik, cat='float')
        fn('wasm_%s_neg' % sh, 'v128_t', V1, v('-%s' % c(k, 'a')), model=L1(lambda x: x if isnan(x) else -x), ins=[k], out=k,
           cat='float')
        fn('wasm_%s_abs' % sh, 'v128_t', V1, M('%s.abs' % sh, ['a']), model=L1(fabs_), ins=[k], out=k, cat='float')
        fn('wasm_%s_sqrt' % sh, 'v128_t', V1, v('__builtin_axiom_simd("sqrt", %s)' % c(k, 'a')),
           model=L1(lambda x, fk=fk: fsqrt(x, fk)), ins=[k], out=k, cat='float')
        for (op, g, r) in (('ceil', 'ceil', 'p'), ('floor', 'floor', 'm'), ('trunc', 'trunc', 'z'), ('nearest', 'nearest', 'n')):
            fn('wasm_%s_%s' % (sh, op), 'v128_t', V1, v('__builtin_axiom_simd("%s", %s)' % (g, c(k, 'a'))),
               model=L1(lambda x, r=r: fround(x, r)), ins=[k], out=k, cat='float')
        fn('wasm_%s_min' % sh, 'v128_t', VV, M('%s.min' % sh, ['a', 'b']), model=L2(fmin), ins=[k, k], out=k, cat='float')
        fn('wasm_%s_max' % sh, 'v128_t', VV, M('%s.max' % sh, ['a', 'b']), model=L2(fmax), ins=[k, k], out=k, cat='float')
        fn('wasm_%s_pmin' % sh, 'v128_t', VV, M('%s.pmin' % sh, ['a', 'b']),
           model=L2(lambda x, y: y if (not isnan(x) and not isnan(y) and y < x) else x), ins=[k, k], out=k, cat='float')
        fn('wasm_%s_pmax' % sh, 'v128_t', VV, M('%s.pmax' % sh, ['a', 'b']),
           model=L2(lambda x, y: y if (not isnan(x) and not isnan(y) and x < y) else x), ins=[k, k], out=k, cat='float')
        fn('wasm_%s_relaxed_min' % sh, 'v128_t', VV, M('%s.relaxed_min' % sh, ['a', 'b'], ext='relaxed-simd'),
           model=L2(fmin), ext='relaxed-simd', ins=['r' + k, 'r' + k], out=k, cat='relaxed')
        fn('wasm_%s_relaxed_max' % sh, 'v128_t', VV, M('%s.relaxed_max' % sh, ['a', 'b'], ext='relaxed-simd'),
           model=L2(fmax), ext='relaxed-simd', ins=['r' + k, 'r' + k], out=k, cat='relaxed')
        for (op, sg) in (('madd', 1), ('nmadd', -1)):
            fn('wasm_%s_relaxed_%s' % (sh, op), 'v128_t', VV + [('v128_t', 'c')],
               M('%s.relaxed_%s' % (sh, op), ['a', 'b', 'c'], ext='relaxed-simd'),
               model=lambda a, b, cc, sg=sg: [sg * x * y + z for (x, y, z) in zip(a, b, cc)], ext='relaxed-simd',
               ins=['s' + k] * 3, out=k, cat='relaxed')
    # ---- conversions
    fn('wasm_f32x4_convert_i32x4', 'v128_t', V1, v('__builtin_axiom_convertvector(%s, __axiom_wf32)' % c('i32', 'a')),
       model=L1(lambda x: i2f(x, 'f32')), ins=['i32'], out='f32', cat='float')
    fn('wasm_f32x4_convert_u32x4', 'v128_t', V1, v('__builtin_axiom_convertvector(%s, __axiom_wf32)' % c('u32', 'a')),
       model=L1(lambda x: i2f(x, 'f32')), ins=['u32'], out='f32', cat='float')
    for (s, k) in (('i32x4', 's32'), ('u32x4', 'u32')):
        sg = 's' if s[0] == 'i' else 'u'
        fn('wasm_%s_trunc_sat_f32x4' % s, 'v128_t', V1, M('i32x4.trunc_sat_f32x4_%s' % sg, ['a']),
           model=L1(lambda x, k=k: f2i(x, k, 'z')), ins=['f32'], out=s[0] + '32', cat='float')
        fn('wasm_%s_trunc_sat_f64x2_zero' % s, 'v128_t', V1, M('i32x4.trunc_sat_f64x2_%s_zero' % sg, ['a']),
           model=lambda a, k=k: [f2i(x, k, 'z') for x in a] + [0, 0], ins=['f64'], out=s[0] + '32', cat='float')
        fn('wasm_f64x2_convert_low_%s' % s, 'v128_t', V1, M('f64x2.convert_low_i32x4_%s' % sg, ['a']),
           model=lambda a: [float(a[0]), float(a[1])], ins=[s[0] + '32'], out='f64', cat='float')
        fn('wasm_%s_relaxed_trunc_f32x4' % s, 'v128_t', V1, M('i32x4.relaxed_trunc_f32x4_%s' % sg, ['a'], ext='relaxed-simd'),
           model=L1(lambda x, k=k: f2i(x, k, 'z')), ext='relaxed-simd', ins=['t' + k + 'f32'], out=s[0] + '32', cat='relaxed')
        fn('wasm_%s_relaxed_trunc_f64x2_zero' % s, 'v128_t', V1,
           M('i32x4.relaxed_trunc_f64x2_%s_zero' % sg, ['a'], ext='relaxed-simd'),
           model=lambda a, k=k: [f2i(x, k, 'z') for x in a] + [0, 0], ext='relaxed-simd', ins=['t' + k + 'f64'],
           out=s[0] + '32', cat='relaxed')
    fn('wasm_f32x4_demote_f64x2_zero', 'v128_t', V1, M('f32x4.demote_f64x2_zero', ['a']),
       model=lambda a: [a[0], a[1], 0.0, 0.0], ins=['f64'], out='f32', cat='float')
    fn('wasm_f64x2_promote_low_f32x4', 'v128_t', V1, M('f64x2.promote_low_f32x4', ['a']),
       model=lambda a: a[:2], ins=['f32'], out='f64', cat='float')


# =============================================================================
# Output
# =============================================================================
PRE = r'''#include <stdint.h>
#if __STDC_VERSION__ < 202311L
#include <stdbool.h>
#endif
/* The function of an intrinsic: inlined into its callers; those of
   relaxed-simd need it (C3240 otherwise) and are compiled with it. */
#define __AXIOM_FN(e) static __axiom_attribute__((__always_inline__, __target__(e), __unused__))
#define __AXIOM_FN0 static __axiom_attribute__((__always_inline__, __unused__))
/* v128_t: 128 bits, of any lanes; the header's vectors of each lane type */
typedef int32_t v128_t __axiom_attribute__((__vector_size__(16), __may_alias__));
'''


def pre_types():
    out = [PRE]
    for (k, (el, b, n)) in SH.items():
        out.append('typedef %s %s __axiom_attribute__((__vector_size__(16)));\n' % (el.replace('long long', '__axiom_int64'), T(k)))
    out.append('typedef v128_t __axiom_wu128 __axiom_attribute__((__aligned__(1)));\n')
    for k in ('u8', 'u16', 'u32', 'u64'):
        out.append('typedef %s __axiom_wu%s __axiom_attribute__((__aligned__(1), __may_alias__));\n' % (CT[k], k))
    out.append('static const __axiom_wi64 __axiom_wz = {0, 0};\n')
    for k in SH:
        out.append('#define __axiom_wz_%s ((%s)__axiom_wz)\n' % (k, T(k)))
    for k in ('u8', 'u16', 'u32', 'u64', 'f32', 'f64'):
        out.append('__AXIOM_FN0 v128_t __axiom_wset_%s(v128_t __a, int __l, %s __x) {\n    %s __r = (%s)__a;\n'
                   '    __r[__l] = __x;\n    return (v128_t)__r;\n}\n' % (k, CT[k], T(k), T(k)))
    out.append('#define __axiom_wset_i8 __axiom_wset_u8\n#define __axiom_wset_i16 __axiom_wset_u16\n'
               '#define __axiom_wset_i32 __axiom_wset_u32\n#define __axiom_wset_i64 __axiom_wset_u64\n')
    return ''.join(out)


def ex(body, names):
    parts = body.split('"')
    pat = re.compile(r'(?<![\w.])(' + '|'.join(sorted(names, key=len, reverse=True)) + r')\b') if names else None
    for i in range(0, len(parts), 2):
        if pat:
            parts[i] = pat.sub(r'__\1', parts[i])
    return '"'.join(parts)


def emit(e):
    name, ret, params, body = e['name'], e['ret'], e['params'], e['body']
    pn = [n for (_, n) in params]
    b = ex(body, pn)
    if e['imms']:
        for (t, n) in params:
            if n in e['imms']:
                b = re.sub(r'(?<![\w.])__%s\b' % n, '(__%s)' % n, b)
            else:
                b = re.sub(r'(?<![\w.])__%s\b' % n, '((%s)(__%s))' % (t, n), b)
        chk = ''.join('(void)sizeof(struct { int __axiom_imm : ((__%s) >= %d && (__%s) <= %d) ? 1 : -1; }), ' % (n, r[0], n, r[1])
                      for (n, r) in e['imms'].items() if r)
        if b.startswith('{'):
            raise Exception(name)
        return '#define %s(%s) (%s%s)\n' % (name, ', '.join('__' + n for n in pn), chk, b)
    sig = ', '.join('%s __%s' % (t, n) for (t, n) in params) or 'void'
    if not b.startswith('{'):
        b = '{ return %s; }' % b if ret != 'void' else '{ %s; }' % b
    head = '__AXIOM_FN("%s")' % e['ext'] if e['ext'] != 'simd128' else '__AXIOM_FN0'
    return '%s %s %s(%s) %s\n' % (head, ret, name, sig, b)


def header_text():
    out = ['/* <wasm_simd128.h> - the WebAssembly vector intrinsics of simd128 and\n',
           '   relaxed-simd (the compiler\'s: docs/c.md, "Intrinsics"). Generated by\n',
           '   cmd/internal/tools/gen_wasm_intrin.py: do not edit. */\n',
           '#ifndef __AXIOM_WASM_SIMD128_H\n#define __AXIOM_WASM_SIMD128_H\n',
           '#if !defined(__wasm_simd128__)\n#error "<wasm_simd128.h> is for WebAssembly with simd128"\n#endif\n',
           pre_types()]
    # (the macros' functions first: make before const, splat before const_splat)
    order = sorted(INTRIN, key=lambda e: 1 if e['name'].endswith('_const') or e['name'].endswith('_const_splat') else 0)
    for e in order:
        out.append(emit(e))
    out.append('#endif\n')
    return ''.join(out)


def table_text():
    keep, keep_a = [], []
    for l in open(TAB).read().split('\n'):
        m = re.match(r'    "([^|]*)\|([^|])\|(.*)",$', l)
        if not m:
            continue
        if m.group(2) in 'x*':
            keep.append(l)
        elif m.group(2) == 'a':
            keep_a.append(l)
    mine = []
    for (name, ext, sig, res, tmpl, cons) in OPLIST:
        mine.append('    "%s",' % '|'.join([name, 'w', ext, sig, res, 'c', tmpl, cons]))
    lines = keep + keep_a + mine
    src = open(TAB).read()
    head = src[:src.index('pub let TABLE')]
    return head + 'pub let TABLE: [%d][]u8 = [\n' % len(lines) + '\n'.join(lines) + '\n];\n'


# -----------------------------------------------------------------------------
# Tests
# -----------------------------------------------------------------------------
RNG = N.RNG


def gen_lane(k):
    """a lane's bits, and its kind (the input kinds of the relaxed forms: values on
    which every allowed result is the same)"""
    if k == 'swz':
        return (RNG.choice([RNG.randint(0, 15), RNG.randint(128, 255)]), 'u8')
    if k == 'q15':
        return (ubits(RNG.randint(-32767, 32767), 16), 'i16')
    if k == 'i7':
        return (RNG.randint(0, 127), 'i8')
    if k.startswith('mask'):
        kk = k[4:]
        return (ubits(RNG.choice([0, -1]), SH[kk][1]), kk)
    if k[0] in 'rs' and k[1:] in ('f32', 'f64'):
        # (relaxed min and max: no NaNs or zeros; madd: small integers, exact either way)
        kk = k[1:]
        x = float(RNG.randint(-50, 50)) if k[0] == 's' else RNG.choice([1, -1]) * (1 + RNG.random()) * 2.0 ** RNG.randint(-10, 10)
        return (fenc(x, kk), kk)
    if k[0] == 't':
        kk = k[-3:]
        rng = (0, 4e9) if k[1] == 'u' else (-2e9, 2e9)
        return (fenc(float(int(RNG.uniform(*rng))), kk), kk)
    nkk = nk(k)
    return (N.rand_lane(nkk, None), k)


def make_case(tf, e):
    args, vals = [], []
    store = None
    ins = list(e['ins'] or [])
    for (t, n) in e['params']:
        if n in e['imms']:
            r = e['imms'][n]
            if r is None:
                # (a constant operand of make, const, splat: a random value, written as a constant)
                k = ins.pop(0)
                bits, kk = gen_lane(k)
                x = N.decode(bits, nk(kk), False)
                vals.append(x)
                args.append(cconst(x, kk))
                continue
            x = RNG.choice([r[0], r[1], RNG.randint(r[0], r[1])])
            vals.append(x)
            args.append(str(x))
            continue
        if t.endswith('*'):
            k = ins.pop(0)
            if 'const' in t:
                nel = 16 * 8 // SH[k][1] // 1
                bits = [gen_lane(k)[0] for _ in range(nel)]
                off = tf.put(b''.join(N.lane_bytes(x, nk(k)) for x in bits))
                args.append('(const void *)(__pool + %d)' % off)
                vals.append([N.decode(x, nk(k), False) for x in bits])
            else:
                store = k
                args.append('(void *)__out')
                vals.append(None)
            continue
        k = ins.pop(0)
        if t == 'v128_t':
            lk = [gen_lane(k) for _ in range(16 * 8 // SH[gen_lane(k)[1]][1])]
            kk = lk[0][1]
            off = tf.put(b''.join(N.lane_bytes(x, nk(kk)) for (x, _) in lk))
            args.append('__L(v128_t, %d)' % off)
            vals.append([N.decode(x, nk(kk), False) for (x, _) in lk])
        else:
            bits, kk = gen_lane(k)
            off = tf.put(N.lane_bytes(bits, nk(kk)))
            args.append('__L(%s, %d)' % (t, off))
            vals.append(N.decode(bits, nk(kk), False))
    r = e['model'](*vals)
    call = '%s(%s)' % (e['name'], ', '.join(args))
    out = e['out']
    fl = out in ('f32', 'f64')
    mode = ({'f32': 2, 'f64': 3}[out] if fl else 0) if e['mode'] != 'exact' else 0
    if store:
        want = bytearray([0x5a] * 64)
        data = b''.join(N.encode(x, nk(store), False).to_bytes(SH[store][1] // 8, 'little') for x in r[1])
        want[:len(data)] = data
        return '__fill(); %s; chk("%s", __out, 64, %s, %d);' % (call, e['name'], cstr(bytes(want)), mode)
    if out == 'b':
        return '{ int __r = %s; chk("%s", &__r, sizeof __r, %s, 0); }' % (call, e['name'], cstr(int(r).to_bytes(4, 'little')))
    if e['ret'] == 'v128_t':
        data = b''.join(N.encode(x, nk(out), False).to_bytes(SH[out][1] // 8, 'little') for x in r)
        assert len(data) == 16, e['name']
        return '{ v128_t __r = %s; chk("%s", &__r, 16, %s, %d); }' % (call, e['name'], cstr(data), mode)
    data = N.encode(r, nk(out), False).to_bytes(SH[out][1] // 8, 'little')
    return '{ %s __r = %s; chk("%s", &__r, %d, %s, %d); }' % (e['ret'], call, e['name'], len(data), cstr(data), mode)


def cconst(x, k):
    """value x of lane kind k as a C constant"""
    if k in ('f32', 'f64'):
        if isnan(x):
            return '(%s)(1e300 * 1e300 - 1e300 * 1e300)' % CT[k]
        if x in (float('inf'), -float('inf')):
            return '(%s)(%s1e300 * 1e300)' % (CT[k], '-' if x < 0 else '')
        return '(%s)%s' % (CT[k], ('-' if str(x).startswith('-') else '') + float.hex(abs(x)))
    if k in ('i64', 'u64'):
        return '(%s)%dU' % (CT[k], ubits(x, 64)) if k == 'u64' or x >= 0 else '(int64_t)(%dLL - 1)' % (x + 1)
    if k == 'i32':
        return '(%d - 1)' % (x + 1)
    if k == 'u32':
        return '%dU' % x
    return '(%s)%d' % (CT[k], x)


GROUPS = [('intrin_wasm_simd', 'simd128: loads, stores, lanes, shuffles, integers, floating, conversions', 'c11',
           ('mem', 'data', 'int', 'float'), ''),
          ('intrin_wasm_relaxed', 'relaxed-simd, on the inputs whose results every implementation agrees on', 'c17',
           ('relaxed',), '--features=+relaxed-simd')]

FIXED = {
    'intrin_wasm_err_relaxed': r"""/* A relaxed-simd intrinsic in a function not compiled for relaxed-simd is
   C3240 at the call. Generated by cmd/internal/tools/gen_wasm_intrin.py: do
   not edit. */
/* flags: --target=wasm32 */
/* expect-error: C3240 */
#include <wasm_simd128.h>

v128_t f(v128_t a, v128_t b, v128_t c) {
    return wasm_f32x4_relaxed_madd(a, b, c);
}

int main(void) {
    return 0;
}
""",
    'intrin_wasm_err_lane': r"""/* A lane must be a constant in its range: C3241 (or C3070, the range's check)
   for lane 4 of i32x4. Generated by cmd/internal/tools/gen_wasm_intrin.py:
   do not edit. */
/* flags: --target=wasm32 */
/* expect-error: C3070 */
#include <wasm_simd128.h>

int f(v128_t a) {
    return wasm_i32x4_extract_lane(a, 4);
}

int main(void) {
    return 0;
}
""",
    'intrin_wasm_err_nosimd': r"""/* <wasm_simd128.h> needs simd128. Generated by cmd/internal/tools/gen_wasm_intrin.py: do not edit. */
/* flags: --target=wasm32 --features=-simd128 */
/* expect-error: C1040 */
#include <wasm_simd128.h>

int main(void) {
    return 0;
}
""",
}


def test_text(tf, name, what, std, flags):
    out = ['/* %s - WebAssembly vector intrinsics checked against the specification\'s\n' % name,
           '   definitions of their instructions (computed by\n',
           '   cmd/internal/tools/gen_wasm_intrin.py, which writes this file: do not\n',
           '   edit): %s. A mismatch prints the intrinsic and the result\'s bytes. */\n' % what,
           '/* wasm-only */\n/* flags: --c-std=%s%s */\n' % (std, ' ' + flags if flags else ''),
           '/* expect: %s %d */\n' % (name, len(tf.cases)),
           '#include <wasm_simd128.h>\n#include "host.h"\n\n',
           '#define __L(t, o) (*(const t *)(const void *)(__pool + (o)))\n']
    lines = ['    "%s"\n' % ''.join('\\x%02x' % b for b in tf.pool[i:i + 32]) for i in range(0, len(tf.pool), 32)]
    out.append('static __axiom_attribute__((__aligned__(64))) unsigned char __pool[%d] =\n%s;\n' % (
        len(tf.pool), ''.join(lines).rstrip('\n')))
    out.append(N.HARNESS)
    parts = [tf.cases[j:j + 16] for j in range(0, len(tf.cases), 16)]
    for (k, part) in enumerate(parts):
        out.append('static void run%d(void) {\n' % k)
        for cs in part:
            out.append('    %s\n' % cs)
        out.append('}\n\n')
    out.append('int main(void) {\n    if (__touch != 0) {\n        __touch(__pool);\n    }\n')
    for k in range(len(parts)):
        out.append('    run%d();\n' % k)
    out.append('    if (__fails == 0) {\n        say("%s ");\n        say_n(__checks);\n        endl();\n    }\n' % name)
    out.append('    return 0;\n}\n')
    return ''.join(out)


def compile_test():
    out = ['/* Every intrinsic of <wasm_simd128.h> is compiled (each with its operands;\n',
           '   constants their least value): its instruction assembles. Generated by\n',
           '   cmd/internal/tools/gen_wasm_intrin.py: do not edit. */\n',
           '/* wasm-only */\n/* flags: --features=+relaxed-simd */\n/* expect: compiled */\n',
           '#include <wasm_simd128.h>\n#include "host.h"\n\n',
           'static __axiom_attribute__((__aligned__(64))) unsigned char __buf[64];\nvolatile int __sink;\n\n']
    for (k, e) in enumerate(INTRIN):
        args = []
        for (t, n) in e['params']:
            if n in e['imms']:
                args.append(str(e['imms'][n][0]) if e['imms'][n] else '1')
            elif t.endswith('*'):
                args.append('(%s)__buf' % t)
            else:
                args.append('*(const %s *)(const void *)__buf' % t)
        call = '%s(%s)' % (e['name'], ', '.join(args))
        out.append('void __t%d(void) {\n' % k)
        if e['ret'] == 'void':
            out.append('    %s;\n' % call)
        else:
            out.append('    %s __r = %s;\n    __sink = *(volatile unsigned char *)(void *)&__r;\n' % (e['ret'], call))
        out.append('}\n')
    out.append('\nint main(void) {\n    print_str("compiled", 8);\n    return 0;\n}\n')
    return ''.join(out)


def main():
    build()
    with open(os.path.join(INC, 'wasm_simd128.h'), 'w') as f:
        f.write(header_text())
    t = table_text()
    with open(TAB, 'w') as f:
        f.write(t)
    with open(os.path.join(TESTS, 'run', 'intrin_wasm_compile.c'), 'w') as f:
        f.write(compile_test())
    for (n, txt) in FIXED.items():
        with open(os.path.join(TESTS, 'run', n + '.c'), 'w') as f:
            f.write(txt)
    for (name, what, std, cats, flags) in GROUPS:
        tf = N.TestFile(name, what)
        for e in INTRIN:
            if e['cat'] in cats and e['model'] and e['test'] != 'no':
                tf.cases.append(make_case(tf, e))
        with open(os.path.join(TESTS, 'run', name + '.c'), 'w') as f:
            f.write(test_text(tf, name, what, std, flags))


if __name__ == '__main__':
    main()
