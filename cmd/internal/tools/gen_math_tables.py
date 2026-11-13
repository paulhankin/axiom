#!/usr/bin/env python3
"""Generates std/internal/math/tables.ax: the tables and polynomial coefficients
of the elementary functions of std/math and the C library (std/internal/math/),
std/c/rt/math_ld_tables.ax, those of the C library's wide long double
functions (math_ld.ax, math_ldx.ax), and std/c/rt/dfp_tables.ax, the powers
of ten of the decimal functions' binary path (dfp_fast.ax).

    python3 cmd/internal/tools/gen_math_tables.py      # needs mpmath

Every number is computed here from its definition, in arbitrary precision
(mpmath, 300 bits and more): the bits of 2/pi, the values of 2^(j/128) and of
log(c), and the minimax polynomials, which a Remez exchange (Remez 1934; see
J.-M. Muller, "Elementary Functions: Algorithms and Implementation", 3rd ed.,
2016, chapter 3) fits to the function on the interval the kernel uses, with
the weight that makes the error relative to the result. After rounding the
coefficients to double, the error of each polynomial is measured on a dense
grid; the comment above each says the bound found (log2 of the relative
error). The output is deterministic.
"""
import os
import struct

import mpmath as mp

mp.mp.prec = 320
ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..')
OUT = os.path.join(ROOT, 'std', 'internal', 'math', 'tables.ax')


def d(x):
    """x rounded to the nearest double."""
    return float(mp.mpf(x))


def bits(x):
    return struct.unpack('<Q', struct.pack('<d', x))[0]


def lit(x):
    """An Axiom literal for the double x (shortest digits that read back)."""
    r = repr(float(x))
    if 'e' not in r and '.' not in r and 'inf' not in r:
        r += '.0'
    if 'e' in r and '.' not in r.split('e')[0]:
        m, e = r.split('e')
        r = m + '.0e' + e
    return r


def split_hi(x, nbits):
    """x rounded to a double of nbits significant bits."""
    x = mp.mpf(x)
    if x == 0:
        return 0.0
    e = mp.floor(mp.log(abs(x), 2))
    scale = mp.mpf(2) ** (nbits - 1 - e)
    return d(mp.nint(x * scale) / scale)


def remez(f, basis, a, b, w, grid=2500, iters=40):
    """Minimax coefficients c of sum c_k basis_k(x) for f(x) on [a, b], with
    the error w(x) (f(x) - sum c_k basis_k(x)): the Remez exchange over
    n + 1 alternation points, which start at the Chebyshev extrema."""
    n = len(basis)
    a = mp.mpf(a)
    b = mp.mpf(b)
    ref = [(a + b) / 2 - (b - a) / 2 * mp.cos(mp.pi * i / n) for i in range(n + 1)]
    xs = [a + (b - a) * mp.mpf(i) / grid for i in range(grid + 1)]
    c = None
    for _ in range(iters):
        A = mp.matrix(n + 1, n + 1)
        rhs = mp.matrix(n + 1, 1)
        for i, x in enumerate(ref):
            wx = w(x)
            for k in range(n):
                A[i, k] = wx * basis[k](x)
            A[i, n] = (-1) ** i
            rhs[i] = wx * f(x)
        sol = mp.lu_solve(A, rhs)
        c = [sol[k] for k in range(n)]
        lev = abs(sol[n])

        def err(x):
            return w(x) * (f(x) - sum(c[k] * basis[k](x) for k in range(n)))

        ev = [err(x) for x in xs]
        ext = []
        for i in range(len(xs)):
            v = abs(ev[i])
            if (i == 0 or v >= abs(ev[i - 1])) and (i + 1 == len(xs) or v >= abs(ev[i + 1])):
                ext.append(i)
        pts = []
        for i in ext:
            if pts and mp.sign(ev[pts[-1]]) == mp.sign(ev[i]):
                if abs(ev[i]) > abs(ev[pts[-1]]):
                    pts[-1] = i
            else:
                pts.append(i)
        while len(pts) > n + 1:
            if abs(ev[pts[0]]) < abs(ev[pts[-1]]):
                pts.pop(0)
            else:
                pts.pop()
        maxe = max(abs(e) for e in ev)
        if len(pts) < n + 1 or maxe / lev - 1 < mp.mpf('1e-8'):
            break
        newref = []
        for i in pts:
            lo = xs[max(i - 1, 0)]
            hi = xs[min(i + 1, len(xs) - 1)]
            for _ in range(50):
                m1 = lo + (hi - lo) * mp.mpf('0.381966')
                m2 = lo + (hi - lo) * mp.mpf('0.618034')
                if abs(err(m1)) > abs(err(m2)):
                    hi = m2
                else:
                    lo = m1
            newref.append((lo + hi) / 2)
        ref = newref
    return c


def measure(f, basis, cs, a, b, w, grid=6000):
    """log2 of the largest weighted error of the polynomial with the double
    coefficients cs, on a grid of [a, b]."""
    a = mp.mpf(a)
    b = mp.mpf(b)
    m = mp.mpf(0)
    for i in range(grid + 1):
        x = a + (b - a) * mp.mpf(i) / grid
        if x == 0:
            continue
        e = abs(w(x) * (f(x) - sum(mp.mpf(cs[k]) * basis[k](x) for k in range(len(cs)))))
        m = max(m, e)
    return float(mp.log(m, 2)) if m > 0 else -999.0


def powers(ks):
    return [(lambda k: (lambda x: x ** k))(k) for k in ks]


def fit(name, f, ks, a, b, w, fixed=None):
    """A fit with basis x^k for k in ks: (doubles, log2 error). fixed gives
    the first coefficients' values (doubles), and the others are fitted to
    what they leave, so that their rounding is compensated."""
    ks = list(ks)
    fixed = fixed or []
    nf = len(fixed)
    basis = powers(ks)
    fx = [mp.mpf(v) for v in fixed]  # (exact: the kernel may add the rounding back)

    def g(x):
        return f(x) - sum(fx[i] * basis[i](x) for i in range(nf))

    c = remez(g, basis[nf:], a, b, w)
    cs = [float(v) for v in fixed] + [d(x) for x in c]
    return cs, measure(f, basis, cs, a, b, w)


out = []


def emit(s=''):
    out.append(s)


def const(name, x, doc=None):
    if doc:
        emit('// ' + doc)
    emit('pub const %s: f64 = %s;' % (name, lit(x)))


def coeffs(name, cs, err, doc):
    emit('// %s (relative error 2^%.1f)' % (doc, err))
    for i, c in enumerate(cs):
        emit('pub const %s%d: f64 = %s;' % (name, i, lit(c)))
    emit()


def u64_array(name, vals, doc):
    if doc:
        emit('// ' + doc)
    emit('pub let %s: [%d]u64 = [' % (name, len(vals)))
    for v in vals:
        emit('    0x%016x,' % v)
    emit('];')
    emit()


# -----------------------------------------------------------------------------
# exp, exp2, expm1: x = (128 k + j) ln2 / 128 + r, e^x = 2^k 2^(j/128) e^r
# -----------------------------------------------------------------------------
LN2 = mp.log(2)
emit('// =============================================================================')
emit('// std/internal/math/tables.ax - generated by')
emit('// cmd/internal/tools/gen_math_tables.py (do not edit): tables and polynomial')
emit('// coefficients of the elementary functions of std/math and the C library,')
emit('// computed in arbitrary precision (see the generator).')
emit('// =============================================================================')
emit()
tab = []
for j in range(128):
    t = mp.mpf(2) ** (mp.mpf(j) / 128)
    hi = d(t)
    lo = d(t - mp.mpf(hi))
    tab += [bits(hi), bits(lo)]
