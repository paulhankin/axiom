#!/usr/bin/env python3
"""Generates the AArch64 intrinsics headers of the C compiler (std/c/include/:
<arm_neon.h>, <arm_fp16.h>, <arm_bf16.h>, <arm_acle.h>), the AArch64 part of
its table of machine operations (cmd/axiom/src/c/csimd_tab.ax) and the tests
of them (cmd/axiom/internal/tests/c/run/intrin_a64_*.c), from the tables
below.

    python3 cmd/internal/tools/gen_neon_intrin.py

The intrinsics are those of Arm's C Language Extensions (ACLE: the Advanced
SIMD intrinsics' names, operand types and lane semantics, and the scalar
intrinsics of <arm_acle.h>) and the instructions those of the Arm
Architecture Reference Manual for A-profile (their encodings and their
pseudocode, from which the reference results of the tests are computed
here), written from those documents. Each intrinsic is a static function
(always inlined, compiled with its extension: the attribute target) or,
when an operand must be a constant (a lane, a shift count), a macro over a
function that checks the other operands' types. Its body (csema_simd.ax
gives the design) is:
  - on 128-bit vectors, where C's vector operators or the IR's operations
    say it (add, mul, the bit operations, compares, shifts by a constant,
    saturating add of 8- and 16-bit lanes, integer min and max, sqrt and
    rounding, fma, widening, loads and stores): plain C, or
    __builtin_axiom_simd of a generic operation;
  - everything else, and everything on 64-bit vectors (int8x8_t ...: the IR
    has no vectors of 8 bytes, and holds one as a D register's bits): one
    instruction (rarely two), __builtin_axiom_simd of a machine operation
    whose template the assembler encodes; its operands in vector registers
    are written %v0.8b (the register as lanes), %s0 (a scalar view) ...
    (c/cgen_simd.ax says how a form reads; "V16" pins an operand to v16,
    for the instructions that take a list of consecutive registers).
The headers are C89 (no // comments, no compound literals, no inline).
"""
import math
import os
import random
import re
import struct
from fractions import Fraction

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..')
INC = os.path.join(ROOT, 'std', 'c', 'include')
TAB = os.path.join(ROOT, 'cmd', 'axiom', 'src', 'c', 'csimd_tab.ax')
TESTS = os.path.join(ROOT, 'cmd', 'axiom', 'internal', 'tests', 'c')

# =============================================================================
# Types
# =============================================================================
KINDS = ['s8', 's16', 's32', 's64', 'u8', 'u16', 'u32', 'u64', 'f16', 'f32', 'f64', 'p8', 'p16', 'p64', 'bf16']
I8_32 = ['s8', 's16', 's32', 'u8', 'u16', 'u32']
INTS = ['s8', 's16', 's32', 's64', 'u8', 'u16', 'u32', 'u64']
SINT = ['s8', 's16', 's32', 's64']
UINT = ['u8', 'u16', 'u32', 'u64']
FLT = ['f32', 'f64']
POLY = ['p8', 'p16', 'p64']
ALL = KINDS  # (bf16's need bf16, as the headers say)


def eb(k):
    """the element's bits"""
    return 16 if k == 'bf16' else 128 if k == 'p128' else int(k[1:])


def nl(k, q):
    return (128 if q else 64) // eb(k)


def vt(k, q):
    base = {'s': 'int', 'u': 'uint', 'f': 'float', 'p': 'poly', 'b': 'bfloat'}[k[0]]
    return '%s%dx%d_t' % (base, eb(k), nl(k, q))


def xt(k, q, n):
    return vt(k, q)[:-2] + 'x%d_t' % n


def st(k):
    """the scalar type"""
    if k == 'bf16':
        return 'bfloat16_t'
    base = {'s': 'int', 'u': 'uint', 'f': 'float', 'p': 'poly'}[k[0]]
    return '%s%d_t' % (base, eb(k))


def ev(k):
    return 'bhsdq'[{8: 0, 16: 1, 32: 2, 64: 3, 128: 4}[eb(k)]]


def ar(k, q):
    return '%d%s' % (nl(k, q), ev(k))


def isf(k):
    return k[0] in 'fb'


def wk(k):
    return {'s8': 's16', 's16': 's32', 's32': 's64', 'u8': 'u16', 'u16': 'u32', 'u32': 'u64', 'f16': 'f32',
            'f32': 'f64', 'p8': 'p16', 'p64': 'p128'}[k]


def nk(k):
    return {'s16': 's8', 's32': 's16', 's64': 's32', 'u16': 'u8', 'u32': 'u16', 'u64': 'u32', 'f32': 'f16',
            'f64': 'f32'}[k]


def uk(k):
    return 'u%d' % eb(k)


def sk(k):
    return 's%d' % eb(k)


def Qs(q):
    return 'q' if q else ''


def RES(q):
    return 'Q' if q else 'D'


def fext(k):
    """the extension of arithmetic on lanes of kind k"""
    return 'fp16' if k == 'f16' else 'bf16' if k == 'bf16' else '-'


def dext(k):
    """the extension of moving lanes of kind k: none (no instruction that moves bits needs one)"""
    return '-'


def pre(k):
    """the instructions' prefix of a kind: s, u (integers), f (floats)"""
    return {'s': 's', 'u': 'u', 'f': 'f', 'p': 'u', 'b': 'f'}[k[0]]


# =============================================================================
# Machine operations and intrinsics
# =============================================================================
OPS = {}       # key -> name
OPLIST = []    # (name, ext, sig, res, template, constraints, flags, imm)
OPNAMES = set()


def mop(tmpl, cons, sig, res, ext='-', flags='', imm='', clob=''):
    """the name of the machine operation of this template (made if new)"""
    if tmpl.split()[0] not in ('saddlp', 'uaddlp', 'sadalp', 'uadalp', 'pmull', 'pmull2'):
        # (a vector of one 64-bit lane: the scalar form, which every instruction of such lanes has)
        tmpl = re.sub(r'%v(\d)\.1d\b', r'%d\1', tmpl)
    key = (tmpl, cons, sig, res, ext, flags, imm, clob)
    if key in OPS:
        return OPS[key]
    mn = tmpl.split()[0]
    parts = re.findall(r'\.(\d*[bhsdq])\b|%([bhsdwx])\d', tmpl)
    base = 'a.' + mn + ''.join('.' + (p[0] or p[1]) for p in parts[:3])
    name = base
    n = 1
    while name in OPNAMES:
        n += 1
        name = '%s.%d' % (base, n)
    OPNAMES.add(name)
    OPS[key] = name
    OPLIST.append((name, ext, sig, res, tmpl, cons, flags, imm, clob))
    return name


INTRIN = []   # dict: hdr, name, ret, params [(type, name)], body, ext, macro, model, imms, test
HDR = 'arm_neon.h'
CAT = ''


def fn(name, ret, params, body, ext='-', model=None, imms=None, raw=False, mode=None, test=None):
    """an intrinsic: params [(C type, name)], body a C expression or block in
    the parameters' names (a, b, c, d, l, n, p ...); imms: {name: (lo, hi)} of
    the parameters that must be constants (then it is a macro)"""
    for e in INTRIN:
        assert e['name'] != name, name
    INTRIN.append(dict(hdr=HDR, name=name, ret=ret, params=params, body=body, ext=ext, model=model,
                       imms=imms or {}, raw=raw, mode=mode, test=test, cat=CAT))


def opsig(t):
    if t.endswith('*'):
        return 'p'
    if t in VEC or t in STRUCT:
        return 'v'
    if t in ('float32_t', 'float', 'float64_t', 'double'):
        return 'f'
    return 'n'


def A(tmpl, cons, args, res, ext='-', flags='', imm=None, sig=None, clob=''):
    """__builtin_axiom_simd of the machine operation of template tmpl and
    constraints cons (';'-separated, the output first), on args (C
    expressions, with their kinds sig: csema_simd.sig_ok's letters)"""
    s = sig if sig is not None else ''.join(x[1] for x in args)
    ims = ','.join('%d:%d' % r for r in (imm or []))
    name = mop(tmpl, cons, s, res, ext, flags, ims, clob)
    exprs = [x[0] for x in args]
    return '__builtin_axiom_simd("%s"%s)' % (name, ''.join(', ' + e for e in exprs))


# the vector types: name -> (kind, q); the structures of 2, 3, 4: name -> (kind, q, n)
VEC = {}
STRUCT = {}
for k in KINDS:
    for q in (0, 1):
        VEC[vt(k, q)] = (k, q)
        for n in (2, 3, 4):
            STRUCT[xt(k, q, n)] = (k, q, n)
SCAL = {st(k): k for k in KINDS}
SCAL.update({'poly128_t': 'p128', 'int': 's32', 'unsigned int': 'u32', 'float': 'f32', 'double': 'f64'})


def ex(body, names):
    """the body with its parameters' names reserved (__a), outside strings"""
    parts = body.split('"')
    pat = re.compile(r'(?<![\w.])(' + '|'.join(sorted(names, key=len, reverse=True)) + r')\b') if names else None
    for i in range(0, len(parts), 2):
        if pat:
            parts[i] = pat.sub(r'__\1', parts[i])
    return '"'.join(parts)


# =============================================================================
# Lane values: integers, and floating values (Python floats, or Fractions
# before their rounding) with the formats' encodings
# =============================================================================
FMT = {'f16': (10, 5), 'bf16': (7, 8), 'f32': (23, 8), 'f64': (52, 11)}
NAN = float('nan')


def lo_hi(k):
    b = eb(k)
    if k[0] == 's':
        return (-(1 << (b - 1)), (1 << (b - 1)) - 1)
    return (0, (1 << b) - 1)


def sat(v, k):
    lo, hi = lo_hi(k)
    return max(lo, min(hi, v))


def wrap(v, k):
    b = eb(k)
    v &= (1 << b) - 1
    if k[0] == 's' and v >> (b - 1):
        v -= 1 << b
    return v


def isnan(x):
    return isinstance(x, float) and x != x


def snan(x):
    """a signaling NaN (decoded keeping its quiet bit in the double's)"""
    return isnan(x) and not ((struct.unpack('<Q', struct.pack('<d', x))[0] >> 51) & 1)


QNAN = struct.unpack('<d', struct.pack('<Q', 0x7ff8000000000000))[0]
SNAN = struct.unpack('<d', struct.pack('<Q', 0x7ff4000000000000))[0]


def fdec(bits, k):
    m, e = FMT[k]
    s = bits >> (m + e)
    ex_ = (bits >> m) & ((1 << e) - 1)
    fr = bits & ((1 << m) - 1)
    bias = (1 << (e - 1)) - 1
    if ex_ == (1 << e) - 1:
        if fr:
            return QNAN if (fr >> (m - 1)) & 1 else SNAN
        return -math.inf if s else math.inf
    if ex_ == 0:
        v = Fraction(fr, 1 << m) * Fraction(2) ** (1 - bias)
    else:
        v = (1 + Fraction(fr, 1 << m)) * Fraction(2) ** (ex_ - bias)
    v = float(v)
    return -v if s else v if v else (-0.0 if s else 0.0)


def rnd(x, k, mode='n'):
    """x (a float or Fraction) rounded to format k (n: to nearest even, z:
    toward zero, m: down, p: up, a: to nearest away, o: to odd), as a float"""
    if isinstance(x, float) and (isnan(x) or math.isinf(x)):
        return x
    if x == 0:
        return x if isinstance(x, float) else 0.0
    m, e = FMT[k]
    bias = (1 << (e - 1)) - 1
    emin = 1 - bias
    v = Fraction(x)
    neg = v < 0
    a = -v if neg else v
    ex_ = a.numerator.bit_length() - a.denominator.bit_length()
    if Fraction(2) ** ex_ > a:
        ex_ -= 1
    ex_ = max(ex_, emin)
    scale = Fraction(2) ** (ex_ - m)
    q = a / scale
    n = q.numerator // q.denominator
    rem = q - n
    md = mode
    if md in 'mp':
        md = 'u' if (md == 'p') != neg else 'z'
    if md == 'n':
        if rem > Fraction(1, 2) or (rem == Fraction(1, 2) and n & 1):
            n += 1
    elif md == 'a':
        if rem >= Fraction(1, 2):
            n += 1
    elif md == 'u':
        if rem:
            n += 1
    elif md == 'o':
        if rem:
            n |= 1
    r = n * scale
    big = (2 - Fraction(2) ** (-m)) * Fraction(2) ** bias
    if r > big:
        if md in 'na' or (md == 'u'):
            return -math.inf if neg else math.inf
        r = big
    f = float(r)
    if f == 0:
        return -0.0 if neg else 0.0
    return -f if neg else f


def fenc(x, k):
    """the bits of value x (rounded to nearest) in format k; a NaN is the default NaN"""
    m, e = FMT[k]
    if isnan(x):
        return ((1 << e) - 1) << m | (1 << (m - 1))
    x = rnd(x, k)
    s = 1 if (x < 0 or (x == 0 and math.copysign(1, x) < 0)) else 0
    a = abs(x)
    if math.isinf(a):
        return (s << (m + e)) | (((1 << e) - 1) << m)
    if a == 0:
        return s << (m + e)
    bias = (1 << (e - 1)) - 1
    v = Fraction(a)
    ex_ = v.numerator.bit_length() - v.denominator.bit_length()
    if Fraction(2) ** ex_ > v:
        ex_ -= 1
    if ex_ < 1 - bias:
        fr = v / Fraction(2) ** (1 - bias - m)
        return (s << (m + e)) | int(fr)
    fr = (v / Fraction(2) ** ex_ - 1) * (1 << m)
    return (s << (m + e)) | ((ex_ + bias) << m) | int(fr)


def fneg(x):
    return -x


def fsign(x):
    return math.copysign(1, x) < 0


def fadd(a, b):
    if isnan(a) or isnan(b):
        return NAN
    return a + b


def fsub(a, b):
    if isnan(a) or isnan(b):
        return NAN
    return a - b


def fmul(a, b):
    if isnan(a) or isnan(b):
        return NAN
    return a * b


def fdiv(a, b):
    if isnan(a) or isnan(b):
        return NAN
    if b == 0:
        if a == 0:
            return NAN
        return -math.inf if fsign(a) != fsign(b) else math.inf
    if math.isinf(a) and math.isinf(b):
        return NAN
    return a / b


def fsqrt(a, k):
    if isnan(a) or (a < 0):
        return NAN
    if a == 0 or math.isinf(a):
        return a
    # (correctly rounded: the root to more bits than the format's, and its sticky bit)
    v = Fraction(a)
    sh = 2 * 200
    n = (v.numerator << sh) // v.denominator
    r = math.isqrt(n)
    exact = r * r * v.denominator == (v.numerator << sh)
    rr = Fraction(r, 1 << 200)
    if not exact:
        rr += Fraction(1, 1 << 202)
    return rnd(rr, k)


def ffma(a, b, c, k):
    """a * b + c, rounded once (to format k)"""
    if isnan(a) or isnan(b) or isnan(c):
        return NAN
    if math.isinf(a) or math.isinf(b) or math.isinf(c):
        if (math.isinf(a) and b == 0) or (math.isinf(b) and a == 0):
            return NAN
        p = a * b
        return p + c  # (inf - inf is NaN)
    p = Fraction(a) * Fraction(b)
    s = p + Fraction(c)
    if s == 0:
        pz = (fsign(a) != fsign(b))
        if p == 0 and c == 0 and pz and fsign(c):
            return -0.0
        return 0.0 if (p != 0 or c != 0 or not (pz and fsign(c))) else -0.0
    return rnd(s, k)


def fmax(a, b):
    if isnan(a) or isnan(b):
        return NAN
    if a == 0 and b == 0:
        return b if fsign(a) else a
    return a if a > b else b


def fmin(a, b):
    if isnan(a) or isnan(b):
        return NAN
    if a == 0 and b == 0:
        return a if fsign(a) else b
    return a if a < b else b


def fmaxnm(a, b):
    if snan(a) or snan(b):
        return NAN
    if isnan(a):
        return b
    if isnan(b):
        return a
    return fmax(a, b)


def fminnm(a, b):
    if snan(a) or snan(b):
        return NAN
    if isnan(a):
        return b
    if isnan(b):
        return a
    return fmin(a, b)


def fabs_(a):
    return a if isnan(a) else abs(a)


