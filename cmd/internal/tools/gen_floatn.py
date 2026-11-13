#!/usr/bin/env python3
"""gen_floatn.py - writes the C library's functions of C23 Annex H's
interchange and extended floating types that are the functions of another
type of the same format (H.11, H.12): their declarations, the wrappers that
define them, and a test that calls each.

The list of functions is H.11.2's synopses (the 7.12 families with the fN,
fNx and dNx suffixes), H.11.3's encoding functions and H.12's numeric
conversions. A function of _Float32 is float's; of _Float64 and _Float32x,
double's; of _Float64x, long double's where long double is wider than double
(the x87 format or binary128), else _Float128's; the wrappers call those
(each is the one of its format: the results are the same bits). The
functions of _Float16 and _Float128 are the library's own (std/c/lib/math/
m_f16.c, m_f128.c, m_f16_c23.c, m_f128_c23.c, m_ldwide.c); this file only
declares them.

Writes:
    std/c/include/axc/math_floatn.h      <math.h>'s declarations (H.11)
    std/c/include/axc/complex_floatn.h   <complex.h>'s (H.11)
    std/c/lib/math/fn_f32.c, fn_f64.c, fn_f32x.c, fn_f64x.c   the wrappers
    std/c/lib/math/fn_cx_f32.c ... fn_cx_f64x.c                the complex ones
    std/c/lib/math/fn_narrow.c   the narrowing operations (H.11.2, 7.12.14)
    std/c/lib/math/fn_enc.c      the encoding conversion functions (H.11.3)
    cmd/axiom/internal/tests/c/lib/math/floatn_names.c        the test

    python3 cmd/internal/tools/gen_floatn.py
"""
import os

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..'))

# (name, signature): R is the real type; the signatures of H.11.2.
FUNCS = []
def add(sig, names):
    for n in names.split():
        FUNCS.append((n, sig))

add('R(R)', 'acos asin atan cos sin tan acospi asinpi atanpi cospi sinpi tanpi acosh asinh atanh cosh sinh tanh '
            'exp exp10 exp10m1 exp2 exp2m1 expm1 log log10 log10p1 log1p logp1 log2 log2p1 logb cbrt fabs sqrt '
            'rsqrt erf erfc lgamma tgamma ceil floor nearbyint rint round roundeven trunc nextup nextdown')
add('R(R,R)', 'atan2 atan2pi hypot pow powr fmod remainder copysign nextafter fdim fmaximum fminimum fmaximum_mag '
              'fminimum_mag fmaximum_num fminimum_num fmaximum_mag_num fminimum_mag_num fmax fmin')
add('R(R,R,R)', 'fma')
add('R(R,int*)', 'frexp')
add('R(R,R*)', 'modf')
add('R(R,R,int*)', 'remquo')
add('R(R,int)', 'ldexp scalbn')
add('R(R,long)', 'scalbln')
add('R(R,ll)', 'rootn pown compoundn')
add('int(R)', 'ilogb')
add('long(R)', 'llogb lrint lround')
add('ll(R)', 'llrint llround')
add('R(R,int,unsigned)', 'fromfp ufromfp fromfpx ufromfpx')
add('R(str)', 'nan')
add('int(cR*,cR*)', 'totalorder totalordermag')
add('R(cR*)', 'getpayload')
add('int(R*,R)', 'setpayload setpayloadsig')
add('int(R*,cR*)', 'canonicalize')

# <complex.h>'s (7.3.5-7.3.9, H.11): C a complex, R a real argument or result
CFUNCS = [(n, 'C(C)') for n in 'cacos casin catan ccos csin ctan cacosh casinh catanh ccosh csinh ctanh cexp clog '
          'csqrt conj cproj'.split()] + [(n, 'R(C)') for n in 'cabs carg cimag creal'.split()] + [('cpow', 'C(C,C)')]

# The suffixes: (suffix, type, the C expression of a test argument's type)
SUFFIXES = [('f16', '_Float16'), ('f32', '_Float32'), ('f64', '_Float64'), ('f128', '_Float128'),
            ('f32x', '_Float32x'), ('f64x', '_Float64x')]
