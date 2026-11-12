#!/usr/bin/env python3
"""Generates cmd/axiom/src/asm/x64tab.ax, the x86-64 assembler's table of
instruction forms (x87, MMX, SSE, AVX, AVX2, FMA, F16C, AVX-512 and the
system instructions), from the encodings below, which are written from
Intel's Software Developer's Manual, volume 2 (the opcode tables: "VEX.128.66.0F.WIG
58 /r" and the like, with the operand encodings and, for EVEX, the tuple
types of compressed displacements):

    python3 cmd/internal/tools/gen_x64_table.py > cmd/axiom/src/asm/x64tab.ax && axiom fmt cmd/axiom/src/asm/x64tab.ax

How a line of the table reads is in cmd/axiom/src/asm/x64.ax ("The
instruction table"). The families here (an SSE operation and its VEX and
EVEX forms, say) only save writing the same encoding three times.
"""
import sys

L = []


def ln(name, roles, types, enc, *attrs):
    L.append(' '.join([name, roles, types, enc] + [a for a in attrs if a]))


def none(name, enc, *opc):
    ln(name, '-0', '-', enc, *opc)


# ---------------------------------------------------------------------------
# x87
# ---------------------------------------------------------------------------
X87_NONE = {
    'f2xm1': 'd9 f0', 'fabs': 'd9 e1', 'fchs': 'd9 e0', 'fclex': '9b db e2', 'fnclex': 'db e2',
    'fcompp': 'de d9', 'fcos': 'd9 ff', 'fdecstp': 'd9 f6', 'fincstp': 'd9 f7', 'finit': '9b db e3',
    'fninit': 'db e3', 'fld1': 'd9 e8', 'fldl2t': 'd9 e9', 'fldl2e': 'd9 ea', 'fldpi': 'd9 eb',
    'fldlg2': 'd9 ec', 'fldln2': 'd9 ed', 'fldz': 'd9 ee', 'fnop': 'd9 d0', 'fpatan': 'd9 f3',
    'fprem': 'd9 f8', 'fprem1': 'd9 f5', 'fptan': 'd9 f2', 'frndint': 'd9 fc', 'fscale': 'd9 fd',
    'fsin': 'd9 fe', 'fsincos': 'd9 fb', 'fsqrt': 'd9 fa', 'ftst': 'd9 e4', 'fucompp': 'da e9',
    'fxam': 'd9 e5', 'fxtract': 'd9 f4', 'fyl2x': 'd9 f1', 'fyl2xp1': 'd9 f9',
    'faddp': 'de c1', 'fmulp': 'de c9', 'fsubp': 'de e1', 'fsubrp': 'de e9', 'fdivp': 'de f1',
    'fdivrp': 'de f9', 'fcom': 'd8 d1', 'fcomp': 'd8 d9', 'fucom': 'dd e1', 'fucomp': 'dd e9',
    'fxch': 'd9 c9', 'fnstsw': '', 'fstsw': '',
}
for n, c in X87_NONE.items():
    if c:
        none(n, 'L.np.-.w0', c)

# arithmetic: name, /digit, register forms (st(i) to st, st to st(i), and pop)
# The AT&T forms of the reversed operations follow the System V assembler's
# convention (fsub %st, %st(i) is Intel's fsubr st(i), st).
X87_ARITH = [('add', 0, 'c0', 'c0', 'c0'), ('mul', 1, 'c8', 'c8', 'c8'), ('sub', 4, 'e0', 'e0', 'e0'),
             ('subr', 5, 'e8', 'e8', 'e8'), ('div', 6, 'f0', 'f0', 'f0'), ('divr', 7, 'f8', 'f8', 'f8')]