u64_array('EXP_T', tab, '2^(j/128) for j = 0..127 as pairs of doubles (hi, lo): hi + lo to 2^-106')
const('EXP_INVLN2N', 128 / LN2, '128 / ln 2')
# ln2/128 in two parts; the first has 35 significant bits, so that n times it is
# exact for |n| < 2^18 (|x| < 1100)
l1 = split_hi(LN2 / 128, 35)
const('EXP_LN2N_HI', l1, 'ln 2 / 128 = EXP_LN2N_HI + EXP_LN2N_LO (the first with 35 bits)')
const('EXP_LN2N_LO', d(LN2 / 128 - l1))
R = LN2 / 256 * mp.mpf('1.002')
cs, err = fit('E', lambda r: (mp.expm1(r) - r) / (r * r) if r != 0 else mp.mpf(0.5), range(4), -R, R,
              lambda r: r * r / mp.exp(r))
coeffs('EXP_P', cs, err, 'e^r - 1 = r + r^2 (P0 + P1 r + P2 r^2 + P3 r^3) for |r| <= ln2/256')
cs, err = fit('E2', lambda t: (mp.power(2, t) - 1) / t if t != 0 else LN2, range(6), -mp.mpf(1) / 256 * mp.mpf(
    '1.002'), mp.mpf(1) / 256 * mp.mpf('1.002'), lambda t: t / mp.power(2, t))
coeffs('EXP2_P', cs, err, '2^t - 1 = t (P0 + P1 t + ... + P5 t^5) for |t| <= 1/256')
M = mp.mpf(1) / 16 * mp.mpf('1.001')
cs, err = fit('EM1', lambda x: (mp.expm1(x) - x) / (x * x) if x != 0 else mp.mpf(0.5), range(9), -M, M,
              lambda x: x * x / mp.expm1(x) if x != 0 else 0)
coeffs('EXPM1_P', cs, err, 'e^x - 1 = x + x^2 (P0 + ... + P8 x^8) for |x| <= 1/16')

# -----------------------------------------------------------------------------
# log: x = 2^k m, m in [z, 2z) (z near 1/sqrt 2); m's interval i of 128 has c_i,
# 1/m rounded to a multiple of 2^-9, and log x = k ln2 - log c_i + log1p(m c_i - 1)
# -----------------------------------------------------------------------------
B1 = bits(1.0)
off0 = bits(0.70710678118654752)
# move OFF so that 1.0 is at the middle of its interval (whose c is 1)
diff = (B1 - off0) % (1 << 52)
i0 = diff >> 45
OFF = (B1 - (i0 << 45) - (1 << 44)) & 0xfffff00000000000
OFF = B1 - ((B1 - OFF) % (1 << 52)) if False else OFF
assert ((B1 - OFF) % (1 << 52)) == (i0 << 45) + (1 << 44), hex(OFF)
logc = []
logl = []
rmax = mp.mpf(0)


def value_of(bx):
    return mp.mpf(struct.unpack('<d', struct.pack('<Q', bx))[0])


for i in range(128):
    lo_b = OFF + (i << 45)
    hi_b = OFF + ((i + 1) << 45)
    # the bit patterns past the binade of OFF are those of 2m
    lo_v = value_of(lo_b)
    hi_v = value_of(hi_b)
    if i == i0:
        c = mp.mpf(1)
    else:
        c = mp.nint(2 / (lo_v + hi_v) * 512) / 512
    rmax = max(rmax, abs(lo_v * c - 1), abs(hi_v * c - 1))
    L = -mp.log(c)
    lh = d(L)
    logc.append(bits(d(c)))
    logl += [bits(lh), bits(d(L - mp.mpf(lh)))]
emit('// log: x = 2^k m with bits(m) - LOG_OFF in [0, 2^52); interval i of m is')
emit('// bits(m) - LOG_OFF in [i 2^45, (i + 1) 2^45). LOG_C[i] is about 1/m there, a')
emit('// multiple of 2^-9 (so that m c - 1 is computed exactly, split), 1 for the')
emit('// interval of 1.0 (i = %d, which has 1.0 at its middle); |m c - 1| <= 2^%.2f.' % (i0, float(mp.log(rmax, 2))))
emit('pub const LOG_OFF: u64 = 0x%016x;' % OFF)
emit('pub const LOG_I1: u64 = %d;' % i0)
emit()
u64_array('LOG_C', logc, 'c_i')
u64_array('LOG_L', logl, '-log(c_i) as pairs (hi, lo)')
l2h = split_hi(LN2, 42)
const('LN2_HI', l2h, 'ln 2 = LN2_HI + LN2_LO + LN2_LO2 (LN2_HI with 42 bits: k LN2_HI is exact for |k| < 2^11)')
l2l = d(LN2 - mp.mpf(l2h))
const('LN2_LO', l2l)
const('LN2_LO2', d(LN2 - mp.mpf(l2h) - mp.mpf(l2l)))
RM = rmax * mp.mpf('1.0001')
cs, err = fit('L', lambda r: (mp.log1p(r) - r) / (r * r) if r != 0 else mp.mpf(-0.5), range(7), -RM, RM,
              lambda r: r * r / mp.log1p(r) if r != 0 else 0, fixed=[-0.5])
coeffs('LOG_P', cs, err, 'log1p(r) = r + r^2 (P0 + P1 r + ... + P6 r^6) for |r| <= the bound above')
cs, err = fit('LD', lambda r: (mp.log1p(r) - r + r * r / 2) / (r ** 3) if r != 0 else mp.mpf(1) / 3, range(10), -RM, RM,
              lambda r: r ** 3 / mp.log1p(r) if r != 0 else 0, fixed=[d(mp.mpf(1) / 3)])
coeffs('LOGD_P', cs, err, 'log1p(r) = r - r^2/2 + r^3 (P0 + ... + P9 r^9), for the double-double logarithm')
const('THIRD_LO', d(mp.mpf(1) / 3 - mp.mpf(d(mp.mpf(1) / 3))), '1/3 - LOGD_P0 (the logarithm adds r^3 THIRD_LO)')
h = d(LN2)
const('LN2D_HI', h, 'ln 2 = LN2D_HI + LN2D_LO (the double nearest, and the rest)')
const('LN2D_LO', d(LN2 - mp.mpf(h)))
INVLN2 = 1 / LN2
h = d(INVLN2)
const('INVLN2_HI', h, '1 / ln 2 = INVLN2_HI + INVLN2_LO')
const('INVLN2_LO', d(INVLN2 - mp.mpf(h)))
INVLN10 = 1 / mp.log(10)
h = d(INVLN10)
const('INVLN10_HI', h, '1 / ln 10 = INVLN10_HI + INVLN10_LO')
const('INVLN10_LO', d(INVLN10 - mp.mpf(h)))
LN10 = mp.log(10)
h = d(LN10)
const('LN10_HI', h, 'ln 10 = LN10_HI + LN10_LO')
const('LN10_LO', d(LN10 - mp.mpf(h)))
emit()

# -----------------------------------------------------------------------------
# sin, cos, tan: x = n pi/2 + r, |r| <= pi/4; the bits of 2/pi for the
# reduction of large arguments (Payne and Hanek, "Radian reduction for
# trigonometric functions", SIGNUM Newsletter 18, 1983)
# -----------------------------------------------------------------------------
with mp.workprec(1500):
    tp = 2 / mp.pi
    words = [0]  # a zero word first: bit j of 2/pi (j >= 1) is bit 63 + j of the array
    v = tp
    for i in range(21):
        v = v * mp.mpf(2) ** 64
        w = int(mp.floor(v))
        words.append(w)
        v -= w