WRAPPED = {'f32': 'f', 'f64': '', 'f32x': '', 'f64x': None}  # the base function's suffix

PARAMS = {'R': 'R __x', 'int*': 'int *__e', 'R*': 'R *__p', 'int': 'int __n', 'long': 'long __n',
          'll': 'long long __n', 'unsigned': 'unsigned int __w', 'str': 'const char *__tag',
          'cR*': 'const R *__x', 'R,int,unsigned': ''}
RET = {'R': 'R', 'int': 'int', 'long': 'long', 'll': 'long long'}


def parse(sig):
    ret, rest = sig.split('(', 1)
    return ret, rest[:-1].split(',')


def decl(name, sig, suf, ty, params=True):
    ret, ps = parse(sig)
    out = []
    xs = 0
    for i, p in enumerate(ps):
        nm = '__%s' % 'xyz'[i] if p in ('R', 'cR*', 'R*') else '__a%d' % i
        if p == 'R':
            out.append('%s %s' % (ty, nm))
        elif p == 'cR*':
            out.append('const %s *%s' % (ty, nm))
        elif p == 'R*':
            out.append('%s *%s' % (ty, nm))
        elif p == 'int*':
            out.append('int *%s' % nm)
        elif p == 'll':
            out.append('long long %s' % nm)
        elif p == 'unsigned':
            out.append('unsigned int %s' % nm)
        elif p == 'str':
            out.append('const char *%s' % nm)
        else:
            out.append('%s %s' % (p, nm))
    r = ty if ret == 'R' else RET[ret]
    return '%s %s%s(%s)' % (r, name, suf, ', '.join(out))


def wrapper(name, sig, suf, ty, base_suffix, base_ty):
    """the definition of name+suf calling name+base_suffix (type base_ty)"""
    ret, ps = parse(sig)
    args = []
    pre = []
    post = []
    for i, p in enumerate(ps):
        nm = '__%s' % 'xyz'[i] if p in ('R', 'cR*', 'R*') else '__a%d' % i
        if p == 'cR*':
            args.append('(const %s *)%s' % (base_ty, nm))
        elif p == 'R*':
            pre.append('%s __t;' % base_ty)
            args.append('&__t')
            post.append('*%s = __t;' % nm)
        else:
            args.append(nm)
    call = '%s%s(%s)' % (name, base_suffix, ', '.join(args))
    head = decl(name, sig, suf, ty)
    if ret == 'R' and sig == 'int(R*,R)':
        pass
    if sig in ('int(R*,R)', 'int(R*,cR*)'):
        # (the result through a pointer of the base type: the same bits)
        a0 = '__x'
        body = '    return %s%s((%s *)%s, %s);' % (name, base_suffix, base_ty, a0,
                                                  '(const %s *)__y' % base_ty if sig == 'int(R*,cR*)' else '__y')
        return '%s\n{\n%s\n}\n' % (head, body)
    if post:
        body = '    %s\n    %s __r = %s;\n    %s\n    return __r;' % (
            ' '.join(pre), ty if ret == 'R' else RET[ret], call, ' '.join(post))
    else:
        body = '    return %s;' % call
    return '%s\n{\n%s\n}\n' % (head, body)


def write(path, text):
    full = os.path.join(ROOT, path)
    with open(full, 'w') as f:
        f.write(text)
    print('wrote', path)


def header():
    out = ['/* axc/math_floatn.h - <math.h>\'s functions of the interchange and extended',
           '   floating types (C23 Annex H.11.2), with __STDC_WANT_IEC_60559_TYPES_EXT__.',
           '   Generated by cmd/internal/tools/gen_floatn.py: do not edit. */']
    for suf, ty in SUFFIXES:
        out.append('/* %s */' % ty)
        for name, sig in FUNCS:
            out.append(decl(name, sig, suf, ty) + ';')
    out.extend(nheader())
    out.extend(eheader())
    return '\n'.join(out) + '\n'


