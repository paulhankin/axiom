#!/usr/bin/env python3
"""Generates the x86-64 intrinsics headers of the C compiler (std/c/include/:
<mmintrin.h> ... <immintrin.h>, <x86intrin.h>, <cpuid.h>), the x86-64 part of
its table of machine operations (cmd/axiom/src/c/csimd_tab.ax) and the tests
of them (cmd/axiom/internal/tests/c/run/intrin_x86_*.c,
cmd/axiom/internal/tests/c/errors/intrin_x86_*.c), from the tables below.

    python3 cmd/internal/tools/gen_x86_intrin.py

The intrinsics are those of Intel's intrinsics reference (names, operand
types and lane semantics) and of the instruction set reference (Intel SDM
vol. 2), written here from those documents. Each is a static function
(always inlined, compiled with its extension: the attribute target) or, when
an operand must be a constant, a macro, whose body is plain C on the vector
types (vector_size, spelled __axiom_attribute__ for every mode), the
compiler's __builtin_axiom_shufflevector and __builtin_axiom_convertvector, or
__builtin_axiom_simd of an operation: a generic one (the IR's) or a machine
one of OPS below (one instruction, or a short fixed sequence, in the Intel
syntax: c/cgen_simd.ax says how a form reads). csema_simd.ax gives the
design. The headers are C89 (no // comments, no compound literals, no
inline before C99: the functions are static, which suffices).

Abbreviations in the bodies below: S( is __builtin_axiom_simd(, SV( is
__builtin_axiom_shufflevector(, CV( is __builtin_axiom_convertvector(; a
lane-typed vector type written as a call, i32x4(x), is a cast to it
(__axiom_i32x4); likewise m128(x), m128d(x), m128i(x), m256(x) ... and
m64(x). One-letter names (a, b, c, d, i, k, m, p, q, x, y) are the
parameters, written __a ...
"""
import os
import random
import re

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..')
INC = os.path.join(ROOT, 'std', 'c', 'include')
TAB = os.path.join(ROOT, 'cmd', 'axiom', 'src', 'c', 'csimd_tab.ax')
TESTS = os.path.join(ROOT, 'cmd', 'axiom', 'internal', 'tests', 'c')

# -----------------------------------------------------------------------------
# Machine operations: name -> (ext, sig, res, form, mnemonic[, constraints, clobbers, flags])
# (csema_simd.ax: sig and res; cgen_simd.ax: form)
# -----------------------------------------------------------------------------
OPS = {}


def op(name, ext, sig, res, form, mn=None, cons='', clob='', flags=''):
    assert name not in OPS, name
    OPS[name] = (ext, sig, res, form, mn or name, cons, clob, flags)


# Generic operations (any architecture; c/cgen_simd.ax)
GENERIC = [
    ('adds', 'VV'), ('addus', 'VV'), ('subs', 'VV'), ('subus', 'VV'), ('min', 'VV'), ('max', 'VV'),
    ('abs', 'V'), ('sqrt', 'V'), ('floor', 'V'), ('ceil', 'V'), ('trunc', 'V'),
    ('nearest', 'V'), ('fma', 'VVV'), ('bitmask', 'v'), ('madd16', 'VV'), ('widen_lo', 'v'), ('widen_hi', 'v'),
    ('shli', 'vn'), ('srli', 'vn'), ('srai', 'vn'), ('shlv', 'VV'), ('srlv', 'VV'), ('srav', 'VV'),
    ('splat', 'vx'), ('popcnt', 's'), ('lzcnt', 's'), ('tzcnt', 's'), ('umulh', 'ss'), ('cpuid', 'ss'),
    ('xgetbv', 's'),
]
GENERIC_RES = {'bitmask': 'i', 'cpuid': 'q', 'xgetbv': 'L'}

# -----------------------------------------------------------------------------
# Intrinsics: (header, ext, name, ret, params, body); a name beginning with
# '#' is a macro (params: its operands' types, for the compile test only)
# -----------------------------------------------------------------------------
INTRIN = []
EXTRA = {}  # a header's text before its intrinsics
HDR = None
EXT = None


def hdr(h, ext):
    global HDR, EXT
    HDR, EXT = h, ext


def fn(name, ret, params, body, ext=None):
    INTRIN.append((HDR, ext or EXT, name, ret, params, body))


def alias(new, old):
    """new: another name of intrinsic old (same header)."""
    for (h, e, n, r, ps, b) in INTRIN:
        if n.lstrip('#') == old:
            if n.startswith('#'):
                args = ', '.join(pname for (_, pname) in split_params(ps))
                INTRIN.append((HDR, e, '#' + new, r, ps, '%s(%s)' % (old, args)))
            else:
                args = ', '.join(pname for (_, pname) in split_params(ps))
                INTRIN.append((HDR, e, new, r, ps, '%s(%s)' % (old, args)))
            return
    raise KeyError(old)


def split_params(ps):
    out = []
    if ps.strip() in ('', 'void'):
        return out
    for p in ps.split(','):
        p = p.strip()
        m = re.match(r'(.*?)(\w+)$', p)
        out.append((m.group(1).strip(), m.group(2)))
    return out