u64_array('TWO_OVER_PI', words, 'the bits of 2/pi after the binary point, 64 to a word, after a zero word')
PIO2 = mp.pi / 2
p1 = split_hi(PIO2, 32)
p2 = split_hi(PIO2 - mp.mpf(p1), 32)
p3 = d(PIO2 - mp.mpf(p1) - mp.mpf(p2))
const('PIO2_1', p1, 'pi/2 = PIO2_1 + PIO2_2 + PIO2_3 (to 2^-120 or so), the first two of 32 bits (n times them is exact for |n| < 2^21)')
const('PIO2_2', p2)
const('PIO2_3', p3)
h = d(PIO2)
const('PIO2_HI', h, 'pi/2 = PIO2_HI + PIO2_LO')
const('PIO2_LO', d(PIO2 - mp.mpf(h)))
const('INVPIO2', 2 / mp.pi, '2/pi')
h = d(mp.pi)
const('PI_HI', h, 'pi = PI_HI + PI_LO')
const('PI_LO', d(mp.pi - mp.mpf(h)))
h = d(mp.pi / 4)
const('PIO4_HI', h, 'pi/4 = PIO4_HI + PIO4_LO')
const('PIO4_LO', d(mp.pi / 4 - mp.mpf(h)))
h = d(1 / mp.pi)
const('INVPI_HI', h, '1/pi = INVPI_HI + INVPI_LO')
const('INVPI_LO', d(1 / mp.pi - mp.mpf(h)))
h = d(3 * mp.pi / 4)
const('PI3O4_HI', h, '3pi/4 = PI3O4_HI + PI3O4_LO')
const('PI3O4_LO', d(3 * mp.pi / 4 - mp.mpf(h)))
emit()
Z = (mp.pi / 4 * mp.mpf('1.0001')) ** 2
s0 = d(-mp.mpf(1) / 6)
cs, err = fit('S', lambda z: (mp.sin(mp.sqrt(z)) / mp.sqrt(z) - 1) / z if z != 0 else -mp.mpf(1) / 6, range(7), 0, Z,
              lambda z: z * mp.sqrt(z) / mp.sin(mp.sqrt(z)) if z != 0 else 0, fixed=[-mp.mpf(1) / 6])
coeffs('SIN_P', cs, err, 'sin r = r + r z (P0 + P1 z + ... + P6 z^6), z = r^2, |r| <= pi/4')
const('SIN_P0_LO', d(-mp.mpf(1) / 6 - mp.mpf(s0)), '-1/6 - SIN_P0')
cs, err = fit('C', lambda z: (mp.cos(mp.sqrt(z)) - 1 + z / 2) / (z * z) if z != 0 else mp.mpf(1) / 24, range(6), 0, Z,
              lambda z: z * z / mp.cos(mp.sqrt(z)) if z != 0 else 0)
coeffs('COS_P', cs, err, 'cos r = 1 - z/2 + z^2 (P0 + P1 z + ... + P5 z^5)')

# -----------------------------------------------------------------------------
# asin, acos, atan
# -----------------------------------------------------------------------------
# (relative to the tail s z P(z), at most a 24th of asin s: 2^-53 is enough)
asin_t = lambda z: (mp.asin(mp.sqrt(z)) / mp.sqrt(z) - 1) / z if z != 0 else mp.mpf(1) / 6
cs, err = fit('AS', asin_t, range(13), 0, mp.mpf('0.25'), lambda z: 1 / asin_t(z))
coeffs('ASIN_P', cs, err, 'asin s = s + s z (P0 + P1 z + ... + P12 z^12), z = s^2 <= 1/4 (relative to s z P)')
U = (mp.mpf(1) / 32 * mp.mpf('1.0001')) ** 2
cs, err = fit('AT', lambda z: (mp.atan(mp.sqrt(z)) / mp.sqrt(z) - 1) / z if z != 0 else -mp.mpf(1) / 3, range(7), 0, U,
              lambda z: z * mp.sqrt(z) / mp.atan(mp.sqrt(z)) if z != 0 else 0)
coeffs('ATAN_P', cs, err, 'atan u = u + u z (P0 + P1 z + ... + P6 z^6), z = u^2, |u| <= 1/32')
cs, err = fit('AT', lambda z: (mp.atan(mp.sqrt(z)) / mp.sqrt(z) - 1) / z if z != 0 else -mp.mpf(1) / 3, range(5), 0, U,
              lambda z: z * mp.sqrt(z) / mp.atan(mp.sqrt(z)) if z != 0 else 0)
coeffs('ATAN_Q', cs, err, 'atan u = u + u z (Q0 + Q1 z + ... + Q4 z^4), z = u^2, |u| <= 1/32 (the fast path)')
# (the reduction below leaves |u| <= 1/65)
U64 = (mp.mpf(1) / 64) ** 2
cs, err = fit('AT', lambda z: (mp.atan(mp.sqrt(z)) / mp.sqrt(z) - 1) / z if z != 0 else -mp.mpf(1) / 3, range(4), 0, U64,
              lambda z: z * mp.sqrt(z) / mp.atan(mp.sqrt(z)) if z != 0 else 0)
coeffs('ATAN_R', cs, err, 'atan u = u + u z (R0 + R1 z + R2 z^2 + R3 z^3), z = u^2, |u| <= 1/64 (the fast path)')
# atan x = atan c + atan((x - c)/(1 + x c)): for x in [2^-6, 2^6), c the middle of
# x's sixteenth of its binade (the exponent's 12 binades and 4 bits of the
# significand index the table), so |u| <= 1/32
# (the row of x is bits(x) >> 48 modulo 256: the 192 values of x's top 16
# bits in [2^-6, 2^6) are distinct modulo 256; the other 64 rows are zero)
at = [0] * (256 * 3)
for e in range(-6, 6):
    for k in range(16):
        c = mp.mpf(2) ** e * (1 + (mp.mpf(k) + mp.mpf('0.5')) / 16)
        v = mp.atan(c)
        h = d(v)
        row = ((e + 1023) * 16 + k) % 256
        at[3 * row:3 * row + 3] = [bits(d(c)), bits(h), bits(d(v - mp.mpf(h)))]
u64_array('ATAN_B', at, 'for x in [2^-6, 2^6): rows of c, atan(c) (hi, lo), c = 2^e (1 + (k + 1/2)/16), row ((e + 1023) 16 + k) mod 256')

# -----------------------------------------------------------------------------
# sinh and tanh near 0
# -----------------------------------------------------------------------------
Z = (mp.mpf('0.25') * mp.mpf('1.0001')) ** 2
cs, err = fit('SH', lambda z: (mp.sinh(mp.sqrt(z)) / mp.sqrt(z) - 1) / z if z != 0 else mp.mpf(1) / 6, range(6), 0, Z,
              lambda z: z * mp.sqrt(z) / mp.sinh(mp.sqrt(z)) if z != 0 else 0)
coeffs('SINH_P', cs, err, 'sinh x = x + x z (P0 + ... + P5 z^5), z = x^2 <= 1/16')
Z = (mp.mpf(1) / 32 * mp.mpf('1.0001')) ** 2
cs, err = fit('TH', lambda z: (mp.tanh(mp.sqrt(z)) / mp.sqrt(z) - 1) / z if z != 0 else -mp.mpf(1) / 3, range(5), 0, Z,
              lambda z: z * mp.sqrt(z) / mp.tanh(mp.sqrt(z)) if z != 0 else 0)