def wrappers(suf, ty):
    bs = WRAPPED[suf]
    out = ['/* fn_%s.c - <math.h>\'s %s functions (C23 Annex H.11.2): those of' % (suf, ty)]
    if suf == 'f32':
        out.append('   float, whose format %s has (binary32): the same values. Generated by' % ty)
    elif suf == 'f64x':
        out.append('   long double where long double is wider than double (the x87 format or')
        out.append('   binary128, as %s), else _Float128\'s: the same values. Generated by' % ty)
    else:
        out.append('   double, whose format %s has (binary64): the same values. Generated by' % ty)
    out.append('   cmd/internal/tools/gen_floatn.py: do not edit. */')
    out.append('#if __STDC_VERSION__ >= 199901L')
    out.append('#define __STDC_WANT_IEC_60559_TYPES_EXT__ 1')
    out.append('#include <math.h>')
    out.append('/* (C23\'s functions, which <math.h> declares in C23 only) */')
    if suf == 'f64x':
        out.append('#if __AXIOM_LDBL_WIDTH__ != 64')
        groups = [('l', 'long double'), ('f128', '_Float128')]
    else:
        groups = [(bs, 'float' if bs == 'f' else 'double')]
    for gi, (b, bty) in enumerate(groups):
        if gi == 1:
            out.append('#else')
        for name, sig in FUNCS:
            out.append(decl(name, sig, b, bty) + ';')
        for name, sig in FUNCS:
            out.append(wrapper(name, sig, suf, ty, b, bty))
    if suf == 'f64x':
        out.append('#endif')
    out.append('#else')
    out.append('typedef int __axc_fn_%s_c89;' % suf)
    out.append('#endif')
    return '\n'.join(out) + '\n'


# The test: every function of every suffix called once, its result's type
# checked with _Generic and its value against the function of its format
# (or, for _Float16 and _Float128, against a known value).
ARG = {'R': '(R)0.5', 'int*': '&ie', 'R*': '&rp', 'int': '3', 'long': '3L', 'll': '3LL', 'unsigned': '8u',
       'str': '"5"', 'cR*': '&rv'}