def fround(a, mode):
    """to an integral value: n (even), a (away), m, p, z"""
    if isnan(a) or math.isinf(a) or a == 0:
        return a
    v = Fraction(a)
    fl = v.numerator // v.denominator
    rem = v - fl
    if mode == 'm':
        r = fl
    elif mode == 'p':
        r = fl + (1 if rem else 0)
    elif mode == 'z':
        r = fl if v >= 0 else fl + (1 if rem else 0)
    elif mode == 'a':
        r = fl + 1 if rem >= Fraction(1, 2) else fl
        if v < 0:
            r = -(((-v).numerator // (-v).denominator) + (1 if (-v) - ((-v).numerator // (-v).denominator) >= Fraction(1, 2) else 0))
    else:
        r = fl + 1 if (rem > Fraction(1, 2) or (rem == Fraction(1, 2) and fl & 1)) else fl
    if r == 0:
        return -0.0 if a < 0 else 0.0
    return float(r)


def f2i(a, k, mode, fbits=0):
    """the conversion to integer kind k (saturating; NaN 0), rounded by mode, of a * 2^fbits"""
    if isnan(a):
        return 0
    lo, hi = lo_hi(k)
    if math.isinf(a):
        return hi if a > 0 else lo
    v = Fraction(a) * (1 << fbits)
    fl = v.numerator // v.denominator
    rem = v - fl
    if mode == 'm':
        r = fl
    elif mode == 'p':
        r = fl + (1 if rem else 0)
    elif mode == 'z':
        r = fl if v >= 0 else fl + (1 if rem else 0)
    elif mode == 'a':
        r = fl + 1 if rem >= Fraction(1, 2) else fl
        if v < 0:
            av = -v
            afl = av.numerator // av.denominator
            r = -(afl + (1 if av - afl >= Fraction(1, 2) else 0))
    else:
        r = fl + 1 if (rem > Fraction(1, 2) or (rem == Fraction(1, 2) and fl & 1)) else fl
    return max(lo, min(hi, r))


def i2f(v, k, fbits=0):
    return rnd(Fraction(v, 1 << fbits), k)


# =============================================================================
# Helpers of the families
# =============================================================================
def V(i, k, q):
    """operand i as a vector of kind k (q: 128 bits)"""
    return '%%v%d.%s' % (i, ar(k, q))


def E(i, k, lane):
    """lane `lane` (a number, or an immediate operand %n) of operand i"""
    return '%%v%d.%s[%s]' % (i, ev(k), lane)


def SV(i, k):
    """operand i's scalar view (b0, h0, s0, d0)"""
    return '%%%s%d' % (ev(k), i)


def cast(t, e):
    return '((%s)%s)' % (t, e)


def scode(k):
    return {'s8': 'i', 's16': 'i', 's32': 'i', 'u8': 'u', 'u16': 'u', 'u32': 'u', 's64': 'l', 'u64': 'L', 'p64': 'L',
            'p8': 'u', 'p16': 'u', 'f32': 'f', 'f64': 'd', 'f16': 'u', 'bf16': 'u'}[k]


def sarg(k, x):
    """scalar x of kind k as a machine operand"""
    if k == 'f16':
        return ('__axiom_h2u(%s)' % x, 'n')
    if k == 'bf16':
        return ('__axiom_b2u(%s)' % x, 'n')
    return (x, 'f' if k in FLT else 'n')


def sres(k, e):
    if k == 'f16':
        return '__axiom_u2h(%s)' % e
    if k == 'bf16':
        return '__axiom_u2b(%s)' % e
    return cast(st(k), e)


def vres(k, q, e):
    return cast(vt(k, q), e)


def L1(f):
    return lambda a: [f(x) for x in a]


def L2(f):
    return lambda a, b: [f(x, y) for (x, y) in zip(a, b)]


def L3(f):
    return lambda a, b, c: [f(x, y, z) for (x, y, z) in zip(a, b, c)]


def vec(k, q, x='a'):
    return (vt(k, q), x)


def plain_q(k, q):
    """may a 128-bit vector of kind k be computed by C's operators?"""
    return q and k not in ('f16', 'bf16') and k[0] != 'p'


def mach3(mn, k, q, ext, rk=None, tied=False):
    """a three-same instruction: mn Vd, Vn, Vm (tied: Vd is also an input, operand 1)"""
    rk = rk or k
    ak, ark = k, rk
    if mn in ('eor', 'and', 'orr', 'bic', 'orn'):
        # (the bit operations: on bytes)
        ak = ark = 'u8'
    if tied:
        return vres(rk, q, A('%s %s, %s, %s' % (mn, V(0, ark, q), V(2, ak, q), V(3, ak, q)), '=w;0;w;w',
                             [('a', 'v'), ('b', 'v'), ('c', 'v')], RES(q), ext))
    return vres(rk, q, A('%s %s, %s, %s' % (mn, V(0, ark, q), V(1, ak, q), V(2, ak, q)), '=w;w;w',
                         [('a', 'v'), ('b', 'v')], RES(q), ext))


def mach2(mn, k, q, ext, rk=None, rq=None):
    rk = rk or k
    rq = q if rq is None else rq
    return vres(rk, rq, A('%s %s, %s' % (mn, V(0, rk, rq), V(1, k, q)), '=w;w', [('a', 'v')], RES(rq), ext))


def same3(op, kinds, mn, qc=None, model=None, rk=None, mode=None, ext=None, raw=False):
    """v<op>[q]_<k>(a, b) on vectors of the same kind"""
    for k in kinds:
        for q in (0, 1):
            T = vt(k, q)
            r = rk(k) if rk else k
            e = ext or fext(k)
            m = mn(k) if callable(mn) else mn
            if q and qc and qc(k) is not None and plain_q(k, q):
                body = qc(k)
            else:
                body = mach3(m, k, q, e, rk=r)
            fn('v%s%s_%s' % (op, Qs(q), k), vt(r, q), [(T, 'a'), (T, 'b')], body, e,
               model=model(k) if model else None, mode=mode, raw=raw)


def same2(op, kinds, mn, qc=None, model=None, rk=None, ext=None, raw=False):
    for k in kinds:
        for q in (0, 1):
            T = vt(k, q)
            r = rk(k) if rk else k
            e = ext or fext(k)
            m = mn(k) if callable(mn) else mn
            if q and qc and qc(k) is not None and plain_q(k, q):
                body = qc(k)
            else:
                body = mach2(m, k, q, e, rk=r)
            fn('v%s%s_%s' % (op, Qs(q), k), vt(r, q), [(T, 'a')], body, e, model=model(k) if model else None,
               raw=raw)


def scalar2(op, k, mn, model=None, rk=None, ext=None, nm=None):
    """the scalar form v<op><b|h|s|d>_<k>(a, b): mn on scalar views"""
    r = rk or k
    e = ext or fext(k)
    body = sres(r, A('%s %s, %s, %s' % (mn, SV(0, r), SV(1, k), SV(2, k)), '=w;w;w', [sarg(k, 'a'), sarg(k, 'b')],
                     scode(r), e))
    fn(nm or 'v%s%s_%s' % (op, ev(k), k), st(r), [(st(k), 'a'), (st(k), 'b')], body, e, model=model)


def scalar1(op, k, mn, model=None, rk=None, ext=None, nm=None):
    r = rk or k
    e = ext or fext(k)
    body = sres(r, A('%s %s, %s' % (mn, SV(0, r), SV(1, k)), '=w;w', [sarg(k, 'a')], scode(r), e))
    fn(nm or 'v%s%s_%s' % (op, ev(k), k), st(r), [(st(k), 'a')], body, e, model=model)


def sw(v, k):
    """v as the bits of an integer lane of kind k, then as kind k's value"""
    return wrap(v, k)


def ubits(v, b):
    return v & ((1 << b) - 1)


# =============================================================================
# The families
# =============================================================================
F16 = ['f16']
FLTH = ['f16', 'f32', 'f64']


def fk(k):
    return lambda x: x


def arith():
    # ---- add, sub, mul, div (C's operators on 128 bits)
    same3('add', INTS + FLTH, lambda k: 'fadd' if isf(k) else 'add', qc=lambda k: 'a + b',
          model=lambda k: L2(fadd if isf(k) else (lambda x, y: x + y)))
    same3('add', POLY, 'eor', qc=lambda k: 'a ^ b', model=lambda k: L2(lambda x, y: x ^ y), raw=True)
    fn('vaddq_p128', 'poly128_t', [('poly128_t', 'a'), ('poly128_t', 'b')], 'a ^ b', model=lambda a, b: a ^ b)
    same3('sub', INTS + FLTH, lambda k: 'fsub' if isf(k) else 'sub', qc=lambda k: 'a - b',
          model=lambda k: L2(fsub if isf(k) else (lambda x, y: x - y)))
    for k in ('s64', 'u64'):
        u = 'uint64_t'
        fn('vaddd_%s' % k, st(k), [(st(k), 'a'), (st(k), 'b')], '(%s)((%s)a + (%s)b)' % (st(k), u, u),
           model=lambda a, b: a + b)
        fn('vsubd_%s' % k, st(k), [(st(k), 'a'), (st(k), 'b')], '(%s)((%s)a - (%s)b)' % (st(k), u, u),
           model=lambda a, b: a - b)
    same3('mul', I8_32 + FLTH, lambda k: 'fmul' if isf(k) else 'mul', qc=lambda k: 'a * b',
          model=lambda k: L2(fmul if isf(k) else (lambda x, y: x * y)))
    same3('mul', ['p8'], 'pmul', model=lambda k: L2(lambda x, y: clmul(x, y) & 0xff))
    same3('div', FLTH, 'fdiv', qc=lambda k: 'a / b', model=lambda k: L2(fdiv))
    same3('mulx', FLTH, 'fmulx', model=lambda k: L2(fmulx))
    for k in FLTH:
        scalar2('mulx', k, 'fmulx', model=fmulx)
    # ---- absolute differences, halving, saturating
    same3('abd', I8_32 + FLTH, lambda k: 'fabd' if isf(k) else pre(k) + 'abd',
          model=lambda k: L2((lambda x, y: fabs_(fsub(x, y))) if isf(k) else (lambda x, y: abs(x - y))))
    for k in FLTH:
        scalar2('abd', k, 'fabd', model=lambda x, y: fabs_(fsub(x, y)))
    same3('hadd', I8_32, lambda k: pre(k) + 'hadd', model=lambda k: L2(lambda x, y: (x + y) >> 1))
    same3('rhadd', I8_32, lambda k: pre(k) + 'rhadd', model=lambda k: L2(lambda x, y: (x + y + 1) >> 1))
    same3('hsub', I8_32, lambda k: pre(k) + 'hsub', model=lambda k: L2(lambda x, y: (x - y) >> 1))
    for (op, g, f) in (('qadd', 'adds', lambda x, y: x + y), ('qsub', 'subs', lambda x, y: x - y)):
        same3(op, INTS, lambda k, op=op: pre(k) + op,
              qc=lambda k, g=g: ('__builtin_axiom_simd("%s", a, b)' % (g if k[0] == 's' else g[:-1] + 'us'))
              if eb(k) <= 16 else None,
              model=lambda k, f=f: L2(lambda x, y: sat(f(x, y), k)))
        for k in INTS:
            scalar2(op, k, pre(k) + op, model=lambda x, y, f=f, k=k: sat(f(x, y), k))
    for k in SINT:
        u = uk(k)
        for q in (0, 1):
            fn('vuqadd%s_%s' % (Qs(q), k), vt(k, q), [(vt(k, q), 'a'), (vt(u, q), 'b')],
               vres(k, q, A('suqadd %s, %s' % (V(0, k, q), V(2, k, q)), '=w;0;w', [('a', 'v'), ('b', 'v')], RES(q))),
               model=lambda a, b, k=k: [sat(x + y, k) for (x, y) in zip(a, b)])
            fn('vsqadd%s_%s' % (Qs(q), u), vt(u, q), [(vt(u, q), 'a'), (vt(k, q), 'b')],
               vres(u, q, A('usqadd %s, %s' % (V(0, u, q), V(2, u, q)), '=w;0;w', [('a', 'v'), ('b', 'v')], RES(q))),
               model=lambda a, b, u=u: [sat(x + y, u) for (x, y) in zip(a, b)])
        fn('vuqadd%s_%s' % (ev(k), k), st(k), [(st(k), 'a'), (st(u), 'b')],
           sres(k, A('suqadd %s, %s' % (SV(0, k), SV(2, k)), '=w;0;w', [sarg(k, 'a'), sarg(u, 'b')], scode(k))),
           model=lambda a, b, k=k: sat(a + b, k))
        fn('vsqadd%s_%s' % (ev(k), u), st(u), [(st(u), 'a'), (st(k), 'b')],
           sres(u, A('usqadd %s, %s' % (SV(0, u), SV(2, u)), '=w;0;w', [sarg(u, 'a'), sarg(k, 'b')], scode(u))),
           model=lambda a, b, u=u: sat(a + b, u))
    # ---- min and max
    same3('max', I8_32 + FLTH, lambda k: 'fmax' if isf(k) else pre(k) + 'max',
          qc=lambda k: None if isf(k) else '__builtin_axiom_simd("max", a, b)',
          model=lambda k: L2(fmax if isf(k) else max))
    same3('min', I8_32 + FLTH, lambda k: 'fmin' if isf(k) else pre(k) + 'min',
          qc=lambda k: None if isf(k) else '__builtin_axiom_simd("min", a, b)',
          model=lambda k: L2(fmin if isf(k) else min))
    same3('maxnm', FLTH, 'fmaxnm', model=lambda k: L2(fmaxnm))
    same3('minnm', FLTH, 'fminnm', model=lambda k: L2(fminnm))
    # ---- pairwise
    for (op, mi, mf, f, ff) in (('padd', 'addp', 'faddp', lambda x, y: x + y, fadd), ('pmax', 'maxp', 'fmaxp', max, fmax),
                                ('pmin', 'minp', 'fminp', min, fmin), ('pmaxnm', None, 'fmaxnmp', None, fmaxnm),
                                ('pminnm', None, 'fminnmp', None, fminnm)):
        for k in INTS + FLTH:
            if (mi is None and not isf(k)):
                continue
            for q in (0, 1):
                if (not q and eb(k) == 64) or (op != 'padd' and eb(k) == 64 and not isf(k)):
                    continue
                m = mf if isf(k) else (mi if op == 'padd' else pre(k) + mi)
                fn('v%s%s_%s' % (op, Qs(q), k), vt(k, q), [vec(k, q), vec(k, q, 'b')], mach3(m, k, q, fext(k)), fext(k),
                   model=(lambda a, b, g=(ff if isf(k) else f): [g(x, y) for (x, y) in zip((a + b)[0::2], (a + b)[1::2])]))
    for k in ('s64', 'u64', 'f64'):
        m = 'faddp' if isf(k) else 'addp'
        fn('vpaddd_%s' % k, st(k), [vec(k, 1)], sres(k, A('%s %s, %s' % (m, SV(0, k), V(1, k, 1)), '=w;w', [('a', 'v')],
                                                            scode(k))), model=lambda a: fadd(a[0], a[1]) if isinstance(a[0], float) else a[0] + a[1])
    fn('vpadds_f32', 'float32_t', [vec('f32', 0)], sres('f32', A('faddp %s, %s' % (SV(0, 'f32'), V(1, 'f32', 0)), '=w;w',
                                                                 [('a', 'v')], 'f')), model=lambda a: fadd(a[0], a[1]))
    for (op, m, f) in (('pmax', 'fmaxp', fmax), ('pmin', 'fminp', fmin), ('pmaxnm', 'fmaxnmp', fmaxnm),
                       ('pminnm', 'fminnmp', fminnm)):
        for (k, q, s) in (('f32', 0, 's'), ('f64', 1, 'qd')):
            fn('v%s%s_%s' % (op, s, k), st(k), [vec(k, q)], sres(k, A('%s %s, %s' % (m, SV(0, k), V(1, k, q)), '=w;w',
                                                                      [('a', 'v')], scode(k))), model=lambda a, f=f: f(a[0], a[1]))
    # pairwise add long and accumulate
    for k in I8_32:
        w = wk(k)
        for q in (0, 1):
            fn('vpaddl%s_%s' % (Qs(q), k), vt(w, q), [vec(k, q)], mach2(pre(k) + 'addlp', k, q, '-', rk=w),
               model=lambda a: [a[i] + a[i + 1] for i in range(0, len(a), 2)])
            fn('vpadal%s_%s' % (Qs(q), k), vt(w, q), [vec(w, q), vec(k, q, 'b')],
               vres(w, q, A('%sadalp %s, %s' % (pre(k), V(0, w, q), V(2, k, q)), '=w;0;w', [('a', 'v'), ('b', 'v')], RES(q))),
               model=lambda a, b: [a[i // 2] + b[i] + b[i + 1] for i in range(0, len(b), 2)])
    # ---- reductions across the lanes
    for k in INTS + FLTH:
        for q in (0, 1):
            n = nl(k, q)
            if n == 1:
                continue
            if isf(k):
                m = 'faddp' if n == 2 else None
                if m is None:
                    continue  # (vaddvq_f32: below)
            if n == 2 and eb(k) == 32 and not isf(k):
                t = 'addp %s, %s, %s' % ('%v0.2s', V(1, k, q), V(1, k, q))
            elif n == 2:
                m = 'faddp' if isf(k) else 'addp'
                t = '%s %s, %s' % (m, SV(0, k), V(1, k, q))
            else:
                t = 'addv %s, %s' % (SV(0, k), V(1, k, q))
            fn('vaddv%s_%s' % (Qs(q), k), st(k), [vec(k, q)], sres(k, A(t, '=w;w', [('a', 'v')], scode(k), fext(k))),
               fext(k), model=(lambda a: fadd(a[0], a[1])) if isf(k) else (lambda a: sum(a)))
    # (the float sums of 4 and 8 lanes: pairwise, as the instructions add)
    for (k, q) in (('f32', 1), ('f16', 0), ('f16', 1)):
        n = nl(k, q)
        t = 'faddp %s, %s, %s' % (V(0, k, q), V(1, k, q), V(1, k, q))
        steps = '\n'.join(['faddp %s, %s, %s' % (V(0, k, q), V(0, k, q), V(0, k, q))] * (n.bit_length() - 3))
        t2 = t + ('\n' + steps if steps else '') + '\nfaddp %s, %s' % (SV(0, k), '%%v0.2%s' % ev(k))
        fn('vaddv%s_%s' % (Qs(q), k), st(k), [vec(k, q)], sres(k, A(t2, '=&w;w', [('a', 'v')], scode(k), fext(k))),
           fext(k), model=lambda a, k=k: pairsum(a, k))
    for k in I8_32:
        for q in (0, 1):
            w = wk(k)
            n = nl(k, q)
            if n == 2:
                t = '%saddlp %s, %s' % (pre(k), '%v0.1d', V(1, k, q))
            else:
                t = '%saddlv %s, %s' % (pre(k), SV(0, w), V(1, k, q))
            fn('vaddlv%s_%s' % (Qs(q), k), st(w), [vec(k, q)], sres(w, A(t, '=w;w', [('a', 'v')], scode(w))),
               model=lambda a: sum(a))
    for (op, mi, mf, f, ff) in (('maxv', 'maxv', 'fmaxv', max, fmax), ('minv', 'minv', 'fminv', min, fmin),
                                ('maxnmv', None, 'fmaxnmv', None, fmaxnm), ('minnmv', None, 'fminnmv', None, fminnm)):
        for k in I8_32 + FLTH:
            if mi is None and not isf(k):
                continue
            for q in (0, 1):
                n = nl(k, q)
                if n == 1:
                    continue
                if n == 2:
                    m = (mf[:-1] + 'p') if isf(k) else pre(k) + mi[:-1] + 'p'
                    t = '%s %s, %s' % (m, SV(0, k), V(1, k, q))
                    if not isf(k):
                        t = '%s %s, %s, %s' % (m, '%v0.2s', V(1, k, q), V(1, k, q))
                else:
                    m = mf if isf(k) else pre(k) + mi
                    t = '%s %s, %s' % (m, SV(0, k), V(1, k, q))
                g = ff if isf(k) else f
                fn('v%s%s_%s' % (op, Qs(q), k), st(k), [vec(k, q)], sres(k, A(t, '=w;w', [('a', 'v')], scode(k), fext(k))),
                   fext(k), model=(lambda a, g=g: red(g, a)) if isf(k) else (lambda a, g=g: red(g, a)))


def red(g, a):
    r = a[0]
    for x in a[1:]:
        r = g(r, x)
    return r


def pairsum(a, k):
    while len(a) > 1:
        a = [rnd(fadd(a[i], a[i + 1]), k) for i in range(0, len(a), 2)]
    return a[0]


def clmul(a, b):
    r = 0
    for i in range(b.bit_length()):
        if (b >> i) & 1:
            r ^= a << i
    return r


def fmulx(a, b):
    if isnan(a) or isnan(b):
        return NAN
    if (math.isinf(a) and b == 0) or (math.isinf(b) and a == 0):
        return -2.0 if fsign(a) != fsign(b) else 2.0
    return a * b


def econs(k):
    """the constraint of an operand whose lane an instruction indexes: 16-bit lanes are in v0-v15"""
    return 'x' if eb(k) == 16 else 'w'


def mla_family():
    # ---- multiply-accumulate, not fused: a + b * c
    for (op, mi, mf, sg) in (('mla', 'mla', 'fadd', 1), ('mls', 'mls', 'fsub', -1)):
        for k in I8_32 + FLT:
            for q in (0, 1):
                T = vt(k, q)
                ps = [(T, 'a'), (T, 'b'), (T, 'c')]
                if isf(k):
                    if plain_q(k, q):
                        body = '{ %s __t = b * c; return a %s __t; }' % (T, '+' if sg > 0 else '-')
                    else:
                        body = vres(k, q, A('fmul %s, %s, %s\n%s %s, %s, %s' % (V(0, k, q), V(2, k, q), V(3, k, q), mf, V(0, k, q),
                                                                               V(1, k, q), V(0, k, q)),
                                            '=&w;w;w;w', [('a', 'v'), ('b', 'v'), ('c', 'v')], RES(q)))
                    m = (lambda a, b, c, sg=sg, k=k: [fadd(x, rnd(fmul(y, z), k)) if sg > 0 else fsub(x, rnd(fmul(y, z), k))
                                                      for (x, y, z) in zip(a, b, c)])
                else:
                    body = ('a %s b * c' % ('+' if sg > 0 else '-')) if plain_q(k, q) else mach3(mi, k, q, '-', tied=True)
                    m = (lambda a, b, c, sg=sg: [x + sg * y * z for (x, y, z) in zip(a, b, c)])
                fn('v%s%s_%s' % (op, Qs(q), k), T, ps, body, model=m)
        # (by a scalar, and by a lane)
        for k in ['s16', 's32', 'u16', 'u32', 'f32']:
            for q in (0, 1):
                T = vt(k, q)
                for (form, lq) in (('n', None), ('lane', 0), ('laneq', 1)):
                    if form == 'n':
                        ps = [(T, 'a'), (T, 'b'), (st(k), 'c')]
                        el = E(3, k, 0)
                        args = [('a', 'v'), ('b', 'v'), sarg(k, 'c')]
                        imm = None
                        ims = None
                    else:
                        ps = [(T, 'a'), (T, 'b'), (vt(k, lq), 'v'), ('const int', 'l')]
                        el = E(3, k, '%4')
                        args = [('a', 'v'), ('b', 'v'), ('v', 'v'), ('l', 'i')]
                        imm = [(0, nl(k, lq) - 1)]
                        ims = {'l': imm[0]}
                    if isf(k):
                        body = vres(k, q, A('fmul %s, %s, %s\n%s %s, %s, %s' % (V(0, k, q), V(2, k, q), el, mf, V(0, k, q),
                                                                               V(1, k, q), V(0, k, q)),
                                            '=&w;w;w;w' + (';i' if imm else ''), args, RES(q), imm=imm))
                    else:
                        body = vres(k, q, A('%s %s, %s, %s' % (mi, V(0, k, q), V(2, k, q), el),
                                            '=w;0;w;%s' % econs(k) + (';i' if imm else ''), args, RES(q), imm=imm))
                    if form == 'n':
                        m = (lambda a, b, c, sg=sg, f=isf(k), k=k: [(fadd(x, rnd(fmul(y, c), k)) if sg > 0 else
                                                                     fsub(x, rnd(fmul(y, c), k))) if f
                                                                    else x + sg * y * c for (x, y) in zip(a, b)])
                    else:
                        m = (lambda a, b, v, l, sg=sg, f=isf(k), k=k: [(fadd(x, rnd(fmul(y, v[l]), k)) if sg > 0 else
                                                                        fsub(x, rnd(fmul(y, v[l]), k)))
                                                                       if f else x + sg * y * v[l] for (x, y) in zip(a, b)])
                    fn('v%s%s_%s_%s' % (op, Qs(q), form, k), T, ps, body, model=m, imms=ims)
    # ---- fused multiply-add: a + b * c, a - b * c
    for (op, mn, sg) in (('fma', 'fmla', 1), ('fms', 'fmls', -1)):
        for k in FLTH:
            e = fext(k)
            for q in (0, 1):
                T = vt(k, q)
                if plain_q(k, q):
                    body = '__builtin_axiom_simd("fma", %sb, c, a)' % ('' if sg > 0 else '-')
                elif nl(k, q) == 1:
                    body = vres(k, q, A('%s %%d0, %%d2, %%d3, %%d1' % ('fmadd' if sg > 0 else 'fmsub'), '=w;w;w;w',
                                        [('a', 'v'), ('b', 'v'), ('c', 'v')], RES(q), e))
                else:
                    body = mach3(mn, k, q, e, tied=True)
                fn('v%s%s_%s' % (op, Qs(q), k), T, [(T, 'a'), (T, 'b'), (T, 'c')], body, e,
                   model=lambda a, b, c, sg=sg, k=k: [ffma(sg * y if not isnan(y) else y, z, x, k) for (x, y, z) in zip(a, b, c)])
                # (by a scalar, by a lane)
                for (form, lq) in (('n', None), ('lane', 0), ('laneq', 1)):
                    if form == 'n':
                        if op == 'fms' and k == 'f16':
                            pass
                        ps = [(T, 'a'), (T, 'b'), (st(k), 'c')]
                        args = [('a', 'v'), ('b', 'v'), sarg(k, 'c')]
                        el = E(3, k, 0)
                        imm = None
                        m = (lambda a, b, c, sg=sg, k=k: [ffma(sg * y if not isnan(y) else y, c, x, k) for (x, y) in zip(a, b)])
                    else:
                        ps = [(T, 'a'), (T, 'b'), (vt(k, lq), 'v'), ('const int', 'l')]
                        args = [('a', 'v'), ('b', 'v'), ('v', 'v'), ('l', 'i')]
                        el = E(3, k, '%4')
                        imm = [(0, nl(k, lq) - 1)]
                        m = (lambda a, b, v, l, sg=sg, k=k: [ffma(sg * y if not isnan(y) else y, v[l], x, k) for (x, y) in zip(a, b)])
                    if nl(k, q) == 1:
                        t = '%s %s, %s, %s' % ('fmadd' if sg > 0 else 'fmsub', SV(0, k), SV(2, k), SV(3, k)) if form == 'n' else \
                            '%s %s, %s, %s' % (mn, SV(0, k), SV(2, k), el)
                        if form == 'n':
                            t = '%s %s, %s, %s, %s' % ('fmadd' if sg > 0 else 'fmsub', SV(0, k), SV(2, k), SV(3, k), SV(1, k))
                            cons = '=w;w;w;w'
                        else:
                            cons = '=w;0;w;w;i'
                    else:
                        t = '%s %s, %s, %s' % (mn, V(0, k, q), V(2, k, q), el)
                        cons = '=w;0;w;%s' % econs(k) + (';i' if imm else '')
                    fn('v%s%s_%s_%s' % (op, Qs(q), form, k), T, ps, vres(k, q, A(t, cons, args, RES(q), e, imm=imm)), e,
                       model=m, imms={'l': imm[0]} if imm else None)
            # (scalars by a lane)
            for lq in (0, 1):
                ps = [(st(k), 'a'), (st(k), 'b'), (vt(k, lq), 'v'), ('const int', 'l')]
                imm = [(0, nl(k, lq) - 1)]
                body = sres(k, A('%s %s, %s, %s' % (mn, SV(0, k), SV(2, k), E(3, k, '%4')), '=w;0;w;%s;i' % econs(k),
                                 [sarg(k, 'a'), sarg(k, 'b'), ('v', 'v'), ('l', 'i')], scode(k), e, imm=imm))
                fn('v%s%s_lane%s_%s' % (op, ev(k), 'q' if lq else '', k), st(k), ps, body, e,
                   model=lambda a, b, v, l, sg=sg, k=k: ffma(sg * b if not isnan(b) else b, v[l], a, k), imms={'l': imm[0]})
        if True:
            k = 'f16'
            fn('v%sh_f16' % op, 'float16_t', [('float16_t', 'a'), ('float16_t', 'b'), ('float16_t', 'c')],
               sres(k, A('%s %s, %s, %s, %s' % ('fmadd' if sg > 0 else 'fmsub', SV(0, k), SV(2, k), SV(3, k), SV(1, k)),
                         '=w;w;w;w', [sarg(k, 'a'), sarg(k, 'b'), sarg(k, 'c')], 'u', 'fp16')), 'fp16',
               model=lambda a, b, c, sg=sg: ffma(sg * b if not isnan(b) else b, c, a, 'f16'))
    # ---- multiply by a scalar, by a lane
    for k in ['s16', 's32', 'u16', 'u32'] + FLTH:
        e = fext(k)
        mn = 'fmul' if isf(k) else 'mul'
        for q in (0, 1):
            T = vt(k, q)
            for (form, lq) in (('n', None), ('lane', 0), ('laneq', 1)):
                if form == 'n':
                    ps = [(T, 'a'), (st(k), 'b')]
                    args = [('a', 'v'), sarg(k, 'b')]
                    el = E(2, k, 0)
                    imm = None
                    m = (lambda a, b, f=isf(k): [fmul(x, b) if f else x * b for x in a])
                else:
                    ps = [(T, 'a'), (vt(k, lq), 'v'), ('const int', 'l')]
                    args = [('a', 'v'), ('v', 'v'), ('l', 'i')]
                    el = E(2, k, '%3')
                    imm = [(0, nl(k, lq) - 1)]
                    m = (lambda a, v, l, f=isf(k): [fmul(x, v[l]) if f else x * v[l] for x in a])
                if nl(k, q) == 1:
                    t = '%s %s, %s, %s' % (mn, SV(0, k), SV(1, k), el)
                else:
                    t = '%s %s, %s, %s' % (mn, V(0, k, q), V(1, k, q), el)
                cons = '=w;w;%s' % econs(k) + (';i' if imm else '')
                fn('vmul%s_%s_%s' % (Qs(q), form, k), T, ps, vres(k, q, A(t, cons, args, RES(q), e, imm=imm)), e,
                   model=m, imms={'l': imm[0]} if imm else None)
                if isf(k) and form != 'n' or (isf(k) and k == 'f16'):
                    if form == 'n' and k != 'f16':
                        continue
                    fn('vmulx%s_%s_%s' % (Qs(q), form, k), T, ps,
                       vres(k, q, A(t.replace(mn, 'fmulx', 1), cons, args, RES(q), e, imm=imm)), e,
                       model=(lambda a, b: [fmulx(x, b) for x in a]) if form == 'n' else
                       (lambda a, v, l: [fmulx(x, v[l]) for x in a]), imms={'l': imm[0]} if imm else None)
        if isf(k):
            for lq in (0, 1):
                imm = [(0, nl(k, lq) - 1)]
                for (op, mm, f) in (('mul', 'fmul', fmul), ('mulx', 'fmulx', fmulx)):
                    fn('v%s%s_lane%s_%s' % (op, ev(k), 'q' if lq else '', k), st(k),
                       [(st(k), 'a'), (vt(k, lq), 'v'), ('const int', 'l')],
                       sres(k, A('%s %s, %s, %s' % (mm, SV(0, k), SV(1, k), E(2, k, '%3')), '=w;w;%s;i' % econs(k),
                                 [sarg(k, 'a'), ('v', 'v'), ('l', 'i')], scode(k), e, imm=imm)), e,
                       model=lambda a, v, l, f=f: f(a, v[l]), imms={'l': imm[0]})
    # ---- saturating doubling multiplies (high halves)
    for (op, mn, rd) in (('qdmulh', 'sqdmulh', 0), ('qrdmulh', 'sqrdmulh', 1)):
        for k in ('s16', 's32'):
            w = eb(k)
            g = (lambda x, y, w=w, rd=rd, k=k: sat((2 * x * y + (rd << (w - 1))) >> w, k))
            for q in (0, 1):
                T = vt(k, q)
                fn('v%s%s_%s' % (op, Qs(q), k), T, [(T, 'a'), (T, 'b')], mach3(mn, k, q, '-'), model=L2(g))
                for (form, lq) in (('n', None), ('lane', 0), ('laneq', 1)):
                    if form == 'n':
                        ps = [(T, 'a'), (st(k), 'b')]
                        args = [('a', 'v'), sarg(k, 'b')]
                        el = E(2, k, 0)
                        imm = None
                        m = (lambda a, b, g=g: [g(x, b) for x in a])
                    else:
                        ps = [(T, 'a'), (vt(k, lq), 'v'), ('const int', 'l')]
                        args = [('a', 'v'), ('v', 'v'), ('l', 'i')]
                        el = E(2, k, '%3')
                        imm = [(0, nl(k, lq) - 1)]
                        m = (lambda a, v, l, g=g: [g(x, v[l]) for x in a])
                    fn('v%s%s_%s_%s' % (op, Qs(q), form, k), T, ps,
                       vres(k, q, A('%s %s, %s, %s' % (mn, V(0, k, q), V(1, k, q), el), '=w;w;%s' % econs(k) + (';i' if imm else ''),
                                    args, RES(q), imm=imm)), model=m, imms={'l': imm[0]} if imm else None)
            scalar2(op, k, mn, model=g)
            for lq in (0, 1):
                imm = [(0, nl(k, lq) - 1)]
                fn('v%s%s_lane%s_%s' % (op, ev(k), 'q' if lq else '', k), st(k), [(st(k), 'a'), (vt(k, lq), 'v'), ('const int', 'l')],
                   sres(k, A('%s %s, %s, %s' % (mn, SV(0, k), SV(1, k), E(2, k, '%3')), '=w;w;%s;i' % econs(k),
                             [sarg(k, 'a'), ('v', 'v'), ('l', 'i')], scode(k), imm=imm)),
                   model=lambda a, v, l, g=g: g(a, v[l]), imms={'l': imm[0]})
    # (ARMv8.1's rounding doubling multiply-accumulate: rdm)
    for (op, mn, sg) in (('qrdmlah', 'sqrdmlah', 1), ('qrdmlsh', 'sqrdmlsh', -1)):
        for k in ('s16', 's32'):
            w = eb(k)
            g = (lambda acc, x, y, w=w, sg=sg, k=k: sat(((acc << w) + sg * 2 * x * y + (1 << (w - 1))) >> w, k))
            for q in (0, 1):
                T = vt(k, q)
                fn('v%s%s_%s' % (op, Qs(q), k), T, [(T, 'a'), (T, 'b'), (T, 'c')], mach3(mn, k, q, 'rdm', tied=True), 'rdm',
                   model=L3(g))
                for lq in (0, 1):
                    imm = [(0, nl(k, lq) - 1)]
                    fn('v%s%s_lane%s_%s' % (op, Qs(q), 'q' if lq else '', k), T,
                       [(T, 'a'), (T, 'b'), (vt(k, lq), 'v'), ('const int', 'l')],
                       vres(k, q, A('%s %s, %s, %s' % (mn, V(0, k, q), V(2, k, q), E(3, k, '%4')), '=w;0;w;%s;i' % econs(k),
                                    [('a', 'v'), ('b', 'v'), ('v', 'v'), ('l', 'i')], RES(q), 'rdm', imm=imm)), 'rdm',
                       model=lambda a, b, v, l, g=g: [g(x, y, v[l]) for (x, y) in zip(a, b)], imms={'l': imm[0]})
            fn('v%s%s_%s' % (op, ev(k), k), st(k), [(st(k), 'a'), (st(k), 'b'), (st(k), 'c')],
               sres(k, A('%s %s, %s, %s' % (mn, SV(0, k), SV(2, k), SV(3, k)), '=w;0;w;w',
                         [sarg(k, 'a'), sarg(k, 'b'), sarg(k, 'c')], scode(k), 'rdm')), 'rdm', model=g)
            for lq in (0, 1):
                imm = [(0, nl(k, lq) - 1)]
                fn('v%s%s_lane%s_%s' % (op, ev(k), 'q' if lq else '', k), st(k),
                   [(st(k), 'a'), (st(k), 'b'), (vt(k, lq), 'v'), ('const int', 'l')],
                   sres(k, A('%s %s, %s, %s' % (mn, SV(0, k), SV(2, k), E(3, k, '%4')), '=w;0;w;%s;i' % econs(k),
                             [sarg(k, 'a'), sarg(k, 'b'), ('v', 'v'), ('l', 'i')], scode(k), 'rdm', imm=imm)), 'rdm',
                   model=lambda a, b, v, l, g=g: g(a, b, v[l]), imms={'l': imm[0]})


def hi_half(a):
    return a[len(a) // 2:]


def widen_family():
    for k in I8_32:
        w = wk(k)
        p = pre(k)
        # (long, wide: on the low halves (D), and on the high halves (_high, Q))
        for (op, mn, f) in (('addl', 'addl', lambda x, y: x + y), ('subl', 'subl', lambda x, y: x - y),
                            ('abdl', 'abdl', lambda x, y: abs(x - y)), ('mull', 'mull', lambda x, y: x * y)):
            for hi in (0, 1):
                fn('v%s%s_%s' % (op, '_high' if hi else '', k), vt(w, 1), [vec(k, hi), vec(k, hi, 'b')],
                   vres(w, 1, A('%s%s%s %s, %s, %s' % (p, mn, '2' if hi else '', V(0, w, 1), V(1, k, hi), V(2, k, hi)), '=w;w;w',
                                [('a', 'v'), ('b', 'v')], 'Q')),
                   model=(lambda a, b, f=f, hi=hi: [f(x, y) for (x, y) in zip(hi_half(a) if hi else a, hi_half(b) if hi else b)]))
        for (op, mn, sg) in (('addw', 'addw', 1), ('subw', 'subw', -1)):
            for hi in (0, 1):
                fn('v%s%s_%s' % (op, '_high' if hi else '', k), vt(w, 1), [vec(w, 1), vec(k, hi, 'b')],
                   vres(w, 1, A('%s%s%s %s, %s, %s' % (p, mn, '2' if hi else '', V(0, w, 1), V(1, w, 1), V(2, k, hi)), '=w;w;w',
                                [('a', 'v'), ('b', 'v')], 'Q')),
                   model=(lambda a, b, sg=sg, hi=hi: [x + sg * y for (x, y) in zip(a, hi_half(b) if hi else b)]))
        for (op, mn, f) in (('abal', 'abal', lambda acc, x, y: acc + abs(x - y)), ('mlal', 'mlal', lambda acc, x, y: acc + x * y),
                            ('mlsl', 'mlsl', lambda acc, x, y: acc - x * y)):
            for hi in (0, 1):
                fn('v%s%s_%s' % (op, '_high' if hi else '', k), vt(w, 1), [vec(w, 1), vec(k, hi, 'b'), vec(k, hi, 'c')],
                   vres(w, 1, A('%s%s%s %s, %s, %s' % (p, mn, '2' if hi else '', V(0, w, 1), V(2, k, hi), V(3, k, hi)),
                                '=w;0;w;w', [('a', 'v'), ('b', 'v'), ('c', 'v')], 'Q')),
                   model=(lambda a, b, c, f=f, hi=hi: [f(z, x, y) for (z, x, y) in zip(a, hi_half(b) if hi else b, hi_half(c) if hi else c)]))
        for q in (0, 1):
            fn('vaba%s_%s' % (Qs(q), k), vt(k, q), [vec(k, q), vec(k, q, 'b'), vec(k, q, 'c')],
               mach3(p + 'aba', k, q, '-', tied=True), model=L3(lambda acc, x, y: acc + abs(x - y)))
        # (by a scalar and by a lane: the 16- and 32-bit kinds)
        if eb(k) >= 16:
            for (op, mn, tied, f) in (('mull', 'mull', 0, lambda acc, x, y: x * y), ('mlal', 'mlal', 1, lambda acc, x, y: acc + x * y),
                                      ('mlsl', 'mlsl', 1, lambda acc, x, y: acc - x * y)):
                for hi in (0, 1):
                    for (form, lq) in (('n', None), ('lane', 0), ('laneq', 1)):
                        ps = ([vec(w, 1)] if tied else []) + [vec(k, hi, 'b')]
                        args = ([('a', 'v')] if tied else []) + [('b', 'v')]
                        j = len(ps) + 1
                        if form == 'n':
                            ps.append((st(k), 'c'))
                            args.append(sarg(k, 'c'))
                            el = E(j, k, 0)
                            imm = None
                        else:
                            ps += [(vt(k, lq), 'v'), ('const int', 'l')]
                            args += [('v', 'v'), ('l', 'i')]
                            el = E(j, k, '%%%d' % (j + 1))
                            imm = [(0, nl(k, lq) - 1)]
                        t = '%s%s%s %s, %s, %s' % (p, mn, '2' if hi else '', V(0, w, 1), V(j - 1, k, hi), el)
                        cons = ('=w;0;w;' if tied else '=w;w;') + econs(k) + (';i' if imm else '')
                        nm = 'v%s%s_%s_%s' % (op, '_high' if hi else '', form, k)
                        if tied:
                            m = (lambda a, b, c, *r, f=f, hi=hi, form=form: [f(z, x, c if form == 'n' else c[r[0]])
                                                                             for (z, x) in zip(a, hi_half(b) if hi else b)])
                        else:
                            m = (lambda b, c, *r, hi=hi, form=form: [x * (c if form == 'n' else c[r[0]]) for x in (hi_half(b) if hi else b)])
                        fn(nm, vt(w, 1), ps, vres(w, 1, A(t, cons, args, 'Q', imm=imm)), model=m,
                           imms={'l': imm[0]} if imm else None)
    # polynomial multiply long
    for hi in (0, 1):
        fn('vmull%s_p8' % ('_high' if hi else ''), vt('p16', 1), [vec('p8', hi), vec('p8', hi, 'b')],
           vres('p16', 1, A('pmull%s %s, %s, %s' % ('2' if hi else '', V(0, 'p16', 1), V(1, 'p8', hi), V(2, 'p8', hi)), '=w;w;w',
                            [('a', 'v'), ('b', 'v')], 'Q')),
           model=lambda a, b, hi=hi: [clmul(x, y) for (x, y) in zip(hi_half(a) if hi else a, hi_half(b) if hi else b)])
    fn('vmull_p64', 'poly128_t', [('poly64_t', 'a'), ('poly64_t', 'b')],
       '__axiom_q2p128(%s)' % A('pmull %v0.1q, %v1.1d, %v2.1d', '=w;w;w', [('a', 'n'), ('b', 'n')], 'Q', 'aes'), 'aes',
       model=lambda a, b: clmul(a, b))
    fn('vmull_high_p64', 'poly128_t', [vec('p64', 1), vec('p64', 1, 'b')],
       '__axiom_q2p128(%s)' % A('pmull2 %v0.1q, %v1.2d, %v2.2d', '=w;w;w', [('a', 'v'), ('b', 'v')], 'Q', 'aes'), 'aes',
       model=lambda a, b: clmul(a[1], b[1]))
    # ---- saturating doubling multiply long, and accumulate
    for k in ('s16', 's32'):
        w = wk(k)
        dm = (lambda x, y, w=w: sat(2 * x * y, w))
        for (op, mn, tied, f) in (('qdmull', 'sqdmull', 0, lambda acc, x, y, dm=dm: dm(x, y)),
                                  ('qdmlal', 'sqdmlal', 1, lambda acc, x, y, dm=dm, w=w: sat(acc + dm(x, y), w)),
                                  ('qdmlsl', 'sqdmlsl', 1, lambda acc, x, y, dm=dm, w=w: sat(acc - dm(x, y), w))):
            for hi in (0, 1):
                for form in ('', 'n', 'lane', 'laneq'):
                    ps = ([vec(w, 1)] if tied else []) + [vec(k, hi, 'b')]
                    args = ([('a', 'v')] if tied else []) + [('b', 'v')]
                    j = len(ps) + 1
                    imm = None
                    if form == '':
                        ps.append(vec(k, hi, 'c'))
                        args.append(('c', 'v'))
                        el = V(j, k, hi)
                    elif form == 'n':
                        ps.append((st(k), 'c'))
                        args.append(sarg(k, 'c'))
                        el = E(j, k, 0)
                    else:
                        lq = form == 'laneq'
                        ps += [(vt(k, lq), 'c'), ('const int', 'l')]
                        args += [('c', 'v'), ('l', 'i')]
                        el = E(j, k, '%%%d' % (j + 1))
                        imm = [(0, nl(k, lq) - 1)]
                    t = '%s%s %s, %s, %s' % (mn, '2' if hi else '', V(0, w, 1), V(j - 1, k, hi), el)
                    cons = ('=w;0;w;' if tied else '=w;w;') + (econs(k) if form not in ('',) else 'w') + (';i' if imm else '')
                    nm = 'v%s%s%s_%s' % (op, '_high' if hi else '', '_' + form if form else '', k)

                    def m(*xs, f=f, hi=hi, form=form, tied=tied):
                        xs = list(xs)
                        acc = xs.pop(0) if tied else None
                        b = hi_half(xs[0]) if hi else xs[0]
                        c = xs[1]
                        if form == '':
                            c = hi_half(c) if hi else c
                            return [f(acc[i] if tied else 0, b[i], c[i]) for i in range(len(b))]
                        if form != 'n':
                            c = c[xs[2]]
                        return [f(acc[i] if tied else 0, b[i], c) for i in range(len(b))]
                    fn(nm, vt(w, 1), ps, vres(w, 1, A(t, cons, args, 'Q', imm=imm)), model=m,
                       imms={'l': imm[0]} if imm else None)
            # (scalars)
            ps = ([(st(w), 'a')] if tied else []) + [(st(k), 'b'), (st(k), 'c')]
            args = ([sarg(w, 'a')] if tied else []) + [sarg(k, 'b'), sarg(k, 'c')]
            j = len(ps) - 1
            fn('v%s%s_%s' % (op, ev(k), k), st(w), ps,
               sres(w, A('%s %s, %s, %s' % (mn, SV(0, w), SV(j, k), SV(j + 1, k)), '=w;0;w;w' if tied else '=w;w;w', args, scode(w))),
               model=(lambda a, b, c, f=f: f(a, b, c)) if tied else (lambda b, c, f=f: f(0, b, c)))
            for lq in (0, 1):
                ps = ([(st(w), 'a')] if tied else []) + [(st(k), 'b'), (vt(k, lq), 'v'), ('const int', 'l')]
                args = ([sarg(w, 'a')] if tied else []) + [sarg(k, 'b'), ('v', 'v'), ('l', 'i')]
                j = len(ps) - 2
                imm = [(0, nl(k, lq) - 1)]
                fn('v%s%s_lane%s_%s' % (op, ev(k), 'q' if lq else '', k), st(w), ps,
                   sres(w, A('%s %s, %s, %s' % (mn, SV(0, w), SV(j, k), E(j + 1, k, '%%%d' % (j + 2))),
                             ('=w;0;w;' if tied else '=w;w;') + econs(k) + ';i', args, scode(w), imm=imm)),
                   model=(lambda a, b, v, l, f=f: f(a, b, v[l])) if tied else (lambda b, v, l, f=f: f(0, b, v[l])),
                   imms={'l': imm[0]})
    # ---- narrowing: high halves of sums, moves, saturating moves
    for k in ('s16', 's32', 's64', 'u16', 'u32', 'u64'):
        n = nk(k)
        h = eb(n)
        for (op, mn, f) in (('addhn', 'addhn', lambda x, y, h=h: (x + y) >> h),
                            ('raddhn', 'raddhn', lambda x, y, h=h: (x + y + (1 << (h - 1))) >> h),
                            ('subhn', 'subhn', lambda x, y, h=h: (x - y) >> h),
                            ('rsubhn', 'rsubhn', lambda x, y, h=h: (x - y + (1 << (h - 1))) >> h)):
            fn('v%s_%s' % (op, k), vt(n, 0), [vec(k, 1), vec(k, 1, 'b')],
               vres(n, 0, A('%s %s, %s, %s' % (mn, V(0, n, 0), V(1, k, 1), V(2, k, 1)), '=w;w;w', [('a', 'v'), ('b', 'v')], 'D')),
               model=L2(f))
            fn('v%s_high_%s' % (op, k), vt(n, 1), [vec(n, 0, 'r'), vec(k, 1), vec(k, 1, 'b')],
               vres(n, 1, A('%s2 %s, %s, %s' % (mn, V(0, n, 1), V(2, k, 1), V(3, k, 1)), '=w;0;w;w',
                            [('r', 'v'), ('a', 'v'), ('b', 'v')], 'Q')),
               model=lambda r, a, b, f=f: r + [f(x, y) for (x, y) in zip(a, b)])
        for (op, mn, rk, f) in (('movn', 'xtn', n, lambda x: x), ('qmovn', pre(k) + 'qxtn', n, None),
                                ('qmovun', 'sqxtun', uk(n), None)):
            if op == 'qmovun' and k[0] == 'u':
                continue
            g = f or (lambda x, rk=rk: sat(x, rk))
            fn('v%s_%s' % (op, k), vt(rk, 0), [vec(k, 1)], mach2(mn, k, 1, '-', rk=rk, rq=0), model=L1(g))
            fn('v%s_high_%s' % (op, k), vt(rk, 1), [vec(rk, 0, 'r'), vec(k, 1)],
               vres(rk, 1, A('%s2 %s, %s' % (mn, V(0, rk, 1), V(2, k, 1)), '=w;0;w', [('r', 'v'), ('a', 'v')], 'Q')),
               model=lambda r, a, g=g: r + [g(x) for x in a])
            if op != 'movn':
                fn('v%s%s_%s' % (op, ev(k), k), st(rk), [(st(k), 'a')],
                   sres(rk, A('%s %s, %s' % (mn, SV(0, rk), SV(1, k)), '=w;w', [sarg(k, 'a')], scode(rk))), model=g)
    # ---- widening moves
    for k in I8_32:
        w = wk(k)
        fn('vmovl_%s' % k, vt(w, 1), [vec(k, 0)], mach2(pre(k) + 'xtl', k, 0, '-', rk=w, rq=1), model=lambda a: a)
        fn('vmovl_high_%s' % k, vt(w, 1), [vec(k, 1)], cast(vt(w, 1), '__builtin_axiom_simd("widen_hi", a)'),
           model=lambda a: hi_half(a))


def sshl_model(k, rnd_=False, sat_=False):
    """SSHL, SRSHL, SQSHL, SQRSHL (and the unsigned forms): a by the signed low byte of the count"""
    w = eb(k)

    def f(x, c):
        s = wrap(c, 's8')
        if s >= 0:
            v = x << s if s < 200 else (x << 200)
        else:
            v = x
            if rnd_:
                v += 1 << (-s - 1)
            v >>= -s
        return sat(v, k) if sat_ else wrap(v, k)
    return f


def compares():
    rels = (('eq', 'cmeq', 'cmeq', 'fcmeq', lambda x, y: x == y),
            ('ge', 'cmge', 'cmhs', 'fcmge', lambda x, y: x >= y),
            ('gt', 'cmgt', 'cmhi', 'fcmgt', lambda x, y: x > y),
            ('le', 'cmge', 'cmhs', 'fcmge', lambda x, y: x <= y),
            ('lt', 'cmgt', 'cmhi', 'fcmgt', lambda x, y: x < y))
    cop = {'eq': '==', 'ge': '>=', 'gt': '>', 'le': '<=', 'lt': '<'}
    for (r, ms, mu, mf, f) in rels:
        swap = r in ('le', 'lt')
        kinds = INTS + FLTH + (['p8', 'p64'] if r == 'eq' else [])
        for k in kinds:
            u = uk(k)
            e = fext(k)
            mn = mf if isf(k) else (mu if k[0] in 'up' else ms)
            ones = (1 << eb(k)) - 1
            g = (lambda x, y, f=f, ones=ones: ones if (not isnan(x) and not isnan(y) and f(x, y)) else 0)
            for q in (0, 1):
                T = vt(k, q)
                if plain_q(k, q):
                    body = cast(vt(u, q), '(a %s b)' % cop[r])
                else:
                    o1, o2 = ('%v2', '%v1') if swap else ('%v1', '%v2')
                    body = vres(u, q, A('%s %s, %s.%s, %s.%s' % (mn, V(0, u, q), o1, ar(k, q), o2, ar(k, q)), '=w;w;w',
                                        [('a', 'v'), ('b', 'v')], RES(q), e))
                fn('vc%s%s_%s' % (r, Qs(q), k), vt(u, q), [(T, 'a'), (T, 'b')], body, e, model=L2(g))
                # (against zero)
                if k[0] == 'u' and r != 'eq' or (k[0] == 'p' and r != 'eq'):
                    continue
                if isf(k):
                    zt = '%s %s, %s, #0.0' % ({'le': 'fcmle', 'lt': 'fcmlt'}.get(r, mf), V(0, u, q), V(1, k, q))
                else:
                    zt = '%s %s, %s, #0' % ({'le': 'cmle', 'lt': 'cmlt'}.get(r, ms), V(0, u, q), V(1, k, q))
                fn('vc%sz%s_%s' % (r, Qs(q), k), vt(u, q), [(T, 'a')], vres(u, q, A(zt, '=w;w', [('a', 'v')], RES(q), e)), e,
                   model=L1(lambda x, g=g: g(x, 0)))
            if k in ('s64', 'u64', 'f32', 'f64', 'f16'):
                o1, o2 = (2, 1) if swap else (1, 2)
                fn('vc%s%s_%s' % (r, ev(k), k), st(u), [(st(k), 'a'), (st(k), 'b')],
                   sres(u, A('%s %s, %s, %s' % (mn, SV(0, u), SV(o1, k), SV(o2, k)), '=w;w;w', [sarg(k, 'a'), sarg(k, 'b')],
                             scode(u), e)), e, model=g)
                if not (k[0] == 'u' and r != 'eq'):
                    if isf(k):
                        zt = '%s %s, %s, #0.0' % ({'le': 'fcmle', 'lt': 'fcmlt'}.get(r, mf), SV(0, u), SV(1, k))
                    else:
                        zt = '%s %s, %s, #0' % ({'le': 'cmle', 'lt': 'cmlt'}.get(r, ms), SV(0, u), SV(1, k))
                    fn('vc%sz%s_%s' % (r, ev(k), k), st(u), [(st(k), 'a')], sres(u, A(zt, '=w;w', [sarg(k, 'a')], scode(u), e)),
                       e, model=lambda x, g=g: g(x, 0))
    # absolute compares
    for (r, mn, swap, f) in (('ge', 'facge', 0, lambda x, y: x >= y), ('gt', 'facgt', 0, lambda x, y: x > y),
                             ('le', 'facge', 1, lambda x, y: x <= y), ('lt', 'facgt', 1, lambda x, y: x < y)):
        for k in FLTH:
            u = uk(k)
            e = fext(k)
            ones = (1 << eb(k)) - 1
            g = (lambda x, y, f=f, ones=ones: ones if (not isnan(x) and not isnan(y) and f(abs(x), abs(y))) else 0)
            o1, o2 = (2, 1) if swap else (1, 2)
            for q in (0, 1):
                fn('vca%s%s_%s' % (r, Qs(q), k), vt(u, q), [vec(k, q), vec(k, q, 'b')],
                   vres(u, q, A('%s %s, %%v%d.%s, %%v%d.%s' % (mn, V(0, u, q), o1, ar(k, q), o2, ar(k, q)), '=w;w;w',
                                [('a', 'v'), ('b', 'v')], RES(q), e)), e, model=L2(g))
            fn('vca%s%s_%s' % (r, ev(k), k), st(u), [(st(k), 'a'), (st(k), 'b')],
               sres(u, A('%s %s, %s, %s' % (mn, SV(0, u), SV(o1, k), SV(o2, k)), '=w;w;w', [sarg(k, 'a'), sarg(k, 'b')],
                         scode(u), e)), e, model=g)
    # bit tests
    for k in INTS + ['p8', 'p64']:
        u = uk(k)
        ones = (1 << eb(k)) - 1
        g = (lambda x, y, ones=ones: ones if (x & y) else 0)
        for q in (0, 1):
            fn('vtst%s_%s' % (Qs(q), k), vt(u, q), [vec(k, q), vec(k, q, 'b')], mach3('cmtst', k, q, '-', rk=u), model=L2(g))
        if eb(k) == 64 and k[0] != 'p':
            fn('vtstd_%s' % k, st(u), [(st(k), 'a'), (st(k), 'b')],
               sres(u, A('cmtst %d0, %d1, %d2', '=w;w;w', [sarg(k, 'a'), sarg(k, 'b')], scode(u))), model=g)


def shifts():
    # ---- by a vector of signed counts
    for (op, mn, r_, s_) in (('shl', 'shl', 0, 0), ('rshl', 'rshl', 1, 0), ('qshl', 'qshl', 0, 1), ('qrshl', 'qrshl', 1, 1)):
        for k in INTS:
            s = sk(k)
            m = ('sq' if k[0] == 's' else 'uq') + mn[1:] if s_ else pre(k) + mn
            for q in (0, 1):
                fn('v%s%s_%s' % (op, Qs(q), k), vt(k, q), [vec(k, q), vec(s, q, 'b')], mach3(m, k, q, '-'),
                   model=lambda a, b, k=k, r_=r_, s_=s_: [sshl_model(k, r_, s_)(x, y) for (x, y) in zip(a, b)])
            if s_ or eb(k) == 64:
                fn('v%s%s_%s' % (op, ev(k), k), st(k), [(st(k), 'a'), (st(s), 'b')],
                   sres(k, A('%s %s, %s, %s' % (m, SV(0, k), SV(1, k), SV(2, k)), '=w;w;w', [sarg(k, 'a'), sarg(s, 'b')], scode(k))),
                   model=lambda a, b, k=k, r_=r_, s_=s_: sshl_model(k, r_, s_)(a, b))
    # ---- by an immediate
    def imm_family(op, kinds, rng, mnf, model, tied=False, scal='d', rkf=None, plainf=None):
        for k in kinds:
            lo, hi = rng(k)
            rk = rkf(k) if rkf else k
            mn = mnf(k)
            for q in (0, 1):
                T = vt(k, q)
                ps = ([(vt(rk, q), 'a')] if tied else []) + [(T, 'b' if tied else 'a'), ('const int', 'n')]
                if plainf and plain_q(k, q):
                    body = plainf(k)
                elif tied:
                    body = vres(rk, q, A('%s %s, %s, #%%3' % (mn, V(0, rk, q), V(2, k, q)), '=w;0;w;i',
                                         [('a', 'v'), ('b', 'v'), ('n', 'i')], RES(q), imm=[(lo, hi)]))
                else:
                    body = vres(rk, q, A('%s %s, %s, #%%2' % (mn, V(0, rk, q), V(1, k, q)), '=w;w;i', [('a', 'v'), ('n', 'i')],
                                         RES(q), imm=[(lo, hi)]))
                fn('v%s%s_n_%s' % (op, Qs(q), k), vt(rk, q), ps, body, model=model(k), imms={'n': (lo, hi)})
            if scal and (eb(k) == 64 or scal == 'all') and k[0] != 'p':
                ps = ([(st(rk), 'a')] if tied else []) + [(st(k), 'b' if tied else 'a'), ('const int', 'n')]
                if tied:
                    body = sres(rk, A('%s %s, %s, #%%3' % (mn, SV(0, rk), SV(2, k)), '=w;0;w;i',
                                      [sarg(rk, 'a'), sarg(k, 'b'), ('n', 'i')], scode(rk), imm=[(lo, hi)]))
                    mm = (lambda a, b, n, f=model(k): f([a], [b], n)[0])
                else:
                    body = sres(rk, A('%s %s, %s, #%%2' % (mn, SV(0, rk), SV(1, k)), '=w;w;i', [sarg(k, 'a'), ('n', 'i')],
                                      scode(rk), imm=[(lo, hi)]))
                    mm = (lambda a, n, f=model(k): f([a], n)[0])
                fn('v%s%s_n_%s' % (op, ev(k), k), st(rk), ps, body, model=mm, imms={'n': (lo, hi)})

    W = (lambda k: (0, eb(k) - 1))
    R = (lambda k: (1, eb(k)))
    imm_family('shl', INTS, W, lambda k: 'shl', lambda k: lambda a, n: [x << n for x in a], plainf=lambda k: 'a << n')
    imm_family('shr', INTS, R, lambda k: pre(k) + 'shr', lambda k: lambda a, n: [x >> n for x in a],
               plainf=lambda k: '__builtin_axiom_simd("%s", a, n)' % ('srai' if k[0] == 's' else 'srli'))
    imm_family('rshr', INTS, R, lambda k: pre(k) + 'rshr', lambda k: lambda a, n: [(x + (1 << (n - 1))) >> n for x in a])
    imm_family('sra', INTS, R, lambda k: pre(k) + 'sra', lambda k: lambda a, b, n: [y + (x >> n) for (y, x) in zip(a, b)],
               tied=True)
    imm_family('rsra', INTS, R, lambda k: pre(k) + 'rsra',
               lambda k: lambda a, b, n: [y + ((x + (1 << (n - 1))) >> n) for (y, x) in zip(a, b)], tied=True)

    def sri(k):
        w = eb(k)
        return lambda a, b, n: [(ubits(y, w) & ~(((1 << w) - 1) >> n)) | (ubits(x, w) >> n) for (y, x) in zip(a, b)]

    def sli(k):
        w = eb(k)
        return lambda a, b, n: [(ubits(y, w) & ((1 << n) - 1)) | ((ubits(x, w) << n) & ((1 << w) - 1)) for (y, x) in zip(a, b)]
    imm_family('sri', INTS + POLY, R, lambda k: 'sri', sri, tied=True)
    imm_family('sli', INTS + POLY, W, lambda k: 'sli', sli, tied=True)
    imm_family('qshl', INTS, W, lambda k: pre(k) + 'qshl', lambda k: lambda a, n: [sat(x << n, k) for x in a], scal='all')
    imm_family('qshlu', SINT, W, lambda k: 'sqshlu', lambda k: lambda a, n: [sat(x << n, uk(k)) for x in a], scal='all',
               rkf=uk)
    # (long, narrowing)
    for k in I8_32:
        w = wk(k)
        for hi in (0, 1):
            nm = 'vshll%s_n_%s' % ('_high' if hi else '', k)
            body = None
            # (#width is shll's, below it sshll's or ushll's: the count is known only as a constant, so a macro chooses)
            t1 = '%sshll%s %s, %s, #%%2' % (pre(k), '2' if hi else '', V(0, w, 1), V(1, k, hi))
            t2 = 'shll%s %s, %s, #%d' % ('2' if hi else '', V(0, w, 1), V(1, k, hi), eb(k))
            a1 = A(t1, '=w;w;i', [('a', 'v'), ('n', 'i')], 'Q', imm=[(0, eb(k) - 1)])
            a2 = A(t2, '=w;w', [('a', 'v')], 'Q')
            a1 = a1.replace(', n)', ', (n) & %d)' % (eb(k) - 1))
            body = cast(vt(w, 1), '((n) == %d ? %s : %s)' % (eb(k), a2, a1))
            fn(nm, vt(w, 1), [vec(k, hi), ('const int', 'n')], body, model=lambda a, n, hi=hi: [x << n for x in (hi_half(a) if hi else a)],
               imms={'n': (0, eb(k))})
    for k in ('s16', 's32', 's64', 'u16', 'u32', 'u64'):
        n_ = nk(k)
        h = eb(n_)
        fams = [('shrn', 'shrn', n_, lambda x, s: x >> s), ('rshrn', 'rshrn', n_, lambda x, s: (x + (1 << (s - 1))) >> s),
                ('qshrn', pre(k) + 'qshrn', n_, lambda x, s, rk=n_: sat(x >> s, rk)),
                ('qrshrn', pre(k) + 'qrshrn', n_, lambda x, s, rk=n_: sat((x + (1 << (s - 1))) >> s, rk))]
        if k[0] == 's':
            fams += [('qshrun', 'sqshrun', uk(n_), lambda x, s, rk=uk(n_): sat(x >> s, rk)),
                     ('qrshrun', 'sqrshrun', uk(n_), lambda x, s, rk=uk(n_): sat((x + (1 << (s - 1))) >> s, rk))]
        for (op, mn, rk, f) in fams:
            fn('v%s_n_%s' % (op, k), vt(rk, 0), [vec(k, 1), ('const int', 'n')],
               vres(rk, 0, A('%s %s, %s, #%%2' % (mn, V(0, rk, 0), V(1, k, 1)), '=w;w;i', [('a', 'v'), ('n', 'i')], 'D',
                             imm=[(1, h)])), model=lambda a, n, f=f: [f(x, n) for x in a], imms={'n': (1, h)})
            fn('v%s_high_n_%s' % (op, k), vt(rk, 1), [vec(rk, 0, 'r'), vec(k, 1), ('const int', 'n')],
               vres(rk, 1, A('%s2 %s, %s, #%%3' % (mn, V(0, rk, 1), V(2, k, 1)), '=w;0;w;i', [('r', 'v'), ('a', 'v'), ('n', 'i')],
                             'Q', imm=[(1, h)])), model=lambda r, a, n, f=f: r + [f(x, n) for x in a], imms={'n': (1, h)})
            if op not in ('shrn', 'rshrn'):
                fn('v%s%s_n_%s' % (op, ev(k), k), st(rk), [(st(k), 'a'), ('const int', 'n')],
                   sres(rk, A('%s %s, %s, #%%2' % (mn, SV(0, rk), SV(1, k)), '=w;w;i', [sarg(k, 'a'), ('n', 'i')], scode(rk),
                              imm=[(1, h)])), model=lambda a, n, f=f: f(a, n), imms={'n': (1, h)})


def cls_(x, w):
    x = ubits(x, w)
    s = x >> (w - 1)
    n = 0
    for i in range(w - 2, -1, -1):
        if ((x >> i) & 1) == s:
            n += 1
        else:
            break
    return n


def clz_(x, w):
    x = ubits(x, w)
    return w - x.bit_length()


def rbit_(x, w):
    x = ubits(x, w)
    return int('{:0{w}b}'.format(x, w=w)[::-1], 2)


def logic_unary():
    for (op, mn, c, f) in (('and', 'and', 'a & b', lambda x, y: x & y), ('orr', 'orr', 'a | b', lambda x, y: x | y),
                           ('eor', 'eor', 'a ^ b', lambda x, y: x ^ y), ('bic', 'bic', 'a & ~b', lambda x, y: x & ~y),
                           ('orn', 'orn', 'a | ~b', lambda x, y: x | ~y)):
        for k in INTS:
            for q in (0, 1):
                T = vt(k, q)
                body = c if plain_q(k, q) else vres(k, q, A('%s %s, %s, %s' % (mn, '%%v0.%s' % ('16b' if q else '8b'),
                                                                               '%%v1.%s' % ('16b' if q else '8b'),
                                                                               '%%v2.%s' % ('16b' if q else '8b')),
                                                            '=w;w;w', [('a', 'v'), ('b', 'v')], RES(q)))
                fn('v%s%s_%s' % (op, Qs(q), k), T, [(T, 'a'), (T, 'b')], body, model=L2(f))
    for k in I8_32 + ['p8']:
        for q in (0, 1):
            T = vt(k, q)
            body = '~a' if plain_q(k, q) else vres(k, q, A('mvn %s, %s' % ('%%v0.%s' % ('16b' if q else '8b'),
                                                                          '%%v1.%s' % ('16b' if q else '8b')),
                                                           '=w;w', [('a', 'v')], RES(q)))
            fn('vmvn%s_%s' % (Qs(q), k), T, [(T, 'a')], body, model=L1(lambda x: ~x), raw=True)
    for k in INTS + FLTH + POLY:
        u = uk(k)
        for q in (0, 1):
            T = vt(k, q)
            b = '16b' if q else '8b'
            fn('vbsl%s_%s' % (Qs(q), k), T, [(vt(u, q), 'a'), (T, 'b'), (T, 'c')],
               vres(k, q, A('bsl %%v0.%s, %%v2.%s, %%v3.%s' % (b, b, b), '=w;0;w;w', [('a', 'v'), ('b', 'v'), ('c', 'v')], RES(q))),
               dext(k), model=lambda a, b, c: [(m & x) | (~m & y) for (m, x, y) in zip(a, b, c)], raw=True)
    # ---- abs, neg (wrapping), and saturating
    for k in SINT + FLTH:
        for q in (0, 1):
            T = vt(k, q)
            e = fext(k)
            fn('vabs%s_%s' % (Qs(q), k), T, [(T, 'a')],
               '__builtin_axiom_simd("abs", a)' if plain_q(k, q) and not isf(k) else mach2('fabs' if isf(k) else 'abs', k, q, e), e,
               model=L1(fabs_ if isf(k) else abs))
            fn('vneg%s_%s' % (Qs(q), k), T, [(T, 'a')], '-a' if plain_q(k, q) else mach2('fneg' if isf(k) else 'neg', k, q, e),
               e, model=L1((lambda x: x if isnan(x) else -x) if isf(k) else (lambda x: -x)))
            if not isf(k):
                fn('vqabs%s_%s' % (Qs(q), k), T, [(T, 'a')], mach2('sqabs', k, q, e), model=L1(lambda x, k=k: sat(abs(x), k)))
                fn('vqneg%s_%s' % (Qs(q), k), T, [(T, 'a')], mach2('sqneg', k, q, e), model=L1(lambda x, k=k: sat(-x, k)))
        if not isf(k):
            scalar1('qabs', k, 'sqabs', model=lambda x, k=k: sat(abs(x), k))
            scalar1('qneg', k, 'sqneg', model=lambda x, k=k: sat(-x, k))
    fn('vabsd_s64', 'int64_t', [('int64_t', 'a')], sres('s64', A('abs %d0, %d1', '=w;w', [sarg('s64', 'a')], 'l')),
       model=lambda x: abs(x))
    fn('vnegd_s64', 'int64_t', [('int64_t', 'a')], sres('s64', A('neg %d0, %d1', '=w;w', [sarg('s64', 'a')], 'l')),
       model=lambda x: -x)
    for k in ('f16',):
        scalar1('abs', k, 'fabs', model=fabs_)
        scalar1('neg', k, 'fneg', model=lambda x: x if isnan(x) else -x)
    # ---- counts, bit reversal, element reversal
    for k in I8_32:
        w = eb(k)
        for q in (0, 1):
            fn('vcls%s_%s' % (Qs(q), k), vt(sk(k), q), [vec(k, q)], mach2('cls', sk(k), q, '-', rk=sk(k)) if k[0] == 's' else
               vres(sk(k), q, A('cls %s, %s' % (V(0, sk(k), q), V(1, sk(k), q)), '=w;w', [('a', 'v')], RES(q))),
               model=L1(lambda x, w=w: cls_(x, w)))
            fn('vclz%s_%s' % (Qs(q), k), vt(k, q), [vec(k, q)], mach2('clz', k, q, '-'), model=L1(lambda x, w=w: clz_(x, w)))
    for k in ('s8', 'u8', 'p8'):
        for q in (0, 1):
            fn('vcnt%s_%s' % (Qs(q), k), vt(k, q), [vec(k, q)], mach2('cnt', k, q, '-'),
               model=L1(lambda x: bin(ubits(x, 8)).count('1')))
            fn('vrbit%s_%s' % (Qs(q), k), vt(k, q), [vec(k, q)], mach2('rbit', k, q, '-'), model=L1(lambda x: rbit_(x, 8)))
    for (op, c, kinds) in (('rev16', 16, ['s8', 'u8', 'p8']), ('rev32', 32, ['s8', 's16', 'u8', 'u16', 'p8', 'p16']),
                           ('rev64', 64, ['s8', 's16', 's32', 'u8', 'u16', 'u32', 'f16', 'f32', 'p8', 'p16'])):
        for k in kinds:
            g = c // eb(k)
            for q in (0, 1):
                T = vt(k, q)
                n = nl(k, q)
                idx = [i // g * g + (g - 1 - i % g) for i in range(n)]
                body = ('__builtin_axiom_shufflevector(a, a, %s)' % ', '.join(map(str, idx))) if q else mach2(op, k, q, '-')
                fn('v%s%s_%s' % (op, Qs(q), k), T, [(T, 'a')], body, model=lambda a, idx=idx: [a[i] for i in idx], raw=True)
    # ---- reciprocal and square root estimates, steps, exponents
    for k in FLTH:
        e = fext(k)
        for q in (0, 1):
            T = vt(k, q)
            fn('vrecpe%s_%s' % (Qs(q), k), T, [(T, 'a')], mach2('frecpe', k, q, e), e, model=L1(lambda x: fdiv(1.0, x)),
               mode='approx')
            fn('vrsqrte%s_%s' % (Qs(q), k), T, [(T, 'a')], mach2('frsqrte', k, q, e), e,
               model=L1(lambda x: fdiv(1.0, math.sqrt(x)) if (not isnan(x) and x > 0 and not math.isinf(x)) else
                        (math.inf if x == 0 and not fsign(x) else -math.inf if x == 0 else 0.0 if x == math.inf else NAN)),
               mode='approx')
            fn('vrecps%s_%s' % (Qs(q), k), T, [(T, 'a'), (T, 'b')], mach3('frecps', k, q, e), e,
               model=L2(lambda x, y, k=k: frecps(x, y, k)))
            fn('vrsqrts%s_%s' % (Qs(q), k), T, [(T, 'a'), (T, 'b')], mach3('frsqrts', k, q, e), e,
               model=L2(lambda x, y, k=k: frsqrts(x, y, k)))
            fn('vsqrt%s_%s' % (Qs(q), k), T, [(T, 'a')], '__builtin_axiom_simd("sqrt", a)' if plain_q(k, q) else
               mach2('fsqrt', k, q, e), e, model=L1(lambda x, k=k: fsqrt(x, k)))
        scalar1('recpe', k, 'frecpe', model=lambda x: fdiv(1.0, x))
        INTRIN[-1]['mode'] = 'approx'
        scalar1('rsqrte', k, 'frsqrte', model=lambda x: fdiv(1.0, math.sqrt(x)) if (not isnan(x) and x > 0 and not math.isinf(x)) else
                (math.inf if x == 0 and not fsign(x) else -math.inf if x == 0 else 0.0 if x == math.inf else NAN))
        INTRIN[-1]['mode'] = 'approx'
        scalar2('recps', k, 'frecps', model=lambda x, y, k=k: frecps(x, y, k))
        scalar2('rsqrts', k, 'frsqrts', model=lambda x, y, k=k: frsqrts(x, y, k))
        scalar1('recpx', k, 'frecpx', model=lambda x, k=k: frecpx(x, k))
        if k == 'f16':
            scalar1('sqrt', k, 'fsqrt', model=lambda x: fsqrt(x, 'f16'))
    for q in (0, 1):
        fn('vrecpe%s_u32' % Qs(q), vt('u32', q), [vec('u32', q)], mach2('urecpe', 'u32', q, '-'),
           model=L1(lambda x: urecpe(x)))
        fn('vrsqrte%s_u32' % Qs(q), vt('u32', q), [vec('u32', q)], mach2('ursqrte', 'u32', q, '-'),
           model=L1(lambda x: ursqrte(x)))


def frecps(x, y, k):
    if isnan(x) or isnan(y):
        return NAN
    if (math.isinf(x) and y == 0) or (math.isinf(y) and x == 0):
        return 2.0
    return ffma(-x, y, 2.0, k)


def frsqrts(x, y, k):
    if isnan(x) or isnan(y):
        return NAN
    if (math.isinf(x) and y == 0) or (math.isinf(y) and x == 0):
        return 1.5
    r = ffma(-x, y, 3.0, k) if not (math.isinf(x) or math.isinf(y)) else -x * y
    if isinstance(r, float) and (math.isinf(r) or isnan(r)):
        return r
    # (3 - x*y) / 2 rounded once: the fused result's exact half
    p = Fraction(-x) * Fraction(y) + 3
    return rnd(p / 2, k)


def frecpx(x, k):
    """FRECPX: the exponent inverted, the fraction zero (of a denormal: the largest exponent)"""
    if isnan(x):
        return NAN
    m, e = FMT[k]
    b = fenc(x, k)
    s = b >> (m + e)
    ex_ = (b >> m) & ((1 << e) - 1)
    if ex_ == 0:
        r = ((1 << e) - 2) << m
    else:
        r = ((~ex_) & ((1 << e) - 1)) << m
    return fdec((s << (m + e)) | r, k)


def recip_est(a):
    """RecipEstimate (the manual's): a 9-bit input of 256..511"""
    a = a * 2 + 1
    b = (1 << 19) // a
    return (b + 1) // 2


def rsqrt_est(a):
    if a < 256:
        a = a * 2 + 1
    else:
        a = (a >> 1) << 1
        a = (a + 1) * 2
    b = 512
    while a * (b + 1) * (b + 1) < (1 << 28):
        b += 1
    return (b + 1) // 2


def urecpe(x):
    x &= 0xffffffff
    if not (x >> 31):
        return 0xffffffff
    return (recip_est(x >> 23) & 0x1ff) << 23


def ursqrte(x):
    x &= 0xffffffff
    if not (x >> 30):
        return 0xffffffff
    return (rsqrt_est(x >> 23) & 0x1ff) << 23


def conversions():
    # ---- integer <-> floating, with the rounding modes; fixed point
    pairs = [('f32', 's32'), ('f32', 'u32'), ('f64', 's64'), ('f64', 'u64'), ('f16', 's16'), ('f16', 'u16')]
    for (f, i) in pairs:
        e = fext(f)
        p = pre(i)
        for q in (0, 1):
            Tf, Ti = vt(f, q), vt(i, q)
            # (to floating: round to nearest)
            body = cast(Tf, '__builtin_axiom_convertvector(a, %s)' % Tf) if plain_q(f, q) else mach2(p + 'cvtf', i, q, e, rk=f)
            fn('vcvt%s_%s_%s' % (Qs(q), f, i), Tf, [(Ti, 'a')], body, e, model=L1(lambda x, f=f: i2f(x, f)))
            for (m, r) in (('', 'z'), ('n', 'n'), ('m', 'm'), ('p', 'p'), ('a', 'a')):
                fn('vcvt%s%s_%s_%s' % (m, Qs(q), i, f), Ti, [(Tf, 'a')], mach2('fcvt%s%s' % (r, p[0]), f, q, e, rk=i), e,
                   model=L1(lambda x, i=i, r=r: f2i(x, i, r)))
            fb = eb(f)
            fn('vcvt%s_n_%s_%s' % (Qs(q), f, i), Tf, [(Ti, 'a'), ('const int', 'n')],
               vres(f, q, A('%scvtf %s, %s, #%%2' % (p, V(0, f, q), V(1, i, q)), '=w;w;i', [('a', 'v'), ('n', 'i')], RES(q), e,
                            imm=[(1, fb)])), e, model=lambda a, n, f=f: [i2f(x, f, n) for x in a], imms={'n': (1, fb)})
            fn('vcvt%s_n_%s_%s' % (Qs(q), i, f), Ti, [(Tf, 'a'), ('const int', 'n')],
               vres(i, q, A('fcvtz%s %s, %s, #%%2' % (p[0], V(0, i, q), V(1, f, q)), '=w;w;i', [('a', 'v'), ('n', 'i')], RES(q), e,
                            imm=[(1, fb)])), e, model=lambda a, n, i=i: [f2i(x, i, 'z', n) for x in a], imms={'n': (1, fb)})
        # (scalars)
        sf = {'f32': 's', 'f64': 'd', 'f16': 'h'}[f]
        fn('vcvt%s_%s_%s' % (sf, f, i), st(f), [(st(i), 'a')],
           sres(f, A('%scvtf %s, %s' % (p, SV(0, f), SV(1, i)), '=w;w', [sarg(i, 'a')], scode(f), e)), e,
           model=lambda x, f=f: i2f(x, f))
        for (m, r) in (('', 'z'), ('n', 'n'), ('m', 'm'), ('p', 'p'), ('a', 'a')):
            fn('vcvt%s%s_%s_%s' % (m, sf, i, f), st(i), [(st(f), 'a')],
               sres(i, A('fcvt%s%s %s, %s' % (r, p[0], SV(0, i), SV(1, f)), '=w;w', [sarg(f, 'a')], scode(i), e)), e,
               model=lambda x, i=i, r=r: f2i(x, i, r))
        fb = eb(f)
        fn('vcvt%s_n_%s_%s' % (sf, f, i), st(f), [(st(i), 'a'), ('const int', 'n')],
           sres(f, A('%scvtf %s, %s, #%%2' % (p, SV(0, f), SV(1, i)), '=w;w;i', [sarg(i, 'a'), ('n', 'i')], scode(f), e,
                     imm=[(1, fb)])), e, model=lambda x, n, f=f: i2f(x, f, n), imms={'n': (1, fb)})
        fn('vcvt%s_n_%s_%s' % (sf, i, f), st(i), [(st(f), 'a'), ('const int', 'n')],
           sres(i, A('fcvtz%s %s, %s, #%%2' % (p[0], SV(0, i), SV(1, f)), '=w;w;i', [sarg(f, 'a'), ('n', 'i')], scode(i), e,
                     imm=[(1, fb)])), e, model=lambda x, n, i=i: f2i(x, i, 'z', n), imms={'n': (1, fb)})
    # (half precision from and to the other integer widths: scalars of FP16)
    for i in ('s32', 'u32', 's64', 'u64'):
        p = pre(i)
        fn('vcvth_f16_%s' % i, 'float16_t', [(st(i), 'a')],
           sres('f16', A('%scvtf %%h0, %s' % (p, '%w1' if eb(i) == 32 else '%x1'), '=w;r', [(('a', 'n'))], 'u', 'fp16')), 'fp16',
           model=lambda x: i2f(x, 'f16'))
        for (m, r) in (('', 'z'), ('n', 'n'), ('m', 'm'), ('p', 'p'), ('a', 'a')):
            fn('vcvt%sh_%s_f16' % (m, i), st(i), [('float16_t', 'a')],
               cast(st(i), A('fcvt%s%s %s, %%h1' % (r, p[0], '%w0' if eb(i) == 32 else '%x0'), '=r;w', [sarg('f16', 'a')],
                             scode(i), 'fp16')), 'fp16', model=lambda x, i=i, r=r: f2i(x, i, r))
        fn('vcvth_n_f16_%s' % i, 'float16_t', [(st(i), 'a'), ('const int', 'n')],
           sres('f16', A('%scvtf %%h0, %s, #%%2' % (p, '%w1' if eb(i) == 32 else '%x1'), '=w;r;i', [('a', 'n'), ('n', 'i')], 'u',
                         'fp16', imm=[(1, 16)])), 'fp16', model=lambda x, n: i2f(x, 'f16', n), imms={'n': (1, 16)})
        fn('vcvth_n_%s_f16' % i, st(i), [('float16_t', 'a'), ('const int', 'n')],
           cast(st(i), A('fcvtz%s %s, %%h1, #%%2' % (p[0], '%w0' if eb(i) == 32 else '%x0'), '=r;w;i', [sarg('f16', 'a'), ('n', 'i')],
                         scode(i), 'fp16', imm=[(1, 16)])), 'fp16', model=lambda x, n, i=i: f2i(x, i, 'z', n), imms={'n': (1, 16)})
    # ---- between the floating formats
    fn('vcvt_f32_f64', vt('f32', 0), [vec('f64', 1)], mach2('fcvtn', 'f64', 1, '-', rk='f32', rq=0), model=L1(lambda x: x))
    fn('vcvt_high_f32_f64', vt('f32', 1), [vec('f32', 0, 'r'), vec('f64', 1)],
       vres('f32', 1, A('fcvtn2 %v0.4s, %v2.2d', '=w;0;w', [('r', 'v'), ('a', 'v')], 'Q')), model=lambda r, a: r + a)
    fn('vcvt_f64_f32', vt('f64', 1), [vec('f32', 0)], mach2('fcvtl', 'f32', 0, '-', rk='f64', rq=1), model=L1(lambda x: x))
    fn('vcvt_high_f64_f32', vt('f64', 1), [vec('f32', 1)], mach2('fcvtl2', 'f32', 1, '-', rk='f64', rq=1),
       model=lambda a: hi_half(a))
    fn('vcvtx_f32_f64', vt('f32', 0), [vec('f64', 1)], mach2('fcvtxn', 'f64', 1, '-', rk='f32', rq=0),
       model=L1(lambda x: rnd(x, 'f32', 'o')))
    fn('vcvtx_high_f32_f64', vt('f32', 1), [vec('f32', 0, 'r'), vec('f64', 1)],
       vres('f32', 1, A('fcvtxn2 %v0.4s, %v2.2d', '=w;0;w', [('r', 'v'), ('a', 'v')], 'Q')),
       model=lambda r, a: r + [rnd(x, 'f32', 'o') for x in a])
    fn('vcvtxd_f32_f64', 'float32_t', [('float64_t', 'a')], sres('f32', A('fcvtxn %s0, %d1', '=w;w', [('a', 'f')], 'f')),
       model=lambda x: rnd(x, 'f32', 'o'))
    fn('vcvt_f16_f32', vt('f16', 0), [vec('f32', 1)], mach2('fcvtn', 'f32', 1, '-', rk='f16', rq=0), model=L1(lambda x: x))
    fn('vcvt_high_f16_f32', vt('f16', 1), [vec('f16', 0, 'r'), vec('f32', 1)],
       vres('f16', 1, A('fcvtn2 %v0.8h, %v2.4s', '=w;0;w', [('r', 'v'), ('a', 'v')], 'Q')), model=lambda r, a: r + a)
    fn('vcvt_f32_f16', vt('f32', 1), [vec('f16', 0)], mach2('fcvtl', 'f16', 0, '-', rk='f32', rq=1), model=L1(lambda x: x))
    fn('vcvt_high_f32_f16', vt('f32', 1), [vec('f16', 1)], mach2('fcvtl2', 'f16', 1, '-', rk='f32', rq=1),
       model=lambda a: hi_half(a))
    # ---- rounding to integral values
    for (op, mn, r) in (('rnd', 'frintz', 'z'), ('rndn', 'frintn', 'n'), ('rndm', 'frintm', 'm'), ('rndp', 'frintp', 'p'),
                        ('rnda', 'frinta', 'a'), ('rndi', 'frinti', 'n'), ('rndx', 'frintx', 'n')):
        g = {'rnd': 'trunc', 'rndn': 'nearest', 'rndm': 'floor', 'rndp': 'ceil'}.get(op)
        for k in FLTH:
            e = fext(k)
            for q in (0, 1):
                T = vt(k, q)
                body = '__builtin_axiom_simd("%s", a)' % g if (g and plain_q(k, q)) else mach2(mn, k, q, e)
                fn('v%s%s_%s' % (op, Qs(q), k), T, [(T, 'a')], body, e, model=L1(lambda x, r=r: fround(x, r)))
            if k == 'f16':
                scalar1(op, k, mn, model=lambda x, r=r: fround(x, r), nm='v%sh_f16' % op)
        if op == 'rndn':
            fn('vrndns_f32', 'float32_t', [('float32_t', 'a')], sres('f32', A('frintn %s0, %s1', '=w;w', [('a', 'f')], 'f')),
               model=lambda x: fround(x, 'n'))


def gpr(k, i):
    return '%%%s%d' % ('x' if eb(k) == 64 else 'w', i)


def data_movement():
    for k in ALL:
        e = dext(k)
        for q in (0, 1):
            T = vt(k, q)
            n = nl(k, q)
            # (vdup_n, vmov_n: from a scalar)
            if isf(k) and k not in ('f16', 'bf16'):
                t = 'dup %s, %s' % (V(0, k, q), E(1, k, 0)) if n > 1 else 'fmov %d0, %d1'
                body = vres(k, q, A(t, '=w;w', [sarg(k, 'a')], RES(q), e))
            elif k in ('f16', 'bf16'):
                body = vres(k, q, A('dup %s, %s' % (V(0, k, q), E(1, k, 0)), '=w;w', [sarg(k, 'a')], RES(q), e))
            else:
                t = 'dup %s, %s' % (V(0, k, q), gpr(k, 1)) if n > 1 else 'fmov %d0, %x1'
                body = vres(k, q, A(t, '=w;r', [('a', 'n')], RES(q), e))
            for op in ('dup', 'mov'):
                if op == 'mov' and k == 'bf16':
                    continue
                fn('v%s%s_n_%s' % (op, Qs(q), k), T, [(st(k), 'a')], body, e, model=lambda a, n=n: [a] * n, raw=True)
            # (lanes)
            imm = [(0, n - 1)]
            if isf(k) or eb(k) == 64:
                gt = sres(k, A('dup %s, %s' % (SV(0, k), E(1, k, '%2')) if n > 1 else 'fmov %d0, %d1',
                               '=w;w;i' if n > 1 else '=w;w', [('a', 'v'), ('l', 'i')] if n > 1 else [('a', 'v')], scode(k), e,
                               imm=imm if n > 1 else None))
                if eb(k) == 64 and not isf(k):
                    gt = cast(st(k), A('umov %%x0, %s' % E(1, k, '%2'), '=r;w;i', [('a', 'v'), ('l', 'i')], scode(k), e, imm=imm))
            else:
                gt = cast(st(k), A('%smov %%w0, %s' % ('s' if k[0] == 's' and eb(k) < 32 else 'u', E(1, k, '%2')), '=r;w;i',
                                   [('a', 'v'), ('l', 'i')], scode(k), e, imm=imm))
            ps = [(T, 'a'), ('const int', 'l')]
            fn('vget%s_lane_%s' % (Qs(q), k), st(k), ps, gt, e, model=lambda a, l: a[l], raw=True, imms={'l': imm[0]})
            if isf(k):
                stt = A('mov %s, %s' % (E(0, k, '%3'), E(2, k, 0)), '=w;0;w;i', [('v', 'v'), sarg(k, 'a'), ('l', 'i')], RES(q), e, imm=imm)
            else:
                stt = A('mov %s, %s' % (E(0, k, '%3'), gpr(k, 2)), '=w;0;r;i', [('v', 'v'), ('a', 'n'), ('l', 'i')], RES(q), e, imm=imm)
            fn('vset%s_lane_%s' % (Qs(q), k), T, [(st(k), 'a'), (T, 'v'), ('const int', 'l')], vres(k, q, stt), e,
               model=lambda a, v, l: v[:l] + [a] + v[l + 1:], raw=True, imms={'l': imm[0]})
            # (vdup_lane: a lane of a 64- or 128-bit vector to every lane)
            for lq in (0, 1):
                ln = nl(k, lq)
                t = ('dup %s, %s' % (V(0, k, q), E(1, k, '%2'))) if n > 1 else 'dup %%d0, %s' % E(1, k, '%2')
                fn('vdup%s_lane%s_%s' % (Qs(q), 'q' if lq else '', k), T, [(vt(k, lq), 'a'), ('const int', 'l')],
                   vres(k, q, A(t, '=w;w;i', [('a', 'v'), ('l', 'i')], RES(q), e, imm=[(0, ln - 1)])), e,
                   model=lambda a, l, n=n: [a[l]] * n, raw=True, imms={'l': (0, ln - 1)})
                # (vcopy_lane: a lane into a lane)
                fn('vcopy%s_lane%s_%s' % (Qs(q), 'q' if lq else '', k), T,
                   [(T, 'a'), ('const int', 'l'), (vt(k, lq), 'b'), ('const int', 'm')],
                   vres(k, q, A('mov %s, %s' % (E(0, k, '%2'), E(3, k, '%4')), '=w;0;i;w;i',
                                [('a', 'v'), ('l', 'i'), ('b', 'v'), ('m', 'i')], RES(q), e, imm=[(0, n - 1), (0, ln - 1)])), e,
                   model=lambda a, l, b, m: a[:l] + [b[m]] + a[l + 1:], raw=True, imms={'l': (0, n - 1), 'm': (0, ln - 1)})
            if k == 'bf16':
                continue
            # (vext)
            if q:
                body = '__builtin_axiom_shufflevector(a, b, %s)' % ', '.join('(n) + %d' % i for i in range(n))
            else:
                body = vres(k, q, A('ext %v0.8b, %v1.8b, %v2.8b, #%3', '=w;w;w;i',
                                    [('a', 'v'), ('b', 'v'), ('(n) * %d' % (eb(k) // 8), 'i')], 'D', e, imm=[(0, 7)]))
            fn('vext%s_%s' % (Qs(q), k), T, [(T, 'a'), (T, 'b'), ('const int', 'n')], body, e,
               model=lambda a, b, nn: (a + b)[nn:nn + len(a)], raw=True, imms={'n': (0, n - 1)})
            # (zip, unzip, transpose)
            for (op, f) in (('zip1', lambda a, b: [x for i in range(len(a) // 2) for x in (a[i], b[i])]),
                            ('zip2', lambda a, b: [x for i in range(len(a) // 2, len(a)) for x in (a[i], b[i])]),
                            ('uzp1', lambda a, b: (a + b)[0::2]), ('uzp2', lambda a, b: (a + b)[1::2]),
                            ('trn1', lambda a, b: [x for i in range(0, len(a), 2) for x in (a[i], b[i])]),
                            ('trn2', lambda a, b: [x for i in range(1, len(a), 2) for x in (a[i], b[i])])):
                if n == 1:
                    continue
                if q:
                    idx = f(list(range(n)), list(range(n, 2 * n)))
                    body = '__builtin_axiom_shufflevector(a, b, %s)' % ', '.join(map(str, idx))
                else:
                    body = mach3(op, k, q, e)
                fn('v%s%s_%s' % (op, Qs(q), k), T, [(T, 'a'), (T, 'b')], body, e, model=f, raw=True)
            if n > 1 and eb(k) < 64:
                for op in ('zip', 'uzp', 'trn'):
                    X = xt(k, q, 2)
                    fn('v%s%s_%s' % (op, Qs(q), k), X, [(T, 'a'), (T, 'b')],
                       '{ %s __r; __r.val[0] = v%s1%s_%s(a, b); __r.val[1] = v%s2%s_%s(a, b); return __r; }' % (
                           X, op, Qs(q), k, op, Qs(q), k), e,
                       model=(lambda a, b, op=op: [ZUT[op + '1'](a, b), ZUT[op + '2'](a, b)]), raw=True)
        # (combine, split, create, reinterpret)
        T, TQ = vt(k, 0), vt(k, 1)
        fn('vcombine_%s' % k, TQ, [(T, 'a'), (T, 'b')],
           vres(k, 1, A('mov %v0.16b, %v1.16b\nmov %v0.d[1], %v2.d[0]', '=&w;w;w', [('a', 'v'), ('b', 'v')], 'Q', e)), e,
           model=lambda a, b: a + b, raw=True)
        fn('vget_low_%s' % k, T, [(TQ, 'a')], vres(k, 0, A('mov %v0.8b, %v1.8b', '=w;w', [('a', 'v')], 'D', e)), e,
           model=lambda a: a[:len(a) // 2], raw=True)
        fn('vget_high_%s' % k, T, [(TQ, 'a')], vres(k, 0, A('dup %d0, %v1.d[1]', '=w;w', [('a', 'v')], 'D', e)), e,
           model=lambda a: a[len(a) // 2:], raw=True)
        fn('vcreate_%s' % k, T, [('uint64_t', 'a')], vres(k, 0, A('fmov %d0, %x1', '=w;r', [('a', 'n')], 'D', e)), e,
           model=lambda a, n=nl(k, 0), b=eb(k): [(a >> (b * i)) & ((1 << b) - 1) for i in range(n)], raw=True)
    # (the scalar lanes: vdupb_lane_s8 ...)
    for k in ALL:
        e = dext(k)
        for lq in (0, 1):
            ln = nl(k, lq)
            if isf(k) or eb(k) == 64:
                body = sres(k, A('dup %s, %s' % (SV(0, k), E(1, k, '%2')), '=w;w;i', [('a', 'v'), ('l', 'i')], scode(k), e,
                                 imm=[(0, ln - 1)]))
            else:
                body = cast(st(k), A('%smov %%w0, %s' % ('s' if k[0] == 's' and eb(k) < 32 else 'u', E(1, k, '%2')), '=r;w;i',
                                     [('a', 'v'), ('l', 'i')], scode(k), e, imm=[(0, ln - 1)]))
            fn('vdup%s_lane%s_%s' % (ev(k), 'q' if lq else '', k), st(k), [(vt(k, lq), 'a'), ('const int', 'l')], body, e,
               model=lambda a, l: a[l], raw=True, imms={'l': (0, ln - 1)})
    # reinterpretations: the same bits as another type
    for q in (0, 1):
        for k in ALL:
            for j in ALL:
                if j == k:
                    continue
                e = '-'
                fn('vreinterpret%s_%s_%s' % (Qs(q), k, j), vt(k, q), [(vt(j, q), 'a')], cast(vt(k, q), 'a'), e,
                   model=None)
        if q:
            for j in ALL:
                e = '-'
                fn('vreinterpretq_p128_%s' % j, 'poly128_t', [(vt(j, 1), 'a')], '__axiom_q2p128((uint8x16_t)a)', e)
                fn('vreinterpretq_%s_p128' % j, vt(j, 1), [('poly128_t', 'a')], cast(vt(j, 1), '__axiom_p1282q(a)'), e)
    # ---- table lookups
    for k in ('s8', 'u8', 'p8'):
        T8, T16 = vt(k, 0), vt(k, 1)
        I8 = vt('u8' if k != 's8' else 's8', 0)
        for n in (1, 2, 3, 4):
            # (vtbl: 64-bit tables, lookups of 8 indices; vtbx: out of range keeps the destination's)
            ps = ([(T8, 'a')] if n == 1 else [(xt(k, 0, n), 't')])
            regs = ', '.join('v%d.16b' % (16 + i) for i in range((n + 1) // 2))
            if n == 1:
                tabs = [('a', 'v')]
            else:
                tabs = [('t.val[%d]' % i, 'v') for i in range(n)]
            # (the 64-bit tables in pairs: 128-bit tables in v16 ...)
            pairs = []
            for i in range(0, n, 2):
                if i + 1 < n:
                    pairs.append('__builtin_axiom_simd("%s", %s, %s)' % (
                        mop('mov %v0.16b, %v1.16b\nmov %v0.d[1], %v2.d[0]', '=&w;w;w', 'vv', 'Q'), tabs[i][0], tabs[i + 1][0]))
                else:
                    pairs.append('__builtin_axiom_simd("%s", %s)' % (mop('mov %v0.8b, %v1.8b', '=w;w', 'v', 'Q'), tabs[i][0]))
            m = len(pairs)
            cons = ';'.join('V%d' % (16 + i) for i in range(m))
            body = vres(k, 0, A('tbl %%v0.8b, {%s}, %%v%d.8b' % (regs, m + 1), '=w;%s;w' % cons,
                                [(p, 'v') for p in pairs] + [('b', 'v')], 'D'))
            fn('vtbl%d_%s' % (n, k), T8, ps + [(I8, 'b')], body,
               model=lambda *xs, n=n: tbl_model(xs[:-1], xs[-1], 8 * n, None), raw=True)
            body = vres(k, 0, A('tbx %%v0.8b, {%s}, %%v%d.8b' % (regs, m + 2), '=w;0;%s;w' % cons,
                                [('r', 'v')] + [(p, 'v') for p in pairs] + [('b', 'v')], 'D'))
            if n % 2 == 1:
                # (a table of 8n bytes in registers of 16: an index from 8n to 16 is out of range, which tbx's is
                # not: the destination's lanes where the index is 8n or more)
                lim = 8 * n
                body = vres(k, 0, A('movi v29.8b, #%d\ncmhs v29.8b, %%v%d.8b, v29.8b\ntbl %%v0.8b, {%s}, %%v%d.8b\n'
                                    'bit %%v0.8b, %%v1.8b, v29.8b' % (lim, m + 2, regs, m + 2), '=&w;w;%s;w' % cons,
                                    [('r', 'v')] + [(p, 'v') for p in pairs] + [('b', 'v')], 'D', clob='v29'))
            fn('vtbx%d_%s' % (n, k), T8, [(T8, 'r')] + ps + [(I8, 'b')], body,
               model=lambda r, *xs, n=n: tbl_model(xs[:-1], xs[-1], 8 * n, r), raw=True)
        for q in (0, 1):
            R = vt(k, q)
            IX = vt('u8', q)
            for n in (1, 2, 3, 4):
                ps = [(T16, 't')] if n == 1 else [(xt(k, 1, n), 't')]
                tabs = [('t', 'v')] if n == 1 else [('t.val[%d]' % i, 'v') for i in range(n)]
                regs = ', '.join('v%d.16b' % (16 + i) for i in range(n))
                cons = ';'.join('V%d' % (16 + i) for i in range(n))
                b = '16b' if q else '8b'
                fn('vqtbl%d%s_%s' % (n, Qs(q), k), R, ps + [(IX, 'b')],
                   vres(k, q, A('tbl %%v0.%s, {%s}, %%v%d.%s' % (b, regs, n + 1, b), '=w;%s;w' % cons, tabs + [('b', 'v')], RES(q))),
                   model=lambda *xs, n=n: tbl_model(xs[:-1], xs[-1], 16 * n, None), raw=True)
                fn('vqtbx%d%s_%s' % (n, Qs(q), k), R, [(R, 'r')] + ps + [(IX, 'b')],
                   vres(k, q, A('tbx %%v0.%s, {%s}, %%v%d.%s' % (b, regs, n + 2, b), '=w;0;%s;w' % cons,
                                [('r', 'v')] + tabs + [('b', 'v')], RES(q))),
                   model=lambda r, *xs, n=n: tbl_model(xs[:-1], xs[-1], 16 * n, r), raw=True)


ZUT = {'zip1': lambda a, b: [x for i in range(len(a) // 2) for x in (a[i], b[i])],
       'zip2': lambda a, b: [x for i in range(len(a) // 2, len(a)) for x in (a[i], b[i])],
       'uzp1': lambda a, b: (a + b)[0::2], 'uzp2': lambda a, b: (a + b)[1::2],
       'trn1': lambda a, b: [x for i in range(0, len(a), 2) for x in (a[i], b[i])],
       'trn2': lambda a, b: [x for i in range(1, len(a), 2) for x in (a[i], b[i])]}


def tbl_model(tabs, idx, size, dst):
    t = []
    for x in tabs:
        if isinstance(x[0], list):
            for y in x:
                t += y
        else:
            t += x
    t = [v & 0xff for v in t][:size]
    return [t[i & 0xff] if (i & 0xff) < size else (0 if dst is None else dst[j]) for (j, i) in enumerate(idx)]


def uty(k, q):
    """the unaligned form of a vector type (loads and stores through it)"""
    return '__axiom_u_' + vt(k, q)


def dstore(k, q, p, a):
    """the store of vector a at p (one of 64 bits: as the double its register holds, which is stored from it)"""
    if q:
        return '{ *(%s *)(void *)%s = %s; }' % (uty(k, q), p, a)
    return '{ union { %s __v; double __d; } __u; __u.__v = %s; *(__axiom_f64u *)(void *)%s = __u.__d; }' % (vt(k, q), a, p)


def loads_stores():
    for k in ALL:
        e = dext(k)
        for q in (0, 1):
            T = vt(k, q)
            n = nl(k, q)
            P, CP = '%s *' % st(k), 'const %s *' % st(k)
            ld = '(*(const %s *)(const void *)(p))' % uty(k, q)
            fn('vld1%s_%s' % (Qs(q), k), T, [(CP, 'p')], ld, e, model=lambda m, n=n: m[:n], raw=True)
            fn('vst1%s_%s' % (Qs(q), k), 'void', [(P, 'p'), (T, 'a')], dstore(k, q, 'p', 'a'), e,
               model=lambda m, a: ('mem', a), raw=True)
            fn('vld1%s_lane_%s' % (Qs(q), k), T, [(CP, 'p'), (T, 'v'), ('const int', 'l')],
               'vset%s_lane_%s(*(p), v, l)' % (Qs(q), k), e, model=lambda m, v, l: v[:l] + [m[0]] + v[l + 1:], raw=True,
               imms={'l': (0, n - 1)})
            fn('vst1%s_lane_%s' % (Qs(q), k), 'void', [(P, 'p'), (T, 'v'), ('const int', 'l')],
               '(void)(*(p) = vget%s_lane_%s(v, l))' % (Qs(q), k), e, model=lambda m, v, l: ('mem', [v[l]]), raw=True,
               imms={'l': (0, n - 1)})
            fn('vld1%s_dup_%s' % (Qs(q), k), T, [(CP, 'p')], 'vdup%s_n_%s(*p)' % (Qs(q), k), e,
               model=lambda m, n=n: [m[0]] * n, raw=True)
            for c in (2, 3, 4):
                X = xt(k, q, c)
                b = '{ %s __r; %s return __r; }' % (X, ' '.join('__r.val[%d] = *(const %s *)(const void *)(p + %d);' % (
                    i, uty(k, q), i * n) for i in range(c)))
                fn('vld1%s_%s_x%d' % (Qs(q), k, c), X, [(CP, 'p')], b, e,
                   model=lambda m, n=n, c=c: [m[i * n:(i + 1) * n] for i in range(c)], raw=True)
                b = '{ %s }' % ' '.join(dstore(k, q, '(p + %d)' % (i * n), 'a.val[%d]' % i) for i in range(c))
                fn('vst1%s_%s_x%d' % (Qs(q), k, c), 'void', [(P, 'p'), (X, 'a')], b, e,
                   model=lambda m, a: ('mem', [x for v in a for x in v]), raw=True)
                # (interleaved: ld2 ... st4, and their lane and dup forms)
                regs = ', '.join('v%d.%s' % (16 + i, ar(k, q)) for i in range(c))
                lregs = ', '.join('v%d.%s' % (16 + i, ev(k)) for i in range(c))
                outs = ';'.join('=V%d' % (16 + i) for i in range(c))
                mn = 'ld%d' % c if n > 1 else 'ld1'
                ptrs = ''.join(', &__r.val[%d]' % i for i in range(1, c))
                body = '{ %s __r; __r.val[0] = %s; return __r; }' % (X, vres(k, q, A('%s {%s}, [%%%d]' % (mn, regs, c), outs + ';r',
                                                                               [('p', 'p')], RES(q), e, flags='M',
                                                                               sig='p' + 'o' * (c - 1))[:-1] + ptrs + ')'))
                fn('vld%d%s_%s' % (c, Qs(q), k), X, [(CP, 'p')], body, e,
                   model=lambda m, n=n, c=c: [[m[j * c + i] for j in range(n)] for i in range(c)], raw=True)
                body = '{ %s __r; __r.val[0] = %s; return __r; }' % (X, vres(k, q, A('ld%dr {%s}, [%%%d]' % (c, regs, c), outs + ';r',
                                                                               [('p', 'p')], RES(q), e, flags='M',
                                                                               sig='p' + 'o' * (c - 1))[:-1] + ptrs + ')'))
                fn('vld%d%s_dup_%s' % (c, Qs(q), k), X, [(CP, 'p')], body, e,
                   model=lambda m, n=n, c=c: [[m[i]] * n for i in range(c)], raw=True)
                ties = ';'.join(str(i) for i in range(c))
                srcs = ''.join(', a.val[%d]' % i for i in range(c))
                call = A('ld%d {%s}[%%%d], [%%%d]' % (c, lregs, 2 * c + 1, c), outs + ';r;' + ties + ';i', [('p', 'p')], RES(q), e,
                         flags='M', sig='p' + 'v' * c + 'i' + 'o' * (c - 1), imm=[(0, n - 1)])
                body = '{ %s __r; __r.val[0] = %s; return __r; }' % (X, vres(k, q, call[:-1] + srcs + ', l' + ptrs + ')'))
                fn('vld%d%s_lane_%s' % (c, Qs(q), k), X, [(CP, 'p'), (X, 'a'), ('const int', 'l')], body, e,
                   model=lambda m, a, l, c=c: [v[:l] + [m[i]] + v[l + 1:] for (i, v) in enumerate(a)], raw=True,
                   imms={'l': (0, n - 1)})
                ins = ';'.join('V%d' % (16 + i) for i in range(c))
                mn = 'st%d' % c if n > 1 else 'st1'
                call = A('%s {%s}, [%%0]' % (mn, regs), 'r;' + ins, [('p', 'p')], 'v', e, flags='M', sig='p' + 'v' * c)
                fn('vst%d%s_%s' % (c, Qs(q), k), 'void', [(P, 'p'), (X, 'a')], '{ %s; }' % (call[:-1] + srcs + ')'), e,
                   model=lambda m, a, n=n, c=c: ('mem', [a[i][j] for j in range(n) for i in range(c)]), raw=True)
                call = A('st%d {%s}[%%%d], [%%0]' % (c, lregs, c + 1), 'r;' + ins + ';i', [('p', 'p')], 'v', e, flags='M',
                         sig='p' + 'v' * c + 'i', imm=[(0, n - 1)])
                fn('vst%d%s_lane_%s' % (c, Qs(q), k), 'void', [(P, 'p'), (X, 'a'), ('const int', 'l')],
                   '{ %s; }' % (call[:-1] + srcs + ', l)'), e,
                   model=lambda m, a, l: ('mem', [v[l] for v in a]), raw=True, imms={'l': (0, n - 1)})
    fn('vldrq_p128', 'poly128_t', [('const poly128_t *', 'p')], '*p', model=lambda m: m[0], raw=True)
    fn('vstrq_p128', 'void', [('poly128_t *', 'p'), ('poly128_t', 'a')], '{ *p = a; }', model=lambda m, a: ('mem', [a]), raw=True)


def dot_crypto():
    # ---- dot products (dotprod; the mixed-sign ones and the matrix multiplies: i8mm)
    for (k, mn, ext, ak, bk) in (('s32', 'sdot', 'dotprod', 's8', 's8'), ('u32', 'udot', 'dotprod', 'u8', 'u8'),
                                 ('s32', 'usdot', 'i8mm', 'u8', 's8')):
        for q in (0, 1):
            T, TA, TB = vt(k, q), vt(ak, q), vt(bk, q)
            nm = 'vusdot' if mn == 'usdot' else 'vdot'
            dm = (lambda r, a, b: [r[i] + sum(a[4 * i + j] * b[4 * i + j] for j in range(4)) for i in range(len(r))])
            fn('%s%s_%s' % (nm, Qs(q), k), T, [(T, 'r'), (TA, 'a'), (TB, 'b')],
               vres(k, q, A('%s %s, %s, %s' % (mn, V(0, k, q), V(2, ak, q), V(3, bk, q)), '=w;0;w;w',
                            [('r', 'v'), ('a', 'v'), ('b', 'v')], RES(q), ext)), ext, model=dm)
            for lq in (0, 1):
                lm = (lambda r, a, b, l: [r[i] + sum(a[4 * i + j] * b[4 * l + j] for j in range(4)) for i in range(len(r))])
                fn('%s%s_lane%s_%s' % (nm, Qs(q), 'q' if lq else '', k), T, [(T, 'r'), (TA, 'a'), (vt(bk, lq), 'b'), ('const int', 'l')],
                   vres(k, q, A('%s %s, %s, %%v3.4b[%%4]' % (mn, V(0, k, q), V(2, ak, q)), '=w;0;w;w;i',
                                [('r', 'v'), ('a', 'v'), ('b', 'v'), ('l', 'i')], RES(q), ext, imm=[(0, nl(k, lq) - 1)])), ext,
                   model=lm, imms={'l': (0, nl(k, lq) - 1)})
                if mn == 'usdot':
                    # (vsudot_lane: signed by an unsigned lane: usdot with the operands' roles exchanged is not it; sudot)
                    fn('vsudot%s_lane%s_s32' % (Qs(q), 'q' if lq else ''), T,
                       [(T, 'r'), (vt('s8', q), 'a'), (vt('u8', lq), 'b'), ('const int', 'l')],
                       vres(k, q, A('sudot %s, %s, %%v3.4b[%%4]' % (V(0, k, q), V(2, 's8', q)), '=w;0;w;w;i',
                                    [('r', 'v'), ('a', 'v'), ('b', 'v'), ('l', 'i')], RES(q), ext, imm=[(0, nl(k, lq) - 1)])),
                       ext, model=lm, imms={'l': (0, nl(k, lq) - 1)})
    for (nm, mn, ak, bk) in (('vmmlaq_s32', 'smmla', 's8', 's8'), ('vmmlaq_u32', 'ummla', 'u8', 'u8'),
                             ('vusmmlaq_s32', 'usmmla', 'u8', 's8')):
        k = 'u32' if 'u32' in nm else 's32'
        fn(nm, vt(k, 1), [(vt(k, 1), 'r'), (vt(ak, 1), 'a'), (vt(bk, 1), 'b')],
           vres(k, 1, A('%s %%v0.4s, %%v2.16b, %%v3.16b' % mn, '=w;0;w;w', [('r', 'v'), ('a', 'v'), ('b', 'v')], 'Q', 'i8mm')),
           'i8mm', model=lambda r, a, b: [r[2 * i + j] + sum(a[8 * i + t] * b[8 * j + t] for t in range(8))
                                          for i in range(2) for j in range(2)])
    # ---- AES, polynomial multiply (above), SHA-1, SHA-256 (sha2), SHA-512, SHA-3, SM3, SM4
    U8, U32, U64 = vt('u8', 1), vt('u32', 1), vt('u64', 1)
    for (nm, mn, f) in (('vaeseq_u8', 'aese', lambda d, k: aes_e(d, k)), ('vaesdq_u8', 'aesd', lambda d, k: aes_d(d, k))):
        fn(nm, U8, [(U8, 'a'), (U8, 'b')], vres('u8', 1, A('%s %%v0.16b, %%v2.16b' % mn, '=w;0;w', [('a', 'v'), ('b', 'v')], 'Q', 'aes')),
           'aes', model=f)
    for (nm, mn, f) in (('vaesmcq_u8', 'aesmc', lambda d: mix_columns(d)), ('vaesimcq_u8', 'aesimc', lambda d: mix_columns(d, True))):
        fn(nm, U8, [(U8, 'a')], vres('u8', 1, A('%s %%v0.16b, %%v1.16b' % mn, '=w;w', [('a', 'v')], 'Q', 'aes')), 'aes', model=f)
    for (c, mn, f) in (('c', 'sha1c', 'ch'), ('p', 'sha1p', 'par'), ('m', 'sha1m', 'maj')):
        fn('vsha1%sq_u32' % c, U32, [(U32, 'a'), ('uint32_t', 'e'), (U32, 'w')],
           vres('u32', 1, A('%s %%q0, %%s2, %%v3.4s' % mn, '=w;0;w;w', [('a', 'v'), ('e', 'n'), ('w', 'v')], 'Q', 'sha2')), 'sha2',
           model=lambda a, e, w, f=f: sha1_rounds(a, e, w, f))
    fn('vsha1h_u32', 'uint32_t', [('uint32_t', 'e')], cast('uint32_t', A('sha1h %s0, %s1', '=w;w', [('e', 'n')], 'u', 'sha2')),
       'sha2', model=lambda e: rol32(e, 30))
    fn('vsha1su0q_u32', U32, [(U32, 'a'), (U32, 'b'), (U32, 'c')],
       vres('u32', 1, A('sha1su0 %v0.4s, %v2.4s, %v3.4s', '=w;0;w;w', [('a', 'v'), ('b', 'v'), ('c', 'v')], 'Q', 'sha2')), 'sha2',
       model=lambda a, b, c: [x ^ y ^ z for (x, y, z) in zip([a[2], a[3], b[0], b[1]], a, c)])
    fn('vsha1su1q_u32', U32, [(U32, 'a'), (U32, 'b')],
       vres('u32', 1, A('sha1su1 %v0.4s, %v2.4s', '=w;0;w', [('a', 'v'), ('b', 'v')], 'Q', 'sha2')), 'sha2', model=sha1su1)
    fn('vsha256hq_u32', U32, [(U32, 'a'), (U32, 'b'), (U32, 'w')],
       vres('u32', 1, A('sha256h %q0, %q2, %v3.4s', '=w;0;w;w', [('a', 'v'), ('b', 'v'), ('w', 'v')], 'Q', 'sha2')), 'sha2',
       model=lambda a, b, w: sha256_hash(a, b, w, True))
    fn('vsha256h2q_u32', U32, [(U32, 'a'), (U32, 'b'), (U32, 'w')],
       vres('u32', 1, A('sha256h2 %q0, %q2, %v3.4s', '=w;0;w;w', [('a', 'v'), ('b', 'v'), ('w', 'v')], 'Q', 'sha2')), 'sha2',
       model=lambda a, b, w: sha256_hash(b, a, w, False))
    fn('vsha256su0q_u32', U32, [(U32, 'a'), (U32, 'b')],
       vres('u32', 1, A('sha256su0 %v0.4s, %v2.4s', '=w;0;w', [('a', 'v'), ('b', 'v')], 'Q', 'sha2')), 'sha2', model=sha256su0)
    fn('vsha256su1q_u32', U32, [(U32, 'a'), (U32, 'b'), (U32, 'c')],
       vres('u32', 1, A('sha256su1 %v0.4s, %v2.4s, %v3.4s', '=w;0;w;w', [('a', 'v'), ('b', 'v'), ('c', 'v')], 'Q', 'sha2')),
       'sha2', model=sha256su1)
    fn('vsha512hq_u64', U64, [(U64, 'a'), (U64, 'b'), (U64, 'c')],
       vres('u64', 1, A('sha512h %q0, %q2, %v3.2d', '=w;0;w;w', [('a', 'v'), ('b', 'v'), ('c', 'v')], 'Q', 'sha512')), 'sha512',
       model=sha512h)
    fn('vsha512h2q_u64', U64, [(U64, 'a'), (U64, 'b'), (U64, 'c')],
       vres('u64', 1, A('sha512h2 %q0, %q2, %v3.2d', '=w;0;w;w', [('a', 'v'), ('b', 'v'), ('c', 'v')], 'Q', 'sha512')), 'sha512',
       model=sha512h2)
    fn('vsha512su0q_u64', U64, [(U64, 'a'), (U64, 'b')],
       vres('u64', 1, A('sha512su0 %v0.2d, %v2.2d', '=w;0;w', [('a', 'v'), ('b', 'v')], 'Q', 'sha512')), 'sha512',
       model=sha512su0)
    fn('vsha512su1q_u64', U64, [(U64, 'a'), (U64, 'b'), (U64, 'c')],
       vres('u64', 1, A('sha512su1 %v0.2d, %v2.2d, %v3.2d', '=w;0;w;w', [('a', 'v'), ('b', 'v'), ('c', 'v')], 'Q', 'sha512')),
       'sha512', model=sha512su1)
    for k in INTS:
        T = vt(k, 1)
        fn('veor3q_%s' % k, T, [(T, 'a'), (T, 'b'), (T, 'c')],
           vres(k, 1, A('eor3 %v0.16b, %v1.16b, %v2.16b, %v3.16b', '=w;w;w;w', [('a', 'v'), ('b', 'v'), ('c', 'v')], 'Q', 'sha3')),
           'sha3', model=L3(lambda x, y, z: x ^ y ^ z))
        fn('vbcaxq_%s' % k, T, [(T, 'a'), (T, 'b'), (T, 'c')],
           vres(k, 1, A('bcax %v0.16b, %v1.16b, %v2.16b, %v3.16b', '=w;w;w;w', [('a', 'v'), ('b', 'v'), ('c', 'v')], 'Q', 'sha3')),
           'sha3', model=L3(lambda x, y, z: x ^ (y & ~z)))
    fn('vrax1q_u64', U64, [(U64, 'a'), (U64, 'b')],
       vres('u64', 1, A('rax1 %v0.2d, %v1.2d, %v2.2d', '=w;w;w', [('a', 'v'), ('b', 'v')], 'Q', 'sha3')), 'sha3',
       model=L2(lambda x, y: x ^ rol64(y, 1)))
    fn('vxarq_u64', U64, [(U64, 'a'), (U64, 'b'), ('const int', 'n')],
       vres('u64', 1, A('xar %v0.2d, %v1.2d, %v2.2d, #%3', '=w;w;w;i', [('a', 'v'), ('b', 'v'), ('n', 'i')], 'Q', 'sha3',
                        imm=[(0, 63)])), 'sha3', model=lambda a, b, n: [rol64(x ^ y, 64 - n) for (x, y) in zip(a, b)],
       imms={'n': (0, 63)})
    fn('vsm3ss1q_u32', U32, [(U32, 'a'), (U32, 'b'), (U32, 'c')],
       vres('u32', 1, A('sm3ss1 %v0.4s, %v1.4s, %v2.4s, %v3.4s', '=w;w;w;w', [('a', 'v'), ('b', 'v'), ('c', 'v')], 'Q', 'sm4')),
       'sm4', model=lambda a, b, c: [0, 0, 0, rol32((rol32(a[3], 12) + b[3] + c[3]) & 0xffffffff, 7)])
    for (nm, mn, f) in (('vsm3tt1aq_u32', 'sm3tt1a', sm3tt1a), ('vsm3tt1bq_u32', 'sm3tt1b', sm3tt1b),
                        ('vsm3tt2aq_u32', 'sm3tt2a', sm3tt2a), ('vsm3tt2bq_u32', 'sm3tt2b', sm3tt2b)):
        fn(nm, U32, [(U32, 'a'), (U32, 'b'), (U32, 'c'), ('const int', 'n')],
           vres('u32', 1, A('%s %%v0.4s, %%v2.4s, %%v3.s[%%4]' % mn, '=w;0;w;w;i', [('a', 'v'), ('b', 'v'), ('c', 'v'), ('n', 'i')],
                            'Q', 'sm4', imm=[(0, 3)])), 'sm4', model=f, imms={'n': (0, 3)})
    fn('vsm3partw1q_u32', U32, [(U32, 'a'), (U32, 'b'), (U32, 'c')],
       vres('u32', 1, A('sm3partw1 %v0.4s, %v2.4s, %v3.4s', '=w;0;w;w', [('a', 'v'), ('b', 'v'), ('c', 'v')], 'Q', 'sm4')), 'sm4',
       model=sm3partw1)
    fn('vsm3partw2q_u32', U32, [(U32, 'a'), (U32, 'b'), (U32, 'c')],
       vres('u32', 1, A('sm3partw2 %v0.4s, %v2.4s, %v3.4s', '=w;0;w;w', [('a', 'v'), ('b', 'v'), ('c', 'v')], 'Q', 'sm4')), 'sm4',
       model=sm3partw2)
    fn('vsm4eq_u32', U32, [(U32, 'a'), (U32, 'b')],
       vres('u32', 1, A('sm4e %v0.4s, %v2.4s', '=w;0;w', [('a', 'v'), ('b', 'v')], 'Q', 'sm4')), 'sm4')
    fn('vsm4ekeyq_u32', U32, [(U32, 'a'), (U32, 'b')],
       vres('u32', 1, A('sm4ekey %v0.4s, %v1.4s, %v2.4s', '=w;w;w', [('a', 'v'), ('b', 'v')], 'Q', 'sm4')), 'sm4')


def bf16_fhm():
    # ---- BF16: conversions, dot products, matrix multiply, widening multiply-add
    F32, F32D = vt('f32', 1), vt('f32', 0)
    B4, B8 = vt('bf16', 0), vt('bf16', 1)
    fn('vcvt_bf16_f32', B4, [(F32, 'a')], vres('bf16', 0, A('bfcvtn %v0.4h, %v1.4s', '=w;w', [('a', 'v')], 'D', 'bf16')), 'bf16',
       model=L1(lambda x: rnd(x, 'bf16')))
    fn('vcvtq_low_bf16_f32', B8, [(F32, 'a')], vres('bf16', 1, A('bfcvtn %v0.4h, %v1.4s', '=w;w', [('a', 'v')], 'Q', 'bf16')),
       'bf16', model=lambda a: [rnd(x, 'bf16') for x in a] + [0.0] * 4)
    fn('vcvtq_high_bf16_f32', B8, [(B8, 'r'), (F32, 'a')],
       vres('bf16', 1, A('bfcvtn2 %v0.8h, %v2.4s', '=w;0;w', [('r', 'v'), ('a', 'v')], 'Q', 'bf16')), 'bf16',
       model=lambda r, a: r[:4] + [rnd(x, 'bf16') for x in a])
    fn('vcvth_bf16_f32', 'bfloat16_t', [('float', 'a')],
       sres('bf16', A('bfcvt %h0, %s1', '=w;w', [('a', 'f')], 'u', 'bf16')), 'bf16', model=lambda x: rnd(x, 'bf16'))
    fn('vcvtah_f32_bf16', 'float', [('bfloat16_t', 'a')],
       '{ union { unsigned int __u; float __f; } __v; __v.__u = __axiom_b2u(a) << 16; return __v.__f; }', 'bf16',
       model=lambda x: x)
    fn('vcvt_f32_bf16', F32, [(B4, 'a')], vres('f32', 1, A('shll %v0.4s, %v1.4h, #16', '=w;w', [('a', 'v')], 'Q', 'bf16')),
       'bf16', model=L1(lambda x: x))
    fn('vcvtq_low_f32_bf16', F32, [(B8, 'a')], vres('f32', 1, A('shll %v0.4s, %v1.4h, #16', '=w;w', [('a', 'v')], 'Q', 'bf16')),
       'bf16', model=lambda a: a[:4])
    fn('vcvtq_high_f32_bf16', F32, [(B8, 'a')], vres('f32', 1, A('shll2 %v0.4s, %v1.8h, #16', '=w;w', [('a', 'v')], 'Q', 'bf16')),
       'bf16', model=lambda a: a[4:])
    for q in (0, 1):
        T, TB = vt('f32', q), vt('bf16', q)
        bd = (lambda r, a, b: [fadd(r[i], fadd(fmul(a[2 * i], b[2 * i]), fmul(a[2 * i + 1], b[2 * i + 1]))) for i in range(len(r))])
        fn('vbfdot%s_f32' % Qs(q), T, [(T, 'r'), (TB, 'a'), (TB, 'b')],
           vres('f32', q, A('bfdot %s, %s, %s' % (V(0, 'f32', q), V(2, 'bf16', q), V(3, 'bf16', q)), '=w;0;w;w',
                            [('r', 'v'), ('a', 'v'), ('b', 'v')], RES(q), 'bf16')), 'bf16', model=bd, mode='bfdot')
        for lq in (0, 1):
            fn('vbfdot%s_lane%s_f32' % (Qs(q), 'q' if lq else ''), T, [(T, 'r'), (TB, 'a'), (vt('bf16', lq), 'b'), ('const int', 'l')],
               vres('f32', q, A('bfdot %s, %s, %%v3.2h[%%4]' % (V(0, 'f32', q), V(2, 'bf16', q)), '=w;0;w;w;i',
                                [('r', 'v'), ('a', 'v'), ('b', 'v'), ('l', 'i')], RES(q), 'bf16', imm=[(0, nl('f32', lq) - 1)])),
               'bf16', model=lambda r, a, b, l: [fadd(r[i], fadd(fmul(a[2 * i], b[2 * l]), fmul(a[2 * i + 1], b[2 * l + 1])))
                                                 for i in range(len(r))], mode='bfdot', imms={'l': (0, nl('f32', lq) - 1)})
    fn('vbfmmlaq_f32', F32, [(F32, 'r'), (B8, 'a'), (B8, 'b')],
       vres('f32', 1, A('bfmmla %v0.4s, %v2.8h, %v3.8h', '=w;0;w;w', [('r', 'v'), ('a', 'v'), ('b', 'v')], 'Q', 'bf16')), 'bf16',
       model=lambda r, a, b: [r[2 * i + j] + sum(a[4 * i + t] * b[4 * j + t] for t in range(4)) for i in range(2) for j in range(2)],
       mode='bfdot')
    for (c, mn, o) in (('b', 'bfmlalb', 0), ('t', 'bfmlalt', 1)):
        fn('vbfmlal%sq_f32' % c, F32, [(F32, 'r'), (B8, 'a'), (B8, 'b')],
           vres('f32', 1, A('%s %%v0.4s, %%v2.8h, %%v3.8h' % mn, '=w;0;w;w', [('r', 'v'), ('a', 'v'), ('b', 'v')], 'Q', 'bf16')),
           'bf16', model=lambda r, a, b, o=o: [ffma(a[2 * i + o], b[2 * i + o], r[i], 'f32') for i in range(4)])
        for lq in (0, 1):
            fn('vbfmlal%sq_lane%s_f32' % (c, 'q' if lq else ''), F32, [(F32, 'r'), (B8, 'a'), (vt('bf16', lq), 'b'), ('const int', 'l')],
               vres('f32', 1, A('%s %%v0.4s, %%v2.8h, %%v3.h[%%4]' % mn, '=w;0;w;x;i', [('r', 'v'), ('a', 'v'), ('b', 'v'), ('l', 'i')],
                                'Q', 'bf16', imm=[(0, nl('bf16', lq) - 1)])), 'bf16',
               model=lambda r, a, b, l, o=o: [ffma(a[2 * i + o], b[l], r[i], 'f32') for i in range(4)],
               imms={'l': (0, nl('bf16', lq) - 1)})
    # ---- FHM: half-precision products added to single precision
    for (op, mn, sg) in (('fmlal', 'fmlal', 1), ('fmlsl', 'fmlsl', -1)):
        for part in ('low', 'high'):
            m2 = mn + ('2' if part == 'high' else '')
            for q in (0, 1):
                T, TH = vt('f32', q), vt('f16', q)
                n = nl('f32', q)
                off = n if part == 'high' else 0
                fn('v%s_%s%s_f16' % (op, part, Qs(q)) if False else 'v%s%s_%s_f16' % (op, Qs(q), part), T,
                   [(T, 'r'), (TH, 'a'), (TH, 'b')],
                   vres('f32', q, A('%s %s, %%v2.%dh, %%v3.%dh' % (m2, V(0, 'f32', q), n, n), '=w;0;w;w',
                                    [('r', 'v'), ('a', 'v'), ('b', 'v')], RES(q), 'fhm')), 'fhm',
                   model=lambda r, a, b, off=off, sg=sg: [ffma(sg * a[off + i], b[off + i], r[i], 'f32') for i in range(len(r))])
                for lq in (0, 1):
                    fn('v%s%s_lane%s_%s_f16' % (op, Qs(q), 'q' if lq else '', part), T,
                       [(T, 'r'), (TH, 'a'), (vt('f16', lq), 'b'), ('const int', 'l')],
                       vres('f32', q, A('%s %s, %%v2.%dh, %%v3.h[%%4]' % (m2, V(0, 'f32', q), n), '=w;0;w;x;i',
                                        [('r', 'v'), ('a', 'v'), ('b', 'v'), ('l', 'i')], RES(q), 'fhm', imm=[(0, nl('f16', lq) - 1)])),
                       'fhm', model=lambda r, a, b, l, off=off, sg=sg: [ffma(sg * a[off + i], b[l], r[i], 'f32') for i in range(len(r))],
                       imms={'l': (0, nl('f16', lq) - 1)})
    # ---- FP16 scalar arithmetic (<arm_fp16.h>)
    k = 'f16'
    for (op, mn, f) in (('add', 'fadd', fadd), ('sub', 'fsub', fsub), ('mul', 'fmul', fmul), ('div', 'fdiv', fdiv),
                        ('max', 'fmax', fmax), ('min', 'fmin', fmin), ('maxnm', 'fmaxnm', fmaxnm), ('minnm', 'fminnm', fminnm)):
        scalar2(op, k, mn, model=f)


# =============================================================================
# The cryptographic instructions' definitions (the manual's pseudocode)
# =============================================================================
M32 = 0xffffffff
M64 = (1 << 64) - 1


def rol32(x, n):
    n %= 32
    x &= M32
    return ((x << n) | (x >> (32 - n))) & M32


def ror32(x, n):
    return rol32(x, 32 - n % 32)


def rol64(x, n):
    n %= 64
    x &= M64
    return ((x << n) | (x >> (64 - n))) & M64


def ror64(x, n):
    return rol64(x, 64 - n % 64)


def gmul(a, b):
    r = 0
    while b:
        if b & 1:
            r ^= a
        a = (a << 1) ^ (0x11b if a & 0x80 else 0)
        b >>= 1
    return r


def _sbox():
    inv = [0] * 256
    for a in range(1, 256):
        for b in range(1, 256):
            if gmul(a, b) == 1:
                inv[a] = b
                break
    s = []
    for a in range(256):
        x = inv[a]
        y = x
        for i in range(1, 5):
            y ^= ((x << i) | (x >> (8 - i))) & 0xff
        s.append(y ^ 0x63)
    return s


SBOX = _sbox()
ISBOX = [SBOX.index(i) for i in range(256)]


def aes_e(d, k):
    s = [SBOX[(x ^ y) & 0xff] for (x, y) in zip(d, k)]
    return [s[r + 4 * ((c + r) % 4)] for c in range(4) for r in range(4)]


def aes_d(d, k):
    s = [(x ^ y) & 0xff for (x, y) in zip(d, k)]
    s = [s[r + 4 * ((c - r) % 4)] for c in range(4) for r in range(4)]
    return [ISBOX[x] for x in s]


def mix_columns(s, inv=False):
    m = [14, 11, 13, 9] if inv else [2, 3, 1, 1]
    out = []
    for c in range(4):
        col = [s[4 * c + r] & 0xff for r in range(4)]
        for r in range(4):
            v = 0
            for j in range(4):
                v ^= gmul(col[j], m[(j - r) % 4])
            out.append(v)
    return out


def sha_f(f, x, y, z):
    if f == 'ch':
        return ((y ^ z) & x) ^ z
    if f == 'par':
        return x ^ y ^ z
    return (x & y) | ((x | y) & z)


def sha1_rounds(a, e, w, f):
    X = list(a)
    Y = e & M32
    for i in range(4):
        t = sha_f(f, X[1], X[2], X[3])
        Y = (Y + rol32(X[0], 5) + t + w[i]) & M32
        X[1] = rol32(X[1], 30)
        # (<Y, X> = ROL(Y : X, 32): Y takes X's top lane)
        X, Y = [Y] + X[:3], X[3]
    return X


def sha1su1(a, b):
    T = [a[0] ^ b[1], a[1] ^ b[2], a[2] ^ b[3], a[3]]
    r = [rol32(t, 1) for t in T]
    r[3] ^= rol32(T[0], 2)
    return r


def ssig0(x):
    return ror32(x, 7) ^ ror32(x, 18) ^ (x >> 3)


def ssig1(x):
    return ror32(x, 17) ^ ror32(x, 19) ^ (x >> 10)


def sha256su0(a, b):
    T = [a[1], a[2], a[3], b[0]]
    return [(ssig0(T[e]) + a[e]) & M32 for e in range(4)]


def sha256su1(a, b, c):
    T0 = [b[1], b[2], b[3], c[0]]
    T1 = [c[2], c[3]]
    r = [0] * 4
    for e in range(2):
        r[e] = (ssig1(T1[e]) + a[e] + T0[e]) & M32
    for e in range(2, 4):
        r[e] = (ssig1(r[e - 2]) + a[e] + T0[e]) & M32
    return r


def sha256_hash(X, Y, W, part1):
    X = list(X)
    Y = list(Y)
    for e in range(4):
        chs = sha_f('ch', Y[0], Y[1], Y[2])
        maj = sha_f('maj', X[0], X[1], X[2])
        S1 = ror32(Y[0], 6) ^ ror32(Y[0], 11) ^ ror32(Y[0], 25)
        S0 = ror32(X[0], 2) ^ ror32(X[0], 13) ^ ror32(X[0], 22)
        t = (Y[3] + S1 + chs + W[e]) & M32
        X[3] = (t + X[3]) & M32
        Y[3] = (t + S0 + maj) & M32
        # (<Y, X> = ROL(Y : X, 32))
        X, Y = [Y[3]] + X[:3], [X[3]] + Y[:3]
    return X if part1 else Y


def sha512h(W, X, Y):
    s1 = ror64(Y[1], 14) ^ ror64(Y[1], 18) ^ ror64(Y[1], 41)
    hi = (Y[1] & X[0]) ^ (~Y[1] & X[1])
    hi = (hi + s1 + W[1]) & M64
    tmp = (hi + Y[0]) & M64
    s1 = ror64(tmp, 14) ^ ror64(tmp, 18) ^ ror64(tmp, 41)
    lo = (tmp & Y[1]) ^ (~tmp & X[0])
    lo = (lo + s1 + W[0]) & M64
    return [lo, hi]


def sha512h2(W, X, Y):
    s0 = ror64(Y[0], 28) ^ ror64(Y[0], 34) ^ ror64(Y[0], 39)
    hi = (X[0] & Y[1]) ^ (X[0] & Y[0]) ^ (Y[1] & Y[0])
    hi = (hi + s0 + W[1]) & M64
    s0 = ror64(hi, 28) ^ ror64(hi, 34) ^ ror64(hi, 39)
    lo = (hi & Y[0]) ^ (hi & Y[1]) ^ (Y[1] & Y[0])
    lo = (lo + s0 + W[0]) & M64
    return [lo, hi]


def sha512su0(W, X):
    s0 = ror64(W[1], 1) ^ ror64(W[1], 8) ^ (W[1] >> 7)
    lo = (W[0] + s0) & M64
    s0 = ror64(X[0], 1) ^ ror64(X[0], 8) ^ (X[0] >> 7)
    hi = (W[1] + s0) & M64
    return [lo, hi]


def sha512su1(W, X, Y):
    s1 = ror64(X[1], 19) ^ ror64(X[1], 61) ^ (X[1] >> 6)
    hi = (W[1] + s1 + Y[1]) & M64
    s1 = ror64(X[0], 19) ^ ror64(X[0], 61) ^ (X[0] >> 6)
    lo = (W[0] + s1 + Y[0]) & M64
    return [lo, hi]


def _sm3tt(d, n, m, i, kind):
    w = m[i]
    if kind == '1a':
        tt = d[3] ^ d[2] ^ d[1]
    elif kind == '1b':
        tt = (d[3] & d[2]) | (d[3] & d[1]) | (d[2] & d[1])
    elif kind == '2a':
        tt = d[1] ^ (d[3] ^ d[2])
    else:
        tt = (d[3] & d[2]) | (~d[3] & d[1])
    if kind[0] == '1':
        ss2 = n[3] ^ rol32(d[3], 12)
        tt = (tt + d[0] + ss2 + w) & M32
        return [d[1], rol32(d[2], 9), d[3], tt]
    tt = (tt + d[0] + n[3] + w) & M32
    return [d[1], rol32(d[2], 19), d[3], tt ^ rol32(tt, 9) ^ rol32(tt, 17)]


def sm3tt1a(d, n, m, i):
    return _sm3tt(d, n, m, i, '1a')


def sm3tt1b(d, n, m, i):
    return _sm3tt(d, n, m, i, '1b')


def sm3tt2a(d, n, m, i):
    return _sm3tt(d, n, m, i, '2a')


def sm3tt2b(d, n, m, i):
    return _sm3tt(d, n, m, i, '2b')


def p1(x):
    return x ^ rol32(x, 15) ^ rol32(x, 23)


def sm3partw1(d, n, m):
    r = [d[0] ^ n[0] ^ rol32(m[1], 15), d[1] ^ n[1] ^ rol32(m[2], 15), d[2] ^ n[2] ^ rol32(m[3], 15), 0]
    for i in range(4):
        if i == 3:
            r[3] = d[3] ^ n[3] ^ rol32(r[0], 15)
        r[i] = p1(r[i])
    return r


def sm3partw2(d, n, m):
    t = [n[i] ^ rol32(m[i], 7) for i in range(4)]
    r = [d[i] ^ t[i] for i in range(4)]
    t2 = rol32(t[0], 15)
    t2 = t2 ^ rol32(t2, 15) ^ rol32(t2, 23)
    r[3] ^= t2
    return r


def acle():
    """<arm_acle.h>: CRC32, the bit operations, barriers and hints, prefetches, and the floating helpers"""
    global HDR
    HDR = 'arm_acle.h'
    for (c, b, t) in (('b', 8, 'uint8_t'), ('h', 16, 'uint16_t'), ('w', 32, 'uint32_t'), ('d', 64, 'uint64_t')):
        for cc in ('', 'c'):
            r2 = '%x2' if b == 64 else '%w2'
            fn('__crc32%s%s' % (cc, c), 'uint32_t', [('uint32_t', 'a'), (t, 'b')],
               cast('uint32_t', A('crc32%s%s %%w0, %%w1, %s' % (cc, 'x' if b == 64 else c, r2), '=r;r;r', [('a', 'n'), ('b', 'n')],
                                  'u', 'crc')), 'crc', model=lambda a, x, b=b, cc=cc: crc32(a, x, b, cc))
    for (n, t, w) in (('', 'uint32_t', 32), ('ll', 'uint64_t', 64)):
        r = '%x' if w == 64 else '%w'
        code = 'L' if w == 64 else 'u'
        fn('__clz' + n, 'unsigned int', [(t, 'a')], cast('unsigned int', A('clz %s0, %s1' % (r, r), '=r;r', [('a', 'n')], code)),
           model=lambda x, w=w: clz_(x, w))
        fn('__cls' + n, 'unsigned int', [(t, 'a')], cast('unsigned int', A('cls %s0, %s1' % (r, r), '=r;r', [('a', 'n')], code)),
           model=lambda x, w=w: cls_(x, w))
        fn('__rev' + n, t, [(t, 'a')], cast(t, A('rev %s0, %s1' % (r, r), '=r;r', [('a', 'n')], code)),
           model=lambda x, w=w: int.from_bytes(ubits(x, w).to_bytes(w // 8, 'little'), 'big'))
        fn('__rev16' + n, t, [(t, 'a')], cast(t, A('rev16 %s0, %s1' % (r, r), '=r;r', [('a', 'n')], code)),
           model=lambda x, w=w: sum(((ubits(x, w) >> (16 * i)) & 0xff) << (16 * i + 8) | ((ubits(x, w) >> (16 * i + 8)) & 0xff) << (16 * i)
                                    for i in range(w // 16)))
        fn('__rbit' + n, t, [(t, 'a')], cast(t, A('rbit %s0, %s1' % (r, r), '=r;r', [('a', 'n')], code)),
           model=lambda x, w=w: rbit_(x, w))
        fn('__ror' + n, t, [(t, 'a'), ('uint32_t', 'b')], cast(t, A('ror %s0, %s1, %s2' % (r, r, r), '=r;r;r', [('a', 'n'), ('b', 'n')], code)),
           model=lambda x, y, w=w: (ubits(x, w) >> (y % w) | ubits(x, w) << (w - y % w)) & ((1 << w) - 1))
    for f in ('clz', 'cls', 'rev', 'rev16', 'rbit'):
        rt = 'unsigned int' if f in ('clz', 'cls') else 'unsigned long'
        fn('__%sl' % f, rt, [('unsigned long', 'a')], '(%s)(sizeof(long) == 8 ? __%sll(a) : __%s((uint32_t)a))' % (rt, f, f), test='no')
    fn('__rorl', 'unsigned long', [('unsigned long', 'a'), ('uint32_t', 'b')],
       '(unsigned long)(sizeof(long) == 8 ? __rorll(a, b) : __ror((uint32_t)a, b))', test='no')
    fn('__revsh', 'int16_t', [('int16_t', 'a')], '(int16_t)(uint16_t)(((uint16_t)a >> 8) | ((uint16_t)a << 8))',
       model=lambda x: wrap(((x & 0xff) << 8) | ((x >> 8) & 0xff), 's16'))
    for (n, ins) in (('__dmb', 'dmb'), ('__dsb', 'dsb'), ('__isb', 'isb')):
        fn(n, 'void', [('const unsigned int', 'n')], A('%s #%%0' % ins, 'i', [('n', 'i')], 'v', flags='M', imm=[(0, 15)]),
           imms={'n': (0, 15)}, test='no')
    for (n, ins) in (('__nop', 'nop'), ('__yield', 'yield'), ('__wfe', 'wfe'), ('__wfi', 'wfi'), ('__sev', 'sev'), ('__sevl', 'sevl')):
        fn(n, 'void', [], A(ins, '', [], 'v', flags='V'), test='no')
    fn('__pld', 'void', [('const void *', 'p')], A('prfm pldl1keep, [%0]', 'r', [('p', 'p')], 'v', flags='V'), test='no')
    fn('__pli', 'void', [('const void *', 'p')], A('prfm plil1keep, [%0]', 'r', [('p', 'p')], 'v', flags='V'), test='no')
    fn('__pldx', 'void', [('unsigned int', 'a'), ('unsigned int', 'b'), ('unsigned int', 'c'), ('const void *', 'p')],
       A('prfm #%0, [%1]', 'i;r', [('(((a) << 4) | ((b) << 1) | (c))', 'i'), ('p', 'p')], 'v', flags='V', imm=[(0, 31)]),
       imms={'a': (0, 1), 'b': (0, 2), 'c': (0, 1)}, test='no')
    fn('__plix', 'void', [('unsigned int', 'b'), ('unsigned int', 'c'), ('const void *', 'p')],
       A('prfm #%0, [%1]', 'i;r', [('(8 | ((b) << 1) | (c))', 'i'), ('p', 'p')], 'v', flags='V', imm=[(0, 31)]),
       imms={'b': (0, 2), 'c': (0, 1)}, test='no')
    fn('__sqrt', 'double', [('double', 'a')], cast('double', A('fsqrt %d0, %d1', '=w;w', [('a', 'f')], 'd')),
       model=lambda x: fsqrt(x, 'f64'))
    fn('__sqrtf', 'float', [('float', 'a')], cast('float', A('fsqrt %s0, %s1', '=w;w', [('a', 'f')], 'f')),
       model=lambda x: fsqrt(x, 'f32'))
    fn('__fma', 'double', [('double', 'a'), ('double', 'b'), ('double', 'c')],
       cast('double', A('fmadd %d0, %d1, %d2, %d3', '=w;w;w;w', [('a', 'f'), ('b', 'f'), ('c', 'f')], 'd')),
       model=lambda a, b, c: ffma(a, b, c, 'f64'))
    fn('__fmaf', 'float', [('float', 'a'), ('float', 'b'), ('float', 'c')],
       cast('float', A('fmadd %s0, %s1, %s2, %s3', '=w;w;w;w', [('a', 'f'), ('b', 'f'), ('c', 'f')], 'f')),
       model=lambda a, b, c: ffma(a, b, c, 'f32'))
    fn('__rintn', 'double', [('double', 'a')], cast('double', A('frintn %d0, %d1', '=w;w', [('a', 'f')], 'd')),
       model=lambda x: fround(x, 'n'))
    fn('__rintnf', 'float', [('float', 'a')], cast('float', A('frintn %s0, %s1', '=w;w', [('a', 'f')], 'f')),
       model=lambda x: fround(x, 'n'))
    HDR = 'arm_neon.h'


def crc32(a, x, b, c):
    poly = 0x82f63b78 if c else 0xedb88320
    r = a & M32
    for i in range(b // 8):
        r ^= (x >> (8 * i)) & 0xff
        for _ in range(8):
            r = (r >> 1) ^ (poly if r & 1 else 0)
    return r


def build():
    global CAT
    for (c, f) in (('arith', arith), ('mla', mla_family), ('widen', widen_family), ('cmp', compares), ('shift', shifts),
                   ('logic', logic_unary), ('conv', conversions), ('data', data_movement), ('mem', loads_stores),
                   ('dot', dot_crypto), ('bf16', bf16_fhm), ('acle', acle)):
        CAT = c
        f()


# =============================================================================
# Output
# =============================================================================
def hdr_of(e):
    if e['hdr'] != 'arm_neon.h':
        return e['hdr']
    ts = [e['ret']] + [t for (t, _) in e['params']]
    if any(t in VEC or t in STRUCT for t in ts):
        return 'arm_neon.h'
    if any('bfloat16_t' in t for t in ts):
        return 'arm_bf16.h'
    if any('float16_t' in t for t in ts):
        return 'arm_fp16.h'
    return 'arm_neon.h'


def ident(t):
    return '__axiom_as_' + re.sub(r'\W+', '_', t.replace('*', ' p')).strip('_')


USED_IDENT = {}


def emit(e):
    name, ret, params, body = e['name'], e['ret'], e['params'], e['body']
    pn = [n for (_, n) in params]
    imms = e['imms']
    if imms:
        b = ex(body, pn)
        if b.lstrip().startswith('{'):
            return emit_lane_switch(e)
        # (the operands that are not immediates, checked as a call's arguments would be)
        for (t, n) in params:
            if n in imms:
                b = re.sub(r'(?<![\w.])__%s\b' % n, '(__%s)' % n, b)
            else:
                USED_IDENT[t] = hdr_of(e)
                b = re.sub(r'(?<![\w.])__%s\b' % n, '%s(__%s)' % (ident(t), n), b)
        chk = ''.join('(void)sizeof(struct { int __axiom_imm : ((__%s) >= %d && (__%s) <= %d) ? 1 : -1; }), ' % (n, lo, n, hi)
                      for (n, (lo, hi)) in imms.items())
        return '#define %s(%s) (%s%s)\n' % (name, ', '.join('__' + n for n in pn), chk, b)
    b = ex(body, pn)
    sig = ', '.join('%s __%s' % (t, n) for (t, n) in params) or 'void'
    if not b.startswith('{'):
        b = '{ return %s; }' % b if ret != 'void' else '{ %s; }' % b
    if e['ext'] == '-':
        return '__AXIOM_FN0 %s %s(%s) %s\n' % (ret, name, sig, b)
    return '__AXIOM_FN("%s") %s %s(%s) %s\n' % (e['ext'], ret, name, sig, b)


def emit_lane_switch(e):
    """an intrinsic with an immediate whose body is a block: a function of a
    switch over the immediate's values (each case a constant), and a macro
    that checks the immediate"""
    name, ret, params, body = e['name'], e['ret'], e['params'], e['body']
    pn = [n for (_, n) in params]
    (ln, (lo, hi)), = e['imms'].items()
    cases = []
    for v in range(lo, hi + 1):
        b = ex(body, [n for n in pn if n != ln])
        b = re.sub(r'(?<![\w.])%s\b' % ln, str(v), b)
        cases.append('case %d: %s%s' % (v, b, ' break;' if ret == 'void' else ''))
    sig = ', '.join('%s __%s' % (t, n) for (t, n) in params)
    head = '__AXIOM_FN0' if e['ext'] == '-' else '__AXIOM_FN("%s")' % e['ext']
    f = '%s %s __axiom_%s(%s) {\n    switch (__%s) {\n%s    default: %s\n    }\n}\n' % (
        head, ret, name, sig, ln, ''.join('    %s\n' % c for c in cases), cases[0].split(': ', 1)[1])
    chk = '(void)sizeof(struct { int __axiom_imm : ((__%s) >= %d && (__%s) <= %d) ? 1 : -1; }), ' % (ln, lo, ln, hi)
    m = '#define %s(%s) (%s__axiom_%s(%s))\n' % (name, ', '.join('__' + n for n in pn), chk, name, ', '.join('__' + n for n in pn))
    return f + m


VEC_TYPES = r'''/* The vector types: of 64 bits (in a D register: the IR holds them as their
   bits) and 128 bits (a Q register), of each lane type; the structures of 2, 3
   and 4 of them (the multiple-register loads and stores, the tables) */
'''

FP16_PRE = r'''/* The function of an intrinsic: inlined into its callers, which must be
   compiled with its extension (C3240 otherwise), and compiled with it. */
#ifndef __AXIOM_FN
#define __AXIOM_FN(e) static __axiom_attribute__((__always_inline__, __target__(e), __unused__))
#define __AXIOM_FN0 static __axiom_attribute__((__always_inline__, __unused__))
#endif
typedef _Float16 float16_t;
/* (a half-precision value's bits, and back: the machine operations take them in a register) */
__AXIOM_FN0 unsigned int __axiom_h2u(float16_t __x) {
    union { float16_t __f; unsigned short __u; } __v;
    __v.__f = __x;
    return __v.__u;
}
__AXIOM_FN0 float16_t __axiom_u2h(unsigned int __x) {
    union { float16_t __f; unsigned short __u; } __v;
    __v.__u = (unsigned short)__x;
    return __v.__f;
}
'''

BF16_PRE = r'''#ifndef __AXIOM_FN
#define __AXIOM_FN(e) static __axiom_attribute__((__always_inline__, __target__(e), __unused__))
#define __AXIOM_FN0 static __axiom_attribute__((__always_inline__, __unused__))
#endif
typedef __axiom_bf16 bfloat16_t;
__AXIOM_FN0 unsigned int __axiom_b2u(bfloat16_t __x) {
    union { bfloat16_t __f; unsigned short __u; } __v;
    __v.__f = __x;
    return __v.__u;
}
__AXIOM_FN0 bfloat16_t __axiom_u2b(unsigned int __x) {
    union { bfloat16_t __f; unsigned short __u; } __v;
    __v.__u = (unsigned short)__x;
    return __v.__f;
}
'''

STDINT = r'''#if __STDC_VERSION__ >= 199901L
#include <stdint.h>
#elif !defined(__AXIOM_ARM_INTN)
/* (C89, which has no <stdint.h>: the exact-width types the intrinsics are declared with) */
#define __AXIOM_ARM_INTN
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef __AXIOM_INT64_TYPE__ int64_t;
typedef __AXIOM_UINT64_TYPE__ uint64_t;
#endif
'''

ACLE_PRE = r'''#ifndef __AXIOM_FN
#define __AXIOM_FN(e) static __axiom_attribute__((__always_inline__, __target__(e), __unused__))
#define __AXIOM_FN0 static __axiom_attribute__((__always_inline__, __unused__))
#endif
'''


def neon_pre():
    out = ['typedef float float32_t;\ntypedef double float64_t;\n',
           'typedef uint8_t poly8_t;\ntypedef uint16_t poly16_t;\ntypedef uint64_t poly64_t;\n',
           'typedef unsigned __axiom_int128 poly128_t;\n', VEC_TYPES]
    for k in KINDS:
        for q in (0, 1):
            out.append('typedef %s %s __axiom_attribute__((__vector_size__(%d)));\n' % (st(k), vt(k, q), 16 if q else 8))
    for k in KINDS:
        for q in (0, 1):
            for n in (2, 3, 4):
                out.append('typedef struct %s {\n    %s val[%d];\n} %s;\n' % (xt(k, q, n), vt(k, q), n, xt(k, q, n)))
    out.append('/* (the unaligned forms: the loads and stores of vld1, vst1 ...) */\n')
    out.append('typedef double __axiom_f64u __axiom_attribute__((__aligned__(1), __may_alias__));\n')
    for k in KINDS:
        for q in (0, 1):
            out.append('typedef %s %s __axiom_attribute__((__aligned__(1), __may_alias__));\n' % (vt(k, q), uty(k, q)))
    out.append(r'''/* (a 128-bit polynomial's bits in a vector, and back) */
__AXIOM_FN0 poly128_t __axiom_q2p128(uint8x16_t __x) {
    union { uint8x16_t __v; poly128_t __p; } __u;
    __u.__v = __x;
    return __u.__p;
}
__AXIOM_FN0 uint8x16_t __axiom_p1282q(poly128_t __x) {
    union { uint8x16_t __v; poly128_t __p; } __u;
    __u.__p = __x;
    return __u.__v;
}
''')
    return ''.join(out)


HEADERS = [  # name, what, includes
    ('arm_fp16.h', 'the half-precision scalar intrinsics (FP16) and float16_t', []),
    ('arm_bf16.h', 'bfloat16_t and its scalar conversions (BF16)', ['arm_fp16.h']),
    ('arm_neon.h', 'Advanced SIMD (with the FP16, BF16, dot product, i8mm, FHM, RDM and cryptographic extensions)',
     ['stdint.h', 'arm_fp16.h', 'arm_bf16.h']),
    ('arm_acle.h', 'ACLE: CRC32, the bit operations, barriers, hints and prefetches', ['stdint.h']),
]


def header_text(h, what, incs):
    g = '__AXIOM_' + re.sub(r'\W', '_', h.upper())
    out = ['/* <%s> - the AArch64 intrinsics of %s (the compiler\'s: docs/c.md,\n' % (h, what),
           '   "Intrinsics"). Generated by cmd/internal/tools/gen_neon_intrin.py: do not edit. */\n',
           '#ifndef %s\n#define %s\n' % (g, g),
           '#if !defined(__aarch64__)\n#error "<%s> is for AArch64"\n#endif\n' % h]
    for i in incs:
        if i == 'stdint.h':
            out.append(STDINT)
        else:
            out.append('#include <%s>\n' % i)
    if h == 'arm_fp16.h':
        out.append(FP16_PRE)
    if h == 'arm_bf16.h':
        out.append(BF16_PRE)
    if h == 'arm_acle.h':
        out.append(ACLE_PRE)
    if h == 'arm_neon.h':
        out.append(neon_pre())
    body = [emit(e) for e in INTRIN if hdr_of(e) == h]
    # (the identity functions of the macros' operands)
    ids = []
    for (t, hh) in sorted(USED_IDENT.items()):
        if hh == h:
            g = '__AXIOM_AS_' + ident(t)[len('__axiom_as_'):].upper()
            ids.append('#ifndef %s\n#define %s\n__AXIOM_FN0 %s %s(%s __x) { return __x; }\n#endif\n' % (g, g, t, ident(t), t))
    out += ids + body
    out.append('#endif\n')
    return ''.join(out)


def write(path, text):
    with open(path, 'w') as f:
        f.write(text)


def table_text():
    keep_x, keep_w = [], []
    for l in open(TAB).read().split('\n'):
        m = re.match(r'    "([^|]*)\|([^|])\|(.*)",$', l)
        if not m:
            continue
        if m.group(2) in 'x*':
            keep_x.append(l)
        elif m.group(2) == 'w':
            keep_w.append(l)
    mine = []
    for (name, ext, sig, res, tmpl, cons, flags, imm, clob) in OPLIST:
        f = [name, 'a', ext, sig, res, 'c', tmpl, cons, clob, flags, imm]
        while f and f[-1] == '':
            f.pop()
        line = '|'.join(f)
        mine.append('    "%s",' % line.replace('\\', '\\\\').replace('"', '\\"').replace('\n', '\\n'))
    lines = keep_x + mine + keep_w
    src = open(TAB).read()
    head = src[:src.index('pub let TABLE')]
    return head + 'pub let TABLE: [%d][]u8 = [\n' % len(lines) + '\n'.join(lines) + '\n];\n'


def main():
    build()
    texts = {h: header_text(h, w, i) for (h, w, i) in HEADERS}
    # (USED_IDENT is filled while the headers are made: again, with all of it)
    texts = {h: header_text(h, w, i) for (h, w, i) in HEADERS}
    for (h, t) in texts.items():
        write(os.path.join(INC, h), t)
    write(TAB, table_text())
    tests()


FEATURES_ALL = 'aes,sha2,sha3,sha512,sm4,crc,rdm,dotprod,i8mm,fp16,fhm,bf16'


def arg_of(t, n, imms):
    """an argument of C type t for the compile test"""
    if n in imms:
        return str(imms[n][0])
    t = t.strip()
    if t.endswith('*'):
        return '(%s)(void *)__buf' % t
    return '*(const %s *)(const void *)__buf' % t.replace('const ', '')


def compile_test():
    """every intrinsic, called once, in a program built with every extension"""
    out = ['/* Every AArch64 intrinsic of <arm_neon.h> and <arm_acle.h> is compiled (each with\n',
           '   its operands; immediates their least value): its instruction assembles.\n',
           '   Generated by cmd/internal/tools/gen_neon_intrin.py: do not edit. */\n',
           '/* aarch64-only */\n/* flags: --features=+%s */\n' % FEATURES_ALL.replace(',', ',+'),
           '/* expect: compiled */\n',
           '#include <arm_neon.h>\n#include <arm_acle.h>\n#include "host.h"\n\n',
           'static __axiom_attribute__((__aligned__(64))) unsigned char __buf[256];\n',
           'volatile int __sink;\n\n']
    for (k, e) in enumerate(INTRIN):
        call = '%s(%s)' % (e['name'], ', '.join(arg_of(t, n, e['imms']) for (t, n) in e['params']))
        out.append('void __t%d(void) {\n' % k)
        if e['ret'] == 'void':
            out.append('    %s;\n' % call)
        else:
            out.append('    %s __r = %s;\n' % (e['ret'], call))
            out.append('    __sink = *(volatile unsigned char *)(void *)&__r;\n')
        out.append('}\n')
    out.append("\nint main(void) {\n    print_str(\"compiled\", 8);\n    return 0;\n}\n")
    return ''.join(out)


# -----------------------------------------------------------------------------
# Semantic tests: each intrinsic applied to inputs of random lanes and edge
# values (NaNs, zeros, infinities, the saturation bounds, shift counts at and
# beyond the width), its result compared with the reference computed here
# from the instructions' definitions
# -----------------------------------------------------------------------------
RNG = random.Random(20261114)


def kind_of(t):
    t = t.replace('const ', '').strip()
    if t in VEC:
        return ('v',) + VEC[t]
    if t in STRUCT:
        return ('x',) + STRUCT[t]
    if t.endswith('*'):
        b = t[:-1].strip()
        return ('p', SCAL.get(b, 'u8'))
    if t in SCAL:
        return ('s', SCAL[t])
    if t in ('unsigned long', 'uint32_t'):
        return ('s', 'u32')
    return ('s', 's32')


EDGE_F = {
    'f16': [0x0000, 0x8000, 0x3c00, 0xbc00, 0x3800, 0x7c00, 0xfc00, 0x7e00, 0x7d00, 0x7bff, 0x0400, 0x0001, 0x3e00,
            0x4100, 0xc100, 0x4200, 0x7800, 0x5bff],
    'bf16': [0x0000, 0x8000, 0x3f80, 0xbf80, 0x3f00, 0x7f80, 0xff80, 0x7fc0, 0x7fa0, 0x7f7f, 0x0080, 0x0001, 0x3fc0,
             0x4020, 0xc020],
    'f32': [0x00000000, 0x80000000, 0x3f800000, 0xbf800000, 0x3f000000, 0x7f800000, 0xff800000, 0x7fc00000, 0x7fa00000,
            0x7f7fffff, 0x00800000, 0x00000001, 0x3fc00000, 0x40200000, 0xc0200000, 0x4f000000, 0xcf000000, 0x4f800000,
            0x5f000000, 0x3effffff, 0x40400000],
    'f64': [0, 1 << 63, 0x3ff0000000000000, 0xbff0000000000000, 0x3fe0000000000000, 0x7ff0000000000000, 0xfff0000000000000,
            0x7ff8000000000000, 0x7ff4000000000000, 0x7fefffffffffffff, 0x0010000000000000, 1, 0x3ff8000000000000,
            0x4004000000000000, 0xc004000000000000, 0x43e0000000000000, 0xc3e0000000000000, 0x41e0000000000000,
            0x43f0000000000000, 0x4008000000000000],
}


def rand_float_bits(k, mode):
    m, e = FMT[k]
    bias = (1 << (e - 1)) - 1
    if mode == 'bfdot':
        return fenc(float(RNG.randint(-6, 6)), k)
    if mode == 'approx':
        r = RNG.random()
        if r < 0.15:
            return RNG.choice([fenc(0.0, k), fenc(-0.0, k), fenc(math.inf, k), fenc(-math.inf, k), fenc(NAN, k)])
        x = (1 + RNG.random()) * 2.0 ** RNG.randint(-12, 12) * RNG.choice([1, -1])
        return fenc(x, k)
    r = RNG.random()
    if r < 0.3:
        return RNG.choice(EDGE_F[k])
    if r < 0.5:
        # (an integral value, or a half: the conversions' and roundings' cases)
        x = RNG.randint(-300, 300) / RNG.choice([1, 2, 4])
        return fenc(float(x), k)
    ex_ = RNG.randint(max(1, bias - 30), min((1 << e) - 2, bias + 30))
    return (RNG.randint(0, 1) << (m + e)) | (ex_ << m) | RNG.getrandbits(m)


def rand_int(k):
    lo, hi = lo_hi(k)
    b = eb(k)
    r = RNG.random()
    if r < 0.25:
        return RNG.choice([0, 1, lo, hi, lo + 1, hi - 1, -1 if lo < 0 else 2, b, b - 1])
    if r < 0.45:
        return max(lo, min(hi, RNG.randint(-70, 70)))
    return RNG.randint(lo, hi)


def rand_lane(k, mode):
    """a lane's bits"""
    if k in FMT:
        return rand_float_bits(k, mode)
    if k == 'p128':
        return RNG.getrandbits(128)
    return ubits(rand_int(k), eb(k))


def lane_bytes(bits, k):
    return ubits(bits, eb(k)).to_bytes(eb(k) // 8, 'little')


def decode(bits, k, raw):
    """a lane's value for the models: an integer of the kind's sign, a float (raw: the bits)"""
    if k in FMT:
        return bits if raw else fdec(bits, k)
    if k[0] == 's':
        return wrap(bits, k)
    return bits


def encode(v, k, raw):
    if k in FMT and not raw:
        return fenc(v, k)
    return ubits(int(v), eb(k))


class TestFile:
    def __init__(self, name, what):
        self.name = name
        self.what = what
        self.pool = bytearray()
        self.cases = []
        self.decl = []

    def put(self, data):
        while len(self.pool) % 16:
            self.pool.append(0)
        off = len(self.pool)
        self.pool += data
        return off


def res_mode(e):
    """the comparison of a result: 0 exact; 1, 2, 3 floating lanes of 2, 4, 8 bytes (any NaN for a NaN); 4, 5, 6 those
    within a relative 2^-7 (the estimates)"""
    if e['raw']:
        return 0
    rt = e['ret']
    if e['model'] and isinstance(e['model'], tuple):
        return 0
    kk = kind_of(rt)
    if e['ret'] == 'void':
        p = [kind_of(t) for (t, _) in e['params'] if t.strip().endswith('*')]
        k = p[0][1] if p else 'u8'
    else:
        k = kk[1]
    if k not in FMT:
        return 0
    w = {16: 1, 32: 2, 64: 3}[eb(k)]
    return w + 3 if e['mode'] == 'approx' else w


def make_case(tf, e):
    """the C statement of one check of intrinsic e, and its expected bytes"""
    args = []
    vals = []
    store = None
    mode = e['mode']
    raw = e['raw']
    for (t, n) in e['params']:
        if n in e['imms']:
            lo, hi = e['imms'][n]
            v = RNG.choice([lo, hi, RNG.randint(lo, hi)])
            args.append(str(v))
            vals.append(v)
            continue
        kk = kind_of(t)
        if kk[0] == 'v':
            (_, k, q) = kk
            bits = [rand_lane(k, mode) for _ in range(nl(k, q))]
            off = tf.put(b''.join(lane_bytes(x, k) for x in bits))
            args.append('__L(%s, %d)' % (t, off))
            vals.append([decode(x, k, raw) for x in bits])
        elif kk[0] == 'x':
            (_, k, q, c) = kk
            bits = [[rand_lane(k, mode) for _ in range(nl(k, q))] for _ in range(c)]
            off = tf.put(b''.join(lane_bytes(x, k) for v in bits for x in v))
            args.append('__L(%s, %d)' % (t, off))
            vals.append([[decode(x, k, raw) for x in v] for v in bits])
        elif kk[0] == 'p':
            k = kk[1]
            nel = 64 // (eb(k) // 8)
            if 'const' in t:
                bits = [rand_lane(k, mode) for _ in range(nel)]
                off = tf.put(b''.join(lane_bytes(x, k) for x in bits))
                args.append('(%s)(const void *)(__pool + %d)' % (t, off))
                vals.append([decode(x, k, raw) for x in bits])
            else:
                store = k
                args.append('(%s)(void *)__out' % t)
                vals.append(None)
        else:
            k = kk[1]
            bits = rand_lane(k, mode)
            off = tf.put(lane_bytes(bits, k))
            args.append('__L(%s, %d)' % (t, off))
            vals.append(decode(bits, k, raw))
    try:
        r = e['model'](*vals)
    except (ZeroDivisionError, OverflowError, ValueError) as x:
        raise Exception('%s: %s %r' % (e['name'], x, vals))
    call = '%s(%s)' % (e['name'], ', '.join(args))
    if store:
        assert r[0] == 'mem', e['name']
        want = bytearray([0x5a] * 64)
        data = b''.join(encode(x, store, raw).to_bytes(eb(store) // 8, 'little') for x in r[1])
        want[:len(data)] = data
        return ('__fill(); %s; chk("%s", __out, 64, %s, %d);' % (call, e['name'], cstr(bytes(want)), res_mode(e)))
    kk = kind_of(e['ret'])
    if kk[0] == 'v':
        (_, k, q) = kk
        assert len(r) == nl(k, q), (e['name'], len(r))
        data = b''.join(encode(x, k, raw).to_bytes(eb(k) // 8, 'little') for x in r)
    elif kk[0] == 'x':
        (_, k, q, c) = kk
        data = b''.join(encode(x, k, raw).to_bytes(eb(k) // 8, 'little') for v in r for x in v)
    else:
        k = kk[1]
        data = encode(r, k, raw).to_bytes(eb(k) // 8, 'little')
    return ('{ %s __r = %s; chk("%s", &__r, %d, %s, %d); }' % (e['ret'], call, e['name'], len(data), cstr(data), res_mode(e)))


def cstr(b):
    return '"' + ''.join('\\x%02x' % c for c in b) + '"'


HARNESS = r"""
static int __checks;
static int __fails;
static char __line[512];
static size_t __len;
static unsigned char __out[64];

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

static void __fill(void) {
    int i;
    for (i = 0; i < 64; i++) {
        __out[i] = 0x5a;
    }
}

/* A floating lane of w bytes at p: its value, and whether it is a NaN */
static double lane(const unsigned char *p, int w, int *nan) {
    unsigned long long b = 0;
    int i;
    for (i = w - 1; i >= 0; i--) {
        b = b << 8 | p[i];
    }
    if (w == 2) {
        int e = (int)(b >> 10 & 31);
        double m = (double)(b & 1023);
        double v;
        *nan = e == 31 && (b & 1023) != 0;
        if (e == 31) {
            v = 1e300 * 1e300;
        } else if (e == 0) {
            v = m / 1024.0 / 16384.0;
        } else {
            v = (1.0 + m / 1024.0);
            while (e > 15) {
                v *= 2.0;
                e--;
            }
            while (e < 15) {
                v /= 2.0;
                e++;
            }
        }
        return (b >> 15) ? -v : v;
    }
    if (w == 4) {
        union { unsigned int u; float f; } x;
        x.u = (unsigned int)b;
        *nan = x.f != x.f;
        return x.f;
    }
    {
        union { unsigned long long u; double f; } x;
        x.u = b;
        *nan = x.f != x.f;
        return x.f;
    }
}

/* The result's n bytes at r against w's: mode 0 exact; 1, 2, 3 floating lanes of 2, 4, 8 bytes, a NaN for a NaN;
   4, 5, 6 those within a relative 2^-7 */
static void chk(const char *name, const void *r, int n, const char *w, int mode) {
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
    if (bad && mode != 0) {
        int lw = mode == 1 || mode == 4 ? 2 : mode == 2 || mode == 5 ? 4 : 8;
        bad = 0;
        for (i = 0; i + lw <= n; i += lw) {
            int an;
            int bn;
            double x = lane(p + i, lw, &an);
            double y = lane(q + i, lw, &bn);
            int j;
            int same = 1;
            for (j = 0; j < lw; j++) {
                if (p[i + j] != q[i + j]) {
                    same = 0;
                }
            }
            if (same || (an && bn)) {
                continue;
            }
            if (mode >= 4 && !an && !bn && y != 0 && x / y > 1 - 1.0 / 128 && x / y < 1 + 1.0 / 128) {
                continue;
            }
            bad = 1;
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

"""


def test_text(tf):
    out = ['/* %s - AArch64 intrinsics checked against the definitions of their\n' % tf.name,
           '   instructions (computed by cmd/internal/tools/gen_neon_intrin.py, which\n',
           '   writes this file: do not edit): %s. A mismatch prints the intrinsic\n' % tf.what,
           '   and the result\'s bytes. */\n',
           '/* aarch64-only */\n/* flags: --c-std=%s --features=+%s */\n' % (tf.std, FEATURES_ALL.replace(',', ',+')),
           '/* expect: %s %d */\n' % (tf.name, len(tf.cases)),
           '#include <arm_neon.h>\n#include <arm_acle.h>\n#include "host.h"\n\n',
           '#define __L(t, o) (*(const t *)(const void *)(__pool + (o)))\n']
    lines = ['    "%s"\n' % ''.join('\\x%02x' % c for c in tf.pool[i:i + 32]) for i in range(0, len(tf.pool), 32)]
    out.append('static __axiom_attribute__((__aligned__(64))) unsigned char __pool[%d] =\n%s;\n' % (
        len(tf.pool), ''.join(lines).rstrip('\n')))
    out.append(HARNESS)
    # (the cases in functions of 16: each is compiled in a moment)
    parts = [tf.cases[j:j + 16] for j in range(0, len(tf.cases), 16)]
    for (k, part) in enumerate(parts):
        out.append('static void run%d(void) {\n' % k)
        for c in part:
            out.append('    %s\n' % c)
        out.append('}\n\n')
    out.append('int main(void) {\n    if (__touch != 0) {\n        __touch(__pool);\n    }\n')
    for k in range(len(parts)):
        out.append('    run%d();\n' % k)
    out.append('    if (__fails == 0) {\n        say("%s ");\n        say_n(__checks);\n        endl();\n    }\n' % tf.name)
    out.append('    return 0;\n}\n')
    return ''.join(out)


GROUPS = [  # file, what, standard, the families' categories
    ('intrin_a64_arith', 'arithmetic, saturating, halving, pairwise, reductions, multiply-accumulate', 'c11', ('arith', 'mla')),
    ('intrin_a64_widen', 'widening and narrowing', 'c99', ('widen',)),
    ('intrin_a64_shift', 'compares and shifts', 'c17', ('cmp', 'shift')),
    ('intrin_a64_conv', 'bit operations, counts, estimates, conversions, rounding', 'gnu17', ('logic', 'conv')),
    ('intrin_a64_data', 'lanes, duplication, combination, permutes, tables, reinterpretations', 'c11', ('data',)),
    ('intrin_a64_mem', 'loads and stores', 'c99', ('mem',)),
    ('intrin_a64_ext', 'dot products, matrix multiplies, cryptography, BF16, FHM, FP16 scalars, <arm_acle.h>', 'c17',
     ('dot', 'bf16', 'acle')),
]


FIXED_TESTS = {
    'intrin_a64_err_call': r"""/* An intrinsic whose extension the calling function is not compiled for
   (here the dot product's, in a build for the baseline) is C3240 at the call.
   Generated by cmd/internal/tools/gen_neon_intrin.py: do not edit. */
/* flags: --target=aarch64 */
/* expect-error: C3240 */
#include <arm_neon.h>

uint32x4_t f(uint32x4_t r, uint8x16_t a, uint8x16_t b) {
    return vdotq_u32(r, a, b);
}

int main(void) {
    return 0;
}
""",
    'intrin_a64_err_crypto': r"""/* The cryptographic intrinsics need their extensions (AES here; CRC32 of
   <arm_acle.h> likewise): C3240 without them. Generated by
   cmd/internal/tools/gen_neon_intrin.py: do not edit. */
/* flags: --target=aarch64 --features=+crc */
/* expect-error: C3240 */
#include <arm_neon.h>
#include <arm_acle.h>

uint32_t g(uint32_t c, uint8_t x) {
    return __crc32b(c, x);
}

uint8x16_t f(uint8x16_t s, uint8x16_t k) {
    return vaesmcq_u8(vaeseq_u8(s, k));
}

int main(void) {
    return 0;
}
""",
    'intrin_a64_err_imm': r"""/* An immediate operand (a shift count, a lane) must be a constant: C3241.
   Generated by cmd/internal/tools/gen_neon_intrin.py: do not edit. */
/* flags: --target=aarch64 */
/* expect-error: C3241 */
#include <arm_neon.h>

int8x8_t f(int8x8_t a, int n) {
    return vshr_n_s8(a, n);
}

int main(void) {
    return 0;
}
""",
    'intrin_a64_err_range': r"""/* An immediate out of its range (lane 8 of a vector of 8 lanes) is C3241.
   Generated by cmd/internal/tools/gen_neon_intrin.py: do not edit. */
/* flags: --target=aarch64 */
/* expect-error: C3241 */
#include <arm_neon.h>

int f(int8x8_t a) {
    return vget_lane_s8(a, 8);
}

int main(void) {
    return 0;
}
""",
    'intrin_a64_target': r"""/* A function compiled for an extension (the attribute target, spelled
   __axiom_attribute__ in an ISO mode) may call its intrinsics in a program
   built for the baseline, inlined or not (-O0). Generated by
   cmd/internal/tools/gen_neon_intrin.py: do not edit. */
/* aarch64-only */
/* flags: --c-std=c11 */
/* expect: 70 */
/* expect: 1 */
#include <arm_neon.h>
#include <arm_acle.h>
#include "host.h"

static __axiom_attribute__((__target__("dotprod"))) int dot(const uint8_t *a, const uint8_t *b) {
    uint32x4_t r = vdotq_u32(vdupq_n_u32(0), vld1q_u8(a), vld1q_u8(b));
    return (int)vaddvq_u32(r);
}

static __axiom_attribute__((__target__("crc"))) unsigned crc(void) {
    return __crc32b(0xffffffffu, 0x61) ^ 0xe8b7be43u;  /* (the CRC-32 of "a", inverted) */
}

int main(void) {
    static const uint8_t a[16] = {1, 2, 3, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    static const uint8_t b[16] = {5, 6, 7, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    print_i64(dot(a, b));
    print_i64(crc() == 0xffffffffu);
    return 0;
}
""",
    'intrin_a64_sm4': r"""/* SM4 (GB/T 32907) by its instructions: the key schedule (SM4EKEY) and the 32
   rounds (SM4E) of the standard's example, whose ciphertext it gives.
   Generated by cmd/internal/tools/gen_neon_intrin.py: do not edit. */
/* aarch64-only */
/* flags: --features=+sm4 */
/* expect: sm4 ok */
#include <arm_neon.h>
#include "host.h"

static const unsigned char key[16] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
                                      0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10};
static const unsigned char want[16] = {0x68, 0x1e, 0xdf, 0x34, 0xd2, 0x06, 0x96, 0x5e,
                                       0x86, 0xb3, 0xe9, 0x4f, 0x53, 0x6e, 0x42, 0x46};

int main(void) {
    static const uint32_t fk[4] = {0xa3b1bac6, 0x56aa3350, 0x677d9197, 0xb27022dc};
    uint32x4_t rk[8];
    uint32x4_t k = vreinterpretq_u32_u8(vrev32q_u8(vld1q_u8(key)));
    uint32x4_t x = k;
    uint8_t out[16];
    int i;
    int ok = 1;
    k = veorq_u32(k, vld1q_u32(fk));
    for (i = 0; i < 8; i++) {
        uint32_t ck[4];
        int j;
        for (j = 0; j < 4; j++) {
            int b = 4 * (4 * i + j);
            ck[j] = (uint32_t)((b * 7) & 255) << 24 | (uint32_t)(((b + 1) * 7) & 255) << 16 |
                    (uint32_t)(((b + 2) * 7) & 255) << 8 | (uint32_t)(((b + 3) * 7) & 255);
        }
        k = vsm4ekeyq_u32(k, vld1q_u32(ck));
        rk[i] = k;
    }
    for (i = 0; i < 8; i++) {
        x = vsm4eq_u32(x, rk[i]);
    }
    /* (the last four words, in reverse order) */
    x = vrev64q_u32(vextq_u32(x, x, 2));
    vst1q_u8(out, vrev32q_u8(vreinterpretq_u8_u32(x)));
    for (i = 0; i < 16; i++) {
        if (out[i] != want[i]) {
            ok = 0;
        }
    }
    print_str(ok ? "sm4 ok" : "sm4 wrong", ok ? 6 : 9);
    return 0;
}
""",
}


LOWER = [  # (regex, count): the instructions of the functions below
    (r'\buqadd\tv\d+\.16b', 1), (r'\bldr\tq\d+, \[x\d+, x\d+\]', 2), (r'\bstr\tq\d+, \[x\d+, x\d+\]', 1),
    (r'\bfmla\tv\d+\.4s', 1), (r'\bmla\tv\d+\.4s', 1), (r'\bsmax\tv\d+\.8h', 1), (r'\bfcmeq\tv\d+\.4s', 1),
    (r'\bshl\tv\d+\.8h, v\d+\.8h, #3', 1), (r'\bsshr\tv\d+\.4s, v\d+\.4s, #5', 1), (r'\bsxtl2\tv\d+\.8h', 1),
    (r'\bzip1\tv\d+\.16b', 1), (r'\bext\tv\d+\.16b, v\d+\.16b, v\d+\.16b, #3', 1), (r'\bscvtf\tv\d+\.4s', 1),
    (r'\bfrintm\tv\d+\.4s', 1), (r'\badd\tv\d+\.8b', 1), (r'\bldr\td\d+, \[x1\]', 1), (r'\bsqabs\tv\d+\.8b', 1),
    (r'\bstr\td\d+, \[x0\]', 1), (r'\baddv\ts\d+, v\d+\.4s', 1), (r'\bsdot\tv\d+\.4s', 1),
    (r'\baese\tv\d+\.16b', 1), (r'\baesmc\tv\d+\.16b', 1), (r'\bcrc32cw\b', 1), (r'\bld2\t', 1), (r'\btbl\t', 1),
    (r'\bfmul\t', 0), (r'\bstp\t', 0),
]

LOWER_SRC = r"""#include <arm_neon.h>
#include <arm_acle.h>
#include <stddef.h>

void blend(uint8_t *d, const uint8_t *a, const uint8_t *b, size_t n) {
    size_t i;
    for (i = 0; i < n; i += 16) {
        vst1q_u8(d + i, vqaddq_u8(vld1q_u8(a + i), vld1q_u8(b + i)));
    }
}

float32x4_t axpy(float32x4_t acc, float32x4_t x, float32x4_t y) {
    return vfmaq_f32(acc, x, y);
}

int32x4_t mla(int32x4_t acc, int32x4_t x, int32x4_t y) {
    return vmlaq_s32(acc, x, y);
}

int16x8_t smax(int16x8_t a, int16x8_t b) {
    return vmaxq_s16(a, b);
}

uint32x4_t feq(float32x4_t a, float32x4_t b) {
    return vceqq_f32(a, b);
}

uint16x8_t shl(uint16x8_t a) {
    return vshlq_n_u16(a, 3);
}

int32x4_t sshr(int32x4_t a) {
    return vshrq_n_s32(a, 5);
}

int16x8_t wide(int8x16_t a) {
    return vmovl_high_s8(a);
}

int8x16_t zip(int8x16_t a, int8x16_t b) {
    return vzip1q_s8(a, b);
}

int8x16_t ext(int8x16_t a, int8x16_t b) {
    return vextq_s8(a, b, 3);
}

float32x4_t cvt(int32x4_t a) {
    return vcvtq_f32_s32(a);
}

float32x4_t floor4(float32x4_t a) {
    return vrndmq_f32(a);
}

int8x8_t dadd(int8x8_t a, int8x8_t b) {
    return vadd_s8(a, b);
}

void dcopy(int8_t *d, const int8_t *s) {
    vst1_s8(d, vqabs_s8(vld1_s8(s)));
}

uint32_t sum(const uint32_t *p, size_t n) {
    uint32x4_t acc = vdupq_n_u32(0);
    size_t i;
    for (i = 0; i < n; i += 4) {
        acc = vaddq_u32(acc, vld1q_u32(p + i));
    }
    return vaddvq_u32(acc);
}

__axiom_attribute__((__target__("dotprod"))) int32x4_t dot(int32x4_t r, int8x16_t a, int8x16_t b) {
    return vdotq_s32(r, a, b);
}

__axiom_attribute__((__target__("aes"))) uint8x16_t round1(uint8x16_t s, uint8x16_t k) {
    return vaesmcq_u8(vaeseq_u8(s, k));
}

__axiom_attribute__((__target__("crc"))) uint32_t crc(uint32_t c, uint32_t x) {
    return __crc32cw(c, x);
}

int32x4x2_t deinterleave(const int32_t *p) {
    return vld2q_s32(p);
}

uint8x16_t lookup(uint8x16x2_t t, uint8x16_t i) {
    return vqtbl2q_u8(t, i);
}
"""


def lower_test():
    out = ['/* The AArch64 intrinsics\' instructions: the plain C of 128-bit vectors and\n',
           '   the IR\'s operations are the instruction selector\'s (uqadd, fmla, mla,\n',
           '   smax, fcmeq, shl and sshr by a constant, sxtl2, zip1, ext, scvtf,\n',
           '   frintm, ldr and str of q registers), the others the instruction each\n',
           '   names (on 64-bit vectors in d registers: add, sqabs, ldr and str of d;\n',
           '   addv, sdot, aese, aesmc, crc32cw, ld2, tbl). A loop of a saturating add\n',
           '   is two loads, the add, a store and the count; no function needs a stack\n',
           '   frame. Generated by cmd/internal/tools/gen_neon_intrin.py: do not edit. */\n']
    for (r, n) in LOWER:
        out.append('// lower-selfhost: --target=aarch64 -O2 --c-freestanding | %s | %d\n' % (r, n))
    out.append(LOWER_SRC)
    return ''.join(out)


def tests():
    write(os.path.join(TESTS, 'run', 'intrin_a64_compile.c'), compile_test())
    for (n, t) in FIXED_TESTS.items():
        write(os.path.join(TESTS, 'run', n + '.c'), t)
    write(os.path.join(TESTS, '..', 'lowering', 'c_intrin_a64.c'), lower_test())
    for (name, what, std, cats) in GROUPS:
        tf = TestFile(name, what)
        tf.std = std
        for e in INTRIN:
            if e['cat'] not in cats or e['test'] == 'no':
                continue
            if e['model'] is None:
                if e['name'].startswith('vreinterpret'):
                    e = dict(e)
                    e['model'] = reinterpret_model(e)
                    e['raw'] = True
                else:
                    continue
            tf.cases.append(make_case(tf, e))
        write(os.path.join(TESTS, 'run', name + '.c'), test_text(tf))


def reinterpret_model(e):
    """the same bytes, as the result's lanes"""
    (t, _), = e['params']
    src = kind_of(t)
    dst = kind_of(e['ret'])

    def m(a):
        if src[0] == 'v':
            data = b''.join(ubits(x, eb(src[1])).to_bytes(eb(src[1]) // 8, 'little') for x in a)
        else:
            data = ubits(a, 128).to_bytes(16, 'little')
        if dst[0] == 'v':
            w = eb(dst[1]) // 8
            return [int.from_bytes(data[i:i + w], 'little') for i in range(0, 16 if dst[2] else 8, w)]
        return int.from_bytes(data, 'little')
    return m


if __name__ == '__main__':
    main()