SYSV = {'sub': 'subr', 'subr': 'sub', 'div': 'divr', 'divr': 'div'}
for n, d, a, b, p in X87_ARITH:
    ln('f%ss' % n, 'M', 'm', 'L.np.-.w0', 'd8', '/%d' % d)
    ln('f%sl' % n, 'M', 'm', 'L.np.-.w0', 'dc', '/%d' % d)
    ln('fi%ss' % n, 'M', 'm', 'L.np.-.w0', 'de', '/%d' % d)
    ln('fi%sl' % n, 'M', 'm', 'L.np.-.w0', 'da', '/%d' % d)
    ln('fi%s' % n, 'M', 'm', 'L.np.-.w0', 'de', '/%d' % d)
    ln('f%s' % n, '-O', 'st0,st', 'L.np.-.w0', 'd8', a)
    ln('f%s' % n, 'O', 'st', 'L.np.-.w0', 'd8', a)
    # st(i) <- st(i) op st: DC with the opcode of the reversed operation (System V)
    r = a
    ln('f%s' % n, 'O-', 'st,st0', 'L.np.-.w0', 'dc', r)
    ln('f%sp' % n, 'O-', 'st,st0', 'L.np.-.w0', 'de', r)
    ln('f%sp' % n, 'O', 'st', 'L.np.-.w0', 'de', r)
for n, d, c in [('com', 2, 'd0'), ('comp', 3, 'd8')]:
    ln('f%ss' % n, 'M', 'm', 'L.np.-.w0', 'd8', '/%d' % d)
    ln('f%sl' % n, 'M', 'm', 'L.np.-.w0', 'dc', '/%d' % d)
    ln('fi%ss' % n, 'M', 'm', 'L.np.-.w0', 'de', '/%d' % d)
    ln('fi%sl' % n, 'M', 'm', 'L.np.-.w0', 'da', '/%d' % d)
    ln('fi%s' % n, 'M', 'm', 'L.np.-.w0', 'de', '/%d' % d)
    ln('f%s' % n, 'O', 'st', 'L.np.-.w0', 'd8', c)
for n, c in [('fucom', 'e0'), ('fucomp', 'e8')]:
    ln(n, 'O', 'st', 'L.np.-.w0', 'dd', c)
for n, p, c in [('fcomi', 'db', 'f0'), ('fcomip', 'df', 'f0'), ('fcompi', 'df', 'f0'), ('fucomi', 'db', 'e8'),
                ('fucomip', 'df', 'e8'), ('fucompi', 'df', 'e8')]:
    ln(n, '-O', 'st0,st', 'L.np.-.w0', p, c)
    ln(n, 'O', 'st', 'L.np.-.w0', p, c)
for n, p, c in [('fcmovb', 'da', 'c0'), ('fcmove', 'da', 'c8'), ('fcmovbe', 'da', 'd0'), ('fcmovu', 'da', 'd8'),
                ('fcmovnb', 'db', 'c0'), ('fcmovae', 'db', 'c0'), ('fcmovne', 'db', 'c8'), ('fcmovnbe', 'db', 'd0'),
                ('fcmova', 'db', 'd0'), ('fcmovnu', 'db', 'd8'), ('fcmovnae', 'da', 'c0'), ('fcmovna', 'da', 'd0')]:
    ln(n, '-O', 'st0,st', 'L.np.-.w0', p, c)
for n, p, c in [('fld', 'd9', 'c0'), ('fst', 'dd', 'd0'), ('fstp', 'dd', 'd8'), ('fxch', 'd9', 'c8'),
                ('ffree', 'dd', 'c0'), ('ffreep', 'df', 'c0')]:
    ln(n, 'O', 'st', 'L.np.-.w0', p, c)
for n, p, d in [('flds', 'd9', 0), ('fldl', 'dd', 0), ('fldt', 'db', 5), ('filds', 'df', 0), ('fildl', 'db', 0),
                ('fild', 'df', 0), ('fildll', 'df', 5), ('fildq', 'df', 5), ('fsts', 'd9', 2), ('fstl', 'dd', 2),
                ('fstps', 'd9', 3), ('fstpl', 'dd', 3), ('fstpt', 'db', 7), ('fists', 'df', 2), ('fistl', 'db', 2),
                ('fist', 'df', 2), ('fistps', 'df', 3), ('fistpl', 'db', 3), ('fistp', 'df', 3),
                ('fistpll', 'df', 7), ('fistpq', 'df', 7), ('fisttps', 'df', 1), ('fisttpl', 'db', 1),
                ('fisttp', 'df', 1), ('fisttpll', 'dd', 1), ('fisttpq', 'dd', 1), ('fbld', 'df', 4),
                ('fbstp', 'df', 6), ('fldcw', 'd9', 5), ('fnstcw', 'd9', 7), ('fldenv', 'd9', 4),
                ('fnstenv', 'd9', 6), ('fnsave', 'dd', 6), ('frstor', 'dd', 4), ('fnstsw', 'dd', 7),
                ('fldenvl', 'd9', 4), ('fnstenvl', 'd9', 6), ('fnsavel', 'dd', 6), ('frstorl', 'dd', 4)]:
    ln(n, 'M', 'm', 'L.np.-.w0', p, '/%d' % d)