def test():
    out = ['/* Every function C23 Annex H.11.2 lists, of every interchange and extended',
           '   binary floating type: its result has the type, and _Float32 ...',
           '   _Float64x give the same values as the functions of their formats.',
           '   Generated by cmd/internal/tools/gen_floatn.py: do not edit. */',
           '/* flags: --c-std=c23 */',
           '#define __STDC_WANT_IEC_60559_TYPES_EXT__ 1',
           '#include "print.h"',
           '#include <complex.h>',
           '#include <math.h>',
           '#include <string.h>',
           '',
           '#define T(e, R) _Generic((e), R: 1, default: 0)',
           '',
           'static int ok = 1, n = 0;',
           '',
           'static void check(int c, const char *what)',
           '{',
           '    n++;',
           '    if (!c) {',
           '        ok = 0;',
           '        print_str(what, strlen(what));',
           '    }',
           '}',
           '']
    base = {'f32': ('f', 'float'), 'f64': ('', 'double'), 'f32x': ('', 'double')}
    for suf, ty in SUFFIXES:
        out.append('static void test_%s(void)' % suf)
        out.append('{')
        out.append('    int ie = 0;')
        out.append('    %s rp = 0, rv = (%s)1.5, rr = 0;' % (ty, ty))
        out.append('    (void)rr;')
        for name, sig in FUNCS:
            ret, ps = parse(sig)
            if sig in ('int(R*,R)',):
                args = ['&rr', '(%s)5' % ty]
            elif sig == 'int(R*,cR*)':
                args = ['&rr', '&rv']
            else:
                args = [ARG[p].replace('(R)', '(%s)' % ty) for p in ps]
            call = '%s%s(%s)' % (name, suf, ', '.join(args))
            rt = ty if ret == 'R' else RET[ret]
            if name in ('ldexp', 'scalbn', 'scalbln', 'rootn', 'pown', 'compoundn') or ret != 'R' or name in ('frexp', 'modf', 'remquo'):
                pass
            cond = 'T(%s, %s)' % (call, rt)
            if suf in base:
                b, bty = base[suf]
                bargs = []
                for a in args:
                    bargs.append(a.replace('(%s)' % ty, '(%s)' % bty))
                if sig in ('int(R*,R)', 'int(R*,cR*)', 'int(cR*,cR*)', 'R(cR*)', 'R(R,R*)'):
                    cond += ' && 1'
                elif name == 'nan':
                    cond += ' && isnan(%s)' % call
                else:
                    cond += ' && (%s == %s%s(%s) || (%s != %s))' % (call, name, b, ', '.join(bargs), call, call)
            out.append('    check(%s, "%s%s\\n");' % (cond, name, suf))
        out.append('    {')
        out.append('        %s _Complex z = CMPLX((%s)0.5, (%s)-0.25), w = CMPLX((%s)1.5, (%s)0.75);' % ((ty,) * 5))
        out.append('        (void)w;')
        for name, sig in CFUNCS:
            ret, ps = parse(sig)
            args = ', '.join('zw'[i] for i in range(len(ps)))
            call = '(%s%s)(%s)' % (name, suf, args)
            rt = ty + ' _Complex' if ret == 'C' else ty
            cond = 'T(%s, %s)' % (call, rt)
            if suf in base:
                b, bty = base[suf]
                bargs = ', '.join('(%s _Complex)%s' % (bty, 'zw'[i]) for i in range(len(ps)))
                cond += ' && %s == (%s%s)(%s)' % (call, name, b, bargs)
            out.append('        check(%s, "%s%s\\n");' % (cond, name, suf))
        out.append('    }')
        out.append('}')
        out.append('')
    out.append('static void test_narrow(void)')
    out.append('{')
    for r, a in NARROW:
        for op, n in NOPS:
            args = ', '.join(['(%s)%s' % (TY[a], v) for v in ('2.5', '1.25', '0.5')][:n])
            out.append('    check(T(%s%s%s(%s), %s), "%s%s%s\\n");' % (r, op, a, args, TY[r], r, op, a))
    out.append('}')
    out.append('')
    out.append('int main(void)')
    out.append('{')
    out.append('    test_narrow();')
    for suf, ty in SUFFIXES:
        out.append('    test_%s();' % suf)
    out.append('    print_i64(n);')
    out.append('    print_i64(ok);')
    out.append('    return 0;')
    out.append('}')
    count = (len(FUNCS) + len(CFUNCS)) * len(SUFFIXES) + len(NARROW) * len(NOPS)
    out.append('/* expect: %d */' % count)
    out.append('/* expect: 1 */')
    return '\n'.join(out) + '\n'


def cdecl(name, sig, suf, ty, paren=False):
    ret, ps = parse(sig)
    ct = ty + ' _Complex'
    args = ', '.join('%s __%s' % (ct, 'zw'[i]) for i in range(len(ps)))
    f = '(%s%s)' % (name, suf) if paren else name + suf
    return '%s %s(%s)' % (ct if ret == 'C' else ty, f, args)


def cheader():
    out = ['/* axc/complex_floatn.h - <complex.h>\'s functions of the interchange and',
           '   extended floating types (C23 Annex H.11), with __STDC_WANT_IEC_60559_TYPES_EXT__.',
           '   Generated by cmd/internal/tools/gen_floatn.py: do not edit. */']
    for suf, ty in SUFFIXES:
        for name, sig in CFUNCS:
            out.append(cdecl(name, sig, suf, ty) + ';')
    return '\n'.join(out) + '\n'