coeffs('TANH_P', cs, err, 'tanh x = x + x z (P0 + ... + P4 z^4), z = x^2 <= 1/1024')
# (relative to the tail x z P(z), at most a 48th of the result)
asinh_t = lambda z: (mp.asinh(mp.sqrt(z)) / mp.sqrt(z) - 1) / z if z != 0 else -mp.mpf(1) / 6
cs, err = fit('ASH', asinh_t, range(9), 0, mp.mpf(1) / 16, lambda z: 1 / asinh_t(z))
coeffs('ASINH_P', cs, err, 'asinh x = x + x z (P0 + ... + P8 z^8), z = x^2 <= 1/16 (relative to x z P)')
atanh_t = lambda z: (mp.atanh(mp.sqrt(z)) / mp.sqrt(z) - 1) / z if z != 0 else mp.mpf(1) / 3
cs, err = fit('ATH', atanh_t, range(10), 0, mp.mpf(1) / 16, lambda z: 1 / atanh_t(z))
coeffs('ATANH_P', cs, err, 'atanh x = x + x z (P0 + ... + P9 z^9), z = x^2 <= 1/16 (relative to x z P)')

# -----------------------------------------------------------------------------
# erf, erfc, lgamma, tgamma: polynomials evaluated by compensated Horner
# (S. Graillat, Ph. Langlois, N. Louvet, "Compensated Horner scheme", 2005),
# so their errors are the fits' (about 2^-60 and below) plus a final rounding
# -----------------------------------------------------------------------------
def pieces(name, f, a, b, n, deg, doc, var=None):
    """Polynomials in t = x - c (c the middle) of degree deg for f on n
    equal pieces of [a, b], fitted for relative error; as a table of rows
    (lo, hi, c, the low part of the coefficient of t^0, then the
    coefficients of t^0 .. t^deg): the constant term is a double-double, so
    that its rounding (2^-54 of the result) is not the error."""
    rows = []
    a = mp.mpf(a)
    b = mp.mpf(b)
    with mp.workprec(160):
        for i in range(n):
            lo = a + (b - a) * i / n
            hi = a + (b - a) * (i + 1) / n
            c = mp.mpf(d((lo + hi) / 2))
            g = lambda t: f(t + c)
            basis = powers(range(deg + 1))
            cs_ = remez(g, basis, lo - c, hi - c, lambda t: 1 / abs(g(t)), grid=400, iters=15)
            csd = [d(v) for v in cs_]
            c0lo = d(cs_[0] - mp.mpf(csd[0]))

            def h(t):
                return mp.mpf(csd[0]) + mp.mpf(c0lo) + sum(mp.mpf(csd[k]) * t ** k for k in range(1, deg + 1))

            e = max(abs(h(lo - c + (hi - lo) * j / 800) / g(lo - c + (hi - lo) * j / 800) - 1) for j in range(801))
            rows.append((d(lo), d(hi), d(c), c0lo, csd, float(mp.log(e, 2))))
    worst = max(r[5] for r in rows)
    emit('// %s: %d pieces of degree %d (relative error 2^%.1f); a row is the piece\'s' % (doc, n, deg, worst))
    emit('// lower and upper ends, its middle c, the low part of the coefficient of t^0, then')
    emit('// the coefficients of t^0 .. t^%d, t = %s - c' % (deg, var or 'x'))
    emit('pub const %s_N: usize = %d;' % (name, n))
    emit('pub const %s_W: usize = %d;' % (name, deg + 5))
    emit('pub const %s_INVW: f64 = %s;  // n / (b - a): the piece of x is about (x - a) times this' % (name, lit(d(n / (b - a)))))
    vals = []
    for r in rows:
        vals += [bits(r[0]), bits(r[1]), bits(r[2]), bits(r[3])] + [bits(v) for v in r[4]]
    u64_array(name, vals, '')


two_sqrtpi = 2 / mp.sqrt(mp.pi)
cs, err = fit('ERF', lambda z: mp.erf(mp.sqrt(z)) / mp.sqrt(z) if z != 0 else two_sqrtpi, range(16), 0, 1,
              lambda z: 1 / (mp.erf(mp.sqrt(z)) / mp.sqrt(z)) if z != 0 else 1 / two_sqrtpi, fixed=[two_sqrtpi])
coeffs('ERF_P', cs, err, 'erf x = x (P0 + P1 z + ... + P15 z^15), z = x^2 <= 1 (P0 = 2/sqrt(pi) to 106 bits: P0 + ERF_P0_LO)')
const('ERF_P0_LO', d(two_sqrtpi - mp.mpf(cs[0])))
pieces('ERFC_A', lambda x: mp.erfc(x) * mp.exp(x * x), '0.5', 4, 28, 9, 'erfc x e^(x^2) for 1/2 <= x <= 4')
# (in pieces of 1/64 from 0, so that v's piece is floor(64 v), exactly: the
# first two are those of x > 32, beyond erfc's underflow)
pieces('ERFC_B', lambda v: mp.erfc(1 / v) * mp.exp(1 / (v * v)) / v if v != 0 else 1 / mp.sqrt(mp.pi), 0,
       mp.mpf('0.25'), 16, 9, 'erfc x e^(x^2) x for x >= 4, in v = 1/x', 'v')
pieces('ERFCF_A', lambda x: mp.erfc(x) * mp.exp(x * x), '0.5', 4, 14, 8, 'erfc x e^(x^2) for 1/2 <= x <= 4 (float)')
pieces('ERFCF_B', lambda v: mp.erfc(1 / v) * mp.exp(1 / (v * v)) / v, mp.mpf(1) / 11, mp.mpf('0.25'), 2, 7,
       'erfc x e^(x^2) x for 4 <= x <= 11, in v = 1/x (float)', 'v')
pieces('GAM', lambda t: mp.gamma(1 + t), 0, 1, 8, 13, 'gamma(1 + t) for 0 <= t <= 1', 't')
pieces('LGAM1', lambda t: mp.loggamma(1 + t) / t if t != 0 else -mp.euler, '-0.25', '0.25', 4, 14,
       'lgamma(1 + t) / t for |t| <= 1/4', 't')
pieces('LGAM2', lambda t: mp.loggamma(2 + t) / t if t != 0 else 1 - mp.euler, '-0.5', '0.5', 8, 14,
       'lgamma(2 + t) / t for |t| <= 1/2', 't')


def stirling(w):
    x = 1 / mp.sqrt(w)
    return (mp.loggamma(x) - ((x - mp.mpf(0.5)) * mp.log(x) - x + mp.log(2 * mp.pi) / 2)) * x


cs, err = fit('ST', lambda w: stirling(w) if w != 0 else mp.mpf(1) / 12, range(9), 0, mp.mpf(1) / 144,
              lambda w: 1 / stirling(w) if w != 0 else 12)
coeffs('STIRLING_P', cs, err, 'x (lgamma x - (x - 1/2) log x + x - log(2 pi)/2) = P0 + P1 w + ... + P8 w^8, w = 1/x^2, x >= 12')
h = d(mp.log(2 * mp.pi) / 2)
const('HLN2PI_HI', h, 'log(2 pi)/2 = HLN2PI_HI + HLN2PI_LO')
const('HLN2PI_LO', d(mp.log(2 * mp.pi) / 2 - mp.mpf(h)))
h = d(mp.log(mp.pi))
const('LNPI_HI', h, 'log(pi) = LNPI_HI + LNPI_LO')
const('LNPI_LO', d(mp.log(mp.pi) - mp.mpf(h)))


# -----------------------------------------------------------------------------
# log2's own table and polynomial; tan's table
# -----------------------------------------------------------------------------
l2 = []
for i in range(128):
    c = mp.mpf(struct.unpack('<d', struct.pack('<Q', logc[i]))[0])
    v = -mp.log(c, 2)
    h = d(mp.nint(v * mp.mpf(2) ** 42) / mp.mpf(2) ** 42)  # (k + h is exact for |k| < 2^11)
    l2 += [bits(h), bits(d(v - mp.mpf(h)))]