for n, p, d in [('fstcw', 'd9', 7), ('fstenv', 'd9', 6), ('fsave', 'dd', 6), ('fstsw', 'dd', 7),
                ('fstenvl', 'd9', 6), ('fsavel', 'dd', 6)]:
    ln(n, 'M', 'm', 'L.np.-.w0', '9b', p, '/%d' % d)
ln('fnstsw', '-', 'ax', 'L.np.-.w0', 'df', 'e0')
ln('fstsw', '-', 'ax', 'L.np.-.w0', '9b', 'df', 'e0')
for n, d, w in [('fxsave', 0, 'w0'), ('fxrstor', 1, 'w0'), ('fxsave64', 0, 'w1'), ('fxrstor64', 1, 'w1'),
                ('xsave', 4, 'w0'), ('xrstor', 5, 'w0'), ('xsave64', 4, 'w1'), ('xrstor64', 5, 'w1'),
                ('xsaveopt', 6, 'w0'), ('xsaveopt64', 6, 'w1')]:
    ln(n, 'M', 'm', 'L.np.0f.%s' % w, 'ae', '/%d' % d)
for n, d, w in [('xsavec', 4, 'w0'), ('xsavec64', 4, 'w1'), ('xsaves', 5, 'w0'), ('xsaves64', 5, 'w1'),
                ('xrstors', 3, 'w0'), ('xrstors64', 3, 'w1')]:
    ln(n, 'M', 'm', 'L.np.0f.%s' % w, 'c7', '/%d' % d)

# ---------------------------------------------------------------------------
# MMX (and the SSE instructions on MMX registers)
# ---------------------------------------------------------------------------
MMX = {'punpcklbw': '60', 'punpcklwd': '61', 'punpckldq': '62', 'packsswb': '63', 'pcmpgtb': '64',
       'pcmpgtw': '65', 'pcmpgtd': '66', 'packuswb': '67', 'punpckhbw': '68', 'punpckhwd': '69',
       'punpckhdq': '6a', 'packssdw': '6b', 'pcmpeqb': '74', 'pcmpeqw': '75', 'pcmpeqd': '76', 'psrlw': 'd1',
       'psrld': 'd2', 'psrlq': 'd3', 'paddq': 'd4', 'pmullw': 'd5', 'psubusb': 'd8', 'psubusw': 'd9',
       'pminub': 'da', 'pand': 'db', 'paddusb': 'dc', 'paddusw': 'dd', 'pmaxub': 'de', 'pandn': 'df',
       'pavgb': 'e0', 'psraw': 'e1', 'psrad': 'e2', 'pavgw': 'e3', 'pmulhuw': 'e4', 'pmulhw': 'e5',
       'psubsb': 'e8', 'psubsw': 'e9', 'pminsw': 'ea', 'por': 'eb', 'paddsb': 'ec', 'paddsw': 'ed',
       'pmaxsw': 'ee', 'pxor': 'ef', 'psllw': 'f1', 'pslld': 'f2', 'psllq': 'f3', 'pmuludq': 'f4',
       'pmaddwd': 'f5', 'psadbw': 'f6', 'psubb': 'f8', 'psubw': 'f9', 'psubd': 'fa', 'psubq': 'fb',
       'paddb': 'fc', 'paddw': 'fd', 'paddd': 'fe'}
for n, c in MMX.items():
    ln(n, 'RM', 'mm,mm/m', 'L.np.0f.w0', c, '/r')