VT = {}  # lane-typed vector types: name -> (element, bytes)
for w in (8, 16, 32, 64):
    for (pre, el, b) in (('i8', 'signed char', 1), ('u8', 'unsigned char', 1), ('i16', 'short', 2),
                         ('u16', 'unsigned short', 2), ('i32', 'int', 4), ('u32', 'unsigned int', 4),
                         ('i64', '__axiom_int64', 8), ('u64', 'unsigned __axiom_int64', 8), ('f32', 'float', 4),
                         ('f64', 'double', 8)):
        VT['%sx%d' % (pre, w // b)] = (el, w)
PUB = ['m64', 'm128', 'm128d', 'm128i', 'm256', 'm256d', 'm256i', 'm128h', 'm256h']
LETTERS = 'abcdikmpqxyjnes'


def castify(b):
    """T(x), for a vector type T: ((T)(x))"""
    out = []
    i = 0
    pat = re.compile(r'\b([iuf](?:8|16|32|64)x\d+|m64|m128[di]?|m256[di]?)\(')
    while True:
        m = pat.search(b, i)
        if not m or (m.group(1) not in VT and not m.group(1).startswith('m')):
            if m:
                out.append(b[i:m.end()])
                i = m.end()
                continue
            out.append(b[i:])
            return ''.join(out)
        out.append(b[i:m.start()])
        t = '__axiom_' + m.group(1) if m.group(1) in VT else '__' + m.group(1)
        depth = 1
        j = m.end()
        while depth:
            if b[j] == '(':
                depth += 1
            elif b[j] == ')':
                depth -= 1
            j += 1
        out.append('((%s)(%s))' % (t, castify(b[m.end():j - 1])))
        i = j


def expand(body, pnames):
    b = body
    b = re.sub(r'\bS\(', '__builtin_axiom_simd(', b)
    b = re.sub(r'\bSV\(', '__builtin_axiom_shufflevector(', b)
    b = re.sub(r'\bCV\(', '__builtin_axiom_convertvector(', b)
    b = castify(b)
    b = re.sub(r'\b([iuf](?:8|16|32|64)x\d+)\b(?!\()', lambda m: '__axiom_' + m.group(1) if m.group(1) in VT else m.group(0), b)
    b = re.sub(r'\b(m64|m128[di]?|m256[di]?)\b', r'__\1', b)
    b = re.sub(r'\bi64\b', '__axiom_int64', b)
    b = re.sub(r'\bu64\b', 'unsigned __axiom_int64', b)
    # parameters, and the bodies' own one-letter names (reserved: a program's macros cannot change them)
    names = list(pnames) + list('abcdefghijklmnopqrstuvwxyz')
    b = re.sub(r'(?<![\w.\'"%])(' + '|'.join(names) + r')\b(?!["\'])', r'__\1', b)
    return b


def ctype(t):
    t = re.sub(r'\b([iuf](?:8|16|32|64)x\d+)\b', lambda m: '__axiom_' + m.group(1) if m.group(1) in VT else m.group(0), t)
    t = re.sub(r'\bi64\b', '__axiom_int64', t)
    t = re.sub(r'\bu64\b', 'unsigned __axiom_int64', t)
    return t


# -----------------------------------------------------------------------------
# Families: integer and floating operations at 128 bits (SSE, SSE2 ...) and
# 256 bits (AVX, AVX2): W is the width, the lane types named by it
# -----------------------------------------------------------------------------
def lt(k, w):
    """the lane-typed vector type of kind k (i8 ... f64) and w bits"""
    b = int(re.sub(r'\D', '', k))
    return '%sx%d' % (k, w // b)


def iname(w):
    return {64: 'm64', 128: 'm128i', 256: 'm256i'}[w]


def pre(w):
    return {128: '_mm_', 256: '_mm256_'}[w]


def isfx(k):
    return {'i8': 'epi8', 'u8': 'epu8', 'i16': 'epi16', 'u16': 'epu16', 'i32': 'epi32', 'u32': 'epu32',
            'i64': 'epi64', 'u64': 'epu64'}[k]


def int_ops(w, ext):
    I = iname(w)
    P = pre(w)
    si = 'si%d' % w
    for k in ('i8', 'i16', 'i32', 'i64'):
        t = lt(k, w)
        fn(P + 'add_' + isfx(k), I, '%s a, %s b' % (I, I), '%s(%s(a) + %s(b))' % (I, t, t), ext)
        fn(P + 'sub_' + isfx(k), I, '%s a, %s b' % (I, I), '%s(%s(a) - %s(b))' % (I, t, t), ext)
    for k in ('i8', 'i16'):
        for (o, g) in (('adds', 'adds'), ('subs', 'subs')):
            t = lt(k, w)
            fn(P + o + '_' + isfx(k), I, '%s a, %s b' % (I, I), '%s(S("%s", %s(a), %s(b)))' % (I, g, t, t), ext)
            u = lt('u' + k[1:], w)
            fn(P + o + '_' + isfx('u' + k[1:]), I, '%s a, %s b' % (I, I),
               '%s(S("%s", %s(a), %s(b)))' % (I, g.replace('s', 'us', 1) if g == 'adds' else 'subus', u, u), ext)
    for (n, m) in (('avg_epu8', 'pavgb'), ('avg_epu16', 'pavgw'), ('mulhi_epi16', 'pmulhw'),
                   ('mulhi_epu16', 'pmulhuw'), ('mul_epu32', 'pmuludq'), ('sad_epu8', 'psadbw'),
                   ('packs_epi16', 'packsswb'), ('packs_epi32', 'packssdw'), ('packus_epi16', 'packuswb')):
        fn(P + n, I, '%s a, %s b' % (I, I), '%s(S("%s", i8x%d(a), i8x%d(b)))' % (I, m, w // 8, w // 8), ext)
    fn(P + 'madd_epi16', I, '%s a, %s b' % (I, I), '%s(S("madd16", %s(a), %s(b)))' % (I, lt('i16', w), lt('i16', w)),
       ext)
    fn(P + 'mullo_epi16', I, '%s a, %s b' % (I, I), '%s(%s(a) * %s(b))' % (I, lt('i16', w), lt('i16', w)), ext)
    for (k, mx) in (('i16', 1), ('u8', 1)):
        t = lt(k, w)
        fn(P + 'max_' + isfx(k), I, '%s a, %s b' % (I, I), '%s(S("max", %s(a), %s(b)))' % (I, t, t), ext)
        fn(P + 'min_' + isfx(k), I, '%s a, %s b' % (I, I), '%s(S("min", %s(a), %s(b)))' % (I, t, t), ext)
    # shifts: by an immediate (any count; 0 past the width, or the sign), by a vector's low 64 bits
    for k in ('i16', 'i32', 'i64'):
        t = lt(k, w)
        u = lt('u' + k[1:], w)
        s = isfx(k)
        fn(P + 'slli_' + s, I, '%s a, int i' % I, '%s(S("shli", %s(a), i))' % (I, t), ext)
        fn(P + 'srli_' + s, I, '%s a, int i' % I, '%s(S("srli", %s(a), i))' % (I, u), ext)
        mn = {'i16': 'w', 'i32': 'd', 'i64': 'q'}[k]
        fn(P + 'sll_' + s, I, '%s a, __m128i c' % I, '%s(S("psll%s", %s(a), i64x2(c)))' % (I, mn, t), ext)
        fn(P + 'srl_' + s, I, '%s a, __m128i c' % I, '%s(S("psrl%s", %s(a), i64x2(c)))' % (I, mn, t), ext)
        if k != 'i64':
            fn(P + 'srai_' + s, I, '%s a, int i' % I, '%s(S("srai", %s(a), i))' % (I, t), ext)
            fn(P + 'sra_' + s, I, '%s a, __m128i c' % I, '%s(S("psra%s", %s(a), i64x2(c)))' % (I, mn, t), ext)
    # byte shifts of each 128-bit lane
    n = w // 8
    def bsh(left):
        idx = []
        for j in range(n):
            lane, o = j // 16, j % 16
            if left:
                idx.append('((i) > 15 ? %d : (%d < (i) ? %d : %d - (i)))' % (n, o, n, lane * 16 + o))
            else:
                idx.append('((i) > 15 ? %d : (%d + (i) > 15 ? %d : %d + (i)))' % (n, o, n, lane * 16 + o))
        return '%s(SV(i8x%d(a), i8x%d(%s), %s))' % (I, n, n, 'S("splat", i8x%d(a), 0)' % n, ', '.join(idx))
    fn('#' + P + 'bslli_' + ('si128' if w == 128 else 'epi128'), I, '%s a, int i' % I, bsh(True), ext)
    fn('#' + P + 'bsrli_' + ('si128' if w == 128 else 'epi128'), I, '%s a, int i' % I, bsh(False), ext)
    alias(P + 'slli_' + si if w == 128 else P + 'slli_si256', P + 'bslli_' + ('si128' if w == 128 else 'epi128'))
    alias(P + 'srli_' + si if w == 128 else P + 'srli_si256', P + 'bsrli_' + ('si128' if w == 128 else 'epi128'))
    # logic, compares
    t = lt('i64', w)
    fn(P + 'and_' + si, I, '%s a, %s b' % (I, I), '%s(%s(a) & %s(b))' % (I, t, t), ext)
    fn(P + 'andnot_' + si, I, '%s a, %s b' % (I, I), '%s(~%s(a) & %s(b))' % (I, t, t), ext)
    fn(P + 'or_' + si, I, '%s a, %s b' % (I, I), '%s(%s(a) | %s(b))' % (I, t, t), ext)
    fn(P + 'xor_' + si, I, '%s a, %s b' % (I, I), '%s(%s(a) ^ %s(b))' % (I, t, t), ext)
    for k in ('i8', 'i16', 'i32'):
        t = lt(k, w)
        fn(P + 'cmpeq_' + isfx(k), I, '%s a, %s b' % (I, I), '%s(%s(a) == %s(b))' % (I, t, t), ext)
        fn(P + 'cmpgt_' + isfx(k), I, '%s a, %s b' % (I, I), '%s(%s(a) > %s(b))' % (I, t, t), ext)
        if w == 128:
            fn(P + 'cmplt_' + isfx(k), I, '%s a, %s b' % (I, I), '%s(%s(a) < %s(b))' % (I, t, t), ext)
    fn(P + 'movemask_epi8', 'int', '%s a' % I, 'S("bitmask", %s(a))' % lt('i8', w), ext)
    # unpacks (within each 128-bit lane)
    for k in ('i8', 'i16', 'i32', 'i64'):
        t = lt(k, w)
        nl = w // int(k[1:])
        per = 128 // int(k[1:])
        for hi in (False, True):
            idx = []
            for lane in range(w // 128):
                base = lane * per + (per // 2 if hi else 0)
                for j in range(per // 2):
                    idx += [base + j, nl + base + j]
            fn(P + ('unpackhi_' if hi else 'unpacklo_') + isfx(k), I, '%s a, %s b' % (I, I),
               '%s(SV(%s(a), %s(b), %s))' % (I, t, t, ', '.join(map(str, idx))), ext)
    # shuffles by an immediate (within each 128-bit lane)
    nl = w // 32
    idx = ', '.join('%d + (((i) >> %d) & 3)' % (4 * (j // 4), 2 * (j % 4)) for j in range(nl))
    fn('#' + P + 'shuffle_epi32', I, '%s a, int i' % I, '%s(SV(i32x%d(a), i32x%d(a), %s))' % (I, nl, nl, idx), ext)
    nl = w // 16
    lo = []
    hi = []
    for j in range(nl):
        g, o = j // 8, j % 8
        lo.append('%d + (((i) >> %d) & 3)' % (8 * g, 2 * o) if o < 4 else str(j))
        hi.append('%d + (((i) >> %d) & 3)' % (8 * g + 4, 2 * (o - 4)) if o >= 4 else str(j))
    fn('#' + P + 'shufflelo_epi16', I, '%s a, int i' % I, '%s(SV(i16x%d(a), i16x%d(a), %s))' % (I, nl, nl, ', '.join(lo)),
       ext)
    fn('#' + P + 'shufflehi_epi16', I, '%s a, int i' % I, '%s(SV(i16x%d(a), i16x%d(a), %s))' % (I, nl, nl, ', '.join(hi)),
       ext)


def float_ops(w, ext, k):
    """the packed operations of floating lanes k (f32: ps, f64: pd)"""
    sx = 'ps' if k == 'f32' else 'pd'
    T = {('f32', 128): 'm128', ('f64', 128): 'm128d', ('f32', 256): 'm256', ('f64', 256): 'm256d'}[(k, w)]
    P = pre(w)
    t = lt(k, w)
    it = lt('i32' if k == 'f32' else 'i64', w)
    for (o, c) in (('add', '+'), ('sub', '-'), ('mul', '*'), ('div', '/')):
        fn(P + o + '_' + sx, T, '%s a, %s b' % (T, T), '%s(a) %s %s(b)' % (T, c, T), ext)
    for (o, c) in (('and', '&'), ('or', '|'), ('xor', '^')):
        fn(P + o + '_' + sx, T, '%s a, %s b' % (T, T), '%s(%s(a) %s %s(b))' % (T, it, c, it), ext)
    fn(P + 'andnot_' + sx, T, '%s a, %s b' % (T, T), '%s(~%s(a) & %s(b))' % (T, it, it), ext)
    fn(P + 'min_' + sx, T, '%s a, %s b' % (T, T), 'S("min%s", a, b)' % sx, ext)
    fn(P + 'max_' + sx, T, '%s a, %s b' % (T, T), 'S("max%s", a, b)' % sx, ext)
    fn(P + 'sqrt_' + sx, T, '%s a' % T, 'S("sqrt", a)', ext)
    fn(P + 'movemask_' + sx, 'int', '%s a' % T, 'S("bitmask", %s(a))' % it, ext)
    nl = w // int(k[1:])
    per = 128 // int(k[1:])
    for hi in (False, True):
        idx = []
        for lane in range(w // 128):
            base = lane * per + (per // 2 if hi else 0)
            for j in range(per // 2):
                idx += [base + j, nl + base + j]
        fn(P + ('unpackhi_' if hi else 'unpacklo_') + sx, T, '%s a, %s b' % (T, T),
           'SV(a, b, %s)' % ', '.join(map(str, idx)), ext)
    return T


# the compare predicates of cmpps (imm8 0-31): name, the C form where it is one (with its quietness)
CMP = ['eq_oq', 'lt_os', 'le_os', 'unord_q', 'neq_uq', 'nlt_us', 'nle_us', 'ord_q',
       'eq_uq', 'nge_us', 'ngt_us', 'false_oq', 'neq_oq', 'ge_os', 'gt_os', 'true_uq',
       'eq_os', 'lt_oq', 'le_oq', 'unord_s', 'neq_us', 'nlt_uq', 'nle_uq', 'ord_s',
       'eq_us', 'nge_uq', 'ngt_uq', 'false_os', 'neq_os', 'ge_oq', 'gt_oq', 'true_us']


# -----------------------------------------------------------------------------
# The machine operations of SSE ... SSE4.2, AES, PCLMUL, AVX, AVX2, FMA, F16C,
# BMI and SHA (the forms of c/cgen_simd.ax; '-': x86-64's baseline, SSE2)
# -----------------------------------------------------------------------------
for m in ('pavgb', 'pavgw', 'pmulhw', 'pmulhuw', 'pmuludq', 'psadbw', 'packsswb', 'packssdw', 'packuswb',
          'addsubps', 'addsubpd', 'haddps', 'haddpd', 'hsubps', 'hsubpd'):
    op(m, 'sse3' if m[1:4] in ('dds', 'add', 'sub') and m.startswith(('a', 'h')) else '-', 'VV', 'a', '2')
for m in ('psllw', 'pslld', 'psllq', 'psrlw', 'psrld', 'psrlq', 'psraw', 'psrad'):
    op(m, '-', 'vv', 'a', '2')
for m in ('rcpps', 'rsqrtps', 'cvtps2dq', 'cvttps2dq'):
    op(m, '-', 'v', 'a', '1')
for m in ('rcpss', 'rsqrtss', 'sqrtss', 'sqrtsd', 'minss', 'maxss', 'minsd', 'maxsd', 'minps', 'maxps', 'minpd', 'maxpd'):
    op(m, '-', 'VV', 'a', '2')
for m in ('cmpps', 'cmppd', 'cmpss', 'cmpsd'):
    op(m, '-', 'VVi', 'a', '2i')
for (m, r) in (('cvtss2si', 'i'), ('cvttss2si', 'i'), ('cvtsd2si', 'i'), ('cvttsd2si', 'i')):
    op(m, '-', 'v', r, '1')
    op(m + '64', '-', 'v', 'l', '1', m)
op('cvtpd2dq', '-', 'v', 'a', '1')
op('cvttpd2dq', '-', 'v', 'a', '1')
# (vcvtpd2dq and vcvttpd2dq of a ymm register: the size is in the mnemonic's operands)
op('cvtpd2dq256', 'avx', 'v', 'h', 'v1', 'vcvtpd2dq')
op('cvttpd2dq256', 'avx', 'v', 'h', 'v1', 'vcvttpd2dq')
op('sfence', '-', '', 'v', 'c', 'sfence', '', '', 'VM')
op('lfence', '-', '', 'v', 'c', 'lfence', '', '', 'VM')
op('mfence', '-', '', 'v', 'c', 'mfence', '', '', 'VM')
op('pause', '-', '', 'v', 'c', 'pause', '', '', 'V')
op('clflush', '-', 'p', 'v', 'c', 'clflush byte ptr [%0]', 'r', '', 'VM')
for h in ('prefetcht0', 'prefetcht1', 'prefetcht2', 'prefetchnta'):
    op(h, '-', 'p', 'v', 'c', h + ' byte ptr [%0]', 'r', '', 'V')
op('stmxcsr', '-', 'p', 'v', 'c', 'stmxcsr dword ptr [%0]', 'r', '', 'VM')
op('ldmxcsr', '-', 'p', 'v', 'c', 'ldmxcsr dword ptr [%0]', 'r', '', 'VM')
op('rdtsc', '-', '', 'L', 'c', 'rdtsc; shl rdx, 32; or %0, rdx', '=a', 'rdx', 'V')
op('rdtscp', '-', 'p', 'L', 'c', 'rdtscp; shl rdx, 32; or %0, rdx; mov dword ptr [%1], ecx', '=a;r', 'rdx;rcx', 'VM')
op('monitor', 'sse3', 'pss', 'v', 'c', 'monitor', 'a;c;d', '', 'VM')
op('mwait', 'sse3', 'ss', 'v', 'c', 'mwait', 'c;a', '', 'VM')
# SSSE3
for m in ('pmaddubsw', 'pmulhrsw', 'pshufb', 'psignb', 'psignw', 'psignd', 'phaddw', 'phaddd', 'phaddsw',
          'phsubw', 'phsubd', 'phsubsw'):
    op(m, 'ssse3', 'VV', 'a', '2')
op('palignr', 'ssse3', 'VVi', 'a', '2i')
# SSE4.1
for m in ('pmuldq', 'packusdw'):
    op(m, 'sse4.1', 'VV', 'a', '2')
for m in ('dpps', 'dppd', 'mpsadbw', 'insertps'):
    op(m, 'sse4.1', 'VVi', 'a', '2i')
for m in ('roundps', 'roundpd'):
    op(m, 'sse4.1', 'vi', 'a', '1i')
for m in ('roundss', 'roundsd'):
    op(m, 'sse4.1', 'VVi', 'a', '2i')
op('phminposuw', 'sse4.1', 'v', 'a', '1')
for x in ('bw', 'bd', 'bq', 'wd', 'wq', 'dq'):
    op('pmovsx' + x, 'sse4.1', 'v', 'a', '1')
    op('pmovzx' + x, 'sse4.1', 'v', 'a', '1')
    # (a ymm result of an xmm operand: twice its size)
    op('pmovsx' + x + '256', 'avx2', 'v', '2', 'v1', 'vpmovsx' + x)
    op('pmovzx' + x + '256', 'avx2', 'v', '2', 'v1', 'vpmovzx' + x)
for (n, cc) in (('testz', 'z'), ('testc', 'c'), ('testnzc', 'a')):
    op('p' + n, 'sse4.1', 'vV', 'i', 'c', 'xor %k0, %k0; ptest %1, %2; set' + cc + ' %b0||xor %k0, %k0; vptest %1, %2; set' + cc + ' %b0',
       '=&r;x;x||=&r;x;x')
    for t in ('ps', 'pd'):
        op('vtest' + t + '_' + n, 'avx', 'vV', 'i', 'c', 'xor %k0, %k0; vtest' + t + ' %1, %2; set' + cc + ' %b0', '=&r;x;x')
# SSE4.2: the string compares (index in ecx, mask in xmm0, flags)
for (e, cons_in, sig) in (('i', 'x;x;i', 'vvi'), ('e', 'x;a;x;d;i', 'vsvsi')):
    op('pcmp%sstri' % e, 'sse4.2', sig, 'i', 'c', 'pcmp%sstri %%1, %%2, %%3' % e if e == 'i' else 'pcmpestri %1, %3, %5',
       '=c;' + cons_in)
    op('pcmp%sstrm' % e, 'sse4.2', sig, 'a', 'c', 'pcmp%sstrm %%1, %%2, %%3' % e if e == 'i' else 'pcmpestrm %1, %3, %5',
       '=Yz;' + cons_in)
    for f in 'acosz':
        cc = {'a': 'a', 'c': 'c', 'o': 'o', 's': 's', 'z': 'e'}[f]
        t = 'pcmpistri %1, %2, %3' if e == 'i' else 'pcmpestri %1, %3, %5'
        op('pcmp%sstr%s' % (e, f), 'sse4.2', sig, 'i', 'c', 'xor %k0, %k0; ' + t + '; set' + cc + ' %b0',
           '=&r;' + cons_in, 'rcx')
for (b, r) in ((8, 'u'), (16, 'u'), (32, 'u'), (64, 'L')):
    op('crc32_%d' % b, 'sse4.2', 'ss', r, 'c', 'crc32 %0, %2', '=r;0;r')
# AES, PCLMULQDQ, SHA
for m in ('aesenc', 'aesenclast', 'aesdec', 'aesdeclast'):
    op(m, 'aes', 'VV', 'a', '2')
op('aesimc', 'aes', 'v', 'a', '1')
op('aeskeygenassist', 'aes', 'vi', 'a', '1i')
op('pclmulqdq', 'pclmul', 'VVi', 'a', '2i')
for m in ('sha1nexte', 'sha1msg1', 'sha1msg2', 'sha256msg1', 'sha256msg2'):
    op(m, 'sha', 'VV', 'a', 'l2')
op('sha1rnds4', 'sha', 'VVi', 'a', 'l2i')
op('sha256rnds2', 'sha', 'VVV', 'a', 'c', 'sha256rnds2 %0, %2, %3', '=x;0;x;Yz')
# AVX, AVX2
for m in ('vpermilps', 'vpermilpd'):
    op(m, 'avx', 'VV', 'a', 'v2')
    op(m + '_i', 'avx', 'vi', 'a', 'v1i', m)
for m in ('vperm2f128',):
    op(m, 'avx', 'VVi', 'a', 'v2i')
op('vperm2i128', 'avx2', 'VVi', 'a', 'v2i')
op('vpermq', 'avx2', 'vi', 'a', 'v1i')
op('vpermpd', 'avx2', 'vi', 'a', 'v1i')
op('vpermd', 'avx2', 'VV', 'a', 'c', 'vpermd %0, %2, %1', '=x;x;x')
op('vpermps', 'avx2', 'VV', 'a', 'c', 'vpermps %0, %2, %1', '=x;x;x')
for (m, e) in (('vmaskmovps', 'avx'), ('vmaskmovpd', 'avx'), ('vpmaskmovd', 'avx2'), ('vpmaskmovq', 'avx2')):
    op(m + '_ld', e, 'pv', 'b', 'c', m + ' %0, %2, [%1]', '=x;r;x', '', 'M')
    op(m + '_st', e, 'pvv', 'v', 'c', m + ' [%0], %1, %2', 'r;x;x', '', 'VM')
op('vzeroupper', 'avx', '', 'v', 'c', 'vzeroupper', '', ';'.join('xmm%d' % k for k in range(16)), 'V')
op('vzeroall', 'avx', '', 'v', 'c', 'vzeroall', '', ';'.join('xmm%d' % k for k in range(16)), 'V')
for (m, e) in (('vfmaddsub213ps', 'fma'), ('vfmaddsub213pd', 'fma'), ('vfmsubadd213ps', 'fma'),
               ('vfmsubadd213pd', 'fma')):
    op(m, e, 'VVV', 'a', 't3')
for n in ('fmadd', 'fmsub', 'fnmadd', 'fnmsub'):
    for t in ('ps', 'pd', 'ss', 'sd'):
        if n != 'fmadd' or t in ('ss', 'sd'):
            op('v%s213%s' % (n, t), 'fma', 'VVV', 'a', 't3')
op('vcvtph2ps', 'f16c', 'v', 'a', 'v1')
op('vcvtps2ph', 'f16c', 'vi', 'a', 'v1i')
# (a ymm result of an xmm operand, and the reverse)
op('vcvtph2ps256', 'f16c', 'v', '2', 'v1', 'vcvtph2ps')
op('vcvtps2ph256', 'f16c', 'vi', 'h', 'v1i', 'vcvtps2ph')
# gathers: dst (tied to the source), base, index, mask (copied to xmm15 or ymm15:
# the instruction clears it), scale; by the mask's width (x: 128 bits, y: 256)
for m in ('vpgatherdd', 'vpgatherdq', 'vpgatherqd', 'vpgatherqq', 'vgatherdps', 'vgatherdpd', 'vgatherqps',
          'vgatherqpd'):
    for (sfx, r) in (('x', 'xmm15'), ('y', 'ymm15')):
        op(m + sfx, 'avx2', 'vpvvi', 'a', 'c', 'vmovdqu %s, %%4; %s %%0, [%%2 + %%3*%%5], %s' % (r, m, r),
           '=&x;0;r;x;x;i', 'xmm15', 'M')
# BMI1, BMI2 (general registers)
op('bextr32', 'bmi', 'ss', 'u', 'c', 'bextr %k0, %k1, %k2', '=r;r;r')
op('bextr64', 'bmi', 'ss', 'L', 'c', 'bextr %q0, %q1, %q2', '=r;r;r')
for (m, e) in (('bzhi', 'bmi2'), ('pdep', 'bmi2'), ('pext', 'bmi2')):
    op(m + '32', e, 'ss', 'u', 'c', m + ' %k0, %k1, %k2', '=r;r;r')
    op(m + '64', e, 'ss', 'L', 'c', m + ' %q0, %q1, %q2', '=r;r;r')
# RDRAND, RDSEED: the value stored at the pointer, the result the carry flag
for (m, e) in (('rdrand', '-'), ('rdseed', '-')):
    for (b, r, w) in ((16, 'cx', 'word'), (32, 'ecx', 'dword'), (64, 'rcx', 'qword')):
        op('%s%d' % (m, b), e, 'p', 'i', 'c', '%s %s; mov %s ptr [%%1], %s; setc %%b0; movzx %%k0, %%b0' % (m, r, w, r),
           '=a;r', 'rcx', 'VM')


# -----------------------------------------------------------------------------
# <xmmintrin.h>: SSE
# -----------------------------------------------------------------------------
def scalar_ops(T, sx, el, ext, w):
    """the scalar (lane 0) forms: _ss, _sd"""
    for (o, c) in (('add', '+'), ('sub', '-'), ('mul', '*'), ('div', '/')):
        fn('_mm_%s_%s' % (o, sx), T, '%s a, %s b' % (T, T), '{ %s r = a; r[0] = a[0] %s b[0]; return r; }' % (T, c), ext)
    fn('_mm_sqrt_%s' % sx, T, '%s a' % T, 'S("sqrt%s", a, a)' % sx, ext) if sx == 'ss' else \
        fn('_mm_sqrt_sd', T, '%s a, %s b' % (T, T), 'S("sqrtsd", a, b)', ext)
    fn('_mm_min_%s' % sx, T, '%s a, %s b' % (T, T), 'S("min%s", a, b)' % sx, ext)
    fn('_mm_max_%s' % sx, T, '%s a, %s b' % (T, T), 'S("max%s", a, b)' % sx, ext)
    # compares: the eight of cmpss's two-operand forms (lane 0), the others' by swapping
    preds = [('eq', 0, False), ('lt', 1, False), ('le', 2, False), ('gt', 1, True), ('ge', 2, True),
             ('unord', 3, False), ('neq', 4, False), ('nlt', 5, False), ('nle', 6, False), ('ngt', 5, True),
             ('nge', 6, True), ('ord', 7, False)]
    pk = 'ps' if sx == 'ss' else 'pd'
    for (n, imm, sw) in preds:
        if sw:
            fn('_mm_cmp%s_%s' % (n, sx), T, '%s a, %s b' % (T, T), 'SV(a, S("cmp%s", b, a, %d), %s)' % (
                sx, imm, '%d, 1, 2, 3' % w if sx == 'ss' else '%d, 1' % w), ext)
        else:
            fn('_mm_cmp%s_%s' % (n, sx), T, '%s a, %s b' % (T, T), 'S("cmp%s", a, b, %d)' % (sx, imm), ext)
        if sw:
            fn('_mm_cmp%s_%s' % (n, pk), T, '%s a, %s b' % (T, T), 'S("cmp%s", b, a, %d)' % (pk, imm), ext)
        else:
            fn('_mm_cmp%s_%s' % (n, pk), T, '%s a, %s b' % (T, T), 'S("cmp%s", a, b, %d)' % (pk, imm), ext)
    # comi (signaling) and ucomi (quiet) of lane 0: 1 where the relation holds (neq: or unordered)
    for (n, c, q) in (('eq', '%s == %s', '%s == %s'), ('lt', '%s < %s', '__builtin_axiom_isless(%s, %s)'),
                      ('le', '%s <= %s', '__builtin_axiom_islessequal(%s, %s)'),
                      ('gt', '%s > %s', '__builtin_axiom_isgreater(%s, %s)'),
                      ('ge', '%s >= %s', '__builtin_axiom_isgreaterequal(%s, %s)'),
                      ('neq', '%s != %s', '%s != %s')):
        fn('_mm_comi%s_%s' % (n, sx), 'int', '%s a, %s b' % (T, T), '(%s)' % (c % ('a[0]', 'b[0]')), ext)
        fn('_mm_ucomi%s_%s' % (n, sx), 'int', '%s a, %s b' % (T, T), '(%s)' % (q % ('a[0]', 'b[0]')), ext)


hdr('mmintrin.h', 'sse2')
# MMX: computed in the low halves of SSE registers (the MMX registers are not used: _mm_empty does nothing)
MMX_ALIAS = {'add_pi8': 'paddb', 'add_pi16': 'paddw', 'add_pi32': 'paddd', 'adds_pi8': 'paddsb',
             'adds_pi16': 'paddsw', 'adds_pu8': 'paddusb', 'adds_pu16': 'paddusw', 'sub_pi8': 'psubb',
             'sub_pi16': 'psubw', 'sub_pi32': 'psubd', 'subs_pi8': 'psubsb', 'subs_pi16': 'psubsw',
             'subs_pu8': 'psubusb', 'subs_pu16': 'psubusw', 'madd_pi16': 'pmaddwd', 'mulhi_pi16': 'pmulhw',
             'mullo_pi16': 'pmullw', 'and_si64': 'pand', 'andnot_si64': 'pandn', 'or_si64': 'por',
             'xor_si64': 'pxor', 'cmpeq_pi8': 'pcmpeqb', 'cmpeq_pi16': 'pcmpeqw', 'cmpeq_pi32': 'pcmpeqd',
             'cmpgt_pi8': 'pcmpgtb', 'cmpgt_pi16': 'pcmpgtw', 'cmpgt_pi32': 'pcmpgtd',
             'packs_pi16': 'packsswb', 'packs_pi32': 'packssdw', 'packs_pu16': 'packuswb',
             'unpackhi_pi8': 'punpckhbw', 'unpackhi_pi16': 'punpckhwd', 'unpackhi_pi32': 'punpckhdq',
             'unpacklo_pi8': 'punpcklbw', 'unpacklo_pi16': 'punpcklwd', 'unpacklo_pi32': 'punpckldq',
             'sll_pi16': 'psllw', 'sll_pi32': 'pslld', 'sll_si64': 'psllq', 'slli_pi16': 'psllwi',
             'slli_pi32': 'pslldi', 'slli_si64': 'psllqi', 'srl_pi16': 'psrlw', 'srl_pi32': 'psrld',
             'srl_si64': 'psrlq', 'srli_pi16': 'psrlwi', 'srli_pi32': 'psrldi', 'srli_si64': 'psrlqi',
             'sra_pi16': 'psraw', 'sra_pi32': 'psrad', 'srai_pi16': 'psrawi', 'srai_pi32': 'psradi',
             'cvtsi32_si64': 'from_int', 'cvtsi64_si32': 'to_int', 'cvtsi64_m64': 'from_int64',
             'cvtm64_si64': 'to_int64', 'empty': 'empty'}
fn('_mm_empty', 'void', 'void', '{ }')
fn('_mm_cvtsi32_si64', 'm64', 'int a', 'm64((i64)(unsigned int)a)')
fn('_mm_cvtsi64_si32', 'int', 'm64 a', '(int)i32x2(a)[0]')
fn('_mm_cvtsi64_m64', 'm64', 'i64 a', 'm64(a)')
fn('_mm_cvtm64_si64', 'i64', 'm64 a', '(i64)(a)')
fn('_mm_setzero_si64', 'm64', 'void', 'm64((i64)0)')
fn('_mm_set_pi32', 'm64', 'int a, int b', '{ i32x2 r = {0, 0}; r[0] = b; r[1] = a; return m64(r); }')
fn('_mm_set_pi16', 'm64', 'short a, short b, short c, short d',
   '{ i16x4 r = {0, 0, 0, 0}; r[0] = d; r[1] = c; r[2] = b; r[3] = a; return m64(r); }')
fn('_mm_set_pi8', 'm64', 'char a, char b, char c, char d, char e, char i, char k, char m',
   '{ i8x8 r = {0, 0, 0, 0, 0, 0, 0, 0}; r[0] = m; r[1] = k; r[2] = i; r[3] = e; r[4] = d; r[5] = c; r[6] = b; r[7] = a; return m64(r); }')
fn('_mm_setr_pi32', 'm64', 'int a, int b', '_mm_set_pi32(b, a)')
fn('_mm_setr_pi16', 'm64', 'short a, short b, short c, short d', '_mm_set_pi16(d, c, b, a)')
fn('_mm_setr_pi8', 'm64', 'char a, char b, char c, char d, char e, char i, char k, char m',
   '_mm_set_pi8(m, k, i, e, d, c, b, a)')
fn('_mm_set1_pi32', 'm64', 'int a', '_mm_set_pi32(a, a)')
fn('_mm_set1_pi16', 'm64', 'short a', '_mm_set_pi16(a, a, a, a)')
fn('_mm_set1_pi8', 'm64', 'char a', '_mm_set_pi8(a, a, a, a, a, a, a, a)')
# (each computed by its 128-bit form on the operands widened: A and B, the
# result's low 64 bits kept)
MMX128 = {
    'add_pi8': 'i8x16(A) + i8x16(B)', 'add_pi16': 'i16x8(A) + i16x8(B)', 'add_pi32': 'i32x4(A) + i32x4(B)',
    'adds_pi8': 'S("adds", i8x16(A), i8x16(B))', 'adds_pi16': 'S("adds", i16x8(A), i16x8(B))',
    'adds_pu8': 'S("addus", u8x16(A), u8x16(B))', 'adds_pu16': 'S("addus", u16x8(A), u16x8(B))',
    'sub_pi8': 'i8x16(A) - i8x16(B)', 'sub_pi16': 'i16x8(A) - i16x8(B)', 'sub_pi32': 'i32x4(A) - i32x4(B)',
    'subs_pi8': 'S("subs", i8x16(A), i8x16(B))', 'subs_pi16': 'S("subs", i16x8(A), i16x8(B))',
    'subs_pu8': 'S("subus", u8x16(A), u8x16(B))', 'subs_pu16': 'S("subus", u16x8(A), u16x8(B))',
    'madd_pi16': 'S("madd16", i16x8(A), i16x8(B))', 'mulhi_pi16': 'S("pmulhw", i16x8(A), i16x8(B))',
    'mullo_pi16': 'i16x8(A) * i16x8(B)', 'and_si64': 'i64x2(A) & i64x2(B)', 'andnot_si64': '~i64x2(A) & i64x2(B)',
    'or_si64': 'i64x2(A) | i64x2(B)', 'xor_si64': 'i64x2(A) ^ i64x2(B)',
    'cmpeq_pi8': 'i8x16(A) == i8x16(B)', 'cmpeq_pi16': 'i16x8(A) == i16x8(B)', 'cmpeq_pi32': 'i32x4(A) == i32x4(B)',
    'cmpgt_pi8': 'i8x16(A) > i8x16(B)', 'cmpgt_pi16': 'i16x8(A) > i16x8(B)', 'cmpgt_pi32': 'i32x4(A) > i32x4(B)',
    'packs_pi16': 'S("packsswb", i16x8(AB), i16x8(AB))', 'packs_pi32': 'S("packssdw", i32x4(AB), i32x4(AB))',
    'packs_pu16': 'S("packuswb", i16x8(AB), i16x8(AB))',
    'unpackhi_pi8': 'SV(i8x16(AB), i8x16(AB), 4, 12, 5, 13, 6, 14, 7, 15, 0, 0, 0, 0, 0, 0, 0, 0)',
    'unpackhi_pi16': 'SV(i16x8(AB), i16x8(AB), 2, 6, 3, 7, 0, 0, 0, 0)',
    'unpackhi_pi32': 'SV(i32x4(AB), i32x4(AB), 1, 3, 0, 0)',
    'unpacklo_pi8': 'SV(i8x16(AB), i8x16(AB), 0, 8, 1, 9, 2, 10, 3, 11, 0, 0, 0, 0, 0, 0, 0, 0)',
    'unpacklo_pi16': 'SV(i16x8(AB), i16x8(AB), 0, 4, 1, 5, 0, 0, 0, 0)',
    'unpacklo_pi32': 'SV(i32x4(AB), i32x4(AB), 0, 2, 0, 0)',
    'sll_pi16': 'S("psllw", i16x8(A), i64x2(B))', 'sll_pi32': 'S("pslld", i32x4(A), i64x2(B))',
    'sll_si64': 'S("psllq", i64x2(A), i64x2(B))', 'srl_pi16': 'S("psrlw", i16x8(A), i64x2(B))',
    'srl_pi32': 'S("psrld", i32x4(A), i64x2(B))', 'srl_si64': 'S("psrlq", i64x2(A), i64x2(B))',
    'sra_pi16': 'S("psraw", i16x8(A), i64x2(B))', 'sra_pi32': 'S("psrad", i32x4(A), i64x2(B))',
}
for (n, e) in MMX128.items():
    e = e.replace('AB', '__axiom_cat64(a, b)').replace('A', '__axiom_w64(a)').replace('B', '__axiom_w64(b)')
    fn('_mm_' + n, 'm64', 'm64 a, m64 b', '__axiom_n64(m128i(%s))' % e)
for (n, k, o) in (('slli_pi16', 'i16x8', 'shli'), ('slli_pi32', 'i32x4', 'shli'), ('slli_si64', 'i64x2', 'shli'),
                  ('srli_pi16', 'u16x8', 'srli'), ('srli_pi32', 'u32x4', 'srli'), ('srli_si64', 'u64x2', 'srli'),
                  ('srai_pi16', 'i16x8', 'srai'), ('srai_pi32', 'i32x4', 'srai')):
    fn('_mm_' + n, 'm64', 'm64 a, int i', '__axiom_n64(m128i(S("%s", %s(__axiom_w64(a)), i)))' % (o, k))
for (n, a) in MMX_ALIAS.items():
    alias('_m_' + a, '_mm_' + n)

hdr('xmmintrin.h', 'sse2')
float_ops(128, 'sse2', 'f32')
scalar_ops('m128', 'ss', 'float', 'sse2', 4)
EXTRA['xmmintrin.h'] = '''#define _MM_SHUFFLE(z, y, x, w) (((z) << 6) | ((y) << 4) | ((x) << 2) | (w))
#define _MM_HINT_NTA 0
#define _MM_HINT_T2 1
#define _MM_HINT_T1 2
#define _MM_HINT_T0 3
#define _MM_HINT_ET0 7
#define _MM_HINT_ET1 6
#define _MM_EXCEPT_INVALID 0x0001u
#define _MM_EXCEPT_DENORM 0x0002u
#define _MM_EXCEPT_DIV_ZERO 0x0004u
#define _MM_EXCEPT_OVERFLOW 0x0008u
#define _MM_EXCEPT_UNDERFLOW 0x0010u
#define _MM_EXCEPT_INEXACT 0x0020u
#define _MM_EXCEPT_MASK 0x003fu
#define _MM_MASK_INVALID 0x0080u
#define _MM_MASK_DENORM 0x0100u
#define _MM_MASK_DIV_ZERO 0x0200u
#define _MM_MASK_OVERFLOW 0x0400u
#define _MM_MASK_UNDERFLOW 0x0800u
#define _MM_MASK_INEXACT 0x1000u
#define _MM_MASK_MASK 0x1f80u
#define _MM_ROUND_NEAREST 0x0000u
#define _MM_ROUND_DOWN 0x2000u
#define _MM_ROUND_UP 0x4000u
#define _MM_ROUND_TOWARD_ZERO 0x6000u
#define _MM_ROUND_MASK 0x6000u
#define _MM_FLUSH_ZERO_MASK 0x8000u
#define _MM_FLUSH_ZERO_ON 0x8000u
#define _MM_FLUSH_ZERO_OFF 0x0000u
#define _MM_GET_EXCEPTION_STATE() (_mm_getcsr() & _MM_EXCEPT_MASK)
#define _MM_GET_EXCEPTION_MASK() (_mm_getcsr() & _MM_MASK_MASK)
#define _MM_GET_ROUNDING_MODE() (_mm_getcsr() & _MM_ROUND_MASK)
#define _MM_GET_FLUSH_ZERO_MODE() (_mm_getcsr() & _MM_FLUSH_ZERO_MASK)
#define _MM_SET_EXCEPTION_STATE(x) _mm_setcsr((_mm_getcsr() & ~_MM_EXCEPT_MASK) | (x))
#define _MM_SET_EXCEPTION_MASK(x) _mm_setcsr((_mm_getcsr() & ~_MM_MASK_MASK) | (x))
#define _MM_SET_ROUNDING_MODE(x) _mm_setcsr((_mm_getcsr() & ~_MM_ROUND_MASK) | (x))
#define _MM_SET_FLUSH_ZERO_MODE(x) _mm_setcsr((_mm_getcsr() & ~_MM_FLUSH_ZERO_MASK) | (x))
#define _MM_TRANSPOSE4_PS(r0, r1, r2, r3)                     \\
    do {                                                      \\
        __m128 __axiom_t0 = _mm_unpacklo_ps((r0), (r1));      \\
        __m128 __axiom_t1 = _mm_unpacklo_ps((r2), (r3));      \\
        __m128 __axiom_t2 = _mm_unpackhi_ps((r0), (r1));      \\
        __m128 __axiom_t3 = _mm_unpackhi_ps((r2), (r3));      \\
        (r0) = _mm_movelh_ps(__axiom_t0, __axiom_t1);         \\
        (r1) = _mm_movehl_ps(__axiom_t1, __axiom_t0);         \\
        (r2) = _mm_movelh_ps(__axiom_t2, __axiom_t3);         \\
        (r3) = _mm_movehl_ps(__axiom_t3, __axiom_t2);         \\
    } while (0)
'''
EXTRA['emmintrin.h'] = '#define _MM_SHUFFLE2(x, y) (((x) << 1) | (y))\n'
fn('_mm_rcp_ps', 'm128', 'm128 a', 'S("rcpps", a)')
fn('_mm_rsqrt_ps', 'm128', 'm128 a', 'S("rsqrtps", a)')
fn('_mm_rcp_ss', 'm128', 'm128 a', 'S("rcpss", a, a)')
fn('_mm_rsqrt_ss', 'm128', 'm128 a', 'S("rsqrtss", a, a)')
fn('_mm_cvtss_si32', 'int', 'm128 a', 'S("cvtss2si", a)')
alias('_mm_cvt_ss2si', '_mm_cvtss_si32')
fn('_mm_cvttss_si32', 'int', 'm128 a', 'S("cvttss2si", a)')
alias('_mm_cvtt_ss2si', '_mm_cvttss_si32')
fn('_mm_cvtss_si64', 'i64', 'm128 a', 'S("cvtss2si64", a)')
fn('_mm_cvttss_si64', 'i64', 'm128 a', 'S("cvttss2si64", a)')
fn('_mm_cvtsi32_ss', 'm128', 'm128 a, int b', '{ m128 r = a; r[0] = (float)b; return r; }')
alias('_mm_cvt_si2ss', '_mm_cvtsi32_ss')
fn('_mm_cvtsi64_ss', 'm128', 'm128 a, i64 b', '{ m128 r = a; r[0] = (float)b; return r; }')
fn('_mm_cvtss_f32', 'float', 'm128 a', 'a[0]')
# with __m64 operands (through SSE2's conversions of the low lanes)
fn('_mm_cvtpi32_ps', 'm128', 'm128 a, m64 b',
   'SV(a, CV(i32x4(__axiom_w64(b)), f32x4), 4, 5, 2, 3)')
alias('_mm_cvt_pi2ps', '_mm_cvtpi32_ps')
fn('_mm_cvtpi16_ps', 'm128', 'm64 a', 'CV(SV(i16x4(a), i16x4(a), 0, 1, 2, 3), f32x4)'.replace('SV(i16x4(a), i16x4(a), 0, 1, 2, 3)', '__axiom_i16x4_i32(a)'))
fn('_mm_cvtpu16_ps', 'm128', 'm64 a', 'CV(__axiom_u16x4_i32(a), f32x4)')
fn('_mm_cvtpi8_ps', 'm128', 'm64 a', 'CV(__axiom_i8x4_i32(a), f32x4)')
fn('_mm_cvtpu8_ps', 'm128', 'm64 a', 'CV(__axiom_u8x4_i32(a), f32x4)')
fn('_mm_cvtpi32x2_ps', 'm128', 'm64 a, m64 b', 'CV(i32x4(__axiom_cat64(a, b)), f32x4)')
fn('_mm_cvtps_pi32', 'm64', 'm128 a', '__axiom_n64(m128i(S("cvtps2dq", a)))')
alias('_mm_cvt_ps2pi', '_mm_cvtps_pi32')
fn('_mm_cvttps_pi32', 'm64', 'm128 a', '__axiom_n64(m128i(S("cvttps2dq", a)))')
alias('_mm_cvtt_ps2pi', '_mm_cvttps_pi32')
fn('_mm_cvtps_pi16', 'm64', 'm128 a', '{ i32x4 v = i32x4(S("cvtps2dq", a)); return __axiom_n64(m128i(S("packssdw", v, v))); }')
fn('_mm_cvtps_pi8', 'm64', 'm128 a',
   '{ i32x4 v = i32x4(S("cvtps2dq", a)); i32x4 z = {0, 0, 0, 0}; i16x8 h = i16x8(S("packssdw", v, z)); return __axiom_n64(m128i(S("packsswb", h, i16x8(z)))); }')
# set, load, store
fn('_mm_setzero_ps', 'm128', 'void', '{ m128 r = {0.0f, 0.0f, 0.0f, 0.0f}; return r; }')
fn('_mm_undefined_ps', 'm128', 'void', '_mm_setzero_ps()')
fn('_mm_set_ss', 'm128', 'float a', '{ m128 r = {0.0f, 0.0f, 0.0f, 0.0f}; r[0] = a; return r; }')
fn('_mm_set1_ps', 'm128', 'float a', 'S("splat", _mm_setzero_ps(), a)')
alias('_mm_set_ps1', '_mm_set1_ps')
fn('_mm_set_ps', 'm128', 'float a, float b, float c, float d', '{ m128 r = {0.0f, 0.0f, 0.0f, 0.0f}; r[0] = d; r[1] = c; r[2] = b; r[3] = a; return r; }')
fn('_mm_setr_ps', 'm128', 'float a, float b, float c, float d', '_mm_set_ps(d, c, b, a)')
fn('_mm_load_ss', 'm128', 'const float *p', '_mm_set_ss(*p)')
fn('_mm_load1_ps', 'm128', 'const float *p', '_mm_set1_ps(*p)')
alias('_mm_load_ps1', '_mm_load1_ps')
fn('_mm_load_ps', 'm128', 'const float *p', '*(const m128 *)p')
fn('_mm_loadu_ps', 'm128', 'const float *p', '*(const __m128_u *)p')
fn('_mm_loadr_ps', 'm128', 'const float *p', '{ m128 v = *(const m128 *)p; return SV(v, v, 3, 2, 1, 0); }')
fn('_mm_loadh_pi', 'm128', 'm128 a, const m64 *p', 'SV(a, f32x4(__axiom_w64(*(const __axiom_m64u *)p)), 0, 1, 4, 5)')
fn('_mm_loadl_pi', 'm128', 'm128 a, const m64 *p', 'SV(a, f32x4(__axiom_w64(*(const __axiom_m64u *)p)), 4, 5, 2, 3)')
fn('_mm_store_ss', 'void', 'float *p, m128 a', '{ *p = a[0]; }')
fn('_mm_store1_ps', 'void', 'float *p, m128 a', '{ *(m128 *)p = SV(a, a, 0, 0, 0, 0); }')
alias('_mm_store_ps1', '_mm_store1_ps')
fn('_mm_store_ps', 'void', 'float *p, m128 a', '{ *(m128 *)p = a; }')
fn('_mm_storeu_ps', 'void', 'float *p, m128 a', '{ *(__m128_u *)p = a; }')
fn('_mm_storer_ps', 'void', 'float *p, m128 a', '{ *(m128 *)p = SV(a, a, 3, 2, 1, 0); }')
fn('_mm_storeh_pi', 'void', 'm64 *p, m128 a', '{ *(__axiom_m64u *)p = __axiom_n64(m128i(SV(a, a, 2, 3, 2, 3))); }')
fn('_mm_storel_pi', 'void', 'm64 *p, m128 a', '{ *(__axiom_m64u *)p = __axiom_n64(m128i(a)); }')
fn('_mm_stream_ps', 'void', 'void *p, m128 a', '{ *(m128 *)p = a; }')
fn('_mm_stream_pi', 'void', 'void *p, m64 a', '{ *(m64 *)p = a; }')
fn('_mm_move_ss', 'm128', 'm128 a, m128 b', 'SV(a, b, 4, 1, 2, 3)')
fn('_mm_movehl_ps', 'm128', 'm128 a, m128 b', 'SV(a, b, 6, 7, 2, 3)')
fn('_mm_movelh_ps', 'm128', 'm128 a, m128 b', 'SV(a, b, 0, 1, 4, 5)')
fn('#_mm_shuffle_ps', 'm128', 'm128 a, m128 b, int i',
   'SV(m128(a), m128(b), (i) & 3, ((i) >> 2) & 3, 4 + (((i) >> 4) & 3), 4 + (((i) >> 6) & 3))')
fn('_mm_sfence', 'void', 'void', '{ S("sfence"); }')
fn('_mm_getcsr', 'unsigned int', 'void', '{ unsigned int r; S("stmxcsr", &r); return r; }')
fn('_mm_setcsr', 'void', 'unsigned int a', '{ S("ldmxcsr", &a); }')
fn('#_mm_prefetch', 'void', 'const void *p, int i',
   '((i) == 0 ? S("prefetchnta", (const void *)(p)) : (i) == 1 ? S("prefetcht2", (const void *)(p)) : (i) == 2 ? S("prefetcht1", (const void *)(p)) : S("prefetcht0", (const void *)(p)))')
# SSE's integer operations on __m64
for (n, e, r) in (('avg_pu8', 'S("pavgb", i8x16(A), i8x16(B))', 'm64'), ('avg_pu16', 'S("pavgw", i8x16(A), i8x16(B))', 'm64'),
                  ('max_pi16', 'S("max", i16x8(A), i16x8(B))', 'm64'), ('max_pu8', 'S("max", u8x16(A), u8x16(B))', 'm64'),
                  ('min_pi16', 'S("min", i16x8(A), i16x8(B))', 'm64'), ('min_pu8', 'S("min", u8x16(A), u8x16(B))', 'm64'),
                  ('mulhi_pu16', 'S("pmulhuw", i8x16(A), i8x16(B))', 'm64'), ('sad_pu8', 'S("psadbw", i8x16(A), i8x16(B))', 'm64')):
    e = e.replace('A', '__axiom_w64(a)').replace('B', '__axiom_w64(b)')
    fn('_mm_' + n, r, 'm64 a, m64 b', '__axiom_n64(m128i(%s))' % e)
fn('_mm_movemask_pi8', 'int', 'm64 a', '(S("bitmask", i8x16(__axiom_w64(a))) & 0xff)')
fn('#_mm_extract_pi16', 'int', 'm64 a, int i', '((int)u16x4(a)[(i) & 3])')
fn('#_mm_insert_pi16', 'm64', 'm64 a, int b, int i', '__axiom_insert_pi16((a), (b), (i) & 3)')
fn('#_mm_shuffle_pi16', 'm64', 'm64 a, int i',
   'm64(SV(i16x4(a), i16x4(a), (i) & 3, ((i) >> 2) & 3, ((i) >> 4) & 3, ((i) >> 6) & 3))')
fn('_mm_maskmove_si64', 'void', 'm64 a, m64 m, char *p',
   '{ i8x8 v = i8x8(a); i8x8 k = i8x8(m); int j; for (j = 0; j < 8; j++) { if (k[j] < 0) { p[j] = (char)v[j]; } } }')
for (n, a) in (('avg_pu8', 'pavgb'), ('avg_pu16', 'pavgw'), ('max_pi16', 'pmaxsw'), ('max_pu8', 'pmaxub'),
               ('min_pi16', 'pminsw'), ('min_pu8', 'pminub'), ('mulhi_pu16', 'pmulhuw'), ('sad_pu8', 'psadbw'),
               ('movemask_pi8', 'pmovmskb'), ('extract_pi16', 'pextrw'), ('insert_pi16', 'pinsrw'),
               ('shuffle_pi16', 'pshufw'), ('maskmove_si64', 'maskmovq')):
    alias('_m_' + a, '_mm_' + n)

# -----------------------------------------------------------------------------
# <emmintrin.h>: SSE2
# -----------------------------------------------------------------------------
hdr('emmintrin.h', 'sse2')
int_ops(128, 'sse2')
float_ops(128, 'sse2', 'f64')
scalar_ops('m128d', 'sd', 'double', 'sse2', 2)
fn('_mm_add_si64', 'm64', 'm64 a, m64 b', 'm64((i64)a + (i64)b)')
fn('_mm_sub_si64', 'm64', 'm64 a, m64 b', 'm64((i64)a - (i64)b)')
fn('_mm_mul_su32', 'm64', 'm64 a, m64 b', 'm64((i64)((u64)(unsigned int)(i64)a * (unsigned int)(i64)b))')
# conversions
fn('_mm_cvtepi32_pd', 'm128d', 'm128i a', 'CV(SV(i32x4(a), i32x4(a), 0, 1), f64x2)')
fn('_mm_cvtepi32_ps', 'm128', 'm128i a', 'CV(i32x4(a), f32x4)')
fn('_mm_cvtpd_epi32', 'm128i', 'm128d a', 'm128i(S("cvtpd2dq", a))')
fn('_mm_cvttpd_epi32', 'm128i', 'm128d a', 'm128i(S("cvttpd2dq", a))')
fn('_mm_cvtpd_pi32', 'm64', 'm128d a', '__axiom_n64(m128i(S("cvtpd2dq", a)))')
fn('_mm_cvttpd_pi32', 'm64', 'm128d a', '__axiom_n64(m128i(S("cvttpd2dq", a)))')
fn('_mm_cvtpi32_pd', 'm128d', 'm64 a', 'CV(i32x2(a), f64x2)')
fn('_mm_cvtpd_ps', 'm128', 'm128d a', '{ f32x2 v = CV(a, f32x2); m128 r = {0.0f, 0.0f, 0.0f, 0.0f}; r[0] = v[0]; r[1] = v[1]; return r; }')
fn('_mm_cvtps_pd', 'm128d', 'm128 a', 'CV(SV(a, a, 0, 1), f64x2)')
fn('_mm_cvtps_epi32', 'm128i', 'm128 a', 'm128i(S("cvtps2dq", a))')
fn('_mm_cvttps_epi32', 'm128i', 'm128 a', 'm128i(S("cvttps2dq", a))')
fn('_mm_cvtsd_f64', 'double', 'm128d a', 'a[0]')
fn('_mm_cvtsd_si32', 'int', 'm128d a', 'S("cvtsd2si", a)')
fn('_mm_cvttsd_si32', 'int', 'm128d a', 'S("cvttsd2si", a)')
fn('_mm_cvtsd_si64', 'i64', 'm128d a', 'S("cvtsd2si64", a)')
fn('_mm_cvttsd_si64', 'i64', 'm128d a', 'S("cvttsd2si64", a)')
alias('_mm_cvtsd_si64x', '_mm_cvtsd_si64')
alias('_mm_cvttsd_si64x', '_mm_cvttsd_si64')
fn('_mm_cvtsd_ss', 'm128', 'm128 a, m128d b', '{ m128 r = a; r[0] = (float)b[0]; return r; }')
fn('_mm_cvtss_sd', 'm128d', 'm128d a, m128 b', '{ m128d r = a; r[0] = (double)b[0]; return r; }')
fn('_mm_cvtsi32_sd', 'm128d', 'm128d a, int b', '{ m128d r = a; r[0] = (double)b; return r; }')
fn('_mm_cvtsi64_sd', 'm128d', 'm128d a, i64 b', '{ m128d r = a; r[0] = (double)b; return r; }')
alias('_mm_cvtsi64x_sd', '_mm_cvtsi64_sd')
fn('_mm_cvtsi128_si32', 'int', 'm128i a', 'i32x4(a)[0]')
fn('_mm_cvtsi128_si64', 'i64', 'm128i a', 'i64x2(a)[0]')
alias('_mm_cvtsi128_si64x', '_mm_cvtsi128_si64')
fn('_mm_cvtsi32_si128', 'm128i', 'int a', '{ i32x4 r = {0, 0, 0, 0}; r[0] = a; return m128i(r); }')
fn('_mm_cvtsi64_si128', 'm128i', 'i64 a', '{ i64x2 r = {0, 0}; r[0] = a; return m128i(r); }')
alias('_mm_cvtsi64x_si128', '_mm_cvtsi64_si128')
# casts (the bits)
for (a, A) in (('ps', 'm128'), ('pd', 'm128d'), ('si128', 'm128i')):
    for (b, B) in (('ps', 'm128'), ('pd', 'm128d'), ('si128', 'm128i')):
        if a != b:
            fn('_mm_cast%s_%s' % (a, b), B, '%s a' % A, '%s(a)' % B)
# set
fn('_mm_setzero_si128', 'm128i', 'void', '{ i64x2 r = {0, 0}; return m128i(r); }')
fn('_mm_undefined_si128', 'm128i', 'void', '_mm_setzero_si128()')
fn('_mm_setzero_pd', 'm128d', 'void', '{ m128d r = {0.0, 0.0}; return r; }')
fn('_mm_undefined_pd', 'm128d', 'void', '_mm_setzero_pd()')
fn('_mm_set_sd', 'm128d', 'double a', '{ m128d r = {0.0, 0.0}; r[0] = a; return r; }')
fn('_mm_set1_pd', 'm128d', 'double a', '{ m128d r = {0.0, 0.0}; r[0] = a; r[1] = a; return r; }')
alias('_mm_set_pd1', '_mm_set1_pd')
fn('_mm_set_pd', 'm128d', 'double a, double b', '{ m128d r = {0.0, 0.0}; r[0] = b; r[1] = a; return r; }')
fn('_mm_setr_pd', 'm128d', 'double a, double b', '_mm_set_pd(b, a)')


def set_int(w, ext):
    I = iname(w)
    P = pre(w)
    for (k, ct) in (('i8', 'char'), ('i16', 'short'), ('i32', 'int'), ('i64', 'i64')):
        t = lt(k, w)
        n = w // int(k[1:])
        names = ['e%d' % j for j in range(n - 1, -1, -1)]
        ps = ', '.join('%s %s' % (ct, x) for x in names)
        zero = ', '.join(['0'] * n)
        body = '{ %s r = {%s}; %s return %s(r); }' % (t, zero, ' '.join('r[%d] = __%s;' % (j, 'e%d' % j) for j in range(n)), I)
        sx = isfx(k) + ('x' if k == 'i64' and w == 128 else '')
        INTRIN.append((HDR, ext, P + 'set_' + sx, I, ps, body))
        if not (k == 'i64' and w == 128):
            INTRIN.append((HDR, ext, P + 'setr_' + sx, I, ps, '%sset_%s(%s)' % (P, sx, ', '.join('__' + x for x in reversed(names)))))
        INTRIN.append((HDR, ext, P + 'set1_' + sx, I, '%s a' % ct, '%s(S("splat", %s(%ssetzero_si%d()), a))' % (I, t, P, w)))


set_int(128, 'sse2')
fn('_mm_set_epi64', 'm128i', 'm64 a, m64 b', '_mm_set_epi64x((i64)a, (i64)b)')
fn('_mm_set1_epi64', 'm128i', 'm64 a', '_mm_set_epi64x((i64)a, (i64)a)')
fn('_mm_setr_epi64', 'm128i', 'm64 a, m64 b', '_mm_set_epi64x((i64)b, (i64)a)')
# loads, stores
fn('_mm_load_pd', 'm128d', 'const double *p', '*(const m128d *)p')
fn('_mm_loadu_pd', 'm128d', 'const double *p', '*(const __m128d_u *)p')
fn('_mm_load1_pd', 'm128d', 'const double *p', '_mm_set1_pd(*p)')
alias('_mm_load_pd1', '_mm_load1_pd')
fn('_mm_load_sd', 'm128d', 'const double *p', '_mm_set_sd(*p)')
fn('_mm_loadr_pd', 'm128d', 'const double *p', '{ m128d v = *(const m128d *)p; return SV(v, v, 1, 0); }')
fn('_mm_loadh_pd', 'm128d', 'm128d a, const double *p', '{ m128d r = a; r[1] = *(const __axiom_f64u *)p; return r; }')
fn('_mm_loadl_pd', 'm128d', 'm128d a, const double *p', '{ m128d r = a; r[0] = *(const __axiom_f64u *)p; return r; }')
fn('_mm_load_si128', 'm128i', 'const m128i *p', '*p')
fn('_mm_loadu_si128', 'm128i', 'const __m128i_u *p', '*p')
fn('_mm_loadl_epi64', 'm128i', 'const __m128i_u *p', '_mm_cvtsi64_si128(*(const __axiom_i64u *)p)')
fn('_mm_loadu_si64', 'm128i', 'const void *p', '_mm_cvtsi64_si128(*(const __axiom_i64u *)p)')
fn('_mm_loadu_si32', 'm128i', 'const void *p', '_mm_cvtsi32_si128(*(const __axiom_i32u *)p)')
fn('_mm_loadu_si16', 'm128i', 'const void *p', '_mm_cvtsi32_si128((unsigned short)*(const __axiom_i16u *)p)')
fn('_mm_store_pd', 'void', 'double *p, m128d a', '{ *(m128d *)p = a; }')
fn('_mm_storeu_pd', 'void', 'double *p, m128d a', '{ *(__m128d_u *)p = a; }')
fn('_mm_store1_pd', 'void', 'double *p, m128d a', '{ *(m128d *)p = SV(a, a, 0, 0); }')
alias('_mm_store_pd1', '_mm_store1_pd')
fn('_mm_store_sd', 'void', 'double *p, m128d a', '{ *(__axiom_f64u *)p = a[0]; }')
fn('_mm_storer_pd', 'void', 'double *p, m128d a', '{ *(m128d *)p = SV(a, a, 1, 0); }')
fn('_mm_storeh_pd', 'void', 'double *p, m128d a', '{ *(__axiom_f64u *)p = a[1]; }')
fn('_mm_storel_pd', 'void', 'double *p, m128d a', '{ *(__axiom_f64u *)p = a[0]; }')
fn('_mm_store_si128', 'void', 'm128i *p, m128i a', '{ *p = a; }')
fn('_mm_storeu_si128', 'void', '__m128i_u *p, m128i a', '{ *p = a; }')
fn('_mm_storel_epi64', 'void', '__m128i_u *p, m128i a', '{ *(__axiom_i64u *)p = i64x2(a)[0]; }')
fn('_mm_storeu_si64', 'void', 'void *p, m128i a', '{ *(__axiom_i64u *)p = i64x2(a)[0]; }')
fn('_mm_storeu_si32', 'void', 'void *p, m128i a', '{ *(__axiom_i32u *)p = i32x4(a)[0]; }')
fn('_mm_storeu_si16', 'void', 'void *p, m128i a', '{ *(__axiom_i16u *)p = i16x8(a)[0]; }')
fn('_mm_stream_pd', 'void', 'void *p, m128d a', '{ *(m128d *)p = a; }')
fn('_mm_stream_si128', 'void', 'void *p, m128i a', '{ *(m128i *)p = a; }')
fn('_mm_stream_si32', 'void', 'void *p, int a', '{ *(int *)p = a; }')
fn('_mm_stream_si64', 'void', 'void *p, i64 a', '{ *(i64 *)p = a; }')
# (the bytes whose mask bit is set, one by one: no other byte is written)
fn('_mm_maskmoveu_si128', 'void', 'm128i a, m128i m, char *p',
   '{ i8x16 v = i8x16(a); i8x16 k = i8x16(m); int j; for (j = 0; j < 16; j++) { if (k[j] < 0) { p[j] = (char)v[j]; } } }')
fn('_mm_move_epi64', 'm128i', 'm128i a', '{ i64x2 r = {0, 0}; r[0] = i64x2(a)[0]; return m128i(r); }')
fn('_mm_move_sd', 'm128d', 'm128d a, m128d b', 'SV(a, b, 2, 1)')
fn('_mm_movepi64_pi64', 'm64', 'm128i a', '__axiom_n64(a)')
fn('_mm_movpi64_epi64', 'm128i', 'm64 a', '__axiom_w64(a)')
fn('#_mm_shuffle_pd', 'm128d', 'm128d a, m128d b, int i', 'SV(m128d(a), m128d(b), (i) & 1, 2 + (((i) >> 1) & 1))')
fn('#_mm_extract_epi16', 'int', 'm128i a, int i', '((int)(unsigned short)i16x8(a)[(i) & 7])')
fn('#_mm_insert_epi16', 'm128i', 'm128i a, int b, int i', '__axiom_insert_epi16((a), (b), (i) & 7)')
fn('_mm_clflush', 'void', 'const void *p', '{ S("clflush", p); }')
fn('_mm_lfence', 'void', 'void', '{ S("lfence"); }')
fn('_mm_mfence', 'void', 'void', '{ S("mfence"); }')
fn('_mm_pause', 'void', 'void', '{ S("pause"); }')



# -----------------------------------------------------------------------------
# <pmmintrin.h> SSE3, <tmmintrin.h> SSSE3
# -----------------------------------------------------------------------------
hdr('pmmintrin.h', 'sse3')
for w in (128,):
    for (n, m) in (('addsub_ps', 'addsubps'), ('hadd_ps', 'haddps'), ('hsub_ps', 'hsubps')):
        fn('_mm_' + n, 'm128', 'm128 a, m128 b', 'S("%s", a, b)' % m)
    for (n, m) in (('addsub_pd', 'addsubpd'), ('hadd_pd', 'haddpd'), ('hsub_pd', 'hsubpd')):
        fn('_mm_' + n, 'm128d', 'm128d a, m128d b', 'S("%s", a, b)' % m)
fn('_mm_lddqu_si128', 'm128i', 'const __m128i_u *p', '*p')
fn('_mm_loaddup_pd', 'm128d', 'const double *p', '_mm_set1_pd(*(const __axiom_f64u *)p)')
fn('_mm_movedup_pd', 'm128d', 'm128d a', 'SV(a, a, 0, 0)')
fn('_mm_movehdup_ps', 'm128', 'm128 a', 'SV(a, a, 1, 1, 3, 3)')
fn('_mm_moveldup_ps', 'm128', 'm128 a', 'SV(a, a, 0, 0, 2, 2)')
fn('_mm_monitor', 'void', 'const void *p, unsigned int x, unsigned int y', '{ S("monitor", p, x, y); }')
fn('_mm_mwait', 'void', 'unsigned int x, unsigned int y', '{ S("mwait", x, y); }')
EXTRA['pmmintrin.h'] = '''#define _MM_DENORMALS_ZERO_MASK 0x0040u
#define _MM_DENORMALS_ZERO_ON 0x0040u
#define _MM_DENORMALS_ZERO_OFF 0x0000u
#define _MM_SET_DENORMALS_ZERO_MODE(m) _mm_setcsr((_mm_getcsr() & ~_MM_DENORMALS_ZERO_MASK) | (m))
#define _MM_GET_DENORMALS_ZERO_MODE() (_mm_getcsr() & _MM_DENORMALS_ZERO_MASK)
'''


def ssse3_ops(w, ext):
    I = iname(w)
    P = pre(w)
    for (k, s) in (('i8', 'epi8'), ('i16', 'epi16'), ('i32', 'epi32')):
        fn(P + 'abs_' + s, I, '%s a' % I, '%s(S("abs", %s(a)))' % (I, lt(k, w)), ext)
    for (n, m) in (('hadd_epi16', 'phaddw'), ('hadd_epi32', 'phaddd'), ('hadds_epi16', 'phaddsw'),
                   ('hsub_epi16', 'phsubw'), ('hsub_epi32', 'phsubd'), ('hsubs_epi16', 'phsubsw'),
                   ('maddubs_epi16', 'pmaddubsw'), ('mulhrs_epi16', 'pmulhrsw'), ('shuffle_epi8', 'pshufb'),
                   ('sign_epi8', 'psignb'), ('sign_epi16', 'psignw'), ('sign_epi32', 'psignd')):
        fn(P + n, I, '%s a, %s b' % (I, I), '%s(S("%s", %s(a), %s(b)))' % (I, m, lt('i8', w), lt('i8', w)), ext)
    fn('#' + P + 'alignr_epi8', I, '%s a, %s b, int i' % (I, I),
       '%s(S("palignr", %s(a), %s(b), (i)))' % (I, lt('i8', w), lt('i8', w)), ext)


hdr('tmmintrin.h', 'ssse3')
ssse3_ops(128, 'ssse3')
for (n, e) in (('abs_pi8', 'S("abs", i8x16(A))'), ('abs_pi16', 'S("abs", i16x8(A))'), ('abs_pi32', 'S("abs", i32x4(A))')):
    fn('_mm_' + n, 'm64', 'm64 a', '__axiom_n64(m128i(%s))' % e.replace('A', '__axiom_w64(a)'))
for (n, m) in (('hadd_pi16', 'phaddw'), ('hadd_pi32', 'phaddd'), ('hadds_pi16', 'phaddsw'), ('hsub_pi16', 'phsubw'),
               ('hsub_pi32', 'phsubd'), ('hsubs_pi16', 'phsubsw')):
    # (the pairs of a then b: one 128-bit horizontal operation of a:b by itself)
    fn('_mm_' + n, 'm64', 'm64 a, m64 b', '__axiom_n64(m128i(S("%s", i8x16(__axiom_cat64(a, b)), i8x16(__axiom_cat64(a, b)))))' % m)
for (n, m) in (('maddubs_pi16', 'pmaddubsw'), ('mulhrs_pi16', 'pmulhrsw'), ('sign_pi8', 'psignb'),
               ('sign_pi16', 'psignw'), ('sign_pi32', 'psignd')):
    fn('_mm_' + n, 'm64', 'm64 a, m64 b', '__axiom_n64(m128i(S("%s", i8x16(__axiom_w64(a)), i8x16(__axiom_w64(b)))))' % m)
# (pshufb of 8 bytes: an index's bit 7 gives 0, its bits 0-2 the byte)
fn('_mm_shuffle_pi8', 'm64', 'm64 a, m64 b',
   '__axiom_n64(m128i(S("pshufb", i8x16(__axiom_cat64(a, a)), i8x16(__axiom_w64(b)) & S("splat", i8x16(__axiom_w64(b)), (signed char)0x87))))')
fn('#_mm_alignr_pi8', 'm64', 'm64 a, m64 b, int i',
   '((i) > 15 ? _mm_setzero_si64() : __axiom_n64(_mm_srli_si128(__axiom_cat64((b), (a)), (i))))')

# -----------------------------------------------------------------------------
# <smmintrin.h> SSE4.1, <nmmintrin.h> SSE4.2 (and <popcntintrin.h>'s), <wmmintrin.h>
# -----------------------------------------------------------------------------
hdr('smmintrin.h', 'sse4.1')
EXTRA['smmintrin.h'] = '''#define _MM_FROUND_TO_NEAREST_INT 0x00
#define _MM_FROUND_TO_NEG_INF 0x01
#define _MM_FROUND_TO_POS_INF 0x02
#define _MM_FROUND_TO_ZERO 0x03
#define _MM_FROUND_CUR_DIRECTION 0x04
#define _MM_FROUND_RAISE_EXC 0x00
#define _MM_FROUND_NO_EXC 0x08
#define _MM_FROUND_NINT (_MM_FROUND_RAISE_EXC | _MM_FROUND_TO_NEAREST_INT)
#define _MM_FROUND_FLOOR (_MM_FROUND_RAISE_EXC | _MM_FROUND_TO_NEG_INF)
#define _MM_FROUND_CEIL (_MM_FROUND_RAISE_EXC | _MM_FROUND_TO_POS_INF)
#define _MM_FROUND_TRUNC (_MM_FROUND_RAISE_EXC | _MM_FROUND_TO_ZERO)
#define _MM_FROUND_RINT (_MM_FROUND_RAISE_EXC | _MM_FROUND_CUR_DIRECTION)
#define _MM_FROUND_NEARBYINT (_MM_FROUND_NO_EXC | _MM_FROUND_CUR_DIRECTION)
#define _MM_EXTRACT_FLOAT(d, x, i) ((d) = ((__axiom_f32x4)(x))[(i) & 3])
#define _MM_MK_INSERTPS_NDX(s, d, z) (((s) << 6) | ((d) << 4) | (z))
#define _MM_PICK_OUT_PS(x, i) _mm_insert_ps(_mm_setzero_ps(), (x), _MM_MK_INSERTPS_NDX((i), 0, 0x0e))
__AXIOM_FN0 __m128i __axiom_insert_epi8(__m128i __a, int __b, int __i) {
    __axiom_i8x16 __r = (__axiom_i8x16)__a;
    __r[__i] = (signed char)__b;
    return (__m128i)__r;
}
__AXIOM_FN0 __m128i __axiom_insert_epi32(__m128i __a, int __b, int __i) {
    __axiom_i32x4 __r = (__axiom_i32x4)__a;
    __r[__i] = __b;
    return (__m128i)__r;
}
__AXIOM_FN0 __m128i __axiom_insert_epi64(__m128i __a, __axiom_int64 __b, int __i) {
    __axiom_i64x2 __r = (__axiom_i64x2)__a;
    __r[__i] = __b;
    return (__m128i)__r;
}
'''


def blend_idx(n, i_bits, other):
    """the lanes of a blend by an immediate: lane j from b where bit j of i is set"""
    return ', '.join('(((i) >> %d) & 1) ? %d : %d' % (j % i_bits, n + j, j) for j in range(n))


def sse41_ops(w, ext):
    I = iname(w)
    P = pre(w)
    n = w // 16
    fn('#' + P + 'blend_epi16', I, '%s a, %s b, int i' % (I, I),
       '%s(SV(%s(a), %s(b), %s))' % (I, lt('i16', w), lt('i16', w), blend_idx(n, 8, 0)), ext)
    fn(P + 'blendv_epi8', I, '%s a, %s b, %s m' % (I, I, I),
       '%s(%s(m) < S("splat", %s(m), 0) ? %s(b) : %s(a))' % (I, lt('i8', w), lt('i8', w), lt('i8', w), lt('i8', w)), ext)
    fn(P + 'cmpeq_epi64', I, '%s a, %s b' % (I, I), '%s(%s(a) == %s(b))' % (I, lt('i64', w), lt('i64', w)), ext)
    for (k, mx) in (('i8', 0), ('i32', 0), ('u16', 0), ('u32', 0)):
        t = lt(k, w)
        fn(P + 'max_' + isfx(k), I, '%s a, %s b' % (I, I), '%s(S("max", %s(a), %s(b)))' % (I, t, t), ext)
        fn(P + 'min_' + isfx(k), I, '%s a, %s b' % (I, I), '%s(S("min", %s(a), %s(b)))' % (I, t, t), ext)
    fn(P + 'mul_epi32', I, '%s a, %s b' % (I, I), '%s(S("pmuldq", %s(a), %s(b)))' % (I, lt('i8', w), lt('i8', w)), ext)
    fn(P + 'mullo_epi32', I, '%s a, %s b' % (I, I), '%s(%s(a) * %s(b))' % (I, lt('i32', w), lt('i32', w)), ext)
    fn(P + 'packus_epi32', I, '%s a, %s b' % (I, I), '%s(S("packusdw", %s(a), %s(b)))' % (I, lt('i8', w), lt('i8', w)), ext)
    fn('#' + P + 'mpsadbw_epu8', I, '%s a, %s b, int i' % (I, I),
       '%s(S("mpsadbw", %s(a), %s(b), (i)))' % (I, lt('i8', w), lt('i8', w)), ext)
    # widening conversions (of the low lanes of a 128-bit operand)
    src = 'm128i'
    for (f, t, x) in (('epi8', 'epi16', 'bw'), ('epi8', 'epi32', 'bd'), ('epi8', 'epi64', 'bq'), ('epi16', 'epi32', 'wd'),
                      ('epi16', 'epi64', 'wq'), ('epi32', 'epi64', 'dq')):
        for (sg, sfx) in (('sx', f), ('zx', f.replace('epi', 'epu'))):
            o = 'pmov%s%s' % (sg, x) + ('' if w == 128 else '256')
            fn(P + 'cvt%s_%s' % (sfx, t), I, '%s a' % src, '%s(S("%s", i8x16(a)))' % (I, o), ext)


sse41_ops(128, 'sse4.1')
fn('#_mm_blend_pd', 'm128d', 'm128d a, m128d b, int i', 'SV(m128d(a), m128d(b), %s)' % blend_idx(2, 2, 0))
fn('#_mm_blend_ps', 'm128', 'm128 a, m128 b, int i', 'SV(m128(a), m128(b), %s)' % blend_idx(4, 4, 0))
fn('_mm_blendv_pd', 'm128d', 'm128d a, m128d b, m128d m', 'm128d(i64x2(m) < S("splat", i64x2(m), 0) ? i64x2(b) : i64x2(a))')
fn('_mm_blendv_ps', 'm128', 'm128 a, m128 b, m128 m', 'm128(i32x4(m) < S("splat", i32x4(m), 0) ? i32x4(b) : i32x4(a))')
for (t, T, sx) in (('ps', 'm128', 'ss'), ('pd', 'm128d', 'sd')):
    fn('_mm_ceil_' + t, T, '%s a' % T, 'S("ceil", a)')
    fn('_mm_floor_' + t, T, '%s a' % T, 'S("floor", a)')
    fn('#_mm_round_' + t, T, '%s a, int i' % T, 'S("round%s", %s(a), (i))' % (t, T))
    fn('_mm_ceil_' + sx, T, '%s a, %s b' % (T, T), 'S("round%s", a, b, 2)' % sx)
    fn('_mm_floor_' + sx, T, '%s a, %s b' % (T, T), 'S("round%s", a, b, 1)' % sx)
    fn('#_mm_round_' + sx, T, '%s a, %s b, int i' % (T, T), 'S("round%s", %s(a), %s(b), (i))' % (sx, T, T))
fn('#_mm_dp_ps', 'm128', 'm128 a, m128 b, int i', 'S("dpps", m128(a), m128(b), (i))')
fn('#_mm_dp_pd', 'm128d', 'm128d a, m128d b, int i', 'S("dppd", m128d(a), m128d(b), (i))')
fn('#_mm_extract_epi8', 'int', 'm128i a, int i', '((int)(unsigned char)i8x16(a)[(i) & 15])')
fn('#_mm_extract_epi32', 'int', 'm128i a, int i', 'i32x4(a)[(i) & 3]')
fn('#_mm_extract_epi64', 'i64', 'm128i a, int i', 'i64x2(a)[(i) & 1]')
fn('#_mm_extract_ps', 'int', 'm128 a, int i', 'i32x4(a)[(i) & 3]')
fn('#_mm_insert_epi8', 'm128i', 'm128i a, int b, int i', '__axiom_insert_epi8((a), (b), (i) & 15)')
fn('#_mm_insert_epi32', 'm128i', 'm128i a, int b, int i', '__axiom_insert_epi32((a), (b), (i) & 3)')
fn('#_mm_insert_epi64', 'm128i', 'm128i a, i64 b, int i', '__axiom_insert_epi64((a), (b), (i) & 1)')
fn('#_mm_insert_ps', 'm128', 'm128 a, m128 b, int i', 'S("insertps", m128(a), m128(b), (i))')
fn('_mm_minpos_epu16', 'm128i', 'm128i a', 'm128i(S("phminposuw", i16x8(a)))')
fn('_mm_stream_load_si128', 'm128i', 'void *p', '*(const m128i *)p')
fn('_mm_testz_si128', 'int', 'm128i a, m128i b', 'S("ptestz", a, b)')
fn('_mm_testc_si128', 'int', 'm128i a, m128i b', 'S("ptestc", a, b)')
fn('_mm_testnzc_si128', 'int', 'm128i a, m128i b', 'S("ptestnzc", a, b)')
fn('_mm_test_all_zeros', 'int', 'm128i a, m128i m', 'S("ptestz", a, m)')
fn('_mm_test_mix_ones_zeros', 'int', 'm128i a, m128i m', 'S("ptestnzc", a, m)')
fn('_mm_test_all_ones', 'int', 'm128i a', 'S("ptestc", a, _mm_cmpeq_epi32(a, a))')

hdr('nmmintrin.h', 'sse4.2')
EXTRA['nmmintrin.h'] = '''#define _SIDD_UBYTE_OPS 0x00
#define _SIDD_UWORD_OPS 0x01
#define _SIDD_SBYTE_OPS 0x02
#define _SIDD_SWORD_OPS 0x03
#define _SIDD_CMP_EQUAL_ANY 0x00
#define _SIDD_CMP_RANGES 0x04
#define _SIDD_CMP_EQUAL_EACH 0x08
#define _SIDD_CMP_EQUAL_ORDERED 0x0c
#define _SIDD_POSITIVE_POLARITY 0x00
#define _SIDD_NEGATIVE_POLARITY 0x10
#define _SIDD_MASKED_POSITIVE_POLARITY 0x20
#define _SIDD_MASKED_NEGATIVE_POLARITY 0x30
#define _SIDD_LEAST_SIGNIFICANT 0x00
#define _SIDD_MOST_SIGNIFICANT 0x40
#define _SIDD_BIT_MASK 0x00
#define _SIDD_UNIT_MASK 0x40
'''
fn('_mm_cmpgt_epi64', 'm128i', 'm128i a, m128i b', 'm128i(i64x2(a) > i64x2(b))')
fn('#_mm_cmpistri', 'int', 'm128i a, m128i b, int i', 'S("pcmpistri", m128i(a), m128i(b), (i))')
fn('#_mm_cmpistrm', 'm128i', 'm128i a, m128i b, int i', 'm128i(S("pcmpistrm", m128i(a), m128i(b), (i)))')
fn('#_mm_cmpestri', 'int', 'm128i a, int x, m128i b, int y, int i', 'S("pcmpestri", m128i(a), (int)(x), m128i(b), (int)(y), (i))')
fn('#_mm_cmpestrm', 'm128i', 'm128i a, int x, m128i b, int y, int i', 'm128i(S("pcmpestrm", m128i(a), (int)(x), m128i(b), (int)(y), (i)))')
for f in 'acosz':
    fn('#_mm_cmpistr' + f, 'int', 'm128i a, m128i b, int i', 'S("pcmpistr%s", m128i(a), m128i(b), (i))' % f)
    fn('#_mm_cmpestr' + f, 'int', 'm128i a, int x, m128i b, int y, int i', 'S("pcmpestr%s", m128i(a), (int)(x), m128i(b), (int)(y), (i))' % f)
fn('_mm_crc32_u8', 'unsigned int', 'unsigned int c, unsigned char x', 'S("crc32_8", c, x)')
fn('_mm_crc32_u16', 'unsigned int', 'unsigned int c, unsigned short x', 'S("crc32_16", c, x)')
fn('_mm_crc32_u32', 'unsigned int', 'unsigned int c, unsigned int x', 'S("crc32_32", c, x)')
fn('_mm_crc32_u64', 'u64', 'u64 c, u64 x', 'S("crc32_64", c, x)')
fn('_mm_popcnt_u32', 'int', 'unsigned int a', '(int)S("popcnt", a)', 'popcnt')
fn('_mm_popcnt_u64', 'i64', 'u64 a', '(i64)S("popcnt", a)', 'popcnt')

hdr('wmmintrin.h', 'aes')
for m in ('aesenc', 'aesenclast', 'aesdec', 'aesdeclast'):
    fn('_mm_%s_si128' % m, 'm128i', 'm128i a, m128i b', 'm128i(S("%s", a, b))' % m)
fn('_mm_aesimc_si128', 'm128i', 'm128i a', 'm128i(S("aesimc", a))')
fn('#_mm_aeskeygenassist_si128', 'm128i', 'm128i a, int i', 'm128i(S("aeskeygenassist", m128i(a), (i)))')
fn('#_mm_clmulepi64_si128', 'm128i', 'm128i a, m128i b, int i', 'm128i(S("pclmulqdq", m128i(a), m128i(b), (i)))', 'pclmul')

# -----------------------------------------------------------------------------
# <immintrin.h>: AVX, AVX2, FMA, F16C, BMI1, BMI2, LZCNT, POPCNT, ADX, SHA,
# RDRAND, RDSEED, and the instructions of no vector extension
# -----------------------------------------------------------------------------
hdr('immintrin.h', 'avx')
EXTRA['immintrin.h'] = '''#define _CMP_EQ_OQ 0x00
#define _CMP_LT_OS 0x01
#define _CMP_LE_OS 0x02
#define _CMP_UNORD_Q 0x03
#define _CMP_NEQ_UQ 0x04
#define _CMP_NLT_US 0x05
#define _CMP_NLE_US 0x06
#define _CMP_ORD_Q 0x07
#define _CMP_EQ_UQ 0x08
#define _CMP_NGE_US 0x09
#define _CMP_NGT_US 0x0a
#define _CMP_FALSE_OQ 0x0b
#define _CMP_NEQ_OQ 0x0c
#define _CMP_GE_OS 0x0d
#define _CMP_GT_OS 0x0e
#define _CMP_TRUE_UQ 0x0f
#define _CMP_EQ_OS 0x10
#define _CMP_LT_OQ 0x11
#define _CMP_LE_OQ 0x12
#define _CMP_UNORD_S 0x13
#define _CMP_NEQ_US 0x14
#define _CMP_NLT_UQ 0x15
#define _CMP_NLE_UQ 0x16
#define _CMP_ORD_S 0x17
#define _CMP_EQ_US 0x18
#define _CMP_NGE_UQ 0x19
#define _CMP_NGT_UQ 0x1a
#define _CMP_FALSE_OS 0x1b
#define _CMP_NEQ_OS 0x1c
#define _CMP_GE_OQ 0x1d
#define _CMP_GT_OQ 0x1e
#define _CMP_TRUE_US 0x1f
'''
for (k, T, sx) in (('f32', 'm256', 'ps'), ('f64', 'm256d', 'pd')):
    float_ops(256, 'avx', k)
    n = 8 if k == 'f32' else 4
    fn('_mm256_addsub_' + sx, T, '%s a, %s b' % (T, T), 'S("addsub%s", a, b)' % sx)
    fn('_mm256_hadd_' + sx, T, '%s a, %s b' % (T, T), 'S("hadd%s", a, b)' % sx)
    fn('_mm256_hsub_' + sx, T, '%s a, %s b' % (T, T), 'S("hsub%s", a, b)' % sx)
    fn('#_mm256_blend_' + sx, T, '%s a, %s b, int i' % (T, T), 'SV(%s(a), %s(b), %s)' % (T, T, blend_idx(n, n, 0)))
    it = lt('i32' if k == 'f32' else 'i64', 256)
    fn('_mm256_blendv_' + sx, T, '%s a, %s b, %s m' % (T, T, T), '%s(%s(m) < S("splat", %s(m), 0) ? %s(b) : %s(a))' % (T, it, it, it, it))
    fn('_mm256_ceil_' + sx, T, '%s a' % T, 'S("ceil", a)')
    fn('_mm256_floor_' + sx, T, '%s a' % T, 'S("floor", a)')
    fn('#_mm256_round_' + sx, T, '%s a, int i' % T, 'S("round%s", %s(a), (i))' % (sx, T))
    fn('#_mm256_cmp_' + sx, T, '%s a, %s b, int i' % (T, T), 'S("cmp%s", %s(a), %s(b), (i))' % (sx, T, T))
    T1 = 'm128' if k == 'f32' else 'm128d'
    fn('#_mm_cmp_' + sx, T1, '%s a, %s b, int i' % (T1, T1), 'S("cmp%s", %s(a), %s(b), (i))' % (sx, T1, T1))
    sx1 = 'ss' if k == 'f32' else 'sd'
    fn('#_mm_cmp_' + sx1, T1, '%s a, %s b, int i' % (T1, T1), 'S("cmp%s", %s(a), %s(b), (i))' % (sx1, T1, T1))
    fn('_mm256_movedup_pd' if k == 'f64' else '_mm256_moveldup_ps', T, '%s a' % T,
       'SV(a, a, 0, 0, 2, 2)' if k == 'f64' else 'SV(a, a, 0, 0, 2, 2, 4, 4, 6, 6)')
    if k == 'f32':
        fn('_mm256_movehdup_ps', T, '%s a' % T, 'SV(a, a, 1, 1, 3, 3, 5, 5, 7, 7)')
        fn('_mm256_rcp_ps', T, '%s a' % T, 'S("rcpps", a)')
        fn('_mm256_rsqrt_ps', T, '%s a' % T, 'S("rsqrtps", a)')
        fn('#_mm256_dp_ps', T, '%s a, %s b, int i' % (T, T), 'S("dpps", m256(a), m256(b), (i))')
        fn('#_mm256_shuffle_ps', T, '%s a, %s b, int i' % (T, T),
           'SV(m256(a), m256(b), (i) & 3, ((i) >> 2) & 3, 8 + (((i) >> 4) & 3), 8 + (((i) >> 6) & 3), 4 + ((i) & 3), 4 + (((i) >> 2) & 3), 12 + (((i) >> 4) & 3), 12 + (((i) >> 6) & 3))')
    else:
        fn('#_mm256_shuffle_pd', T, '%s a, %s b, int i' % (T, T),
           'SV(m256d(a), m256d(b), (i) & 1, 4 + (((i) >> 1) & 1), 2 + (((i) >> 2) & 1), 6 + (((i) >> 3) & 1))')
    # permutes within 128-bit lanes
    if k == 'f32':
        fn('#_mm_permute_ps', 'm128', 'm128 a, int i', 'SV(m128(a), m128(a), (i) & 3, ((i) >> 2) & 3, ((i) >> 4) & 3, ((i) >> 6) & 3)')
        fn('#_mm256_permute_ps', T, '%s a, int i' % T,
           'SV(m256(a), m256(a), (i) & 3, ((i) >> 2) & 3, ((i) >> 4) & 3, ((i) >> 6) & 3, 4 + ((i) & 3), 4 + (((i) >> 2) & 3), 4 + (((i) >> 4) & 3), 4 + (((i) >> 6) & 3))')
        fn('_mm_permutevar_ps', 'm128', 'm128 a, m128i c', 'S("vpermilps", a, m128(c))')
        fn('_mm256_permutevar_ps', T, '%s a, m256i c' % T, 'S("vpermilps", a, m256(c))')
    else:
        fn('#_mm_permute_pd', 'm128d', 'm128d a, int i', 'SV(m128d(a), m128d(a), (i) & 1, ((i) >> 1) & 1)')
        fn('#_mm256_permute_pd', T, '%s a, int i' % T,
           'SV(m256d(a), m256d(a), (i) & 1, ((i) >> 1) & 1, 2 + (((i) >> 2) & 1), 2 + (((i) >> 3) & 1))')
        fn('_mm_permutevar_pd', 'm128d', 'm128d a, m128i c', 'S("vpermilpd", a, m128d(c))')
        fn('_mm256_permutevar_pd', T, '%s a, m256i c' % T, 'S("vpermilpd", a, m256d(c))')
    fn('#_mm256_permute2f128_' + sx, T, '%s a, %s b, int i' % (T, T), 'S("vperm2f128", %s(a), %s(b), (i))' % (T, T))
    # test
    for (n2, o) in (('testz', 'testz'), ('testc', 'testc'), ('testnzc', 'testnzc')):
        fn('_mm256_%s_%s' % (n2, sx), 'int', '%s a, %s b' % (T, T), 'S("vtest%s_%s", a, b)' % (sx, o))
        fn('_mm_%s_%s' % (n2, sx), 'int', '%s a, %s b' % (T1, T1), 'S("vtest%s_%s", a, b)' % (sx, o))
    # masked loads and stores
    fn('_mm256_maskload_' + sx, T, 'const %s *p, m256i m' % ('float' if k == 'f32' else 'double'),
       'S("vmaskmov%s_ld", p, %s(m))' % (sx, T))
    fn('_mm_maskload_' + sx, T1, 'const %s *p, m128i m' % ('float' if k == 'f32' else 'double'),
       'S("vmaskmov%s_ld", p, %s(m))' % (sx, T1))
    fn('_mm256_maskstore_' + sx, 'void', '%s *p, m256i m, %s a' % ('float' if k == 'f32' else 'double', T),
       '{ S("vmaskmov%s_st", p, %s(m), a); }' % (sx, T))
    fn('_mm_maskstore_' + sx, 'void', '%s *p, m128i m, %s a' % ('float' if k == 'f32' else 'double', T1),
       '{ S("vmaskmov%s_st", p, %s(m), a); }' % (sx, T1))
# broadcasts, casts, conversions, insert/extract of 128-bit halves
fn('_mm256_broadcast_ss', 'm256', 'const float *p', 'S("splat", _mm256_setzero_ps(), *p)')
fn('_mm_broadcast_ss', 'm128', 'const float *p', 'S("splat", _mm_setzero_ps(), *p)')
fn('_mm256_broadcast_sd', 'm256d', 'const double *p', 'S("splat", _mm256_setzero_pd(), *p)')
fn('_mm256_broadcast_ps', 'm256', 'const m128 *p', '{ m128 v = *(const __m128_u *)p; return SV(v, v, 0, 1, 2, 3, 0, 1, 2, 3); }')
fn('_mm256_broadcast_pd', 'm256d', 'const m128d *p', '{ m128d v = *(const __m128d_u *)p; return SV(v, v, 0, 1, 0, 1); }')
for (a, A) in (('ps', 'm256'), ('pd', 'm256d'), ('si256', 'm256i')):
    for (b, B) in (('ps', 'm256'), ('pd', 'm256d'), ('si256', 'm256i')):
        if a != b:
            fn('_mm256_cast%s_%s' % (a, b), B, '%s a' % A, '%s(a)' % B)
for (a, A, h, H, L) in (('ps', 'm256', 'ps128', 'm128', 'f32x4'), ('pd', 'm256d', 'pd128', 'm128d', 'f64x2'),
                        ('si256', 'm256i', 'si128', 'm128i', 'i64x2')):
    n = {'f32x4': 4, 'f64x2': 2, 'i64x2': 2}[L]
    full = ', '.join(map(str, range(2 * n)))
    W2 = L.replace('x%d' % n, 'x%d' % (2 * n))
    w = {'ps': 'ps', 'pd': 'pd', 'si256': 'si'}[a]
    fn('_mm256_cast%s256_%s128' % (w, w), H, '%s a' % A, '%s(SV(%s(a), %s(a), %s))' % (H, W2, W2, ', '.join(map(str, range(n)))))
    # (the upper half undefined: here a's lanes again; zext: zeros)
    fn('_mm256_cast%s128_%s256' % (w, w), A, '%s a' % H,
       '%s(SV(%s(a), %s(a), %s))' % (A, L, L, ', '.join(map(str, list(range(n)) + list(range(n))))))
    z = 'S("splat", %s(a), 0)' % L
    fn('_mm256_zext%s128_%s256' % (w, w), A, '%s a' % H, '%s(SV(%s(a), %s, %s))' % (A, L, z, full))
    # insert b as half i of a: lanes of a, but half i from b (indices 2n..: b widened)
    idx = []
    for j in range(2 * n):
        if j < n:
            idx.append('((i) & 1) ? %d : %d' % (j, 2 * n + j))
        else:
            idx.append('((i) & 1) ? %d : %d' % (2 * n + j - n, j))
    nm = {'ps': '_mm256_insertf128_ps', 'pd': '_mm256_insertf128_pd', 'si256': '_mm256_insertf128_si256'}[a]
    fn('#' + nm, A, '%s a, %s b, int i' % (A, H),
       '%s(SV(%s(a), %s(SV(%s(b), %s(b), %s)), %s))' % (A, W2, W2, L, L, ', '.join(map(str, list(range(n)) * 2)), ', '.join(idx)))
    nm = {'ps': '_mm256_extractf128_ps', 'pd': '_mm256_extractf128_pd', 'si256': '_mm256_extractf128_si256'}[a]
    fn('#' + nm, H, '%s a, int i' % A, '%s(SV(%s(a), %s(a), %s))' % (H, W2, W2, ', '.join('((i) & 1) * %d + %d' % (n, j) for j in range(n))))
    s = {'ps': 'm128', 'pd': 'm128d', 'si256': 'm128i'}[a]
    fn('_mm256_set_' + {'ps': 'm128', 'pd': 'm128d', 'si256': 'm128i'}[a], A, '%s x, %s y' % (H, H),
       '%s(SV(%s(y), %s(x), %s))' % (A, L, L, full))
    fn('_mm256_setr_' + {'ps': 'm128', 'pd': 'm128d', 'si256': 'm128i'}[a], A, '%s x, %s y' % (H, H),
       '%s(SV(%s(x), %s(y), %s))' % (A, L, L, full))
    q = {'ps': 'float', 'pd': 'double', 'si256': 'm128i'}[a]
    u = {'ps': '__m128_u', 'pd': '__m128d_u', 'si256': '__m128i_u'}[a]
    fn('_mm256_loadu2_' + {'ps': 'm128', 'pd': 'm128d', 'si256': 'm128i'}[a], A, 'const %s *p, const %s *q' % (q, q),
       '%s(SV(%s(*(const %s *)q), %s(*(const %s *)p), %s))' % (A, L, u, L, u, full))
    fn('_mm256_storeu2_' + {'ps': 'm128', 'pd': 'm128d', 'si256': 'm128i'}[a], 'void', '%s *p, %s *q, %s a' % (q, q, A),
       '{ *(%s *)q = %s(SV(%s(a), %s(a), %s)); *(%s *)p = %s(SV(%s(a), %s(a), %s)); }' % (
           u, H, W2, W2, ', '.join(map(str, range(n))), u, H, W2, W2, ', '.join(map(str, range(n, 2 * n)))))
fn('#_mm256_permute2f128_si256', 'm256i', 'm256i a, m256i b, int i', 'm256i(S("vperm2f128", m256(a), m256(b), (i)))')
fn('_mm256_cvtepi32_pd', 'm256d', 'm128i a', 'CV(i32x4(a), f64x4)')
fn('_mm256_cvtepi32_ps', 'm256', 'm256i a', 'CV(i32x8(a), f32x8)')
fn('_mm256_cvtpd_epi32', 'm128i', 'm256d a', 'm128i(S("cvtpd2dq256", a))')
fn('_mm256_cvttpd_epi32', 'm128i', 'm256d a', 'm128i(S("cvttpd2dq256", a))')
fn('_mm256_cvtpd_ps', 'm128', 'm256d a', 'CV(a, f32x4)')
fn('_mm256_cvtps_pd', 'm256d', 'm128 a', 'CV(a, f64x4)')
fn('_mm256_cvtps_epi32', 'm256i', 'm256 a', 'm256i(S("cvtps2dq", a))')
fn('_mm256_cvttps_epi32', 'm256i', 'm256 a', 'm256i(S("cvttps2dq", a))')
fn('_mm256_cvtsd_f64', 'double', 'm256d a', 'a[0]')
fn('_mm256_cvtss_f32', 'float', 'm256 a', 'a[0]')
fn('_mm256_cvtsi256_si32', 'int', 'm256i a', 'i32x8(a)[0]')
# set, load, store (256 bits)
fn('_mm256_setzero_ps', 'm256', 'void', '{ m256 r = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}; return r; }')
fn('_mm256_setzero_pd', 'm256d', 'void', '{ m256d r = {0.0, 0.0, 0.0, 0.0}; return r; }')
fn('_mm256_setzero_si256', 'm256i', 'void', '{ i64x4 r = {0, 0, 0, 0}; return m256i(r); }')
fn('_mm256_undefined_ps', 'm256', 'void', '_mm256_setzero_ps()')
fn('_mm256_undefined_pd', 'm256d', 'void', '_mm256_setzero_pd()')
fn('_mm256_undefined_si256', 'm256i', 'void', '_mm256_setzero_si256()')
fn('_mm256_set1_ps', 'm256', 'float a', 'S("splat", _mm256_setzero_ps(), a)')
fn('_mm256_set1_pd', 'm256d', 'double a', 'S("splat", _mm256_setzero_pd(), a)')
fn('_mm256_set_ps', 'm256', 'float a, float b, float c, float d, float e, float f, float g, float h',
   '{ m256 r = _mm256_setzero_ps(); r[0] = h; r[1] = g; r[2] = f; r[3] = e; r[4] = d; r[5] = c; r[6] = b; r[7] = a; return r; }')
fn('_mm256_setr_ps', 'm256', 'float a, float b, float c, float d, float e, float f, float g, float h',
   '_mm256_set_ps(h, g, f, e, d, c, b, a)')
fn('_mm256_set_pd', 'm256d', 'double a, double b, double c, double d',
   '{ m256d r = _mm256_setzero_pd(); r[0] = d; r[1] = c; r[2] = b; r[3] = a; return r; }')
fn('_mm256_setr_pd', 'm256d', 'double a, double b, double c, double d', '_mm256_set_pd(d, c, b, a)')
set_int(256, 'avx')
fn('_mm256_load_ps', 'm256', 'const float *p', '*(const m256 *)p')
fn('_mm256_loadu_ps', 'm256', 'const float *p', '*(const __m256_u *)p')
fn('_mm256_load_pd', 'm256d', 'const double *p', '*(const m256d *)p')
fn('_mm256_loadu_pd', 'm256d', 'const double *p', '*(const __m256d_u *)p')
fn('_mm256_load_si256', 'm256i', 'const m256i *p', '*p')
fn('_mm256_loadu_si256', 'm256i', 'const __m256i_u *p', '*p')
fn('_mm256_lddqu_si256', 'm256i', 'const __m256i_u *p', '*p')
fn('_mm256_store_ps', 'void', 'float *p, m256 a', '{ *(m256 *)p = a; }')
fn('_mm256_storeu_ps', 'void', 'float *p, m256 a', '{ *(__m256_u *)p = a; }')
fn('_mm256_store_pd', 'void', 'double *p, m256d a', '{ *(m256d *)p = a; }')
fn('_mm256_storeu_pd', 'void', 'double *p, m256d a', '{ *(__m256d_u *)p = a; }')
fn('_mm256_store_si256', 'void', 'm256i *p, m256i a', '{ *p = a; }')
fn('_mm256_storeu_si256', 'void', '__m256i_u *p, m256i a', '{ *p = a; }')
fn('_mm256_stream_ps', 'void', 'void *p, m256 a', '{ *(m256 *)p = a; }')
fn('_mm256_stream_pd', 'void', 'void *p, m256d a', '{ *(m256d *)p = a; }')
fn('_mm256_stream_si256', 'void', 'void *p, m256i a', '{ *(m256i *)p = a; }')
for (k, n, ct) in (('epi8', 32, 'int'), ('epi16', 16, 'int'), ('epi32', 8, 'int'), ('epi64', 4, 'i64')):
    lk = {'epi8': 'i8', 'epi16': 'i16', 'epi32': 'i32', 'epi64': 'i64'}[k]
    fn('#_mm256_extract_' + k, ct, 'm256i a, int i', '(%s%s(a)[(i) & %d])' % ('(int)(unsigned %s)' % {'i8': 'char', 'i16': 'short'}[lk] if lk in ('i8', 'i16') else '', lt(lk, 256), n - 1))
    fn('#_mm256_insert_' + k, 'm256i', 'm256i a, %s b, int i' % ct, '__axiom_insert256_%s((a), (b), (i) & %d)' % (k, n - 1))
fn('_mm256_testz_si256', 'int', 'm256i a, m256i b', 'S("ptestz", a, b)')
fn('_mm256_testc_si256', 'int', 'm256i a, m256i b', 'S("ptestc", a, b)')
fn('_mm256_testnzc_si256', 'int', 'm256i a, m256i b', 'S("ptestnzc", a, b)')
fn('_mm256_zeroupper', 'void', 'void', '{ S("vzeroupper"); }')
fn('_mm256_zeroall', 'void', 'void', '{ S("vzeroall"); }')
EXTRA['immintrin.h'] += ''.join('''__AXIOM_FN("avx") __m256i __axiom_insert256_%s(__m256i __a, %s __b, int __i) {
    __axiom_%s __r = (__axiom_%s)__a;
    __r[__i] = __b;
    return (__m256i)__r;
}
''' % (k, ct, t, t) for (k, ct, t) in (('epi8', 'int', 'i8x32'), ('epi16', 'int', 'i16x16'), ('epi32', 'int', 'i32x8'),
                                    ('epi64', '__axiom_int64', 'i64x4')))

# AVX2
hdr('immintrin.h', 'avx2')
int_ops(256, 'avx2')
ssse3_ops(256, 'avx2')
sse41_ops(256, 'avx2')
fn('_mm256_cmpgt_epi64', 'm256i', 'm256i a, m256i b', 'm256i(i64x4(a) > i64x4(b))')
fn('#_mm256_blend_epi32', 'm256i', 'm256i a, m256i b, int i', 'm256i(SV(i32x8(a), i32x8(b), %s))' % blend_idx(8, 8, 0))
fn('#_mm_blend_epi32', 'm128i', 'm128i a, m128i b, int i', 'm128i(SV(i32x4(a), i32x4(b), %s))' % blend_idx(4, 4, 0))
for (k, L1, L2, n) in (('epi8', 'i8x16', 'i8x32', 32), ('epi16', 'i16x8', 'i16x16', 16), ('epi32', 'i32x4', 'i32x8', 8),
                       ('epi64', 'i64x2', 'i64x4', 4)):
    fn('_mm256_broadcast%s_%s' % ({'epi8': 'b', 'epi16': 'w', 'epi32': 'd', 'epi64': 'q'}[k], k), 'm256i', 'm128i a',
       'm256i(SV(%s(a), %s(a), %s))' % (L1, L1, ', '.join(['0'] * n)))
    fn('_mm_broadcast%s_%s' % ({'epi8': 'b', 'epi16': 'w', 'epi32': 'd', 'epi64': 'q'}[k], k), 'm128i', 'm128i a',
       'm128i(SV(%s(a), %s(a), %s))' % (L1, L1, ', '.join(['0'] * (n // 2))))
fn('_mm256_broadcastsi128_si256', 'm256i', 'm128i a', 'm256i(SV(i64x2(a), i64x2(a), 0, 1, 0, 1))')
alias('_mm_broadcastsi128_si256', '_mm256_broadcastsi128_si256')
fn('_mm256_broadcastss_ps', 'm256', 'm128 a', 'SV(a, a, 0, 0, 0, 0, 0, 0, 0, 0)')
fn('_mm_broadcastss_ps', 'm128', 'm128 a', 'SV(a, a, 0, 0, 0, 0)')
fn('_mm256_broadcastsd_pd', 'm256d', 'm128d a', 'SV(a, a, 0, 0, 0, 0)')
fn('_mm_broadcastsd_pd', 'm128d', 'm128d a', 'SV(a, a, 0, 0)')
fn('#_mm256_permute2x128_si256', 'm256i', 'm256i a, m256i b, int i', 'm256i(S("vperm2i128", m256i(a), m256i(b), (i)))')
fn('#_mm256_permute4x64_epi64', 'm256i', 'm256i a, int i',
   'm256i(SV(i64x4(a), i64x4(a), (i) & 3, ((i) >> 2) & 3, ((i) >> 4) & 3, ((i) >> 6) & 3))')
fn('#_mm256_permute4x64_pd', 'm256d', 'm256d a, int i',
   'SV(m256d(a), m256d(a), (i) & 3, ((i) >> 2) & 3, ((i) >> 4) & 3, ((i) >> 6) & 3)')
fn('_mm256_permutevar8x32_epi32', 'm256i', 'm256i a, m256i c', 'm256i(S("vpermd", i32x8(a), i32x8(c)))')
fn('_mm256_permutevar8x32_ps', 'm256', 'm256 a, m256i c', 'S("vpermps", a, m256(c))')
fn('#_mm256_extracti128_si256', 'm128i', 'm256i a, int i', 'm128i(SV(i64x4(a), i64x4(a), ((i) & 1) * 2, ((i) & 1) * 2 + 1))')
fn('#_mm256_inserti128_si256', 'm256i', 'm256i a, m128i b, int i', '_mm256_insertf128_si256((a), (b), (i))')
fn('_mm256_stream_load_si256', 'm256i', 'const void *p', '*(const m256i *)p')
for (k, t, w) in (('epi32', 'i32', 128), ('epi64', 'i64', 128), ('epi32', 'i32', 256), ('epi64', 'i64', 256)):
    I = iname(w)
    P = pre(w)
    L = lt(t, w)
    U = lt('u' + t[1:], w)
    fn(P + 'sllv_' + k, I, '%s a, %s c' % (I, I), '%s(S("shlv", %s(a), %s(c)))' % (I, L, L))
    fn(P + 'srlv_' + k, I, '%s a, %s c' % (I, I), '%s(S("srlv", %s(a), %s(c)))' % (I, U, U))
    if k == 'epi32':
        fn(P + 'srav_' + k, I, '%s a, %s c' % (I, I), '%s(S("srav", %s(a), %s(c)))' % (I, L, L))
    ct = {'i32': 'int', 'i64': 'i64'}[t]
    fn(P + 'maskload_' + k, I, 'const %s *p, %s m' % (ct, I), '%s(S("vpmaskmov%s_ld", p, %s(m)))' % (I, 'd' if t == 'i32' else 'q', L))
    fn(P + 'maskstore_' + k, 'void', '%s *p, %s m, %s a' % (ct, I, I), '{ S("vpmaskmov%s_st", p, %s(m), %s(a)); }' % ('d' if t == 'i32' else 'q', L, L))
# gathers: (index width d/q) x (element: epi32, epi64, ps, pd) x 128/256
GATHER = [('i32gather_epi32', 'vpgatherdd', 'int', 'i32', 'm128i', 'm128i', 'm256i', 'm256i'),
          ('i32gather_epi64', 'vpgatherdq', 'i64', 'i64', 'm128i', 'm128i', 'm256i', 'm128i'),
          ('i64gather_epi32', 'vpgatherqd', 'int', 'i32', 'm128i', 'm128i', 'm128i', 'm256i'),
          ('i64gather_epi64', 'vpgatherqq', 'i64', 'i64', 'm128i', 'm128i', 'm256i', 'm256i'),
          ('i32gather_ps', 'vgatherdps', 'float', 'f32', 'm128', 'm128i', 'm256', 'm256i'),
          ('i32gather_pd', 'vgatherdpd', 'double', 'f64', 'm128d', 'm128i', 'm256d', 'm128i'),
          ('i64gather_ps', 'vgatherqps', 'float', 'f32', 'm128', 'm128i', 'm128', 'm256i'),
          ('i64gather_pd', 'vgatherqpd', 'double', 'f64', 'm128d', 'm128i', 'm256d', 'm256i')]
for (n, m, ct, el, R1, X1, R2, X2) in GATHER:
    for (P, R, X) in (('_mm_', R1, X1), ('_mm256_', R2, X2)):
        mx = 'y' if R in ('m256i', 'm256', 'm256d') else 'x'
        fn('#' + P + 'mask_' + n, R, '%s s, const %s *p, %s x, %s m, int i' % (R, ct, X, R),
           '%s(S("%s%s", %s(s), (const void *)(p), %s(x), %s(m), (i)))' % (R, m, mx, R, X, R))
        zero = {'m128i': '_mm_setzero_si128()', 'm256i': '_mm256_setzero_si256()', 'm128': '_mm_setzero_ps()',
                'm256': '_mm256_setzero_ps()', 'm128d': '_mm_setzero_pd()', 'm256d': '_mm256_setzero_pd()'}[R]
        allones = {'m128i': '_mm_set1_epi32(-1)', 'm256i': '_mm256_set1_epi32(-1)', 'm128': '_mm_castsi128_ps(_mm_set1_epi32(-1))',
                   'm256': '_mm256_castsi256_ps(_mm256_set1_epi32(-1))', 'm128d': '_mm_castsi128_pd(_mm_set1_epi32(-1))',
                   'm256d': '_mm256_castsi256_pd(_mm256_set1_epi32(-1))'}[R]
        fn('#' + P + n, R, 'const %s *p, %s x, int i' % (ct, X), '%smask_%s(%s, (p), (x), %s, (i))' % (P, n, zero, allones))

# FMA (fmadd: the IR's fma; the others its operands negated, which the
# instruction selector folds or computes with a sign flip)
hdr('immintrin.h', 'fma')
# (fmadd: the IR's fma; the others the instructions, whose NaN results keep their operands' signs)
for (P, T, sx, sc) in (('_mm_', 'm128', 'ps', 'ss'), ('_mm_', 'm128d', 'pd', 'sd'), ('_mm256_', 'm256', 'ps', None),
                       ('_mm256_', 'm256d', 'pd', None)):
    for n in ('fmadd', 'fmsub', 'fnmadd', 'fnmsub'):
        e = 'S("fma", a, b, c)' if n == 'fmadd' else 'S("v%s213%s", a, b, c)' % (n, sx)
        fn('%s%s_%s' % (P, n, sx), T, '%s a, %s b, %s c' % (T, T, T), e)
        if sc:
            fn('%s%s_%s' % (P, n, sc), T, '%s a, %s b, %s c' % (T, T, T), 'S("v%s213%s", a, b, c)' % (n, sc))
    fn('%sfmaddsub_%s' % (P, sx), T, '%s a, %s b, %s c' % (T, T, T), 'S("vfmaddsub213%s", a, b, c)' % sx)
    fn('%sfmsubadd_%s' % (P, sx), T, '%s a, %s b, %s c' % (T, T, T), 'S("vfmsubadd213%s", a, b, c)' % sx)
# F16C
hdr('immintrin.h', 'f16c')
fn('_mm_cvtph_ps', 'm128', 'm128i a', 'S("vcvtph2ps", m128(a))')
fn('_mm256_cvtph_ps', 'm256', 'm128i a', 'S("vcvtph2ps256", m128(a))')
fn('#_mm_cvtps_ph', 'm128i', 'm128 a, int i', 'm128i(S("vcvtps2ph", m128(a), (i)))')
fn('#_mm256_cvtps_ph', 'm128i', 'm256 a, int i', 'm128i(S("vcvtps2ph256", m256(a), (i)))')
fn('_cvtsh_ss', 'float', 'unsigned short a', '_mm_cvtph_ps(_mm_cvtsi32_si128(a))[0]')
fn('#_cvtss_sh', 'unsigned short', 'float a, int i', '((unsigned short)_mm_extract_epi16(_mm_cvtps_ph(_mm_set_ss(a), (i)), 0))')
# BMI1, BMI2, LZCNT, POPCNT (general registers)
hdr('immintrin.h', 'bmi')
fn('_andn_u32', 'unsigned int', 'unsigned int a, unsigned int b', '~a & b')
fn('_andn_u64', 'u64', 'u64 a, u64 b', '~a & b')
alias('__andn_u32', '_andn_u32')
alias('__andn_u64', '_andn_u64')
fn('_bextr_u32', 'unsigned int', 'unsigned int a, unsigned int b, unsigned int c', 'S("bextr32", a, (b & 0xff) | ((c & 0xff) << 8))')
fn('_bextr_u64', 'u64', 'u64 a, unsigned int b, unsigned int c', 'S("bextr64", a, (u64)((b & 0xff) | ((c & 0xff) << 8)))')
fn('_bextr2_u32', 'unsigned int', 'unsigned int a, unsigned int c', 'S("bextr32", a, c)')
fn('_bextr2_u64', 'u64', 'u64 a, u64 c', 'S("bextr64", a, c)')
alias('__bextr_u32', '_bextr2_u32')
alias('__bextr_u64', '_bextr2_u64')
for (b, t) in ((32, 'unsigned int'), (64, 'u64')):
    fn('_blsi_u%d' % b, t, '%s a' % t, 'a & (0 - a)')
    fn('_blsmsk_u%d' % b, t, '%s a' % t, 'a ^ (a - 1)')
    fn('_blsr_u%d' % b, t, '%s a' % t, 'a & (a - 1)')
    fn('_tzcnt_u%d' % b, t, '%s a' % t, 'S("tzcnt", a)')
    alias('__blsi_u%d' % b, '_blsi_u%d' % b)
    alias('__blsmsk_u%d' % b, '_blsmsk_u%d' % b)
    alias('__blsr_u%d' % b, '_blsr_u%d' % b)
    alias('__tzcnt_u%d' % b, '_tzcnt_u%d' % b)
fn('_tzcnt_u16', 'unsigned short', 'unsigned short a', '(unsigned short)(a == 0 ? 16 : S("tzcnt", (unsigned int)a))')
alias('__tzcnt_u16', '_tzcnt_u16')
fn('_mm_tzcnt_32', 'int', 'unsigned int a', '(int)S("tzcnt", a)')
fn('_mm_tzcnt_64', 'i64', 'u64 a', '(i64)S("tzcnt", a)')
hdr('immintrin.h', 'bmi2')
for (b, t) in ((32, 'unsigned int'), (64, 'u64')):
    fn('_bzhi_u%d' % b, t, '%s a, unsigned int i' % t, 'S("bzhi%d", a, (%s)i)' % (b, t))
    fn('_pdep_u%d' % b, t, '%s a, %s m' % (t, t), 'S("pdep%d", a, m)' % b)
    fn('_pext_u%d' % b, t, '%s a, %s m' % (t, t), 'S("pext%d", a, m)' % b)
fn('_mulx_u32', 'unsigned int', 'unsigned int a, unsigned int b, unsigned int *p',
   '{ u64 r = (u64)a * b; *p = (unsigned int)(r >> 32); return (unsigned int)r; }')
fn('_mulx_u64', 'u64', 'u64 a, u64 b, u64 *p', '{ *p = S("umulh", a, b); return a * b; }')
hdr('immintrin.h', 'lzcnt')
fn('_lzcnt_u32', 'unsigned int', 'unsigned int a', 'S("lzcnt", a)')
fn('_lzcnt_u64', 'u64', 'u64 a', 'S("lzcnt", a)')
alias('__lzcnt32', '_lzcnt_u32')
alias('__lzcnt64', '_lzcnt_u64')
fn('__lzcnt16', 'unsigned short', 'unsigned short a', '(unsigned short)(S("lzcnt", (unsigned int)a) - 16)')
hdr('immintrin.h', 'popcnt')
fn('_popcnt32', 'int', 'int a', '(int)S("popcnt", (unsigned int)a)')
fn('_popcnt64', 'int', 'i64 a', '(int)S("popcnt", (u64)a)')
# ADX and the carry chains (_addcarry_*, _subborrow_* need no extension: their C is any processor's)
hdr('immintrin.h', 'sse2')
for (b, t) in ((32, 'unsigned int'), (64, 'u64')):
    fn('_addcarry_u%d' % b, 'unsigned char', 'unsigned char c, %s a, %s b, %s *p' % (t, t, t),
       '{ %s s = a + b; %s r = s + (c != 0); *p = r; return (unsigned char)((s < a) | (r < s)); }' % (t, t))
    fn('_subborrow_u%d' % b, 'unsigned char', 'unsigned char c, %s a, %s b, %s *p' % (t, t, t),
       '{ %s s = a - b; %s r = s - (c != 0); *p = r; return (unsigned char)((s > a) | (r > s)); }' % (t, t))
    fn('_addcarryx_u%d' % b, 'unsigned char', 'unsigned char c, %s a, %s b, %s *p' % (t, t, t),
       '_addcarry_u%d(c, a, b, p)' % b, 'adx')
# SHA
hdr('immintrin.h', 'sha')
for m in ('sha1msg1', 'sha1msg2', 'sha1nexte', 'sha256msg1', 'sha256msg2'):
    fn('_mm_%s_epu32' % m, 'm128i', 'm128i a, m128i b', 'm128i(S("%s", i32x4(a), i32x4(b)))' % m)
fn('#_mm_sha1rnds4_epu32', 'm128i', 'm128i a, m128i b, int i', 'm128i(S("sha1rnds4", i32x4(a), i32x4(b), (i)))')
fn('_mm_sha256rnds2_epu32', 'm128i', 'm128i a, m128i b, m128i k', 'm128i(S("sha256rnds2", i32x4(a), i32x4(b), i32x4(k)))')
# RDRAND, RDSEED (the processor's support is not checked: they are not among the extensions of --features)
hdr('immintrin.h', 'sse2')
for m in ('rdrand', 'rdseed'):
    fn('_%s16_step' % m, 'int', 'unsigned short *p', 'S("%s16", p)' % m)
    fn('_%s32_step' % m, 'int', 'unsigned int *p', 'S("%s32", p)' % m)
    fn('_%s64_step' % m, 'int', 'u64 *p', 'S("%s64", p)' % m)
# the time stamp counter, the processor's identification, bits
fn('__rdtsc', 'u64', 'void', 'S("rdtsc")')
alias('_rdtsc', '__rdtsc')
fn('__rdtscp', 'u64', 'unsigned int *p', 'S("rdtscp", p)')
alias('_rdtscp', '__rdtscp')
fn('_bit_scan_forward', 'int', 'int a', '(int)S("tzcnt", (unsigned int)a)')
fn('_bit_scan_reverse', 'int', 'int a', '31 - (int)S("lzcnt", (unsigned int)a)')
fn('__bsfd', 'int', 'int a', '(int)S("tzcnt", (unsigned int)a)')
fn('__bsrd', 'int', 'int a', '31 - (int)S("lzcnt", (unsigned int)a)')
fn('__bsfq', 'int', 'i64 a', '(int)S("tzcnt", (u64)a)')
fn('__bsrq', 'int', 'i64 a', '63 - (int)S("lzcnt", (u64)a)')
fn('_bswap', 'int', 'int a', '(int)(((unsigned int)a >> 24) | (((unsigned int)a >> 8) & 0xff00u) | (((unsigned int)a << 8) & 0xff0000u) | ((unsigned int)a << 24))')
alias('__bswapd', '_bswap')
fn('_bswap64', 'i64', 'i64 a', '(i64)(((u64)(unsigned int)_bswap((int)a) << 32) | (unsigned int)_bswap((int)((u64)a >> 32)))')
alias('__bswapq', '_bswap64')
fn('_castf32_u32', 'unsigned int', 'float a', '{ union { float __axiom_f; unsigned int __axiom_u; } v; v.__axiom_f = a; return v.__axiom_u; }')
fn('_castu32_f32', 'float', 'unsigned int a', '{ union { float __axiom_f; unsigned int __axiom_u; } v; v.__axiom_u = a; return v.__axiom_f; }')
fn('_castf64_u64', 'u64', 'double a', '{ union { double __axiom_f; u64 __axiom_u; } v; v.__axiom_f = a; return v.__axiom_u; }')
fn('_castu64_f64', 'double', 'u64 a', '{ union { double __axiom_f; u64 __axiom_u; } v; v.__axiom_u = a; return v.__axiom_f; }')
for (n, t, b) in (('_rotl', 'unsigned int', 32), ('_rotr', 'unsigned int', 32), ('_rotwl', 'unsigned short', 16),
                  ('_rotwr', 'unsigned short', 16), ('__rolb', 'unsigned char', 8), ('__rorb', 'unsigned char', 8),
                  ('__rolw', 'unsigned short', 16), ('__rorw', 'unsigned short', 16), ('__rold', 'unsigned int', 32),
                  ('__rord', 'unsigned int', 32), ('__rolq', 'u64', 64), ('__rorq', 'u64', 64)):
    left = n.endswith('l') or n.startswith('__rol')
    k = '(i & %d)' % (b - 1)
    e = '(%s)(%s ? ((a %s %s) | (a %s (%d - %s))) : a)' % (t, k, '<<' if left else '>>', k, '>>' if left else '<<', b, k)
    fn(n, t, '%s a, int i' % t, e)
fn('_xgetbv', 'u64', 'unsigned int a', 'S("xgetbv", a)')



# =============================================================================
# The tests
# =============================================================================
FEATURES_ALL = 'avx2,fma,f16c,bmi,bmi2,lzcnt,popcnt,aes,pclmul,sha,adx,sse4.2'


def arg_of(t):
    """an argument of C type t for the compile test"""
    t = t.strip()
    ct = ctype(expand(t, []))
    if t.endswith('*'):
        return '(%s)(void *)__buf' % ct
    if t in PUB or t in VT or t.startswith('__m'):
        return '*(const %s *)(const void *)__buf' % ct
    if t in ('float', 'double'):
        return '(%s)__x' % t
    return '(%s)1' % ct


def compile_test():
    """every intrinsic, called once, in a program built with every extension of item 1"""
    out = ['/* Every x86-64 intrinsic of <x86intrin.h> and <cpuid.h> is compiled (each with\n',
           '   its operands; immediates 1): its instruction assembles. Generated by\n',
           '   cmd/internal/tools/gen_x86_intrin.py: do not edit. */\n',
           '/* x86-64-only */\n/* flags: --features=+%s */\n' % FEATURES_ALL.replace(',', ',+'),
           '/* expect: compiled */\n',
           '#include <x86intrin.h>\n#include <cpuid.h>\n#include "host.h"\n\n',
           'static __axiom_attribute__((__aligned__(64))) unsigned char __buf[256];\n',
           'volatile double __x = 1.0;\nvolatile int __sink;\n\n']
    k = 0
    for (h, e, name, ret, params, body) in INTRIN:
        nm = name.lstrip('#')
        ps = split_params(params)
        call = '%s(%s)' % (nm, ', '.join(arg_of(t) for (t, n) in ps))
        out.append('void __t%d(void) {\n' % k)
        r = ctype(expand(ret, []))
        if r == 'void':
            out.append('    %s;\n' % call)
        else:
            out.append('    %s __r = %s;\n' % (r, call))
            out.append('    __sink = *(volatile unsigned char *)(void *)&__r;\n')
        out.append('}\n')
        k += 1
    out.append("\nint main(void) {\n    print_str(\"compiled\", 8);\n    return 0;\n}\n")
    return ''.join(out)



# -----------------------------------------------------------------------------
# Semantic tests: each intrinsic applied to fixed inputs (random lanes and
# edge values), its result compared with the reference computed here from
# the instruction set reference's definition of the operation
# -----------------------------------------------------------------------------
import struct
from fractions import Fraction

RNG = random.Random(20261114)
LK = {'i8': ('b', 1), 'u8': ('B', 1), 'i16': ('h', 2), 'u16': ('H', 2), 'i32': ('i', 4), 'u32': ('I', 4),
      'i64': ('q', 8), 'u64': ('Q', 8), 'f32': ('f', 4), 'f64': ('d', 8)}


def lanes(b, k):
    f, n = LK[k]
    if k == 'f32':
        return [struct.unpack('<I', b[i:i + 4])[0] for i in range(0, len(b), 4)]
    if k == 'f64':
        return [struct.unpack('<Q', b[i:i + 8])[0] for i in range(0, len(b), 8)]
    return list(struct.unpack('<%d%s' % (len(b) // n, f), b))


def pack(vals, k):
    f, n = LK[k]
    if k == 'f32':
        return b''.join(struct.pack('<I', v & 0xffffffff) for v in vals)
    if k == 'f64':
        return b''.join(struct.pack('<Q', v & 0xffffffffffffffff) for v in vals)
    bits = 8 * n
    out = []
    for v in vals:
        v &= (1 << bits) - 1
        if f.islower() and v >= 1 << (bits - 1):
            v -= 1 << bits
        out.append(v)
    return struct.pack('<%d%s' % (len(vals), f), *out)


def sat(v, k):
    bits = 8 * LK[k][1]
    if k[0] == 'u':
        return max(0, min(v, (1 << bits) - 1))
    return max(-(1 << (bits - 1)), min(v, (1 << (bits - 1)) - 1))


def wrap(v, k):
    bits = 8 * LK[k][1]
    v &= (1 << bits) - 1
    if k[0] == 'i' and v >= 1 << (bits - 1):
        v -= 1 << bits
    return v


# floating point: values as their bits; arithmetic as IEEE 754 binary32/64,
# round to nearest even, x86's NaN rules (a NaN operand's payload, quieted:
# the first one's; an invalid operation's default NaN, negative)
FMT = {'f32': (24, 8, 0x7fc00000, 0xffc00000), 'f64': (53, 11, 0x7ff8000000000000, 0xfff8000000000000)}


def fparts(k):
    p, e, q, d = FMT[k]
    return p, e, q, d, (1 << (p + e)) - 1


def isnan(x, k):
    p, e, q, d, m = fparts(k)
    ex = (x >> (p - 1)) & ((1 << e) - 1)
    return ex == (1 << e) - 1 and (x & ((1 << (p - 1)) - 1)) != 0


def quiet(x, k):
    return x | (1 << (FMT[k][0] - 2))


def to_frac(x, k):
    """the exact value of bits x (not a NaN): a Fraction, or +-inf as a string"""
    p, e, q, d, m = fparts(k)
    s = x >> (p + e - 1)
    ex = (x >> (p - 1)) & ((1 << e) - 1)
    mant = x & ((1 << (p - 1)) - 1)
    bias = (1 << (e - 1)) - 1
    if ex == (1 << e) - 1:
        return '-inf' if s else 'inf'
    if ex == 0:
        v = Fraction(mant, 1 << (p - 1)) * Fraction(2) ** (1 - bias)
    else:
        v = Fraction((1 << (p - 1)) + mant, 1 << (p - 1)) * Fraction(2) ** (ex - bias)
    return -v if s else v


def from_frac(v, k, neg_zero=False):
    """v (a Fraction, or 'inf'/'-inf') rounded to nearest even into bits"""
    p, e, q, d, m = fparts(k)
    bias = (1 << (e - 1)) - 1
    sb = 1 << (p + e - 1)
    if isinstance(v, str):
        return (sb if v[0] == '-' else 0) | (((1 << e) - 1) << (p - 1))
    if v == 0:
        return sb if neg_zero else 0
    s = sb if v < 0 else 0
    a = abs(v)
    ex = a.numerator.bit_length() - a.denominator.bit_length()
    if Fraction(2) ** ex > a:
        ex -= 1
    while Fraction(2) ** (ex + 1) <= a:
        ex += 1
    emin = 1 - bias
    if ex < emin:
        ex = emin
    # the significand in units of 2^(ex - p + 1)
    scaled = a / Fraction(2) ** (ex - p + 1)
    n = scaled.numerator // scaled.denominator
    r = scaled - n
    if r > Fraction(1, 2) or (r == Fraction(1, 2) and n % 2 == 1):
        n += 1
    if n == 1 << p:
        n >>= 1
        ex += 1
    if ex > bias:
        return s | (((1 << e) - 1) << (p - 1))
    if n < 1 << (p - 1):
        return s | n  # (subnormal)
    return s | ((ex + bias) << (p - 1)) | (n - (1 << (p - 1)))


def fop(o, xs, k):
    """an arithmetic operation of x86's SSE on bits"""
    for x in xs:
        if isnan(x, k):
            return quiet(x, k)
    p, e, q, d, m = fparts(k)
    sb = 1 << (p + e - 1)
    v = [to_frac(x, k) for x in xs]
    sg = [(x & sb) != 0 for x in xs]
    if o in ('add', 'sub'):
        a, b = v
        if o == 'sub':
            b = ('-inf' if b == 'inf' else 'inf') if isinstance(b, str) else -b
            sgb = not sg[1]
        else:
            sgb = sg[1]
        if isinstance(a, str) and isinstance(b, str):
            return from_frac(a, k) if a == b else d
        if isinstance(a, str) or isinstance(b, str):
            return from_frac(a if isinstance(a, str) else b, k)
        r = a + b
        return from_frac(r, k, neg_zero=(r == 0 and sg[0] and sgb))
    if o == 'mul':
        a, b = v
        neg = sg[0] != sg[1]
        if isinstance(a, str) or isinstance(b, str):
            if (not isinstance(a, str) and a == 0) or (not isinstance(b, str) and b == 0):
                return d
            return from_frac('-inf' if neg else 'inf', k)
        return from_frac(a * b, k, neg_zero=neg)
    if o == 'div':
        a, b = v
        neg = sg[0] != sg[1]
        if isinstance(a, str):
            return d if isinstance(b, str) else from_frac('-inf' if neg else 'inf', k)
        if isinstance(b, str):
            return sb if neg else 0
        if b == 0:
            return d if a == 0 else from_frac('-inf' if neg else 'inf', k)
        return from_frac(a / b, k, neg_zero=neg)
    if o == 'sqrt':
        a, = v
        if a == 0 and not isinstance(a, str):
            return xs[0]
        if sg[0]:
            return d
        if isinstance(a, str):
            return xs[0]
        # (the root to p + 3 bits, its remainder a sticky bit: rounding that
        # rounds the exact root, which is never halfway between two values)
        import math
        ex = a.numerator.bit_length() - a.denominator.bit_length()
        sh = 2 * (p + 3) - (ex - (ex % 2))
        N = a * Fraction(2) ** sh
        n = N.numerator // N.denominator
        r = math.isqrt(n)
        sticky = (r * r != n) or (N.denominator != 1 and N.numerator % N.denominator != 0)
        return from_frac(Fraction(2 * r + (1 if sticky else 0), 2) / Fraction(2) ** (sh // 2), k)
    if o == 'fma':
        a, b, c = v
        if any(isinstance(x, str) for x in v):
            # (infinities: the product's then the sum's rules)
            pneg = sg[0] != sg[1]
            if isinstance(a, str) or isinstance(b, str):
                if (not isinstance(a, str) and a == 0) or (not isinstance(b, str) and b == 0):
                    return d
                pi = '-inf' if pneg else 'inf'
                if isinstance(c, str) and c != pi:
                    return d
                return from_frac(pi, k)
            return xs[2]
        r = a * b + c
        pneg = (sg[0] != sg[1])
        return from_frac(r, k, neg_zero=(r == 0 and a * b == 0 and c == 0 and pneg and sg[2]))
    raise KeyError(o)


def fcmp(imm, x, y, k):
    un = isnan(x, k) or isnan(y, k)
    if un:
        lt = eq = gt = False
    else:
        conv = lambda t: Fraction(10) ** 400 if t == 'inf' else (-Fraction(10) ** 400 if t == '-inf' else t)
        a, b = conv(to_frac(x, k)), conv(to_frac(y, k))
        lt, eq, gt = a < b, a == b, a > b
    # (the predicates of the instruction set reference's table: 0-7, then 8-15 with
    # the unordered result flipped for eq and the false/true ones, repeated at 16-31)
    return [eq, lt, lt or eq, un, not eq, not lt, not (lt or eq), not un,
            eq or un, un or lt, un or lt or eq, False, (not eq) and not un, gt or eq, gt, True][imm & 15]

def f32b(x):
    return struct.unpack('<I', struct.pack('<f', x))[0]


def f64b(x):
    return struct.unpack('<Q', struct.pack('<d', x))[0]


def pools():
    """the inputs: integer bytes A, B, C (64 each), floats FA, FB, FC (16 f32), DA, DB, DC (8 f64), and
    small integers in floating lanes FN, DN (exact sums and products)"""
    edges = [0x00, 0xff, 0x7f, 0x80, 0x01, 0xfe, 0x81, 0x7e]
    P = {}
    for n in 'ABC':
        b = bytearray(RNG.getrandbits(8) for _ in range(64))
        for j, e in enumerate(edges):
            b[(j * 3 + 'ABC'.index(n) * 5) % 64] = e
        # (16-bit edges: lanes 8 and 9 of the first half; 32-bit ones in lanes 6, 7)
        b[16:18] = bytes([0x00, 0x80]) if n != 'B' else bytes([0xff, 0x7f])
        b[24:28] = bytes([0xff, 0xff, 0xff, 0x7f]) if n == 'A' else bytes([0x00, 0x00, 0x00, 0x80])
        P[n] = bytes(b)
    fe = [0.0, -0.0, 1.0, -1.5, float('inf'), float('-inf'), None, 1e-45, 3.4e38, 2.5, -2.5, 0.5, 1e30, -7.25, 65504.0,
          1.0 / 3]
    for (n, rot) in (('FA', 0), ('FB', 5), ('FC', 11)):
        vals = []
        for j in range(16):
            x = fe[(j + rot) % 16] if j < 8 else RNG.uniform(-100, 100)
            vals.append(0x7fc00001 if x is None else f32b(x))
        P[n] = pack(vals, 'f32')
    de = [0.0, -0.0, 1.0, -1.5, float('inf'), None, 5e-324, 1.7e308]
    for (n, rot) in (('DA', 0), ('DB', 3), ('DC', 6)):
        vals = []
        for j in range(8):
            x = de[(j + rot) % 8] if j < 4 else RNG.uniform(-1e6, 1e6)
            vals.append(0x7ff8000000000001 if x is None else f64b(x))
        P[n] = pack(vals, 'f64')
    P['AZ'] = P['A'][:7] + b'\x00' + P['A'][8:]
    P['FN'] = pack([f32b(float(RNG.randint(-8, 8))) for _ in range(16)], 'f32')
    P['DN'] = pack([f64b(float(RNG.randint(-8, 8))) for _ in range(8)], 'f64')
    return P


POOLS = pools()


def per128(f, *xs):
    """f of each 128-bit part of xs (bytes)"""
    n = len(xs[0]) // 16
    return b''.join(f(*[x[16 * j:16 * j + 16] for x in xs]) for j in range(n))


def lw(k, g, rk=None):
    """lane by lane: g of the lanes of kind k, the result's lanes of kind rk (k), wrapped"""
    rk = rk or k

    def m(*xs):
        ls = [lanes(x, k) for x in xs]
        return pack([wrap(g(*v), rk) if rk[0] in 'iu' else g(*v) for v in zip(*ls)], rk)
    return m


def flw(k, o):
    return lambda *xs: pack([fop(o, list(v), k) for v in zip(*[lanes(x, k) for x in xs])], k)


class TestFile:
    def __init__(self, name, what, std, guard, target):
        self.name, self.what, self.std, self.guard, self.target = name, what, std, guard, target
        self.cases = []

    def case(self, label, expr, rty, want, approx=False):
        self.cases.append((label, expr, rty, want, approx))


TEST_FILES = []


def cstr(b):
    return '"' + ''.join('\\x%02x' % c for c in b) + '"'


def ld(t, pool, off=0):
    """a C expression: the vector of type t at pool's byte off"""
    return '__L(%s, %s, %d)' % (ctype(expand(t, [])), pool, off)


def pb(pool, n, off=0):
    return POOLS[pool][off:off + n]


def test_text(tf):
    out = ['/* %s - x86-64 intrinsics checked against the instruction set reference\'s\n' % tf.name,
           '   definitions (computed by cmd/internal/tools/gen_x86_intrin.py, which\n',
           '   writes this file: do not edit): %s. A mismatch prints the intrinsic\n' % tf.what,
           '   and the result\'s bytes. */\n',
           '/* x86-64-only */\n/* flags: --c-std=%s */\n' % tf.std,
           '/* expect: %s %d */\n' % (tf.name, len(tf.cases)),
           '#include <x86intrin.h>\n#include <cpuid.h>\n#include "host.h"\n\n',
           '#define __L(t, p, o) (*(const t *)(const void *)((p) + (o)))\n',
           '#define __I64(h, l) ((__axiom_int64)(((unsigned __axiom_int64)(h) << 32) | (unsigned __axiom_int64)(l)))\n']
    for n in sorted(POOLS):
        out.append('static __axiom_attribute__((__aligned__(64))) unsigned char %s[%d] = %s;\n' % (
            n, len(POOLS[n]), '{' + ', '.join('0x%02x' % c for c in POOLS[n]) + '}'))
    out.append(r'''
static int __checks;
static int __fails;

static char __line[512];
static size_t __len;

static void say(const char *s) {
    while (*s != 0 && __len < sizeof __line) {
        __line[__len++] = *s++;
    }
}

static void endl(void) {
    print_str(__line, __len);
    __len = 0;
}

static void say_n(int v) {
    char b[16];
    int k = 15;
    b[15] = 0;
    do {
        b[--k] = (char)('0' + v % 10);
        v /= 10;
    } while (v != 0);
    say(b + k);
}

static void hex(const unsigned char *p, int n) {
    static const char d[] = "0123456789abcdef";
    char b[3];
    int i;
    b[2] = 0;
    for (i = 0; i < n; i++) {
        b[0] = d[p[i] >> 4];
        b[1] = d[p[i] & 15];
        say(b);
    }
}

/* The result's n bytes at r, against w's; approx: float lanes within a relative 1.5 * 2^-12 */
static void chk(const char *name, const void *r, int n, const char *w, int approx) {
    const unsigned char *p = (const unsigned char *)r;
    const unsigned char *q = (const unsigned char *)w;
    int i;
    int bad = 0;
    __checks++;
    for (i = 0; i < n; i++) {
        if (p[i] != q[i]) {
            bad = 1;
        }
    }
    if (bad && approx) {
        bad = 0;
        for (i = 0; i < n; i += 4) {
            float x;
            float y;
            float e;
            unsigned char *xb = (unsigned char *)&x;
            unsigned char *yb = (unsigned char *)&y;
            int j;
            for (j = 0; j < 4; j++) {
                xb[j] = p[i + j];
                yb[j] = q[i + j];
            }
            e = (x - y) / y;
            if (!(e < 0.000367f && e > -0.000367f) && !(x == y)) {
                bad = 1;
            }
        }
    }
    if (bad) {
        __fails++;
        say(name);
        say(": ");
        hex(p, n);
        say(" expected ");
        hex(q, n);
        endl();
    }
}

/* (the inputs, written where the compiler cannot see: no result is folded) */
static void (*volatile __touch)(unsigned char *) = 0;

''')
    out.append(GUARDS)
    # (the cases in functions of 32: each is compiled in a moment)
    attr = '__axiom_attribute__((__target__("%s"))) ' % tf.target if tf.target else ''
    parts = [tf.cases[j:j + 32] for j in range(0, len(tf.cases), 32)]
    for (k, part) in enumerate(parts):
        out.append('static %svoid run%d(void) {\n' % (attr, k))
        out.append('    unsigned __axiom_int64 __hi = 0;\n    unsigned int __hi32 = 0;\n    unsigned int __o32 = 0;\n'
                   '    unsigned __axiom_int64 __o64 = 0;\n')
        for (label, expr, rty, want, approx) in part:
            r = ctype(expand(rty, []))
            out.append('    {\n        %s __r = %s;\n        chk("%s", &__r, %d, %s, %d);\n    }\n' % (
                r, expr, label, len(want), cstr(want), 1 if approx else 0))
        out.append('    (void)__hi;\n    (void)__hi32;\n    (void)__o32;\n    (void)__o64;\n}\n\n')
    out.append('static %svoid run(void) {\n' % attr)
    for k in range(len(parts)):
        out.append('    run%d();\n' % k)
    out.append('}\n\n')
    out.append('int main(void) {\n')
    out.append('    if (__touch != 0) {\n')
    for n in sorted(POOLS):
        out.append('        __touch(%s);\n' % n)
    out.append('    }\n')
    out.append('    if (%s) {\n        run();\n    } else {\n        __checks = %d;\n    }\n' % (tf.guard, len(tf.cases)))
    out.append('    if (__fails == 0) {\n        say("%s ");\n        say_n(__checks);\n        endl();\n    }\n' % tf.name)
    out.append('    return 0;\n}\n')
    return ''.join(out)

GUARDS = r'''static int __cpu(unsigned int leaf, int reg, unsigned int bit) {
    unsigned int r[4];
    if (!__get_cpuid_count(leaf, 0, &r[0], &r[1], &r[2], &r[3])) {
        return 0;
    }
    return (r[reg] & bit) != 0;
}

static int __os_avx(void) {
    return __cpu(1, 2, bit_OSXSAVE) && (_xgetbv(0) & 6) == 6;
}

'''

# ---- the cases of the 128-bit (SSE, SSE2, MMX) and 256-bit integer families
INT_K = {'epi8': 'i8', 'epu8': 'u8', 'epi16': 'i16', 'epu16': 'u16', 'epi32': 'i32', 'epu32': 'u32', 'epi64': 'i64',
         'epu64': 'u64'}


def mulhrs(x, y):
    return ((x * y >> 14) + 1) >> 1


def int_cases(tf, w, P, groups):
    """the integer families of width w (128, 256) whose group is in groups (sse2, ssse3, sse41, avx2)"""
    I = 'm%di' % w
    n = w // 8
    A, B, C = pb('A', n), pb('B', n), pb('C', n)
    a, b, c = ld(I, 'A'), ld(I, 'B'), ld(I, 'C')
    def two(nm, model):
        tf.case(P + nm, '%s%s(%s, %s)' % (P, nm, a, b), I, model(A, B))
    def one(nm, model, x=None, X=None):
        tf.case(P + nm, '%s%s(%s)' % (P, nm, x or a), I, model(X or A))
    if 'sse2' in groups:
        for k in ('epi8', 'epi16', 'epi32', 'epi64'):
            two('add_' + k, lw(INT_K[k], lambda x, y: x + y))
            two('sub_' + k, lw(INT_K[k], lambda x, y: x - y))
        for k in ('epi8', 'epi16', 'epu8', 'epu16'):
            two('adds_' + k, lw(INT_K[k], lambda x, y, kk=INT_K[k]: sat(x + y, kk)))
            two('subs_' + k, lw(INT_K[k], lambda x, y, kk=INT_K[k]: sat(x - y, kk)))
        two('avg_epu8', lw('u8', lambda x, y: (x + y + 1) >> 1))
        two('avg_epu16', lw('u16', lambda x, y: (x + y + 1) >> 1))
        two('mulhi_epi16', lw('i16', lambda x, y: (x * y) >> 16))
        two('mulhi_epu16', lw('u16', lambda x, y: (x * y) >> 16))
        two('mullo_epi16', lw('i16', lambda x, y: x * y))
        two('mul_epu32', lambda x, y: pack([(p & 0xffffffff) * (q & 0xffffffff) for p, q in zip(lanes(x, 'u64'), lanes(y, 'u64'))], 'u64'))
        two('madd_epi16', lambda x, y: pack([wrap(p[0] * q[0] + p[1] * q[1], 'i32') for p, q in zip(zip(*[iter(lanes(x, 'i16'))] * 2), zip(*[iter(lanes(y, 'i16'))] * 2))], 'i32'))
        two('sad_epu8', lambda x, y: pack([sum(abs(p - q) for p, q in zip(lanes(x[j:j + 8], 'u8'), lanes(y[j:j + 8], 'u8'))) for j in range(0, len(x), 8)], 'u64'))
        for (k, f) in (('epi16', 'i16'), ('epu8', 'u8')):
            two('min_' + k, lw(f, min))
            two('max_' + k, lw(f, max))
        for k in ('epi8', 'epi16', 'epi32'):
            two('cmpeq_' + k, lw(INT_K[k], lambda x, y: -(x == y)))
            two('cmpgt_' + k, lw(INT_K[k], lambda x, y: -(x > y)))
        tf.case(P + 'and_si%d' % w, '%sand_si%d(%s, %s)' % (P, w, a, b), I, bytes(p & q for p, q in zip(A, B)))
        tf.case(P + 'andnot_si%d' % w, '%sandnot_si%d(%s, %s)' % (P, w, a, b), I, bytes(~p & q & 255 for p, q in zip(A, B)))
        tf.case(P + 'or_si%d' % w, '%sor_si%d(%s, %s)' % (P, w, a, b), I, bytes(p | q for p, q in zip(A, B)))
        tf.case(P + 'xor_si%d' % w, '%sxor_si%d(%s, %s)' % (P, w, a, b), I, bytes(p ^ q for p, q in zip(A, B)))
        def packs(sk, dk):
            return lambda x, y: per128(lambda p, q: pack([sat(v, dk) for v in lanes(p, sk) + lanes(q, sk)], dk), x, y)
        two('packs_epi16', packs('i16', 'i8'))
        two('packs_epi32', packs('i32', 'i16'))
        two('packus_epi16', packs('i16', 'u8'))
        for (k, f) in (('epi8', 'i8'), ('epi16', 'i16'), ('epi32', 'i32'), ('epi64', 'i64')):
            def unp(hi, f=f):
                def g(p, q):
                    lp, lq = lanes(p, f), lanes(q, f)
                    h = len(lp) // 2
                    lp, lq = (lp[h:], lq[h:]) if hi else (lp[:h], lq[:h])
                    return pack([v for pair in zip(lp, lq) for v in pair], f)
                return lambda x, y: per128(g, x, y)
            two('unpacklo_' + k, unp(False))
            two('unpackhi_' + k, unp(True))
        tf.case(P + 'movemask_epi8', '%smovemask_epi8(%s)' % (P, a), 'int',
                pack([sum(((v >> 7) & 1) << j for j, v in enumerate(A)) & 0xffffffff], 'u32'))
        # shifts by immediates (0, 1, the width less one, the width, 255) and by a vector's count
        for (k, f, bits) in (('epi16', 'i16', 16), ('epi32', 'i32', 32), ('epi64', 'i64', 64)):
            for cnt in (0, 1, bits - 1, bits, 255):
                tf.case('%sslli_%s/%d' % (P, k, cnt), '%sslli_%s(%s, %d)' % (P, k, a, cnt), I,
                        lw(f, lambda x, cnt=cnt, bits=bits: 0 if cnt >= bits else x << cnt)(A))
                tf.case('%ssrli_%s/%d' % (P, k, cnt), '%ssrli_%s(%s, %d)' % (P, k, a, cnt), I,
                        lw('u' + f[1:], lambda x, cnt=cnt, bits=bits: 0 if cnt >= bits else x >> cnt)(A))
                if bits != 64:
                    tf.case('%ssrai_%s/%d' % (P, k, cnt), '%ssrai_%s(%s, %d)' % (P, k, a, cnt), I,
                            lw(f, lambda x, cnt=cnt, bits=bits: x >> min(cnt, bits - 1))(A))
            for cnt in (3, bits + 1):
                cv = pack([cnt, 0], 'u64')
                cexpr = '_mm_set_epi64x(0, %d)' % cnt
                tf.case('%ssll_%s/%d' % (P, k, cnt), '%ssll_%s(%s, %s)' % (P, k, a, cexpr), I,
                        lw(f, lambda x, cnt=cnt, bits=bits: 0 if cnt >= bits else x << cnt)(A))
                tf.case('%ssrl_%s/%d' % (P, k, cnt), '%ssrl_%s(%s, %s)' % (P, k, a, cexpr), I,
                        lw('u' + f[1:], lambda x, cnt=cnt, bits=bits: 0 if cnt >= bits else x >> cnt)(A))
                if bits != 64:
                    tf.case('%ssra_%s/%d' % (P, k, cnt), '%ssra_%s(%s, %s)' % (P, k, a, cexpr), I,
                            lw(f, lambda x, cnt=cnt, bits=bits: x >> min(cnt, bits - 1))(A))
        bs = 'si128' if w == 128 else 'si256'
        for cnt in (0, 3, 15, 16):
            def bsl(p, cnt=cnt):
                return bytes(([0] * cnt + list(p))[:16]) if cnt < 16 else bytes(16)
            def bsr(p, cnt=cnt):
                return bytes((list(p) + [0] * 16)[cnt:cnt + 16]) if cnt < 16 else bytes(16)
            tf.case('%sslli_%s/%d' % (P, bs, cnt), '%sslli_%s(%s, %d)' % (P, bs, a, cnt), I, per128(bsl, A))
            tf.case('%ssrli_%s/%d' % (P, bs, cnt), '%ssrli_%s(%s, %d)' % (P, bs, a, cnt), I, per128(bsr, A))
        for imm in (0x1b, 0xe4, 0x4e):
            sel = lambda l, imm=imm: [l[(imm >> (2 * j)) & 3] for j in range(4)]
            tf.case('%sshuffle_epi32/%x' % (P, imm), '%sshuffle_epi32(%s, 0x%x)' % (P, a, imm), I,
                    per128(lambda p: pack(sel(lanes(p, 'i32')), 'i32'), A))
            tf.case('%sshufflelo_epi16/%x' % (P, imm), '%sshufflelo_epi16(%s, 0x%x)' % (P, a, imm), I,
                    per128(lambda p: pack(sel(lanes(p, 'i16')[:4]) + lanes(p, 'i16')[4:], 'i16'), A))
            tf.case('%sshufflehi_epi16/%x' % (P, imm), '%sshufflehi_epi16(%s, 0x%x)' % (P, a, imm), I,
                    per128(lambda p: pack(lanes(p, 'i16')[:4] + sel(lanes(p, 'i16')[4:]), 'i16'), A))
    if 'ssse3' in groups:
        for (k, f) in (('epi8', 'i8'), ('epi16', 'i16'), ('epi32', 'i32')):
            one('abs_' + k, lw(f, abs))
            two('sign_' + k, lw(f, lambda x, y: x if y > 0 else (0 if y == 0 else -x)))
        def hop(f, g, sk):
            def m(p, q):
                lp, lq = lanes(p, f), lanes(q, f)
                r = [g(lp[j], lp[j + 1]) for j in range(0, len(lp), 2)] + [g(lq[j], lq[j + 1]) for j in range(0, len(lq), 2)]
                return pack([sat(v, f) if sk else v for v in r], f)
            return lambda x, y: per128(m, x, y)
        two('hadd_epi16', hop('i16', lambda x, y: x + y, False))
        two('hadd_epi32', hop('i32', lambda x, y: x + y, False))
        two('hadds_epi16', hop('i16', lambda x, y: x + y, True))
        two('hsub_epi16', hop('i16', lambda x, y: x - y, False))
        two('hsub_epi32', hop('i32', lambda x, y: x - y, False))
        two('hsubs_epi16', hop('i16', lambda x, y: x - y, True))
        two('maddubs_epi16', lambda x, y: pack([sat(lanes(x, 'u8')[2 * j] * lanes(y, 'i8')[2 * j] + lanes(x, 'u8')[2 * j + 1] * lanes(y, 'i8')[2 * j + 1], 'i16') for j in range(len(x) // 2)], 'i16'))
        two('mulhrs_epi16', lw('i16', mulhrs))
        two('shuffle_epi8', lambda x, y: per128(lambda p, q: bytes(0 if v & 0x80 else p[v & 15] for v in q), x, y))
        for cnt in (0, 5, 16, 20, 32):
            tf.case('%salignr_epi8/%d' % (P, cnt), '%salignr_epi8(%s, %s, %d)' % (P, a, b, cnt), I,
                    per128(lambda p, q, cnt=cnt: bytes((list(q) + list(p) + [0] * 32)[cnt:cnt + 16]), A, B))
    if 'sse41' in groups:
        two('cmpeq_epi64', lw('i64', lambda x, y: -(x == y)))
        for (k, f) in (('epi8', 'i8'), ('epi32', 'i32'), ('epu16', 'u16'), ('epu32', 'u32')):
            two('min_' + k, lw(f, min))
            two('max_' + k, lw(f, max))
        two('mul_epi32', lambda x, y: pack([wrap(p, 'i32') * wrap(q, 'i32') for p, q in zip(lanes(x, 'i64'), lanes(y, 'i64'))], 'i64'))
        two('mullo_epi32', lw('i32', lambda x, y: x * y))
        two('packus_epi32', lambda x, y: per128(lambda p, q: pack([sat(v, 'u16') for v in lanes(p, 'i32') + lanes(q, 'i32')], 'u16'), x, y))
        tf.case(P + 'blendv_epi8', '%sblendv_epi8(%s, %s, %s)' % (P, a, b, c), I, bytes(q if m & 0x80 else p for p, q, m in zip(A, B, C)))
        for imm in (0x00, 0xa5, 0xff):
            tf.case('%sblend_epi16/%x' % (P, imm), '%sblend_epi16(%s, %s, 0x%x)' % (P, a, b, imm), I,
                    pack([q if (imm >> (j % 8)) & 1 else p for j, (p, q) in enumerate(zip(lanes(A, 'i16'), lanes(B, 'i16')))], 'i16'))
        for imm in (0, 5, 7):
            def mps(p, q, imm=imm):
                ao = 4 * ((imm >> 2) & 1)
                bo = 4 * (imm & 3)
                return pack([sum(abs(p[ao + j + t] - q[bo + t]) for t in range(4)) for j in range(8)], 'u16')
            tf.case('%smpsadbw_epu8/%d' % (P, imm), '%smpsadbw_epu8(%s, %s, %d)' % (P, a, b, imm), I,
                    per128(mps, A, B) if w == 128 else mps(A[:16], B[:16]) + mps(A[16:], B[16:], imm >> 3))
        cv = {'epi8_epi16': ('i8', 'i16'), 'epi8_epi32': ('i8', 'i32'), 'epi8_epi64': ('i8', 'i64'),
              'epi16_epi32': ('i16', 'i32'), 'epi16_epi64': ('i16', 'i64'), 'epi32_epi64': ('i32', 'i64'),
              'epu8_epi16': ('u8', 'i16'), 'epu8_epi32': ('u8', 'i32'), 'epu8_epi64': ('u8', 'i64'),
              'epu16_epi32': ('u16', 'i32'), 'epu16_epi64': ('u16', 'i64'), 'epu32_epi64': ('u32', 'i64')}
        for (nm, (f, t)) in cv.items():
            cnt = w // (8 * LK[t][1])
            tf.case(P + 'cvt' + nm, '%scvt%s(%s)' % (P, nm, ld('m128i', 'A')), I, pack(lanes(pb('A', 16), f)[:cnt], t))



def float_cases(tf, w, P, groups):
    for (k, sx, T) in (('f32', 'ps', 'm%d' % w), ('f64', 'pd', 'm%dd' % w)):
        n = w // 8
        pa, pbb, pc = ('FA', 'FB', 'FC') if k == 'f32' else ('DA', 'DB', 'DC')
        A, B, C = pb(pa, n), pb(pbb, n), pb(pc, n)
        a, b, c = ld(T, pa), ld(T, pbb), ld(T, pc)
        it = 'i32' if k == 'f32' else 'i64'
        def two(nm, model, x=a, y=b, X=A, Y=B):
            tf.case(P + nm, '%s%s(%s, %s)' % (P, nm, x, y), T, model(X, Y))
        if 'base' in groups:
            for o in ('add', 'sub', 'mul', 'div'):
                two(o + '_' + sx, flw(k, o))
            tf.case(P + 'sqrt_' + sx, '%ssqrt_%s(%s)' % (P, sx, a), T, flw(k, 'sqrt')(A))
            # (minps: the second operand unless the first is less, NaNs and zeros too)
            lt = lambda x, y: fcmp(1, x, y, k)
            two('min_' + sx, lambda X, Y: pack([x if lt(x, y) else y for x, y in zip(lanes(X, k), lanes(Y, k))], k))
            two('max_' + sx, lambda X, Y: pack([x if lt(y, x) else y for x, y in zip(lanes(X, k), lanes(Y, k))], k))
            for (o, g) in (('and', lambda x, y: x & y), ('or', lambda x, y: x | y), ('xor', lambda x, y: x ^ y),
                           ('andnot', lambda x, y: ~x & y)):
                two(o + '_' + sx, lw(it, g))
            tf.case(P + 'movemask_' + sx, '%smovemask_%s(%s)' % (P, sx, a), 'int',
                    pack([sum((v >> (8 * LK[k][1] - 1) & 1) << j for j, v in enumerate(lanes(A, k)))], 'u32'))
            for hi in (False, True):
                def unp(p, q, hi=hi):
                    lp, lq = lanes(p, k), lanes(q, k)
                    h = len(lp) // 2
                    lp, lq = (lp[h:], lq[h:]) if hi else (lp[:h], lq[:h])
                    return pack([v for pr in zip(lp, lq) for v in pr], k)
                two(('unpackhi_' if hi else 'unpacklo_') + sx, lambda X, Y, g=unp: per128(g, X, Y))
        if 'cmp' in groups:
            if w == 128:
                for (nm, imm, sw) in (('eq', 0, 0), ('lt', 1, 0), ('le', 2, 0), ('gt', 1, 1), ('ge', 2, 1), ('unord', 3, 0),
                                      ('neq', 4, 0), ('nlt', 5, 0), ('nle', 6, 0), ('ngt', 5, 1), ('nge', 6, 1), ('ord', 7, 0)):
                    def pk(X, Y, imm=imm, sw=sw):
                        return pack([-1 if (fcmp(imm, y, x, k) if sw else fcmp(imm, x, y, k)) else 0 for x, y in zip(lanes(X, k), lanes(Y, k))], it)
                    two('cmp%s_%s' % (nm, sx), pk)
                    ss = 'ss' if k == 'f32' else 'sd'
                    def sc(X, Y, imm=imm, sw=sw):
                        x, y = lanes(X, k)[0], lanes(Y, k)[0]
                        r = fcmp(imm, y, x, k) if sw else fcmp(imm, x, y, k)
                        return pack([-1 if r else 0], it) + X[LK[k][1]:]
                    two('cmp%s_%s' % (nm, ss), sc)
                for (nm, rel) in (('eq', lambda x, y: fcmp(0, x, y, k)), ('lt', lambda x, y: fcmp(1, x, y, k)),
                                  ('le', lambda x, y: fcmp(2, x, y, k)), ('gt', lambda x, y: fcmp(14, x, y, k)),
                                  ('ge', lambda x, y: fcmp(13, x, y, k)), ('neq', lambda x, y: fcmp(4, x, y, k))):
                    ss = 'ss' if k == 'f32' else 'sd'
                    for pre_ in ('comi', 'ucomi'):
                        tf.case('_mm_%s%s_%s' % (pre_, nm, ss), '_mm_%s%s_%s(%s, %s)' % (pre_, nm, ss, a, b), 'int',
                                pack([1 if rel(lanes(A, k)[0], lanes(B, k)[0]) else 0], 'i32'))
        if 'avx' in groups:
            for imm in range(32):
                two('cmp_%s/%d' % (sx, imm), lambda X, Y, imm=imm: pack([-1 if fcmp(imm, x, y, k) else 0 for x, y in zip(lanes(X, k), lanes(Y, k))], it))
                tf.cases[-1] = (tf.cases[-1][0], '%scmp_%s(%s, %s, %d)' % (P, sx, a, b, imm), T, tf.cases[-1][3], False)
        if 'sse3' in groups:
            def addsub(X, Y):
                return pack([fop('sub' if j % 2 == 0 else 'add', [x, y], k) for j, (x, y) in enumerate(zip(lanes(X, k), lanes(Y, k)))], k)
            two('addsub_' + sx, addsub)
            for (o, f) in (('hadd', 'add'), ('hsub', 'sub')):
                def h(p, q, f=f):
                    lp, lq = lanes(p, k), lanes(q, k)
                    return pack([fop(f, [lp[j], lp[j + 1]], k) for j in range(0, len(lp), 2)] + [fop(f, [lq[j], lq[j + 1]], k) for j in range(0, len(lq), 2)], k)
                two(o + '_' + sx, lambda X, Y, h=h: per128(h, X, Y))
        if 'sse41' in groups:
            # (round: to nearest even, down, up, toward zero; floor and ceil)
            def rnd(x, mode):
                if isnan(x, k):
                    return quiet(x, k)
                v = to_frac(x, k)
                if isinstance(v, str) or v == 0:
                    return x
                import math
                fl = math.floor(v)
                if mode == 0:
                    r = fl + (1 if (v - fl > Fraction(1, 2) or (v - fl == Fraction(1, 2) and fl % 2)) else 0)
                elif mode == 1:
                    r = fl
                elif mode == 2:
                    r = math.ceil(v)
                else:
                    r = math.trunc(v)
                return from_frac(Fraction(r), k, neg_zero=(r == 0 and v < 0))
            for mode in (0, 1, 2, 3):
                tf.case('%sround_%s/%d' % (P, sx, mode), '%sround_%s(%s, %d)' % (P, sx, a, mode | 8), T,
                        pack([rnd(x, mode) for x in lanes(A, k)], k))
            tf.case(P + 'floor_' + sx, '%sfloor_%s(%s)' % (P, sx, a), T, pack([rnd(x, 1) for x in lanes(A, k)], k))
            tf.case(P + 'ceil_' + sx, '%sceil_%s(%s)' % (P, sx, a), T, pack([rnd(x, 2) for x in lanes(A, k)], k))
            tf.case(P + 'blendv_' + sx, '%sblendv_%s(%s, %s, %s)' % (P, sx, a, b, c), T,
                    pack([y if (m >> (8 * LK[k][1] - 1)) & 1 else x for x, y, m in zip(lanes(A, k), lanes(B, k), lanes(C, k))], k))
        if 'fma' in groups:
            for (nm, sa, sc_) in (('fmadd', 1, 1), ('fmsub', 1, -1), ('fnmadd', -1, 1), ('fnmsub', -1, -1)):
                sb = 1 << (8 * LK[k][1] - 1)
                neg = lambda v, s: v ^ sb if s < 0 and not isnan(v, k) else v
                tf.case('%s%s_%s' % (P, nm, sx), '%s%s_%s(%s, %s, %s)' % (P, nm, sx, a, b, c), T,
                        pack([fop('fma', [neg(x, sa), y, neg(z, sc_)], k) for x, y, z in zip(lanes(A, k), lanes(B, k), lanes(C, k))], k))
            for (nm, first) in (('fmaddsub', 'sub'), ('fmsubadd', 'add')):
                sb = 1 << (8 * LK[k][1] - 1)
                def ms(j, z, first=first):
                    o = first if j % 2 == 0 else ('add' if first == 'sub' else 'sub')
                    return z ^ sb if o == 'sub' and not isnan(z, k) else z
                tf.case('%s%s_%s' % (P, nm, sx), '%s%s_%s(%s, %s, %s)' % (P, nm, sx, a, b, c), T,
                        pack([fop('fma', [x, y, ms(j, z)], k) for j, (x, y, z) in enumerate(zip(lanes(A, k), lanes(B, k), lanes(C, k)))], k))


def conv_cases(tf, w, P):
    """the conversions between integer and floating lanes, x86's out-of-range result (the integer indefinite)"""
    def f2i(x, k, trunc, bits=32):
        if isnan(x, k):
            return -(1 << (bits - 1))
        v = to_frac(x, k)
        if isinstance(v, str):
            return -(1 << (bits - 1))
        import math
        if trunc:
            r = math.trunc(v)
        else:
            fl = math.floor(v)
            r = fl + (1 if (v - fl > Fraction(1, 2) or (v - fl == Fraction(1, 2) and fl % 2)) else 0)
        if r < -(1 << (bits - 1)) or r >= 1 << (bits - 1):
            return -(1 << (bits - 1))
        return r
    n = w // 8
    I = 'm%di' % w
    T = 'm%d' % w
    tf.case(P + 'cvtps_epi32', '%scvtps_epi32(%s)' % (P, ld(T, 'FA')), I, pack([f2i(x, 'f32', False) for x in lanes(pb('FA', n), 'f32')], 'i32'))
    tf.case(P + 'cvttps_epi32', '%scvttps_epi32(%s)' % (P, ld(T, 'FA')), I, pack([f2i(x, 'f32', True) for x in lanes(pb('FA', n), 'f32')], 'i32'))
    tf.case(P + 'cvtepi32_ps', '%scvtepi32_ps(%s)' % (P, ld(I, 'A')), T, pack([from_frac(Fraction(v), 'f32') for v in lanes(pb('A', n), 'i32')], 'f32'))
    D = 'm%dd' % w
    half = 'm128i'
    nd = w // 64
    tf.case(P + 'cvtpd_epi32', '%scvtpd_epi32(%s)' % (P, ld(D, 'DA')), half,
            pack([f2i(x, 'f64', False) for x in lanes(pb('DA', n), 'f64')] + [0] * (4 - nd), 'i32'))
    tf.case(P + 'cvttpd_epi32', '%scvttpd_epi32(%s)' % (P, ld(D, 'DA')), half,
            pack([f2i(x, 'f64', True) for x in lanes(pb('DA', n), 'f64')] + [0] * (4 - nd), 'i32'))
    tf.case(P + 'cvtepi32_pd', '%scvtepi32_pd(%s)' % (P, ld('m128i', 'A')), D, pack([f64b(float(v)) for v in lanes(pb('A', 16), 'i32')[:nd]], 'f64'))
    tf.case(P + 'cvtps_pd', '%scvtps_pd(%s)' % (P, ld('m128', 'FA')), D,
            pack([from_frac(to_frac(x, 'f32'), 'f64', neg_zero=bool(x >> 31)) if not isnan(x, 'f32') else (0xfff8000000000000 if x >> 31 else 0x7ff8000000000000) | ((x & 0x3fffff) << 29) for x in lanes(pb('FA', 16), 'f32')[:nd]], 'f64'))
    def d2f(x):
        if isnan(x, 'f64'):
            return ((x >> 63) << 31) | 0x7fc00000 | ((x >> 29) & 0x3fffff)
        return from_frac(to_frac(x, 'f64'), 'f32', neg_zero=bool(x >> 63))
    tf.case(P + 'cvtpd_ps', '%scvtpd_ps(%s)' % (P, ld(D, 'DA')), 'm128', pack([d2f(x) for x in lanes(pb('DA', n), 'f64')] + [0] * (4 - nd), 'f32'))
    if w == 128:
        for (nm, k, t, bits, r) in (('cvtss_si32', 'f32', False, 32, 'int'), ('cvttss_si32', 'f32', True, 32, 'int'),
                                    ('cvtss_si64', 'f32', False, 64, 'i64'), ('cvttss_si64', 'f32', True, 64, 'i64'),
                                    ('cvtsd_si32', 'f64', False, 32, 'int'), ('cvttsd_si32', 'f64', True, 32, 'int'),
                                    ('cvtsd_si64', 'f64', False, 64, 'i64'), ('cvttsd_si64', 'f64', True, 64, 'i64')):
            pool = 'FA' if k == 'f32' else 'DA'
            for off in (0, 3, 8, 12) if k == 'f32' else (0, 3, 8, 24):
                tf.case('_mm_%s/%d' % (nm, off), '_mm_%s(%s)' % (nm, ld('m128' if k == 'f32' else 'm128d', pool, off)), r,
                        pack([f2i(lanes(pb(pool, 4 if k == 'f32' else 8, off), k)[0], k, t, bits)], 'i%d' % bits))



def crc32c(c, data):
    for byte in data:
        c ^= byte
        for _ in range(8):
            c = (c >> 1) ^ (0x82f63b78 if c & 1 else 0)
    return c


def clmul(a, b):
    r = 0
    for j in range(64):
        if (b >> j) & 1:
            r ^= a << j
    return r


def pdep(x, m, bits):
    r, k = 0, 0
    for j in range(bits):
        if (m >> j) & 1:
            if (x >> k) & 1:
                r |= 1 << j
            k += 1
    return r


def pext(x, m, bits):
    r, k = 0, 0
    for j in range(bits):
        if (m >> j) & 1:
            if (x >> j) & 1:
                r |= 1 << k
            k += 1
    return r


# AES (FIPS-197): the S-box from the multiplicative inverse in GF(2^8) and the affine map
def gmul(a, b):
    r = 0
    while b:
        if b & 1:
            r ^= a
        a = ((a << 1) ^ 0x11b) if a & 0x80 else a << 1
        b >>= 1
    return r


SBOX = []
for x in range(256):
    inv = 0 if x == 0 else next(y for y in range(1, 256) if gmul(x, y) == 1)
    s = inv
    for k in range(1, 5):
        s ^= ((inv << k) | (inv >> (8 - k))) & 0xff
    SBOX.append(s ^ 0x63)
ISBOX = [SBOX.index(v) for v in range(256)]


def shift_rows(s, inv=False):
    # (the state's bytes column by column: s[4c + r]; row r rotates by r)
    return bytes(s[4 * ((c + (-r if inv else r)) % 4) + r] for c in range(4) for r in range(4))


def mix_columns(s, inv=False):
    m = [14, 11, 13, 9] if inv else [2, 3, 1, 1]
    out = bytearray(16)
    for c in range(4):
        col = s[4 * c:4 * c + 4]
        for r in range(4):
            out[4 * c + r] = gmul(col[0], m[(0 - r) % 4]) ^ gmul(col[1], m[(1 - r) % 4]) ^ gmul(col[2], m[(2 - r) % 4]) ^ gmul(col[3], m[(3 - r) % 4])
    return bytes(out)


def xorb(a, b):
    return bytes(p ^ q for p, q in zip(a, b))


def aesenc(s, k, last=False):
    t = bytes(SBOX[v] for v in shift_rows(s))
    return xorb(t if last else mix_columns(t), k)


def aesdec(s, k, last=False):
    t = bytes(ISBOX[v] for v in shift_rows(s, True))
    return xorb(t if last else mix_columns(t, True), k)


def keygen(s, rcon):
    w = lanes(s, 'u32')
    sub = lambda v: int.from_bytes(bytes(SBOX[b] for b in v.to_bytes(4, 'little')), 'little')
    rot = lambda v: ((v >> 8) | (v << 24)) & 0xffffffff
    x1, x3 = sub(w[1]), sub(w[3])
    return pack([x1, rot(x1) ^ rcon, x3, rot(x3) ^ rcon], 'u32')


# SHA (the instruction set reference's definitions)
def rol(v, n):
    return ((v << n) | (v >> (32 - n))) & 0xffffffff


def ror(v, n):
    return ((v >> n) | (v << (32 - n))) & 0xffffffff


def sha1rnds4(x, y, imm):
    A, B, C, D = lanes(x, 'u32')[::-1]
    W = lanes(y, 'u32')[::-1]
    E = 0
    f = [lambda b, c, d: (b & c) ^ (~b & d), lambda b, c, d: b ^ c ^ d, lambda b, c, d: (b & c) ^ (b & d) ^ (c & d),
         lambda b, c, d: b ^ c ^ d][imm & 3]
    K = [0x5a827999, 0x6ed9eba1, 0x8f1bbcdc, 0xca62c1d6][imm & 3]
    for i in range(4):
        A2 = (f(B, C, D) + rol(A, 5) + W[i] + E + K) & 0xffffffff
        E, D, C, B, A = D, C, rol(B, 30), A, A2
    return pack([D, C, B, A], 'u32')


def sha1nexte(x, y):
    w = lanes(y, 'u32')
    w[3] = (w[3] + rol(lanes(x, 'u32')[3], 30)) & 0xffffffff
    return pack(w, 'u32')


def sha1msg1(x, y):
    a, b = lanes(x, 'u32'), lanes(y, 'u32')
    W0, W1, W2, W3 = a[3], a[2], a[1], a[0]
    W4, W5 = b[3], b[2]
    return pack([a[0] ^ W5, a[1] ^ W4, W1 ^ W3, W0 ^ W2], 'u32')


def sha1msg2(x, y):
    a, b = lanes(x, 'u32'), lanes(y, 'u32')
    W13, W14, W15 = b[2], b[1], b[0]
    W16 = rol(a[3] ^ W13, 1)
    W17 = rol(a[2] ^ W14, 1)
    W18 = rol(a[1] ^ W15, 1)
    W19 = rol(a[0] ^ W16, 1)
    return pack([W19, W18, W17, W16], 'u32')


def s0(v):
    return ror(v, 7) ^ ror(v, 18) ^ (v >> 3)


def s1(v):
    return ror(v, 17) ^ ror(v, 19) ^ (v >> 10)


def sha256msg1(x, y):
    a, b = lanes(x, 'u32'), lanes(y, 'u32')
    W4 = b[0]
    return pack([(a[0] + s0(a[1])) & 0xffffffff, (a[1] + s0(a[2])) & 0xffffffff, (a[2] + s0(a[3])) & 0xffffffff,
                 (a[3] + s0(W4)) & 0xffffffff], 'u32')


def sha256msg2(x, y):
    a, b = lanes(x, 'u32'), lanes(y, 'u32')
    W14, W15 = b[2], b[3]
    W16 = (a[0] + s1(W14)) & 0xffffffff
    W17 = (a[1] + s1(W15)) & 0xffffffff
    W18 = (a[2] + s1(W16)) & 0xffffffff
    W19 = (a[3] + s1(W17)) & 0xffffffff
    return pack([W16, W17, W18, W19], 'u32')


def sha256rnds2(x, y, k):
    # (the reference: SRC1 = {C, D, G, H}, SRC2 = {A, B, E, F}, from the high lane down)
    C, D, G, H = lanes(x, 'u32')[3], lanes(x, 'u32')[2], lanes(x, 'u32')[1], lanes(x, 'u32')[0]
    A, B, E, F = lanes(y, 'u32')[3], lanes(y, 'u32')[2], lanes(y, 'u32')[1], lanes(y, 'u32')[0]
    wk = lanes(k, 'u32')
    S0 = lambda v: ror(v, 2) ^ ror(v, 13) ^ ror(v, 22)
    S1 = lambda v: ror(v, 6) ^ ror(v, 11) ^ ror(v, 25)
    ch = lambda e, f, g: (e & f) ^ (~e & g)
    maj = lambda a, b, c: (a & b) ^ (a & c) ^ (b & c)
    for i in range(2):
        t = (ch(E, F, G) + S1(E) + wk[i] + H) & 0xffffffff
        A2 = (t + maj(A, B, C) + S0(A)) & 0xffffffff
        E2 = (t + D) & 0xffffffff
        H, G, F, E, D, C, B, A = G, F, E, E2, C, B, A, A2
    return pack([F, E, B, A], 'u32')



def misc_cases(tf, which):
    A, B, C = pb('A', 16), pb('B', 16), pb('C', 16)
    a, b, c = ld('m128i', 'A'), ld('m128i', 'B'), ld('m128i', 'C')
    u32 = lambda p, o=0: lanes(pb(p, 4, o), 'u32')[0]
    u64 = lambda p, o=0: lanes(pb(p, 8, o), 'u64')[0]
    if 'sse42' in which:
        tf.case('_mm_cmpgt_epi64', '_mm_cmpgt_epi64(%s, %s)' % (a, b), 'm128i', lw('i64', lambda x, y: -(x > y))(A, B))
        for (nm, bits, r) in (('u8', 8, 'unsigned int'), ('u16', 16, 'unsigned int'), ('u32', 32, 'unsigned int'), ('u64', 64, 'u64')):
            tf.case('_mm_crc32_' + nm, '_mm_crc32_%s(0x%xu, (%s)0x%xu)' % (nm, u32('B'), {8: 'unsigned char', 16: 'unsigned short', 32: 'unsigned int', 64: 'u64'}[bits].replace('u64', 'unsigned __axiom_int64'), u64('A') & ((1 << bits) - 1)),
                    r, pack([crc32c(u32('B'), (u64('A') & ((1 << bits) - 1)).to_bytes(bits // 8, 'little'))], 'u64' if bits == 64 else 'u32'))
        tf.case('_mm_popcnt_u32', '_mm_popcnt_u32(0x%xu)' % u32('A'), 'int', pack([bin(u32('A')).count('1')], 'i32'))
        tf.case('_mm_popcnt_u64', '_mm_popcnt_u64(0x%xull)' % u64('A'), 'i64', pack([bin(u64('A')).count('1')], 'i64'))
        # the string compares: equal any, ranges, equal each, equal ordered, of bytes and words
        for imm in (0x00, 0x04, 0x08, 0x0c, 0x01, 0x12, 0x40, 0x3a):
            for (la, lb) in ((16, 16), (5, 9), (0, 3)):
                res, idx, flags = pcmpstr(A, la, B, lb, imm)
                tf.case('_mm_cmpestri/%x/%d/%d' % (imm, la, lb), '_mm_cmpestri(%s, %d, %s, %d, 0x%x)' % (a, la, b, lb, imm), 'int', pack([idx], 'i32'))
                tf.case('_mm_cmpestrm/%x/%d/%d' % (imm, la, lb), '_mm_cmpestrm(%s, %d, %s, %d, 0x%x)' % (a, la, b, lb, imm), 'm128i', mask_of(res, imm))
                for f in 'acosz':
                    tf.case('_mm_cmpestr%s/%x/%d/%d' % (f, imm, la, lb), '_mm_cmpestr%s(%s, %d, %s, %d, 0x%x)' % (f, a, la, b, lb, imm), 'int', pack([flags[f]], 'i32'))
            # (implicit lengths: a zero element ends each operand)
            Z = bytes(A[:7]) + b'\x00' + bytes(A[8:])
            zz = ld('m128i', 'AZ')
            la = strlen_of(Z, imm)
            lb = strlen_of(B, imm)
            res, idx, flags = pcmpstr(Z, la, B, lb, imm)
            tf.case('_mm_cmpistri/%x' % imm, '_mm_cmpistri(%s, %s, 0x%x)' % (zz, b, imm), 'int', pack([idx], 'i32'))
            tf.case('_mm_cmpistrm/%x' % imm, '_mm_cmpistrm(%s, %s, 0x%x)' % (zz, b, imm), 'm128i', mask_of(res, imm))
            for f in 'acosz':
                tf.case('_mm_cmpistr%s/%x' % (f, imm), '_mm_cmpistr%s(%s, %s, 0x%x)' % (f, zz, b, imm), 'int', pack([flags[f]], 'i32'))
    if 'aes' in which:
        for (nm, f) in (('aesenc', lambda s, k: aesenc(s, k)), ('aesenclast', lambda s, k: aesenc(s, k, True)),
                        ('aesdec', lambda s, k: aesdec(s, k)), ('aesdeclast', lambda s, k: aesdec(s, k, True))):
            tf.case('_mm_%s_si128' % nm, '_mm_%s_si128(%s, %s)' % (nm, a, b), 'm128i', f(A, B))
        tf.case('_mm_aesimc_si128', '_mm_aesimc_si128(%s)' % a, 'm128i', mix_columns(A, True))
        for rc in (0x01, 0x1b, 0x36):
            tf.case('_mm_aeskeygenassist_si128/%x' % rc, '_mm_aeskeygenassist_si128(%s, 0x%x)' % (a, rc), 'm128i', keygen(A, rc))
    if 'pclmul' in which:
        for imm in (0x00, 0x01, 0x10, 0x11):
            x = lanes(A, 'u64')[imm & 1]
            y = lanes(B, 'u64')[(imm >> 4) & 1]
            r = clmul(x, y)
            tf.case('_mm_clmulepi64_si128/%x' % imm, '_mm_clmulepi64_si128(%s, %s, 0x%x)' % (a, b, imm), 'm128i',
                    pack([r & 0xffffffffffffffff, r >> 64], 'u64'))
    if 'sha' in which:
        for (nm, f) in (('sha1nexte', sha1nexte), ('sha1msg1', sha1msg1), ('sha1msg2', sha1msg2),
                        ('sha256msg1', sha256msg1), ('sha256msg2', sha256msg2)):
            tf.case('_mm_%s_epu32' % nm, '_mm_%s_epu32(%s, %s)' % (nm, a, b), 'm128i', f(A, B))
        for imm in range(4):
            tf.case('_mm_sha1rnds4_epu32/%d' % imm, '_mm_sha1rnds4_epu32(%s, %s, %d)' % (a, b, imm), 'm128i', sha1rnds4(A, B, imm))
        tf.case('_mm_sha256rnds2_epu32', '_mm_sha256rnds2_epu32(%s, %s, %s)' % (a, b, c), 'm128i', sha256rnds2(A, B, C))
    if 'bmi' in which:
        x32, y32, x64, y64 = u32('A'), u32('B'), u64('A', 8), u64('B', 8)
        tf.case('_andn_u32', '_andn_u32(0x%xu, 0x%xu)' % (x32, y32), 'unsigned int', pack([~x32 & y32], 'u32'))
        tf.case('_andn_u64', '_andn_u64(0x%xull, 0x%xull)' % (x64, y64), 'u64', pack([~x64 & y64], 'u64'))
        for (st, ln) in ((3, 9), (0, 32), (28, 8), (40, 3)):
            v = (x32 >> st) & ((1 << ln) - 1) if st < 32 else 0
            tf.case('_bextr_u32/%d/%d' % (st, ln), '_bextr_u32(0x%xu, %d, %d)' % (x32, st, ln), 'unsigned int', pack([v], 'u32'))
            v = (x64 >> st) & ((1 << ln) - 1) if st < 64 else 0
            tf.case('_bextr_u64/%d/%d' % (st, ln), '_bextr_u64(0x%xull, %d, %d)' % (x64, st, ln), 'u64', pack([v], 'u64'))
        for (nm, f) in (('blsi', lambda v, m: v & (-v) & m), ('blsmsk', lambda v, m: (v ^ (v - 1)) & m), ('blsr', lambda v, m: v & (v - 1) & m)):
            tf.case('_%s_u32' % nm, '_%s_u32(0x%xu)' % (nm, x32), 'unsigned int', pack([f(x32, 0xffffffff)], 'u32'))
            tf.case('_%s_u64' % nm, '_%s_u64(0x%xull)' % (nm, x64), 'u64', pack([f(x64, (1 << 64) - 1)], 'u64'))
        for v in (x32, 0, 0x80000000):
            tz = 32 if v == 0 else (v & -v).bit_length() - 1
            tf.case('_tzcnt_u32/%x' % v, '_tzcnt_u32(0x%xu)' % v, 'unsigned int', pack([tz], 'u32'))
    if 'bmi2' in which:
        x32, y32, x64, y64 = u32('A'), u32('B'), u64('A', 8), u64('B', 8)
        for n in (0, 7, 31, 32, 200):
            tf.case('_bzhi_u32/%d' % n, '_bzhi_u32(0x%xu, %d)' % (x32, n), 'unsigned int', pack([x32 & ((1 << min(n & 255, 32)) - 1)], 'u32'))
            tf.case('_bzhi_u64/%d' % n, '_bzhi_u64(0x%xull, %d)' % (x64, n), 'u64', pack([x64 & ((1 << min(n & 255, 64)) - 1)], 'u64'))
        tf.case('_pdep_u32', '_pdep_u32(0x%xu, 0x%xu)' % (x32, y32), 'unsigned int', pack([pdep(x32, y32, 32)], 'u32'))
        tf.case('_pext_u32', '_pext_u32(0x%xu, 0x%xu)' % (x32, y32), 'unsigned int', pack([pext(x32, y32, 32)], 'u32'))
        tf.case('_pdep_u64', '_pdep_u64(0x%xull, 0x%xull)' % (x64, y64), 'u64', pack([pdep(x64, y64, 64)], 'u64'))
        tf.case('_pext_u64', '_pext_u64(0x%xull, 0x%xull)' % (x64, y64), 'u64', pack([pext(x64, y64, 64)], 'u64'))
        tf.case('_mulx_u64', '_mulx_u64(0x%xull, 0x%xull, &__hi) ^ __hi' % (x64, y64), 'u64', pack([((x64 * y64) & ((1 << 64) - 1)) ^ ((x64 * y64) >> 64)], 'u64'))
        tf.case('_mulx_u32', '_mulx_u32(0x%xu, 0x%xu, &__hi32) ^ __hi32' % (x32, y32), 'unsigned int', pack([((x32 * y32) & 0xffffffff) ^ ((x32 * y32) >> 32)], 'u32'))
    if 'lzcnt' in which:
        for v in (u32('A'), 0, 1):
            tf.case('_lzcnt_u32/%x' % v, '_lzcnt_u32(0x%xu)' % v, 'unsigned int', pack([32 - v.bit_length()], 'u32'))
        for v in (u64('A'), 0, 1):
            tf.case('_lzcnt_u64/%x' % v, '_lzcnt_u64(0x%xull)' % v, 'u64', pack([64 - v.bit_length()], 'u64'))
    if 'adx' in which:
        for (cin, x, y) in ((0, 0xffffffff, 1), (1, 0xffffffff, 0), (1, 5, 7), (0, 3, 4)):
            s_ = x + y + cin
            tf.case('_addcarry_u32/%d/%x/%x' % (cin, x, y), '(unsigned int)_addcarry_u32(%d, 0x%xu, 0x%xu, &__o32) * 0x100000000ull + __o32' % (cin, x, y), 'u64',
                    pack([(s_ >> 32) * 0x100000000 + (s_ & 0xffffffff)], 'u64'))
            d_ = x - y - cin
            tf.case('_subborrow_u32/%d/%x/%x' % (cin, x, y), '(unsigned int)_subborrow_u32(%d, 0x%xu, 0x%xu, &__o32) * 0x100000000ull + __o32' % (cin, x, y), 'u64',
                    pack([(1 if d_ < 0 else 0) * 0x100000000 + (d_ & 0xffffffff)], 'u64'))
            tf.case('_addcarryx_u32/%d/%x/%x' % (cin, x, y), '(unsigned int)_addcarryx_u32(%d, 0x%xu, 0x%xu, &__o32) * 0x100000000ull + __o32' % (cin, x, y), 'u64',
                    pack([(s_ >> 32) * 0x100000000 + (s_ & 0xffffffff)], 'u64'))
        for (cin, x, y) in ((1, (1 << 64) - 1, 0), (0, 1 << 63, 1 << 63)):
            s_ = x + y + cin
            tf.case('_addcarry_u64/%d' % cin, '_addcarry_u64(%d, 0x%xull, 0x%xull, &__o64) + __o64' % (cin, x, y), 'u64',
                    pack([((s_ >> 64) + s_) & ((1 << 64) - 1)], 'u64'))


def strlen_of(v, imm):
    w = 2 if imm & 1 else 1
    n = 16 // w
    for j in range(n):
        if v[j * w:(j + 1) * w] == bytes(w):
            return j
    return n


def pcmpstr(a, la, b, lb, imm):
    """PCMPxSTRx (Intel SDM vol. 2B, "Packed Compare String"): the result bits, index and flags"""
    w = 2 if imm & 1 else 1
    sg = (imm >> 1) & 1
    n = 16 // w
    k = ('i16' if sg else 'u16') if w == 2 else ('i8' if sg else 'u8')
    A, B = lanes(a, k), lanes(b, k)
    la, lb = min(abs(la), n), min(abs(lb), n)
    agg = (imm >> 2) & 3
    av = lambda i: i < la
    bv = lambda j: j < lb
    res = []
    for j in range(n):
        if agg == 0:  # equal any: b[j] in a's set
            r = bv(j) and any(av(i) and A[i] == B[j] for i in range(n))
        elif agg == 1:  # ranges: a holds (lo, hi) pairs
            r = bv(j) and any(av(i) and av(i + 1) and A[i] <= B[j] <= A[i + 1] for i in range(0, n, 2))
        elif agg == 2:  # equal each
            if av(j) and bv(j):
                r = A[j] == B[j]
            else:
                r = (not av(j)) and (not bv(j))
        else:  # equal ordered: a, as a substring, starts at b[j]
            r = True
            for i in range(n):
                if not av(i):
                    break
                if j + i >= n or not bv(j + i):
                    r = False
                    break
                if A[i] != B[j + i]:
                    r = False
                    break
        res.append(bool(r))
    pol = (imm >> 4) & 3
    if pol == 1:
        res = [not r for r in res]
    elif pol == 3:
        res = [(not r) if bv(j) else r for j, r in enumerate(res)]
    bits = [j for j, r in enumerate(res) if r]
    if imm & 0x40:
        idx = bits[-1] if bits else n
    else:
        idx = bits[0] if bits else n
    flags = {'c': int(bool(bits)), 'z': int(lb < n), 's': int(la < n), 'o': int(res[0]), 'a': int(not bits and lb >= n)}
    return res, idx, flags


def mask_of(res, imm):
    w = 2 if imm & 1 else 1
    if imm & 0x40:  # unit mask
        return b''.join((b'\xff' * w) if r else bytes(w) for r in res)
    v = sum(1 << j for j, r in enumerate(res) if r)
    return pack([v, 0], 'u64')



def base_extra_cases(tf):
    """set, load, store, insert, extract, shuffle and the __m64 forms"""
    A = pb('A', 16)
    a = ld('m128i', 'A')
    l32 = lanes(A, 'i32')
    tf.case('_mm_set_epi32', '_mm_set_epi32(%d, %d, %d, %d)' % (l32[3], l32[2], l32[1], l32[0]), 'm128i', A)
    tf.case('_mm_setr_epi32', '_mm_setr_epi32(%d, %d, %d, %d)' % tuple(l32), 'm128i', A)
    tf.case('_mm_set1_epi8', '_mm_set1_epi8(-3)', 'm128i', bytes([0xfd] * 16))
    tf.case('_mm_set1_epi16', '_mm_set1_epi16(-3)', 'm128i', pack([-3] * 8, 'i16'))
    l8 = lanes(A, 'i8')
    tf.case('_mm_set_epi8', '_mm_set_epi8(%s)' % ', '.join(str(v) for v in reversed(l8)), 'm128i', A)
    q = lanes(A, 'u64')
    tf.case('_mm_set_epi64x', '_mm_set_epi64x(__I64(0x%xu, 0x%xu), __I64(0x%xu, 0x%xu))' % (q[1] >> 32, q[1] & 0xffffffff, q[0] >> 32, q[0] & 0xffffffff), 'm128i', A)
    tf.case('_mm_setzero_si128', '_mm_setzero_si128()', 'm128i', bytes(16))
    tf.case('_mm_loadu_si128/3', '_mm_loadu_si128((const __m128i_u *)(const void *)(A + 3))', 'm128i', pb('A', 16, 3))
    tf.case('_mm_loadl_epi64', '_mm_loadl_epi64((const __m128i_u *)(const void *)(A + 5))', 'm128i', pb('A', 8, 5) + bytes(8))
    tf.case('_mm_loadu_si32', '_mm_loadu_si32(A + 1)', 'm128i', pb('A', 4, 1) + bytes(12))
    for k in (0, 3, 7):
        tf.case('_mm_extract_epi16/%d' % k, '_mm_extract_epi16(%s, %d)' % (a, k), 'int', pack([lanes(A, 'u16')[k]], 'i32'))
        tf.case('_mm_insert_epi16/%d' % k, '_mm_insert_epi16(%s, -2, %d)' % (a, k), 'm128i',
                pack([(-2 if j == k else v) for j, v in enumerate(lanes(A, 'i16'))], 'i16'))
    tf.case('_mm_cvtsi128_si32', '_mm_cvtsi128_si32(%s)' % a, 'int', A[:4])
    tf.case('_mm_cvtsi32_si128', '_mm_cvtsi32_si128(%d)' % l32[2], 'm128i', A[8:12] + bytes(12))
    tf.case('_mm_move_epi64', '_mm_move_epi64(%s)' % a, 'm128i', A[:8] + bytes(8))
    FA = pb('FA', 16)
    fa, fb = ld('m128', 'FA'), ld('m128', 'FB')
    FB = pb('FB', 16)
    fl, gl = lanes(FA, 'f32'), lanes(FB, 'f32')
    for imm in (0x1b, 0x4e, 0xd8):
        tf.case('_mm_shuffle_ps/%x' % imm, '_mm_shuffle_ps(%s, %s, 0x%x)' % (fa, fb, imm), 'm128',
                pack([fl[imm & 3], fl[(imm >> 2) & 3], gl[(imm >> 4) & 3], gl[(imm >> 6) & 3]], 'f32'))
    tf.case('_mm_movehl_ps', '_mm_movehl_ps(%s, %s)' % (fa, fb), 'm128', pack([gl[2], gl[3], fl[2], fl[3]], 'f32'))
    tf.case('_mm_movelh_ps', '_mm_movelh_ps(%s, %s)' % (fa, fb), 'm128', pack([fl[0], fl[1], gl[0], gl[1]], 'f32'))
    tf.case('_mm_move_ss', '_mm_move_ss(%s, %s)' % (fa, fb), 'm128', pack([gl[0]] + fl[1:4], 'f32'))
    for (o, f) in (('add', 'add'), ('sub', 'sub'), ('mul', 'mul'), ('div', 'div')):
        for off in (0, 4, 16):
            x, y = lanes(pb('FA', 16, off), 'f32'), lanes(pb('FB', 16, off), 'f32')
            tf.case('_mm_%s_ss/%d' % (o, off), '_mm_%s_ss(%s, %s)' % (o, ld('m128', 'FA', off), ld('m128', 'FB', off)), 'm128',
                    pack([fop(f, [x[0], y[0]], 'f32')] + x[1:], 'f32'))
    tf.case('_mm_rcp_ps', '_mm_rcp_ps(%s)' % ld('m128', 'FC', 32), 'm128',
            pack([f32b(1.0 / struct.unpack('<f', struct.pack('<I', v))[0]) for v in lanes(pb('FC', 16, 32), 'f32')], 'f32'), True)
    pos = [f32b(float(v)) for v in (1, 2, 3, 4)]
    POOLS.setdefault('FP', pack(pos, 'f32'))
    tf.case('_mm_rsqrt_ps', '_mm_rsqrt_ps(%s)' % ld('m128', 'FP'), 'm128', pack([f32b(v ** -0.5) for v in (1.0, 2.0, 3.0, 4.0)], 'f32'), True)
    # (the __m64 forms: computed in the low half of an SSE register)
    M, N = pb('A', 8), pb('B', 8)
    m, n_ = ld('m64', 'A'), ld('m64', 'B')
    for (nm, model) in (('add_pi8', lw('i8', lambda x, y: x + y)), ('adds_pu8', lw('u8', lambda x, y: sat(x + y, 'u8'))),
                        ('subs_pi16', lw('i16', lambda x, y: sat(x - y, 'i16'))), ('mulhi_pi16', lw('i16', lambda x, y: (x * y) >> 16)),
                        ('madd_pi16', lambda x, y: pack([lanes(x, 'i16')[0] * lanes(y, 'i16')[0] + lanes(x, 'i16')[1] * lanes(y, 'i16')[1], wrap(lanes(x, 'i16')[2] * lanes(y, 'i16')[2] + lanes(x, 'i16')[3] * lanes(y, 'i16')[3], 'i32')], 'i32')),
                        ('cmpgt_pi16', lw('i16', lambda x, y: -(x > y))), ('xor_si64', lw('i64', lambda x, y: x ^ y)),
                        ('packs_pi16', lambda x, y: pack([sat(v, 'i8') for v in lanes(x, 'i16') + lanes(y, 'i16')], 'i8')),
                        ('packs_pu16', lambda x, y: pack([sat(v, 'u8') for v in lanes(x, 'i16') + lanes(y, 'i16')], 'u8')),
                        ('unpacklo_pi8', lambda x, y: bytes(v for pr in zip(x[:4], y[:4]) for v in pr)),
                        ('unpackhi_pi16', lambda x, y: pack([v for pr in zip(lanes(x, 'i16')[2:], lanes(y, 'i16')[2:]) for v in pr], 'i16')),
                        ('avg_pu8', lw('u8', lambda x, y: (x + y + 1) >> 1)), ('max_pi16', lw('i16', max)),
                        ('min_pu8', lw('u8', min)), ('sad_pu8', lambda x, y: pack([sum(abs(p - q) for p, q in zip(x, y))], 'u64'))):
        tf.case('_mm_' + nm, '_mm_%s(%s, %s)' % (nm, m, n_), 'm64', model(M, N))
    for cnt in (1, 15, 16):
        tf.case('_mm_slli_pi16/%d' % cnt, '_mm_slli_pi16(%s, %d)' % (m, cnt), 'm64', lw('i16', lambda x, cnt=cnt: 0 if cnt >= 16 else x << cnt)(M))
        tf.case('_mm_srai_pi32/%d' % cnt, '_mm_srai_pi32(%s, %d)' % (m, cnt), 'm64', lw('i32', lambda x, cnt=cnt: x >> min(cnt, 31))(M))
    tf.case('_mm_movemask_pi8', '_mm_movemask_pi8(%s)' % m, 'int', pack([sum(((v >> 7) & 1) << j for j, v in enumerate(M))], 'i32'))
    tf.case('_mm_shuffle_pi16/1b', '_mm_shuffle_pi16(%s, 0x1b)' % m, 'm64', pack(lanes(M, 'i16')[::-1], 'i16'))


def avx_extra_cases(tf):
    A, B = pb('A', 32), pb('B', 32)
    a, b = ld('m256i', 'A'), ld('m256i', 'B')
    l64 = lanes(A, 'i64')
    for imm in (0x1b, 0x4e, 0xe4):
        tf.case('_mm256_permute4x64_epi64/%x' % imm, '_mm256_permute4x64_epi64(%s, 0x%x)' % (a, imm), 'm256i',
                pack([l64[(imm >> (2 * j)) & 3] for j in range(4)], 'i64'))
    for imm in (0x01, 0x20, 0x31, 0x08, 0x83):
        def half(sel):
            if sel & 8:
                return bytes(16)
            return (A + B)[16 * (sel & 3):16 * (sel & 3) + 16]
        tf.case('_mm256_permute2x128_si256/%x' % imm, '_mm256_permute2x128_si256(%s, %s, 0x%x)' % (a, b, imm), 'm256i',
                half(imm & 15) + half(imm >> 4))
    idx = pack([5, 0, 7, 2, 2, 9, 3, 1], 'i32')
    POOLS.setdefault('IX', idx + bytes(32))
    tf.case('_mm256_permutevar8x32_epi32', '_mm256_permutevar8x32_epi32(%s, %s)' % (a, ld('m256i', 'IX')), 'm256i',
            pack([lanes(A, 'i32')[v & 7] for v in lanes(idx, 'i32')], 'i32'))
    for (nm, f, g) in (('sllv_epi32', 'i32', lambda x, c: 0 if c >= 32 or c < 0 else x << c), ('srlv_epi32', 'u32', lambda x, c: 0 if c >= 32 or c < 0 else x >> c),
                       ('srav_epi32', 'i32', lambda x, c: x >> (31 if c >= 32 or c < 0 else c)), ('sllv_epi64', 'i64', lambda x, c: 0 if c >= 64 or c < 0 else x << c),
                       ('srlv_epi64', 'u64', lambda x, c: 0 if c >= 64 or c < 0 else x >> c)):
        cnt = pack([3, 0, 33, 31, 1, 200, 7, 16], 'i32') if f[1:] == '32' else pack([3, 63, 64, 1000], 'i64')
        POOLS.setdefault('SC' + f[1:], cnt)
        tf.case('_mm256_' + nm, '_mm256_%s(%s, %s)' % (nm, a, ld('m256i', 'SC' + f[1:])), 'm256i',
                pack([wrap(g(x, wrap(c, 'u' + f[1:]) if f[0] == 'u' else (c if c >= 0 else 1 << 40)), f) for x, c in zip(lanes(A, f), lanes(cnt, 'i' + f[1:]))], f))
    # gathers: base A, indexes in IG
    ig = pack([0, 5, 2, 7, 1, 1, 6, 3], 'i32')
    POOLS.setdefault('IG', ig + bytes(32))
    for (nm, el, w, scale) in (('_mm256_i32gather_epi32', 'i32', 8, 4), ('_mm_i32gather_epi32', 'i32', 4, 4),
                               ('_mm256_i32gather_epi64', 'i64', 4, 8), ('_mm256_i32gather_ps', 'f32', 8, 4)):
        T = {('i32', 8): 'm256i', ('i32', 4): 'm128i', ('i64', 4): 'm256i', ('f32', 8): 'm256'}[(el, w)]
        ct = {'i32': 'int', 'i64': '__axiom_int64', 'f32': 'float'}[el]
        X = 'm256i' if (w == 8) else 'm128i'
        src = POOLS['FA'] if el == 'f32' else POOLS['A']
        vals = lanes(src, el)
        tf.case(nm, '%s((const %s *)(const void *)%s, %s, %d)' % (nm, ct, 'FA' if el == 'f32' else 'A', ld(X, 'IG'), scale), T,
                pack([vals[i] for i in lanes(ig, 'i32')[:w]], el))
    tf.case('_mm256_movemask_epi8', '_mm256_movemask_epi8(%s)' % a, 'int', pack([sum(((v >> 7) & 1) << j for j, v in enumerate(A)) & 0xffffffff], 'u32'))
    tf.case('_mm256_extracti128_si256/1', '_mm256_extracti128_si256(%s, 1)' % a, 'm128i', A[16:])
    tf.case('_mm256_inserti128_si256/0', '_mm256_inserti128_si256(%s, %s, 0)' % (a, ld('m128i', 'B')), 'm256i', B[:16] + A[16:])
    tf.case('_mm256_broadcastd_epi32', '_mm256_broadcastd_epi32(%s)' % ld('m128i', 'B'), 'm256i', B[:4] * 8)
    tf.case('_mm256_set_epi32', '_mm256_set_epi32(%s)' % ', '.join(str(v) for v in reversed(lanes(A, 'i32'))), 'm256i', A)
    tf.case('_mm256_testz_si256', '_mm256_testz_si256(%s, %s)' % (a, b), 'int', pack([int(all((p & q) == 0 for p, q in zip(A, B)))], 'i32'))
    tf.case('_mm256_testc_si256', '_mm256_testc_si256(%s, %s)' % (a, b), 'int', pack([int(all((~p & q & 255) == 0 for p, q in zip(A, B)))], 'i32'))
    tf.case('_mm256_testc_si256/self', '_mm256_testc_si256(%s, %s)' % (a, a), 'int', pack([1], 'i32'))
    # F16C
    F = pb('FN', 16)
    tf.case('_mm_cvtps_ph', '_mm_cvtps_ph(%s, 0)' % ld('m128', 'FN'), 'm128i', b''.join(struct.pack('<e', struct.unpack('<f', F[4 * j:4 * j + 4])[0]) for j in range(4)) + bytes(8))
    tf.case('_mm256_cvtph_ps', '_mm256_cvtph_ps(%s)' % ld('m128i', 'A'), 'm256',
            pack([f32b(struct.unpack('<e', pb('A', 16)[2 * j:2 * j + 2])[0]) if not (lanes(pb('A', 16), 'u16')[j] & 0x7c00 == 0x7c00 and lanes(pb('A', 16), 'u16')[j] & 0x3ff) else ((lanes(pb('A', 16), 'u16')[j] & 0x8000) << 16) | 0x7fc00000 | ((lanes(pb('A', 16), 'u16')[j] & 0x3ff) << 13) for j in range(8)], 'f32'))
    tf.case('_mm256_maskload_epi32', '_mm256_maskload_epi32((const int *)(const void *)A, %s)' % b, 'm256i',
            pack([v if m < 0 else 0 for v, m in zip(lanes(A, 'i32'), lanes(B, 'i32'))], 'i32'))
    tf.case('_mm256_blend_epi32/a5', '_mm256_blend_epi32(%s, %s, 0xa5)' % (a, b), 'm256i',
            pack([q if (0xa5 >> j) & 1 else p for j, (p, q) in enumerate(zip(lanes(A, 'i32'), lanes(B, 'i32')))], 'i32'))


def build_tests():
    t1 = TestFile('intrin_x86_sse2', 'SSE, SSE2 and the __m64 forms (the baseline), in C89', 'c89', '1', None)
    int_cases(t1, 128, '_mm_', ['sse2'])
    float_cases(t1, 128, '_mm_', ['base', 'cmp'])
    conv_cases(t1, 128, '_mm_')
    base_extra_cases(t1)
    t2 = TestFile('intrin_x86_sse4', 'SSE3, SSSE3, SSE4.1 and SSE4.2, where the processor has them', 'c11',
                  '__cpu(1, 2, bit_SSE4_2) && __cpu(1, 2, bit_POPCNT)', 'sse4.2,popcnt')
    int_cases(t2, 128, '_mm_', ['ssse3', 'sse41'])
    float_cases(t2, 128, '_mm_', ['sse3', 'sse41'])
    misc_cases(t2, ['sse42'])
    t3 = TestFile('intrin_x86_avx2', 'AVX, AVX2, FMA and F16C, where the processor and system have them', 'gnu17',
                  '__os_avx() && __cpu(7, 1, bit_AVX2) && __cpu(1, 2, bit_FMA) && __cpu(1, 2, bit_F16C)', 'avx2,fma,f16c')
    int_cases(t3, 256, '_mm256_', ['sse2', 'ssse3', 'sse41'])
    float_cases(t3, 256, '_mm256_', ['base', 'avx', 'sse3', 'sse41', 'fma'])
    float_cases(t3, 128, '_mm_', ['avx', 'fma'])
    conv_cases(t3, 256, '_mm256_')
    avx_extra_cases(t3)
    t4 = TestFile('intrin_x86_scalar', 'AES, PCLMULQDQ, SHA, BMI1, BMI2, LZCNT and the carry chains, where the processor has them', 'c17',
                  '__cpu(1, 2, bit_AES) && __cpu(1, 2, bit_PCLMUL) && __cpu(7, 1, bit_SHA) && __cpu(7, 1, bit_BMI) && __cpu(7, 1, bit_BMI2) && __cpu(0x80000001, 2, bit_LZCNT) && __cpu(7, 1, bit_ADX)',
                  'aes,pclmul,sha,bmi,bmi2,lzcnt,adx')
    misc_cases(t4, ['aes', 'pclmul', 'sha', 'bmi', 'bmi2', 'lzcnt', 'adx'])
    return [t1, t2, t3, t4]


# =============================================================================
# Output
# =============================================================================
HEADERS = [  # name, what, includes
    ('mmintrin.h', 'MMX (computed with SSE2), and the vector types of every x86-64 intrinsics header', []),
    ('xmmintrin.h', 'SSE', ['mmintrin.h', 'mm_malloc.h']),
    ('emmintrin.h', 'SSE2', ['xmmintrin.h']),
    ('pmmintrin.h', 'SSE3', ['emmintrin.h']),
    ('tmmintrin.h', 'SSSE3', ['pmmintrin.h']),
    ('smmintrin.h', 'SSE4.1', ['tmmintrin.h']),
    ('nmmintrin.h', 'SSE4.2', ['smmintrin.h']),
    ('wmmintrin.h', 'AES and PCLMULQDQ', ['nmmintrin.h']),
    ('immintrin.h', 'AVX, AVX2, FMA, F16C, BMI1, BMI2, LZCNT, POPCNT, ADX, SHA, RDRAND, RDSEED and the rest',
     ['wmmintrin.h']),
    ('x86intrin.h', 'every x86-64 intrinsic (as <immintrin.h>)', ['immintrin.h']),
]

LANE_TYPES = ''.join('typedef %s __axiom_%s __axiom_attribute__((__vector_size__(%d)));\n' % (el, n, w)
                     for (n, (el, w)) in VT.items())

MM_PRE = r'''/* The function of an intrinsic: inlined into its callers, which must be
   compiled with its extension (C3240 otherwise), and compiled with it. */
#define __AXIOM_FN(e) static __axiom_attribute__((__always_inline__, __target__(e), __unused__))
#define __AXIOM_FN0 static __axiom_attribute__((__always_inline__, __unused__))

/* Vectors of lanes of each type (the headers' own) */
''' + LANE_TYPES + r'''
/* The intrinsics' types: the vectors, which may alias any object (no
   type-based aliasing is assumed of them), and their unaligned forms (_u) */
typedef __axiom_int64 __m64 __axiom_attribute__((__vector_size__(8), __may_alias__));
typedef float __m128 __axiom_attribute__((__vector_size__(16), __may_alias__));
typedef double __m128d __axiom_attribute__((__vector_size__(16), __may_alias__));
typedef __axiom_int64 __m128i __axiom_attribute__((__vector_size__(16), __may_alias__));
typedef float __m256 __axiom_attribute__((__vector_size__(32), __may_alias__));
typedef double __m256d __axiom_attribute__((__vector_size__(32), __may_alias__));
typedef __axiom_int64 __m256i __axiom_attribute__((__vector_size__(32), __may_alias__));
typedef __m128 __m128_u __axiom_attribute__((__aligned__(1)));
typedef __m128d __m128d_u __axiom_attribute__((__aligned__(1)));
typedef __m128i __m128i_u __axiom_attribute__((__aligned__(1)));
typedef __m256 __m256_u __axiom_attribute__((__aligned__(1)));
typedef __m256d __m256d_u __axiom_attribute__((__aligned__(1)));
typedef __m256i __m256i_u __axiom_attribute__((__aligned__(1)));
typedef __m64 __axiom_m64u __axiom_attribute__((__aligned__(1)));
typedef __axiom_int64 __axiom_i64u __axiom_attribute__((__aligned__(1), __may_alias__));
typedef int __axiom_i32u __axiom_attribute__((__aligned__(1), __may_alias__));
typedef short __axiom_i16u __axiom_attribute__((__aligned__(1), __may_alias__));
typedef double __axiom_f64u __axiom_attribute__((__aligned__(1), __may_alias__));
typedef float __axiom_f32u __axiom_attribute__((__aligned__(1), __may_alias__));

/* __m64 in the low half of an SSE register, and back */
__AXIOM_FN0 __m128i __axiom_w64(__m64 __a) {
    __axiom_i64x2 __r = {0, 0};
    __r[0] = (__axiom_int64)__a;
    return (__m128i)__r;
}
__AXIOM_FN0 __m64 __axiom_n64(__m128i __a) {
    return (__m64)((__axiom_i64x2)__a)[0];
}
__AXIOM_FN0 __m128i __axiom_cat64(__m64 __a, __m64 __b) {
    __axiom_i64x2 __r = {0, 0};
    __r[0] = (__axiom_int64)__a;
    __r[1] = (__axiom_int64)__b;
    return (__m128i)__r;
}
__AXIOM_FN0 __axiom_i32x4 __axiom_i16x4_i32(__m64 __a) {
    return (__axiom_i32x4)__builtin_axiom_simd("widen_lo", (__axiom_i16x8)__axiom_w64(__a));
}
__AXIOM_FN0 __axiom_i32x4 __axiom_u16x4_i32(__m64 __a) {
    return (__axiom_i32x4)__builtin_axiom_simd("widen_lo", (__axiom_u16x8)__axiom_w64(__a));
}
__AXIOM_FN0 __axiom_i32x4 __axiom_i8x4_i32(__m64 __a) {
    return (__axiom_i32x4)__builtin_axiom_simd(
        "widen_lo", (__axiom_i16x8)__builtin_axiom_simd("widen_lo", (__axiom_i8x16)__axiom_w64(__a)));
}
__AXIOM_FN0 __axiom_i32x4 __axiom_u8x4_i32(__m64 __a) {
    return (__axiom_i32x4)__builtin_axiom_simd(
        "widen_lo", (__axiom_u16x8)__builtin_axiom_simd("widen_lo", (__axiom_u8x16)__axiom_w64(__a)));
}
__AXIOM_FN0 __m64 __axiom_insert_pi16(__m64 __a, int __b, int __i) {
    __axiom_i16x4 __r = (__axiom_i16x4)__a;
    __r[__i] = (short)__b;
    return (__m64)__r;
}
__AXIOM_FN0 __m128i __axiom_insert_epi16(__m128i __a, int __b, int __i) {
    __axiom_i16x8 __r = (__axiom_i16x8)__a;
    __r[__i] = (short)__b;
    return (__m128i)__r;
}
'''


def wide_machine(ret, params, body):
    """does the intrinsic use a machine operation on 256-bit vectors (which needs AVX2: the
    code generator computes 256-bit vectors in one register only with AVX2)?"""
    if 'm256' not in ret + params and '256' not in body:
        return False
    return any(m.group(1) in OPS for m in re.finditer(r'S\("(\w+)"', body))


def emit_fn(e, name, ret, params, body):
    if e in ('avx', 'fma', 'f16c') and wide_machine(ret, params, body):
        e = e + '+avx2' if e != 'avx' else 'avx2'
    ps = split_params(params)
    pn = [n for (_, n) in ps]
    b = expand(body, pn).strip()
    if name.startswith('#'):
        args = ', '.join('__' + n for n in pn)
        return '#define %s(%s) (%s)\n' % (name[1:], args, b)
    sig = ', '.join('%s __%s' % (ctype(expand(t, [])), n) for (t, n) in ps) or 'void'
    r = ctype(expand(ret, []))
    if not b.startswith('{'):
        b = '{ return %s; }' % b if r != 'void' else '{ %s; }' % b
    if e == 'sse2':
        return '__AXIOM_FN0 %s %s(%s) %s\n' % (r, name, sig, b)  # (the baseline)
    target = ','.join(e.split('+'))
    return '__AXIOM_FN("%s") %s %s(%s) %s\n' % (target, r, name, sig, b)


def header_text(h, what, incs):
    g = '__AXIOM_' + re.sub(r'\W', '_', h.upper())
    out = ['/* <%s> - the x86-64 intrinsics of %s (the compiler\'s: docs/c.md,\n' % (h, what),
           '   "Intrinsics"). Generated by cmd/internal/tools/gen_x86_intrin.py: do not edit. */\n',
           '#ifndef %s\n#define %s\n' % (g, g),
           '#if !defined(__x86_64__)\n#error "<%s> is for x86-64"\n#endif\n' % h]
    for i in incs:
        if i == 'mm_malloc.h':
            out.append('#if __STDC_HOSTED__\n#include <mm_malloc.h>\n#endif\n')
        else:
            out.append('#include <%s>\n' % i)
    if h == 'mmintrin.h':
        out.append(MM_PRE)
    out.append(EXTRA.get(h, ''))
    # (setzero first: the others use them)
    for (hh, e, name, ret, params, body) in sorted(INTRIN, key=lambda x: 0 if 'setzero' in x[2] else 1):
        if hh == h:
            out.append(emit_fn(e, name, ret, params, body))
    out.append('#endif\n')
    return ''.join(out)



def write(path, text):
    with open(path, 'w') as f:
        f.write(text)


def table_text():
    lines = []
    for (n, sig) in GENERIC:
        lines.append('%s|%s|-|%s|%s|G' % (n, 'x' if n in ('cpuid', 'xgetbv') else '*', sig, GENERIC_RES.get(n, 'a')))
    for (n, (ext, sig, res, form, mn, cons, clob, flags)) in OPS.items():
        lines.append('|'.join([n, 'x', ext, sig, res, form, mn.replace('||', '@@'), cons.replace('||', '@@'), clob,
                               flags]).rstrip('|'))
    lines += EXTRA_TABLE
    out = ['// =============================================================================\n',
           '// csimd_tab.ax - the operations of __builtin_axiom_simd (csema_simd.ax says\n',
           '// what they are; c/cgen_simd.ax makes their code). A line is\n',
           '//     name|arch|ext|sig|res|form|mnemonic|constraints|clobbers|flags\n',
           '// arch: x (x86-64), a (AArch64), w (WebAssembly), * (any); ext: the\n',
           "// extensions needed ('+'-joined --features names, - none); sig: the\n",
           "// operands' kinds (csema_simd.sig_ok); res: the result's type\n",
           '// (csema_simd.result_type); form: G for a generic operation, else a machine\n',
           '// form (cgen_simd.ax), whose instruction the processor manuals define.\n',
           '// Generated by cmd/internal/tools/gen_x86_intrin.py (and its kin for the\n',
           '// other architectures): do not edit.\n',
           '// =============================================================================\n',
           'pub let TABLE: [%d][]u8 = [\n' % len(lines)]
    for l in lines:
        out.append('    "%s",\n' % l.replace('\\', '\\\\').replace('"', '\\"'))
    out.append('];\n')
    return ''.join(out)


# the operations of the other architectures' generators (gen_neon.py, gen_wasm_intrin.py), kept
EXTRA_TABLE = []


def keep_other_arches():
    if not os.path.exists(TAB):
        return
    for l in open(TAB).read().split('\n'):
        m = re.match(r'    "([^|]*)\|([aw])\|(.*)",$', l)
        if m:
            EXTRA_TABLE.append(m.group(1) + '|' + m.group(2) + '|' + m.group(3))


def main():
    keep_other_arches()
    write(os.path.join(TESTS, 'run', 'intrin_x86_compile.c'), compile_test())
    for tf in build_tests():
        write(os.path.join(TESTS, 'run', tf.name + '.c'), test_text(tf))
    write(os.path.join(TESTS, 'run', 'intrin_x86_compile.c'), compile_test())
    for tf in build_tests():
        write(os.path.join(TESTS, 'run', tf.name + '.c'), test_text(tf))
    for (h, what, incs) in HEADERS:
        write(os.path.join(INC, h), header_text(h, what, incs))
    write(TAB, table_text())


if __name__ == '__main__':
    main()