u64_array('LOG2_L', l2, '-log2(c_i) as pairs (hi, lo), hi a multiple of 2^-42')
# log2's reduction to u = r/ln2 directly: c_i/ln 2 as a double-double, and
# log2(1 + u ln2) in u
ci = []
for i in range(128):
    c = mp.mpf(struct.unpack('<d', struct.pack('<Q', logc[i]))[0])
    v = c / mp.log(2)
    h = d(v)
    ci += [bits(h), bits(d(v - mp.mpf(h)))]
u64_array('LOG2_CI', ci, 'c_i/ln 2 as pairs (hi, lo), to 2^-106')
UM = RM / mp.log(2)
cs, err = fit('L2U', lambda u: (mp.log1p(u * mp.log(2)) / mp.log(2) - u) / (u * u) if u != 0 else -mp.log(2) / 2,
              range(6), -UM, UM, lambda u: u * u / (mp.log1p(u * mp.log(2)) / mp.log(2)) if u != 0 else 0)
coeffs('LOG2U_P', cs, err, 'log2(1 + u ln2) = u + u^2 (P0 + P1 u + ... + P5 u^5), |u| <= the bound of LOG_P / ln 2')
# log10's: -log10 c with a high part that k log10(2)'s keeps exact
L10_2 = mp.log10(2)
l10h = split_hi(L10_2, 42)
const('LOG10_2_HI', l10h, 'log10(2) = LOG10_2_HI + LOG10_2_LO (42 bits: k LOG10_2_HI is exact for |k| < 2^11)')
const('LOG10_2_LO', d(L10_2 - mp.mpf(l10h)))
l10 = []
for i in range(128):
    c = mp.mpf(struct.unpack('<d', struct.pack('<Q', logc[i]))[0])
    v = -mp.log10(c)
    h = d(mp.nint(v * mp.mpf(2) ** 43) / mp.mpf(2) ** 43)  # (k LOG10_2_HI + h is exact for |k| < 2^11)
    l10 += [bits(h), bits(d(v - mp.mpf(h)))]
u64_array('LOG10_L', l10, '-log10(c_i) as pairs (hi, lo), hi a multiple of 2^-43')
cs, err = fit('L10', lambda r: (mp.log1p(r) - r) / mp.log(10) / (r * r) if r != 0 else -1 / (2 * mp.log(10)),
              range(7), -RM, RM, lambda r: r * r / mp.log1p(r) * mp.log(10) if r != 0 else 0)
coeffs('LOG10_P', cs, err, 'log10(1 + r) = r/ln10 + r^2 (P0 + P1 r + ... + P6 r^6)')
# cbrt's first estimate: cbrt f for f in [1, 2]
cs, err = fit('CB', mp.cbrt, range(6), 1, 2, lambda f: 1 / mp.cbrt(f))
coeffs('CBRT_P', cs, err, 'cbrt f = P0 + P1 f + ... + P5 f^5 for 1 <= f <= 2')
const('CBRT2', d(mp.cbrt(2)), '2^(1/3)')
const('CBRT4', d(mp.cbrt(4)), '2^(2/3)')
# cbrt's table: m = 2^j f in [1, 8) (j = 0, 1, 2), in 64 intervals a binade;
# the middle c of m's interval, cbrt m = cbrt(c) (1 + t)^(1/3), t = (m - c)/c
cb = []
for j in range(3):
    for i in range(64):
        c = mp.mpf(2) ** j * (1 + (mp.mpf(i) + mp.mpf(1) / 2) / 64)
        h = d(mp.cbrt(c))
        cb += [bits(d(1 / c)), bits(h), bits(d(mp.cbrt(c) - mp.mpf(h)))]
u64_array('CBRT_T', cb, 'for m = 2^j f in [1, 8): rows of 1/c, cbrt(c) (hi, lo), c = 2^j (1 + (i + 1/2)/64), row 64 j + i')
CBT = mp.mpf(1) / 129
cs, err = fit('CBQ', lambda t: (mp.cbrt(1 + t) - 1) / t if t != 0 else mp.mpf(1) / 3, range(6), -CBT, CBT,
              lambda t: abs(t))
coeffs('CBRT_Q', cs, err, '(1 + t)^(1/3) = 1 + t (Q0 + Q1 t + ... + Q5 t^5) for |t| <= 1/129 (error relative to 1)')
# log1p's for |x| <= 1/32, without a reduction
cs, err = fit('L1P', lambda r: (mp.log1p(r) - r) / (r * r) if r != 0 else mp.mpf(-0.5), range(9), -mp.mpf(1) / 32,
              mp.mpf(1) / 32, lambda r: r * r / mp.log1p(r) if r != 0 else 0, fixed=[-0.5])
coeffs('LOG1P_P', cs, err, 'log1p(x) = x + x^2 (P0 + P1 x + ... + P8 x^8) for |x| <= 1/32')
# tan(c + u) = (tan c + tan u)/(1 - tan c tan u) for c = j/256, j = 0 .. 201
# (|r| <= pi/4 + a little), |u| <= 1/512 (256 rows, for an index of 8 bits)
tt = []
for j in range(256):
    v = mp.tan(mp.mpf(j) / 256)
    h = d(v)
    tt += [bits(h), bits(d(v - mp.mpf(h)))]
# and -cot(j/256) = tan(j/256 - pi/2), for the odd form -1/tan (j = 0: unused)
for j in range(256):
    if j == 0:
        tt += [0, 0]
        continue
    v = -mp.cot(mp.mpf(j) / 256)
    h = d(v)
    tt += [bits(h), bits(d(v - mp.mpf(h)))]
u64_array('TAN_T', tt, 'tan(j/256), then -cot(j/256), as pairs (hi, lo), j = 0 .. 255 (row j, row 256 + j)')
Z = (mp.mpf(1) / 512 * mp.mpf('1.0001')) ** 2
cs, err = fit('TN', lambda z: (mp.tan(mp.sqrt(z)) / mp.sqrt(z) - 1) / z if z != 0 else mp.mpf(1) / 3, range(3), 0, Z,
              lambda z: z * mp.sqrt(z) / mp.tan(mp.sqrt(z)) if z != 0 else 0)
coeffs('TAN_Q', cs, err, 'tan u = u + u z (Q0 + Q1 z + Q2 z^2), z = u^2, |u| <= 1/512')

# -----------------------------------------------------------------------------
# the float functions' kernels, in double (to about 2^-40: a 2^-16th of an ulp
# of float)
# -----------------------------------------------------------------------------
RE = mp.log(2) / 256 * mp.mpf('1.0001')
cs, err = fit('EF', lambda r: (mp.exp(r) - 1 - r) / (r * r) if r != 0 else mp.mpf(1) / 2, range(2), -RE, RE,
              lambda r: r * r / mp.exp(r))
coeffs('EXPF_P', cs, err, 'e^r = 1 + r + r^2 (P0 + P1 r), |r| <= ln2/256')
cs, err = fit('LF', lambda r: (mp.log1p(r) - r) / (r * r) if r != 0 else mp.mpf(-0.5), range(4), -RM, RM,
              lambda r: r * r / mp.log1p(r) if r != 0 else 0)
coeffs('LOGF_P', cs, err, 'log1p(r) = r + r^2 (P0 + P1 r + P2 r^2 + P3 r^3) for |r| <= the bound of LOG_P')
# powf's: log2(1 + r) directly, and 2^u, without the conversions by ln2
cs, err = fit('L2F', lambda r: mp.log1p(r) / r / LN2 if r != 0 else 1 / LN2, range(5), -RM, RM,
              lambda r: r * LN2 / mp.log1p(r) if r != 0 else LN2)