MMX38 = {'pshufb': '00', 'phaddw': '01', 'phaddd': '02', 'phaddsw': '03', 'pmaddubsw': '04', 'phsubw': '05',
         'phsubd': '06', 'phsubsw': '07', 'psignb': '08', 'psignw': '09', 'psignd': '0a', 'pmulhrsw': '0b',
         'pabsb': '1c', 'pabsw': '1d', 'pabsd': '1e'}
for n, c in MMX38.items():
    ln(n, 'RM', 'mm,mm/m', 'L.np.0f38.w0', c, '/r')
ln('palignr', 'RMI', 'mm,mm/m,i', 'L.np.0f3a.w0', '0f', '/r')
for n, c, d in [('psrlw', '71', 2), ('psraw', '71', 4), ('psllw', '71', 6), ('psrld', '72', 2),
                ('psrad', '72', 4), ('pslld', '72', 6), ('psrlq', '73', 2), ('psllq', '73', 6)]:
    ln(n, 'MI', 'mm,i', 'L.np.0f.w0', c, '/%d' % d)
ln('pshufw', 'RMI', 'mm,mm/m,i', 'L.np.0f.w0', '70', '/r')
ln('pinsrw', 'RMI', 'mm,r32/m,i', 'L.np.0f.w0', 'c4', '/r')
ln('pextrw', 'RMI', 'r32/r64,mm,i', 'L.np.0f.w0', 'c5', '/r')
ln('pmovmskb', 'RM', 'r32/r64,mm', 'L.np.0f.w0', 'd7', '/r')
ln('movntq', 'MR', 'm,mm', 'L.np.0f.w0', 'e7', '/r')
ln('maskmovq', 'RM', 'mm,mm', 'L.np.0f.w0', 'f7', '/r')
ln('movd', 'RM', 'mm,r32/m', 'L.np.0f.w0', '6e', '/r')
ln('movd', 'MR', 'r32/m,mm', 'L.np.0f.w0', '7e', '/r')
ln('movq', 'RM', 'mm,mm/m', 'L.np.0f.w0', '6f', '/r')
ln('movq', 'MR', 'm,mm', 'L.np.0f.w0', '7f', '/r')
ln('movq', 'RM', 'mm,r64', 'L.np.0f.w1', '6e', '/r')
ln('movq', 'MR', 'r64,mm', 'L.np.0f.w1', '7e', '/r')
ln('movd', 'RM', 'mm,r64', 'L.np.0f.w1', '6e', '/r')
ln('movd', 'MR', 'r64,mm', 'L.np.0f.w1', '7e', '/r')
ln('cvtpi2ps', 'RM', 'x,mm/m', 'L.np.0f.w0', '2a', '/r')
ln('cvtps2pi', 'RM', 'mm,x/m', 'L.np.0f.w0', '2d', '/r')
ln('cvttps2pi', 'RM', 'mm,x/m', 'L.np.0f.w0', '2c', '/r')
ln('cvtpi2pd', 'RM', 'x,mm/m', 'L.66.0f.w0', '2a', '/r')
ln('cvtpd2pi', 'RM', 'mm,x/m', 'L.66.0f.w0', '2d', '/r')
ln('cvttpd2pi', 'RM', 'mm,x/m', 'L.66.0f.w0', '2c', '/r')
ln('movdq2q', 'RM', 'mm,x', 'L.f2.0f.w0', 'd6', '/r')
ln('movq2dq', 'RM', 'x,mm', 'L.f3.0f.w0', 'd6', '/r')
none('femms', 'L.np.0f.w0', '0e')

exec(open(__file__.rsplit("/",1)[0] + "/gen_x64_table_vec.py").read())

print('''// =============================================================================
// asm/x64tab.ax - the table of the x86-64 assembler's instruction forms
// beyond those asm/x64.ax encodes by hand: x87, MMX, SSE, AVX, AVX2, FMA,
// F16C, AVX-512 and the system instructions (asm/x64.ax, "The instruction
// table", says how a line reads). Generated from Intel's SDM vol. 2's
// encodings by cmd/internal/tools/gen_x64_table.py: do not edit.
// =============================================================================''')
print('pub let TABLE: [%d][]u8 = [' % len(L))
for x in L:
    print('    "%s",' % x)
print('];')