def cwrappers(suf, ty):
    out = ['/* fn_cx_%s.c - <complex.h>\'s %s functions (C23 Annex H.11): those of the' % (suf, ty),
           '   standard (or _Float128) type of its format, as fn_%s.c\'s real ones. Generated' % suf,
           '   by cmd/internal/tools/gen_floatn.py: do not edit. */',
           '#if __STDC_VERSION__ >= 199901L',
           '#define __STDC_WANT_IEC_60559_TYPES_EXT__ 1',
           '#include <complex.h>']
    if suf == 'f64x':
        groups = [('l', 'long double'), ('f128', '_Float128')]
        out.append('#if __AXIOM_LDBL_WIDTH__ != 64')
    else:
        groups = [(WRAPPED[suf], 'float' if WRAPPED[suf] == 'f' else 'double')]
    for gi, (b, bty) in enumerate(groups):
        if gi == 1:
            out.append('#else')
        for name, sig in CFUNCS:
            ret, ps = parse(sig)
            args = ', '.join('__%s' % 'zw'[i] for i in range(len(ps)))
            out.append('%s\n{\n    return (%s%s)(%s);\n}\n' % (cdecl(name, sig, suf, ty, True), name, b, args))
    if suf == 'f64x':
        out.append('#endif')
    out.append('#else')
    out.append('typedef int __axc_fn_cx_%s_c89;' % suf)
    out.append('#endif')
    return '\n'.join(out) + '\n'


# The narrowing operations (H.11.2's 7.12.14): (result suffix, argument
# suffix) for fMopfN (M < N), fMopfNx (M <= N), fMxopfN (M < N), fMxopfNx (M < N)
NARROW = [('f16', 'f32'), ('f16', 'f64'), ('f16', 'f128'), ('f32', 'f64'), ('f32', 'f128'), ('f64', 'f128'),
          ('f16', 'f32x'), ('f32', 'f32x'), ('f16', 'f64x'), ('f32', 'f64x'), ('f64', 'f64x'),
          ('f32x', 'f64'), ('f32x', 'f128'), ('f64x', 'f128'), ('f32x', 'f64x')]
NOPS = [('add', 2), ('sub', 2), ('mul', 2), ('div', 2), ('fma', 3), ('sqrt', 1)]
TY = dict(SUFFIXES)


def ndecl(op, n, r, a):
    ps = ', '.join('%s __%s' % (TY[a], 'xyz'[i]) for i in range(n))
    return '%s %s%s%s(%s)' % (TY[r], r, op, a, ps)


def nheader():
    out = ['/* the narrowing operations (C23 Annex H.11.2, 7.12.14): one rounding */']
    for r, a in NARROW:
        for op, n in NOPS:
            out.append(ndecl(op, n, r, a) + ';')
    return out