coeffs('LOG2F_P', cs, err, 'log2(1 + r) = r (P0 + P1 r + ... + P4 r^4) for |r| <= the bound of LOG_P')
U2 = mp.mpf(1) / 256 * mp.mpf('1.0001')
cs, err = fit('E2F', lambda u: (mp.mpf(2) ** u - 1) / u if u != 0 else LN2, range(4), -U2, U2,
              lambda u: u / mp.mpf(2) ** u)
coeffs('EXP2F_P', cs, err, '2^u = 1 + u (P0 + P1 u + P2 u^2 + P3 u^3) for |u| <= 1/256')
Z = (mp.pi / 4 * mp.mpf('1.0001')) ** 2
cs, err = fit('SF', lambda z: (mp.sin(mp.sqrt(z)) / mp.sqrt(z) - 1) / z if z != 0 else -mp.mpf(1) / 6, range(4), 0, Z,
              lambda z: z * mp.sqrt(z) / mp.sin(mp.sqrt(z)) if z != 0 else 0)
coeffs('SINF_P', cs, err, 'sin r = r + r z (P0 + P1 z + P2 z^2 + P3 z^3), z = r^2, |r| <= pi/4')
cs, err = fit('CF', lambda z: (mp.cos(mp.sqrt(z)) - 1) / z if z != 0 else -mp.mpf(1) / 2, range(5), 0, Z,
              lambda z: z / mp.cos(mp.sqrt(z)))
coeffs('COSF_P', cs, err, 'cos r = 1 + z (P0 + P1 z + ... + P4 z^4), z = r^2, |r| <= pi/4')
cs, err = fit('EM1F', lambda x: (mp.expm1(x) - x) / (x * x) if x != 0 else mp.mpf(1) / 2, range(8), -mp.mpf(1) / 4,
              mp.mpf(1) / 4, lambda x: x * x / mp.expm1(x) if x != 0 else 0)
coeffs('EXPM1F_P', cs, err, 'e^x - 1 = x + x^2 (P0 + P1 x + ... + P7 x^7), |x| <= 1/4')
TP8 = mp.tan(mp.pi / 8) * mp.mpf('1.0001')
cs, err = fit('ATF', lambda z: (mp.atan(mp.sqrt(z)) / mp.sqrt(z) - 1) / z if z != 0 else -mp.mpf(1) / 3, range(7), 0,
              TP8 * TP8, lambda z: z * mp.sqrt(z) / mp.atan(mp.sqrt(z)) if z != 0 else 0)
coeffs('ATANF_P', cs, err, 'atan u = u + u z (P0 + P1 z + ... + P6 z^6), z = u^2, |u| <= tan(pi/8)')
asin_t = lambda z: (mp.asin(mp.sqrt(z)) / mp.sqrt(z) - 1) / z if z != 0 else mp.mpf(1) / 6
cs, err = fit('ASF', asin_t, range(9), 0, mp.mpf(1) / 4, lambda z: 1 / asin_t(z))
coeffs('ASINF_P', cs, err, 'asin s = s + s z (P0 + P1 z + ... + P8 z^8), z = s^2 <= 1/4 (relative to s z P)')
cs, err = fit('SHF', lambda z: (mp.sinh(mp.sqrt(z)) / mp.sqrt(z) - 1) / z if z != 0 else mp.mpf(1) / 6, range(4), 0,
              mp.mpf(1) / 4, lambda z: z * mp.sqrt(z) / mp.sinh(mp.sqrt(z)) if z != 0 else 0)
coeffs('SINHF_P', cs, err, 'sinh x = x + x z (P0 + P1 z + P2 z^2 + P3 z^3), z = x^2 <= 1/4')
cs, err = fit('THF', lambda z: (mp.tanh(mp.sqrt(z)) / mp.sqrt(z) - 1) / z if z != 0 else -mp.mpf(1) / 3, range(7), 0,
              mp.mpf(1) / 4, lambda z: z * mp.sqrt(z) / mp.tanh(mp.sqrt(z)) if z != 0 else 0)
coeffs('TANHF_P', cs, err, 'tanh x = x + x z (P0 + P1 z + ... + P6 z^6), z = x^2 <= 1/4')
erf_r = lambda z: mp.erf(mp.sqrt(z)) / mp.sqrt(z) if z != 0 else 2 / mp.sqrt(mp.pi)
cs, err = fit('ERFF', erf_r, range(9), 0, 1, lambda z: 1 / erf_r(z))
coeffs('ERFF_P', cs, err, 'erf x = x (P0 + P1 z + ... + P8 z^8), z = x^2 <= 1')


# lgamma near its zeros for x < -2: in each (-n - 1, -n), n = 2 .. 17, two
# zeros x0, where log|gamma| cancels to below the reflection formula's error;
# there lgamma(x0 + d) = d (P0 + P1 d + ...), P_k = psi^(k)(x0) / (k + 1)!
# (the derivatives of log|gamma| are the polygamma functions), for |d| <= r
# with r = 2^-3 / |psi(x0)| (beyond, |lgamma| > 2^-3), and at most an eighth of
# the distance to the pole (the series' terms fall by 8 each)
zrows = []


def bisect(f, x1, x2):
    """The root of f between x1 and x2 (a sign change), to 2^-350."""
    s1 = mp.sign(f(x1))
    for _ in range(360):
        xm = (x1 + x2) / 2
        if mp.sign(f(xm)) == s1:
            x1 = xm
        else:
            x2 = xm
    return (x1 + x2) / 2


with mp.workprec(400):
    eps = mp.mpf(2) ** -340
    for n in range(2, 18):
        lo = mp.mpf(-n - 1) + eps
        hi = mp.mpf(-n) - eps
        mid = bisect(mp.digamma, lo, hi)  # psi increases from -inf to +inf here
        for a, b in ((lo, mid), (mid, hi)):
            x0 = bisect(lambda x: mp.re(mp.loggamma(x)), a, b)
            ps = abs(mp.digamma(x0))
            r = min(mp.mpf(2) ** -3 / ps, abs(x0 - mp.nint(x0)) / 8)
            h = d(x0)
            # (in u = d / sc, sc the power of two at or above r, so that the
            # coefficients stay in range: P_k sc^k)
            sc = mp.mpf(2) ** mp.ceil(mp.log(r, 2))
            q0 = mp.digamma(x0)
            row = [h, d(x0 - mp.mpf(h)), d(r), d(sc), d(q0 - mp.mpf(d(q0)))]
            row += [d(mp.polygamma(k, x0) / mp.factorial(k + 1) * sc ** k) for k in range(25)]
            zrows.append(row)
emit('// lgamma near its zeros below -2: rows of x0 (hi, lo), the radius r, a power of')
emit('// two sc >= r, the low part of Q0, then Q0 .. Q24: lgamma(x0 + d) = d (Q0 + Q1 u + ...),')
emit('// u = d/sc, |d| <= r')
emit('pub const LGZ_N: usize = %d;' % len(zrows))
emit('pub const LGZ_W: usize = 30;')
u64_array('LGZ', [bits(v) for row in zrows for v in row], '')

with open(OUT, 'w') as f:
    f.write('\n'.join(out).rstrip('\n') + '\n')
print('wrote', OUT)

# =============================================================================
# std/c/rt/math_ld_tables.ax: the wide long double functions' (math_ld.ax),
# whose values are triple-doubles (h, m, l: h + m + l to 2^-159 or so)
# =============================================================================
out = []
emit('// =============================================================================')
emit('// std/c/rt/math_ld_tables.ax - generated by cmd/internal/tools/gen_math_tables.py')
emit('// (do not edit): the tables and constants of the long double functions')
emit('// (math_ld.ax), as triple-doubles (three u64 a value: h, m, l) and')
emit('// double-doubles, computed in arbitrary precision.')
emit('// =============================================================================')
emit()


def td(x):
    """x as a triple-double: [h, m, l], each the nearest double to what is left."""
    x = mp.mpf(x)
    h = d(x)
    m = d(x - mp.mpf(h))
    l_ = d(x - mp.mpf(h) - mp.mpf(m))
    return [h, m, l_]


def ddv(x):
    x = mp.mpf(x)
    h = d(x)
    return [h, d(x - mp.mpf(h))]


def td_const(name, x, doc):
    v = td(x)
    emit('// ' + doc)
    for k, c in zip('HML', v):
        emit('pub const %s_%s: f64 = %s;' % (name, k, lit(c)))


with mp.workprec(800):
    td_const('LN2', mp.log(2), 'ln 2')
    td_const('INVLN2', 1 / mp.log(2), '1 / ln 2')
    td_const('LN10', mp.log(10), 'ln 10')
    td_const('INVLN10', 1 / mp.log(10), '1 / ln 10')
    td_const('PI', mp.pi, 'pi')
    td_const('PIO2', mp.pi / 2, 'pi / 2')
    td_const('INVPI', 1 / mp.pi, '1 / pi')
    td_const('PIO4', mp.pi / 4, 'pi / 4')
    td_const('THIRD', mp.mpf(1) / 3, '1 / 3')
    # ln2/64 in parts of 32 bits (k times each is exact for |k| < 2^21) and a
    # last double, and E ln2 for the logarithm in parts of 36 bits (|E| < 2^17)
    rest = mp.log(2) / 64
    parts = []
    for _ in range(3):
        v = split_hi(rest, 32)
        parts.append(v)
        rest -= mp.mpf(v)
    parts.append(d(rest))
    emit('// ln2/64 = LN2_64_0 + ... + LN2_64_3, the first three of 32 bits')
    for i, v in enumerate(parts):
        emit('pub const LN2_64_%d: f64 = %s;' % (i, lit(v)))
    rest = mp.log(2)
    parts = []
    for _ in range(3):
        v = split_hi(rest, 36)
        parts.append(v)
        rest -= mp.mpf(v)
    parts.append(d(rest))
    emit('// ln2 = LN2_S0 + ... + LN2_S3, the first three of 36 bits')
    for i, v in enumerate(parts):
        emit('pub const LN2_S%d: f64 = %s;' % (i, lit(v)))
    emit('pub const INVLN2_64: f64 = %s;' % lit(d(64 / mp.log(2))))
    # pi/2 in parts of 30 bits and a last double (k times each of the first
    # three is exact for |k| < 2^23), for the x87 format's reduction
    rest = mp.pi / 2
    parts = []
    for _ in range(3):
        v = split_hi(rest, 30)
        parts.append(v)
        rest -= mp.mpf(v)
    parts.append(d(rest))
    emit('// pi/2 = PIO2_30_0 + ... + PIO2_30_3, the first three of 30 bits')
    for i, v in enumerate(parts):
        emit('pub const PIO2_30_%d: f64 = %s;' % (i, lit(v)))
    emit()
    tab = []
    for j in range(64):
        tab += [bits(v) for v in td(mp.mpf(2) ** (mp.mpf(j) / 64))]
    u64_array('EXP_T', tab, '2^(j/64), j = 0 .. 63, as triple-doubles')
    tab = []
    for i in range(128):
        c = mp.mpf(struct.unpack('<d', struct.pack('<Q', logc[i]))[0])
        tab += [bits(v) for v in td(-mp.log(c))]
    u64_array('LOG_L', tab, '-log(c_i) for tables.ax\'s LOG_C, as triple-doubles')
    tab = []
    for j in range(52):
        tab += [bits(v) for v in td(mp.sin(mp.mpf(j) / 64)) + td(mp.cos(mp.mpf(j) / 64))]
    u64_array('SC_T', tab, 'sin(j/64) and cos(j/64), j = 0 .. 51, as triple-doubles (six u64 a row)')
    tab = []
    for e in range(-6, 6):
        for k in range(16):
            c = mp.mpf(2) ** e * (1 + (mp.mpf(k) + mp.mpf('0.5')) / 16)
            tab += [bits(d(c))] + [bits(v) for v in td(mp.atan(c))]
    u64_array('ATAN_B', tab, 'for x in [2^-6, 2^6): rows of c, atan(c) (a triple-double), c = 2^e (1 + (k + 1/2)/16), row (e + 6) 16 + k')
    # Taylor coefficients, as double-doubles
    u64_array('EXP_C', [bits(v) for n in range(20) for v in ddv(1 / mp.factorial(n))], '1/n!, n = 0 .. 19 (pairs)')
    u64_array('INV_N', [bits(v) for n in range(40) for v in ddv(mp.mpf(1) / n if n else 0)], '1/n, n = 0 .. 39 (pairs; 0 for n = 0)')
    # the error function and the gamma functions (erfl, erfcl, lgammal, tgammal)
    td_const('TWO_SQRTPI', 2 / mp.sqrt(mp.pi), '2 / sqrt(pi)')
    td_const('INV_SQRTPI', 1 / mp.sqrt(mp.pi), '1 / sqrt(pi)')
    td_const('LN_SQRT2PI', mp.log(2 * mp.pi) / 2, 'log(2 pi) / 2')
    td_const('LNPI', mp.log(mp.pi), 'log pi')
    td_const('EULER', mp.euler, "Euler's constant")
    tab = []
    for k in range(1, 31):
        tab += [bits(v) for v in td(mp.bernoulli(2 * k) / (2 * k * (2 * k - 1)))]
    u64_array('STIRLING_C', tab, 'B_2k / (2k (2k - 1)), k = 1 .. 30: lgamma(x) = (x - 1/2) log x - x + log(2 pi)/2 '
              '+ sum of these / x^(2k - 1) (as triple-doubles)')
    tab = []
    for k in range(2, 82):
        tab += [bits(v) for v in td((-1) ** k * (mp.zeta(k) - 1) / k)]
    u64_array('LGAMMA2_C', tab, '(-1)^k (zeta(k) - 1) / k, k = 2 .. 81: lgamma(2 + z) = (1 - gamma) z + sum of '
              'these z^k, |z| < 2 (as triple-doubles)')
    td_const('ONE_MINUS_EULER', 1 - mp.euler, "1 - Euler's constant")
    tab = []
    for n in range(0, 81):
        tab += [bits(v) for v in td(mp.mpf((-1) ** n) / (mp.factorial(n) * (2 * n + 1)))]
    u64_array('ERF_A', tab, '(-1)^n / (n! (2n + 1)), n = 0 .. 80: erf a = 2/sqrt(pi) a sum of these a^(2n) (A&S 7.1.5)')
    # the zeros x0 of lgamma in (-(n+1), -n), n = 2 .. 33 (two each: the one
    # nearer -(n+1), then the one nearer -n), to about 2^-300 (two
    # triple-doubles: x0 and what is left), with the Taylor coefficients
    # psi^(j-1)(x0) / j!, j = 1 .. 16, of lgamma(x0 + d) (each a triple-double)
    tab = []
    lg = lambda v: mp.log(abs(mp.gamma(v)))
    for n in range(2, 34):
        mid = -n - mp.mpf(1) / 2
        lo_pole = mp.mpf(-(n + 1))
        hi_pole = mp.mpf(-n)
        z = []
        for a_, b_ in ((lo_pole, mid), (mid, hi_pole)):
            # bisection: lg > 0 at the pole's side, < 0 at the middle
            near = a_ if a_ == lo_pole else b_
            lo, hi = near, mid
            eps = mp.mpf(1) / 4
            while lg(near + (eps if near == lo_pole else -eps)) < 0:
                eps /= 2
            lo = near + (eps if near == lo_pole else -eps)
            hi = mid
            for _ in range(340):
                c = (lo + hi) / 2
                if lg(c) > 0:
                    lo = c
                else:
                    hi = c
            z.append((lo + hi) / 2)
        for x0 in z:
            hi = td(x0)
            row = hi + td(x0 - mp.mpf(hi[0]) - mp.mpf(hi[1]) - mp.mpf(hi[2]))
            for j in range(1, 17):
                row += td(mp.polygamma(j - 1, x0) / mp.factorial(j))
            tab += [bits(v) for v in row]
    u64_array('LGZERO', tab, 'the zeros of lgamma in (-(n+1), -n), n = 2 .. 33, two each (the one nearer -(n+1) first): '
              'x0 (two triple-doubles), then psi^(j-1)(x0) / j!, j = 1 .. 16 (18 triple-doubles a row)')