def narrow_c():
    """fn_narrow.c: each operation exact and rounded once (math_narrow.ax's
    __axc_narrow on binary128 or x87 operands, to binary16, 32 or 64), or the
    operation of the result's format where the operands have it"""
    out = ['/* fn_narrow.c - the narrowing operations of C23 Annex H.11.2 (7.12.14):',
           '   f16addf32 ... f32xsqrtf64x, each the operation\'s exact result rounded once',
           '   to the result\'s format in the current rounding direction, with the',
           '   IEEE 754 exceptions: std/c/rt/math_narrow.ax\'s __axc_narrow on the',
           '   operands as binary128 (binary32 and binary64 ones converted exactly) or in',
           '   the x87 format, where the result is binary16, 32 or 64 or the x87 format\'s (to double: the',
           '   standard narrowing operations\' core, m_narrow.c); where the operands',
           '   have the result\'s format, the operation of that format (one rounding).',
           '   Generated by cmd/internal/tools/gen_floatn.py: do not edit. */',
           '#if __STDC_VERSION__ >= 199901L',
           '#define __STDC_WANT_IEC_60559_TYPES_EXT__ 1',
           '#include <math.h>',
           'unsigned long long __axc_narrow(unsigned long long op, unsigned long long f, unsigned long long dst,',
           '                                const void *x, const void *y, const void *z);',
           '/* _Float64x\'s format: the x87\'s (1) or binary128 (0) */',
           '#define FX (__AXIOM_LDBL_WIDTH__ == 80)',
           'static _Float16 h_of(unsigned long long b)',
           '{',
           '    union { unsigned short u; _Float16 v; } r;',
           '    r.u = (unsigned short)b;',
           '    return r.v;',
           '}',
           'static float s_of(unsigned long long b)',
           '{',
           '    union { unsigned int u; float v; } r;',
           '    r.u = (unsigned int)b;',
           '    return r.v;',
           '}',
           '#if FX',
           '/* the x87 value of significand m and sign and exponent se (one operation',
           '   computed twice: math_narrow.ax gives a part each time) */',
           'static _Float64x x87_of(unsigned long long m, unsigned long long se)',
           '{',
           '    union { struct { unsigned long long m; unsigned short se; } p; _Float64x v; } r;',
           '    r.v = 0;',
           '    r.p.m = m;',
           '    r.p.se = (unsigned short)se;',
           '    return r.v;',
           '}',
           '#endif',
           'static double d_of(unsigned long long b)',
           '{',
           '    union { unsigned long long u; double v; } r;',
           '    r.u = b;',
           '    return r.v;',
           '}',
           '']
    dst = {'f16': ('2', 'h_of'), 'f32': ('0', 's_of'), 'f64': ('1', 'd_of'), 'f32x': ('1', 'd_of')}
    sym = {'add': '+', 'sub': '-', 'mul': '*', 'div': '/'}
    for r, a in NARROW:
        for oi, (op, n) in enumerate(NOPS):
            head = ndecl(op, n, r, a)
            names = ['__%s' % 'xyz'[i] for i in range(n)]
            if r == 'f64x':
                # (from binary128: the _Float128 operation where _Float64x is binary128)
                if op == 'sqrt':
                    expr = 'sqrtf128(__x)'
                elif op == 'fma':
                    expr = 'fmaf128(__x, __y, __z)'
                else:
                    expr = '__x %s __y' % sym[op]
                out.append('#if !FX')
                out.append('%s\n{\n    return %s;\n}\n' % (head, expr))
                out.append('#else')
                ptrs = ['&%s' % nm for nm in names]
                while len(ptrs) < 3:
                    ptrs.append(ptrs[-1])
                out.append('%s\n{\n    return x87_of(__axc_narrow(%d, 0, 3, %s), __axc_narrow(%d, 0, 4, %s));\n}\n' % (
                    head, oi, ', '.join(ptrs), oi, ', '.join(ptrs)))
                out.append('#endif')
                continue
            if (r, a) in (('f32x', 'f64'),):
                if op == 'sqrt':
                    expr = 'sqrt(__x)'
                elif op == 'fma':
                    expr = 'fma(__x, __y, __z)'
                else:
                    expr = '__x %s __y' % sym[op]
                out.append('%s\n{\n    return %s;\n}\n' % (head, expr))
                continue
            d, conv = dst[r]
            body = []
            if a in ('f32', 'f64', 'f32x'):
                # (converted exactly to binary128)
                body.append('    _Float128 %s;' % ', '.join('q%d = %s' % (i, nm) for i, nm in enumerate(names)))
                ptrs = ['&q%d' % i for i in range(n)]
                fmt = '0'
            elif a == 'f128':
                ptrs = ['&%s' % nm for nm in names]
                fmt = '0'
            else:
                ptrs = ['&%s' % nm for nm in names]
                fmt = 'FX'
            while len(ptrs) < 3:
                ptrs.append(ptrs[-1])
            body.append('    return %s(__axc_narrow(%d, %s, %s, %s));' % (conv, oi, fmt, d, ', '.join(ptrs)))
            out.append('%s\n{\n%s\n}\n' % (head, '\n'.join(body)))
    out.append('#else')
    out.append('typedef int __axc_fn_narrow_c89;')
    out.append('#endif')
    return '\n'.join(out) + '\n'


# H.11.3's encoding functions: encodefN, decodefN; fMencfN, dMencdecdN and
# dMencbindN for M != N
BW = [16, 32, 64, 128]
DW = [32, 64, 128]


def eheader():
    out = ['/* the encoding functions (H.11.3) */']
    for n in BW:
        out.append('void encodef%d(unsigned char __enc[restrict static %d], const _Float%d *restrict __x);' % (n, n // 8, n))
        out.append('void decodef%d(_Float%d *restrict __x, const unsigned char __enc[restrict static %d]);' % (n, n, n // 8))
    for mm in BW:
        for n in BW:
            if mm != n:
                out.append('void f%dencf%d(unsigned char __m[restrict static %d], const unsigned char __n[restrict static %d]);'
                           % (mm, n, mm // 8, n // 8))
    out.append('#if __STDC_VERSION__ >= 202311L')
    for kind in ('dec', 'bin'):
        for mm in DW:
            for n in DW:
                if mm != n:
                    out.append('void d%denc%sd%d(unsigned char __m[restrict static %d], const unsigned char __n[restrict static %d]);'
                               % (mm, kind, n, mm // 8, n // 8))
    out.append('#endif')
    return out


def enc_c():
    out = ['/* fn_enc.c - the encoding conversion functions of C23 Annex H.11.3:',
           '   encodefN and decodefN (the encoding is the representation\'s bytes, in',
           '   memory\'s order: little-endian on every target, __STDC_ENDIAN_NATIVE__),',
           '   fMencfN (the encoding of width N decoded, converted to _FloatM: rounded',
           '   in the current direction, with the exceptions; encoded), dMencdecdN and',
           '   dMencbindN (the same through decodedecdN or decodebindN, the conversion',
           '   in the decimal rounding direction, and encodedecdM or encodebindM).',
           '   Generated by cmd/internal/tools/gen_floatn.py: do not edit. */',
           '#if __STDC_VERSION__ >= 199901L',
           '#define __STDC_WANT_IEC_60559_TYPES_EXT__ 1',
           '#include <math.h>',
           'static void copy_bytes(void *d, const void *s, int n)',
           '{',
           '    unsigned char *dp = (unsigned char *)d;',
           '    const unsigned char *sp = (const unsigned char *)s;',
           '    int i;',
           '    for (i = 0; i < n; i++)',
           '        dp[i] = sp[i];',
           '}',
           '']
    for n in BW:
        out.append('void encodef%d(unsigned char *restrict __enc, const _Float%d *restrict __x)\n{\n    copy_bytes(__enc, __x, %d);\n}\n'
                   % (n, n, n // 8))
        out.append('void decodef%d(_Float%d *restrict __x, const unsigned char *restrict __enc)\n{\n    copy_bytes(__x, __enc, %d);\n}\n'
                   % (n, n, n // 8))
    for mm in BW:
        for n in BW:
            if mm != n:
                out.append('void f%dencf%d(unsigned char *restrict __m, const unsigned char *restrict __n)\n{\n'
                           '    _Float%d a;\n    _Float%d b;\n    decodef%d(&a, __n);\n    b = (_Float%d)a;\n'
                           '    encodef%d(__m, &b);\n}\n' % (mm, n, n, mm, n, mm, mm))
    out.append('/* (the decimal types by the names they have in every mode) */')
    for kind in ('dec', 'bin'):
        for n in DW:
            out.append('void encode%sd%d(unsigned char *restrict, const __axiom_decimal%d *restrict);' % (kind, n, n))
            out.append('void decode%sd%d(__axiom_decimal%d *restrict, const unsigned char *restrict);' % (kind, n, n))
    for kind in ('dec', 'bin'):
        for mm in DW:
            for n in DW:
                if mm != n:
                    out.append('void d%denc%sd%d(unsigned char *restrict __m, const unsigned char *restrict __n)\n{\n'
                               '    __axiom_decimal%d a;\n    __axiom_decimal%d b;\n    decode%sd%d(&a, __n);\n'
                               '    b = (__axiom_decimal%d)a;\n    encode%sd%d(__m, &b);\n}\n'
                               % (mm, kind, n, n, mm, kind, n, mm, kind, mm))
    out.append('#else')
    out.append('typedef int __axc_fn_enc_c89;')
    out.append('#endif')
    return '\n'.join(out) + '\n'


# H.12's numeric conversions of the types whose formats are standard ones'
STRS = [('f32', 'f', 'float'), ('f64', 'd', 'double'), ('f32x', 'd', 'double')]


def str_c():
    out = ['/* fn_str.c - the numeric conversions of C23 Annex H.12 (strtofN, strfromfN,',
           '   wcstofN) of _Float32, _Float64, _Float32x and _Float64x: those of the',
           '   standard (or _Float128) type of their format; and wcstof16, wcstof128,',
           '   which convert the wide string\'s first characters below 128 (the only',
           '   ones a number has) with strtof16 and strtof128. Generated by',
           '   cmd/internal/tools/gen_floatn.py: do not edit. */',
           '#if __STDC_VERSION__ >= 199901L',
           '#define __STDC_WANT_IEC_60559_TYPES_EXT__ 1',
           '#include <stdlib.h>',
           '#include <wchar.h>',
           '']
    def three(suf, ty, sfx_strto, sfx_from, wcs):
        out.append('%s strto%s(const char *restrict __n, char **restrict __e)\n{\n    return %s(__n, __e);\n}\n'
                   % (ty, suf, sfx_strto))
        out.append('int strfrom%s(char *restrict __s, size_t __n, const char *restrict __f, %s __v)\n{\n'
                   '    return %s(__s, __n, __f, __v);\n}\n' % (suf, ty, sfx_from))
        out.append('%s wcsto%s(const wchar_t *restrict __n, wchar_t **restrict __e)\n{\n    return %s(__n, __e);\n}\n'
                   % (ty, suf, wcs))
    three('f32', '_Float32', 'strtof', 'strfromf', 'wcstof')
    three('f64', '_Float64', 'strtod', 'strfromd', 'wcstod')
    three('f32x', '_Float32x', 'strtod', 'strfromd', 'wcstod')
    out.append('/* (the wide string\'s first characters below 128, narrowed; NULL if no memory) */')
    out.append('static char *narrow(const wchar_t *w, size_t *len)')
    out.append('{')
    out.append('    size_t n = 0, i;')
    out.append('    char *s;')
    out.append('    while (w[n] != 0 && (unsigned long)w[n] < 128)')
    out.append('        n++;')
    out.append('    s = (char *)malloc(n + 1);')
    out.append('    if (s == NULL)')
    out.append('        return NULL;')
    out.append('    for (i = 0; i < n; i++)')
    out.append('        s[i] = (char)w[i];')
    out.append('    s[n] = 0;')
    out.append('    *len = n;')
    out.append('    return s;')
    out.append('}')
    out.append('')
    for suf, ty in (('f16', '_Float16'), ('f128', '_Float128')):
        out.append('%s wcsto%s(const wchar_t *restrict __n, wchar_t **restrict __e)\n{\n'
                   '    size_t len;\n    char *end, *s = narrow(__n, &len);\n    %s r;\n'
                   '    if (s == NULL) {\n        if (__e != NULL)\n            *__e = (wchar_t *)__n;\n        return 0;\n    }\n'
                   '    r = strto%s(s, &end);\n    if (__e != NULL)\n        *__e = (wchar_t *)__n + (end - s);\n'
                   '    free(s);\n    return r;\n}\n' % (ty, suf, ty, suf))
    out.append('#if __AXIOM_LDBL_WIDTH__ != 64')
    three('f64x', '_Float64x', 'strtold', 'strfroml', 'wcstold')
    out.append('#else')
    three('f64x', '_Float64x', 'strtof128', 'strfromf128', 'wcstof128')
    out.append('#endif')
    out.append('#else')
    out.append('typedef int __axc_fn_str_c89;')
    out.append('#endif')
    return '\n'.join(out) + '\n'


def main():
    write('std/c/include/axc/math_floatn.h', header())
    for suf in ('f32', 'f64', 'f32x', 'f64x'):
        write('std/c/lib/math/fn_%s.c' % suf, wrappers(suf, dict(SUFFIXES)[suf]))
    write('std/c/lib/math/fn_narrow.c', narrow_c())
    write('std/c/lib/math/fn_enc.c', enc_c())
    write('std/c/lib/math/fn_str.c', str_c())
    write('std/c/include/axc/complex_floatn.h', cheader())
    for suf in ('f32', 'f64', 'f32x', 'f64x'):
        write('std/c/lib/math/fn_cx_%s.c' % suf, cwrappers(suf, dict(SUFFIXES)[suf]))
    write('cmd/axiom/internal/tests/c/lib/math/floatn_names.c', test())


if __name__ == '__main__':
    main()