# the x87 format's double-double paths (math_ldx.ax): Taylor polynomials of
# degree 23 about the middles c of pieces, each coefficient a double-double.
# erfcx(x) = e^(x^2) erfc(x) on [1/2, 128): 8 pieces a binade, c = 2^e (1 +
# (k + 1/2)/8), |x - c| <= 2^e/16; its derivatives from erfcx' = 2x erfcx -
# 2/sqrt(pi): (n + 1) b_(n+1) = 2c b_n + 2 b_(n-1) (the recurrence loses bits
# forward, which the working precision covers); the terms after the 23rd are
# below 2^-96 of the value on every piece.
with mp.workprec(1500):
    tab = []
    for e in range(-1, 7):
        for k in range(8):
            c = mp.mpf(2) ** e * (1 + (mp.mpf(k) + mp.mpf('0.5')) / 8)
            y0 = mp.exp(c * c) * mp.erfc(c)
            b = [y0, 2 * c * y0 - 2 / mp.sqrt(mp.pi)]
            for n in range(1, 23):
                b.append((2 * c * b[n] + 2 * b[n - 1]) / (n + 1))
            tab += [bits(v) for co in b[:24] for v in ddv(co)]
    u64_array('ERFCX_T', tab, 'erfcx(c + t) = sum b_n t^n, n = 0 .. 23, for x in [2^e, 2^(e + 1)), e = -1 .. 6, '
              'in eighths k: c = 2^e (1 + (k + 1/2)/8), row (e + 1) 8 + k (pairs)')
    # the same to degree 31 in triple-doubles, for binary128 (math_ld.ax:
    # the terms after the 31st below 2^-124)
    tab = []
    for e in range(-1, 7):
        for k in range(8):
            c = mp.mpf(2) ** e * (1 + (mp.mpf(k) + mp.mpf('0.5')) / 8)
            y0 = mp.exp(c * c) * mp.erfc(c)
            b = [y0, 2 * c * y0 - 2 / mp.sqrt(mp.pi)]
            for n in range(1, 31):
                b.append((2 * c * b[n] + 2 * b[n - 1]) / (n + 1))
            tab += [bits(v) for co in b[:32] for v in td(co)]
    u64_array('ERFCX_Q', tab, 'erfcx(c + t) = sum b_n t^n, n = 0 .. 31, the pieces of ERFCX_T (triple-doubles)')
# lgamma(c + t) = sum b_n t^n, n = 0 .. 23 (b_n = psi^(n-1)(c)/n!), c = 3/2 + j/8,
# j = 0 .. 8, |t| <= 1/16 (lgamma(2) = 0: the piece about 2 keeps the relative
# precision near that zero); the terms after the 23rd below 2^-96
with mp.workprec(400):
    tab = []
    for j in range(9):
        c = mp.mpf(3) / 2 + mp.mpf(j) / 8
        b = [mp.mpf(0) if j == 4 else mp.loggamma(c)]
        b += [mp.polygamma(n - 1, c) / mp.factorial(n) for n in range(1, 24)]
        tab += [bits(v) for co in b for v in ddv(co)]
    u64_array('LGK_T', tab, 'lgamma(c + t) = sum b_n t^n, n = 0 .. 23, c = 3/2 + j/8, j = 0 .. 8 (pairs)')
# asin(c + t) = sum b_n t^n, n = 0 .. 23, c = j/32, j = 0 .. 24, |t| <= 1/64
# (asin' = g = u^(-1/2), u = 1 - (c + t)^2 = a0 + a1 t - t^2: u g' = -u' g/2
# gives g's coefficients; the radius of convergence is 1 - c >= 1/4, so the
# terms after the 23rd are below 2^-96)
with mp.workprec(600):
    tab = []
    for j in range(25):
        c = mp.mpf(j) / 32
        a0, a1, a2 = 1 - c * c, -2 * c, mp.mpf(-1)
        g = [1 / mp.sqrt(a0)]
        for n in range(0, 23):
            gm = g[n - 1] if n >= 1 else 0
            g.append((-(a1 * g[n] + 2 * a2 * gm) / 2 - a1 * n * g[n] - a2 * (n - 1) * gm) / (a0 * (n + 1)))
        b = [mp.asin(c)] + [g[n] / (n + 1) for n in range(23)]
        tab += [bits(v) for co in b for v in ddv(co)]
    u64_array('ASIN_T', tab, 'asin(c + t) = sum b_n t^n, n = 0 .. 23, c = j/32, j = 0 .. 24 (pairs)')
# 2/pi for the reduction of any binary128 (exponent up to 16383): 16896 bits
with mp.workprec(17500):
    v = 2 / mp.pi
    words = [0]
    for i in range(264):
        v = v * mp.mpf(2) ** 64
        w = int(mp.floor(v))
        words.append(w)
        v -= w
u64_array('TWO_OVER_PI', words, 'the bits of 2/pi after the binary point, 64 to a word, after a zero word')

OUT2 = os.path.join(ROOT, 'std', 'c', 'rt', 'math_ld_tables.ax')
with open(OUT2, 'w') as f:
    f.write('\n'.join(out).rstrip('\n') + '\n')
print('wrote', OUT2)

# =============================================================================
# std/c/rt/dfp_tables.ax: the powers of ten of the decimal functions' binary
# path (dfp_fast.ax): 10^q = M 2^(E - 127), M a 128-bit integer in [2^127,
# 2^128) (10^q rounded down to it), for q = -440 .. 440
# =============================================================================
out = []
emit('// =============================================================================')
emit('// std/c/rt/dfp_tables.ax - generated by cmd/internal/tools/gen_math_tables.py')
emit('// (do not edit): 10^q = M 2^(E - 127) for q = -440 .. 440, M in [2^127, 2^128)')
emit('// rounded down, as rows of three u64: M\'s high and low words, then E (two\'s')
emit('// complement), row q + 440.')
emit('// =============================================================================')
emit()
tab = []
with mp.workprec(3000):
    for q in range(-440, 441):
        v = mp.mpf(10) ** q
        e = int(mp.floor(mp.log(v, 2)))
        while mp.mpf(2) ** e > v:
            e -= 1
        while mp.mpf(2) ** (e + 1) <= v:
            e += 1
        mm = int(mp.floor(v * mp.mpf(2) ** (127 - e)))
        assert (1 << 127) <= mm < (1 << 128)
        tab += [mm >> 64, mm & ((1 << 64) - 1), e & ((1 << 64) - 1)]
u64_array('P10_T', tab, '')
OUT3 = os.path.join(ROOT, 'std', 'c', 'rt', 'dfp_tables.ax')
with open(OUT3, 'w') as f:
    f.write('\n'.join(out).rstrip('\n') + '\n')
print('wrote', OUT3)
