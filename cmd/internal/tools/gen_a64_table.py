#!/usr/bin/env python3
"""Generates cmd/axiom/src/asm/a64tab.ax, the AArch64 assembler's table of
instruction forms beyond those asm/a64.ax encodes by hand (Advanced SIMD,
and later crypto, SVE and the rest), from the encodings below, which are
written from the Arm Architecture Reference Manual's A64 encoding tables
(the instruction classes' bit diagrams: "Advanced SIMD three same" and the
like, with each class's fixed bits and its operand fields):

    python3 cmd/internal/tools/gen_a64_table.py > cmd/axiom/src/asm/a64tab.ax && axiom fmt cmd/axiom/src/asm/a64tab.ax

A line of the table reads

    mnemonic  operands  base-word  arrangements

The operands are comma-separated, each a kind and the bit position of its
register field: `v0` a vector register with an arrangement at bit 0 (Rd),
`v5` at bit 5 (Rn), `v16` at bit 16 (Rm). The arrangements name the set
the form takes (ARRS below): each member gives the bits (size, Q, sz)
that arrangement sets in the base word.
"""

L = []
Q = 1 << 30
SZ = 1 << 22


def iset(*arrs):
    """An integer arrangement set: size at 22 from the element, Q from the length."""
    out = []
    for a in arrs:
        lanes, e = int(a[:-1]), a[-1]
        size = 'bhsd'.index(e)
        es = 8 << size
        out.append('%s:%x' % (a, (size << 22) | (Q if es * lanes == 128 else 0)))
    return ' '.join(out)


def qset(*arrs):
    """An arrangement set where only Q comes from the arrangement."""
    out = []
    for a in arrs:
        lanes, e = int(a[:-1]), a[-1]
        es = 8 << 'bhsd'.index(e)
        out.append('%s:%x' % (a, Q if es * lanes == 128 else 0))
    return ' '.join(out)


def fset(*arrs):
    """A floating-point set: sz at 22 (0 single, 1 double), Q from the length."""
    out = []
    for a in arrs:
        lanes, e = int(a[:-1]), a[-1]
        es = 8 << 'bhsd'.index(e)
        out.append('%s:%x' % (a, (SZ if e == 'd' else 0) | (Q if es * lanes == 128 else 0)))
    return ' '.join(out)


ARRS = {
    'I7': iset('8b', '16b', '4h', '8h', '2s', '4s', '2d'),
    'I6': iset('8b', '16b', '4h', '8h', '2s', '4s'),
    'H4': iset('4h', '8h', '2s', '4s'),
    'B2': iset('8b', '16b'),
    'D3': iset('8b', '4h', '2s'),
    'Q3': iset('16b', '8h', '4s'),
    'R4': iset('8b', '16b', '4h', '8h'),
    'A5': iset('8b', '16b', '4h', '8h', '4s'),
    # shifts by immediate: the element size is in immh, only Q here
    'S7': qset('8b', '16b', '4h', '8h', '2s', '4s', '2d'),
    'SD3': qset('8b', '4h', '2s'),
    'SQ3': qset('16b', '8h', '4s'),
    'SF': qset('4h', '8h', '2s', '4s', '2d'),
    # floating point: single and double (sz), half precision (FEAT_FP16, its own encodings)
    'F3': fset('2s', '4s', '2d'),
    'FH': qset('4h', '8h'),
    'FS2': fset('2s', '4s'),
    # fcvtn/fcvtl (the narrower arrangement): sz 0 half from single, 1 single from double
    'FN': '4h:0 2s:%x' % SZ,
    'FN2': '8h:%x 4s:%x' % (Q, Q | SZ),
    'FX': '2s:%x' % SZ,
    'FX2': '4s:%x' % (Q | SZ),
}


def ln(name, ops, base, arr):
    L.append('%s %s %08x %s' % (name, ops, base, arr))


# Advanced SIMD three same: 0 Q U 01110 size 1 Rm opcode 1 Rn Rd
# (name, U, opcode, arrangement set)
#   I7: 8b 16b 4h 8h 2s 4s 2d     I6: 8b 16b 4h 8h 2s 4s
#   H4: 4h 8h 2s 4s               B2: 8b 16b
THREE_SAME = [
    ('shadd', 0, 0x00, 'I6'), ('sqadd', 0, 0x01, 'I7'), ('srhadd', 0, 0x02, 'I6'),
    ('shsub', 0, 0x04, 'I6'), ('sqsub', 0, 0x05, 'I7'), ('cmgt', 0, 0x06, 'I7'),
    ('cmge', 0, 0x07, 'I7'), ('sshl', 0, 0x08, 'I7'), ('sqshl', 0, 0x09, 'I7'),
    ('srshl', 0, 0x0a, 'I7'), ('sqrshl', 0, 0x0b, 'I7'), ('smax', 0, 0x0c, 'I6'),
    ('smin', 0, 0x0d, 'I6'), ('sabd', 0, 0x0e, 'I6'), ('saba', 0, 0x0f, 'I6'),
    ('add', 0, 0x10, 'I7'), ('cmtst', 0, 0x11, 'I7'), ('mla', 0, 0x12, 'I6'),
    ('mul', 0, 0x13, 'I6'), ('smaxp', 0, 0x14, 'I6'), ('sminp', 0, 0x15, 'I6'),
    ('sqdmulh', 0, 0x16, 'H4'), ('addp', 0, 0x17, 'I7'),
    ('uhadd', 1, 0x00, 'I6'), ('uqadd', 1, 0x01, 'I7'), ('urhadd', 1, 0x02, 'I6'),
    ('uhsub', 1, 0x04, 'I6'), ('uqsub', 1, 0x05, 'I7'), ('cmhi', 1, 0x06, 'I7'),
    ('cmhs', 1, 0x07, 'I7'), ('ushl', 1, 0x08, 'I7'), ('uqshl', 1, 0x09, 'I7'),
    ('urshl', 1, 0x0a, 'I7'), ('uqrshl', 1, 0x0b, 'I7'), ('umax', 1, 0x0c, 'I6'),
    ('umin', 1, 0x0d, 'I6'), ('uabd', 1, 0x0e, 'I6'), ('uaba', 1, 0x0f, 'I6'),
    ('sub', 1, 0x10, 'I7'), ('cmeq', 1, 0x11, 'I7'), ('mls', 1, 0x12, 'I6'),
    ('pmul', 1, 0x13, 'B2'), ('umaxp', 1, 0x14, 'I6'), ('uminp', 1, 0x15, 'I6'),
    ('sqrdmulh', 1, 0x16, 'H4'),
]
for name, u, opc, arr in THREE_SAME:
    ln(name, 'v0,v5,v16', 0x0e200400 | (u << 29) | (opc << 11), arr)

# the logical ones: size is part of the opcode, arrangement 8b or 16b only
LOGIC = [('and', 0, 0), ('bic', 0, 1), ('orr', 0, 2), ('orn', 0, 3),
         ('eor', 1, 0), ('bsl', 1, 1), ('bit', 1, 2), ('bif', 1, 3)]
for name, u, size in LOGIC:
    ln(name, 'v0,v5,v16', 0x0e201c00 | (u << 29) | (size << 22), 'B2')

# Advanced SIMD three different: 0 Q U 01110 size 1 Rm opcode 00 Rn Rd. The
# long forms (Vd.Ta, Vn.Tb, Vm.Tb), the wide (Vd.Ta, Vn.Ta, Vm.Tb) and the
# narrowing (Vd.Tb, Vn.Ta, Vm.Ta); the "2" forms (Q 1) take the upper half.
# (name, U, opcode, shape); T is Tb, of b h or s elements (D3 or Q3)
THREE_DIFF = [
    ('saddl', 0, 0x0, 'L'), ('saddw', 0, 0x1, 'W'), ('ssubl', 0, 0x2, 'L'), ('ssubw', 0, 0x3, 'W'),
    ('addhn', 0, 0x4, 'N'), ('sabal', 0, 0x5, 'L'), ('subhn', 0, 0x6, 'N'), ('sabdl', 0, 0x7, 'L'),
    ('smlal', 0, 0x8, 'L'), ('smlsl', 0, 0xa, 'L'), ('smull', 0, 0xc, 'L'),
    ('uaddl', 1, 0x0, 'L'), ('uaddw', 1, 0x1, 'W'), ('usubl', 1, 0x2, 'L'), ('usubw', 1, 0x3, 'W'),
    ('raddhn', 1, 0x4, 'N'), ('uabal', 1, 0x5, 'L'), ('rsubhn', 1, 0x6, 'N'), ('uabdl', 1, 0x7, 'L'),
    ('umlal', 1, 0x8, 'L'), ('umlsl', 1, 0xa, 'L'), ('umull', 1, 0xc, 'L'),
]
SHAPES = {'L': 'w0,v5,v16', 'W': 'w0,w5,v16', 'N': 'v0,w5,w16'}
for name, u, opc, shape in THREE_DIFF:
    base = 0x0e200000 | (u << 29) | (opc << 12)
    ln(name, SHAPES[shape], base, 'D3')
    ln(name + '2', SHAPES[shape], base, 'Q3')

# Advanced SIMD two-register miscellaneous: 0 Q U 01110 size 10000 opcode 10 Rn Rd
# (name, U, opcode, operands, arrangements); `p`: the pairwise long result,
# `z`: the #0 of the compares with zero, `k`: shll's shift (the element size)
TWO_MISC = [
    ('rev64', 0, 0x00, 'v0,v5', 'I6'), ('rev16', 0, 0x01, 'v0,v5', 'B2'),
    ('saddlp', 0, 0x02, 'p0,v5', 'I6'), ('suqadd', 0, 0x03, 'v0,v5', 'I7'),
    ('cls', 0, 0x04, 'v0,v5', 'I6'), ('cnt', 0, 0x05, 'v0,v5', 'B2'),
    ('sadalp', 0, 0x06, 'p0,v5', 'I6'), ('sqabs', 0, 0x07, 'v0,v5', 'I7'),
    ('cmgt', 0, 0x08, 'v0,v5,z', 'I7'), ('cmeq', 0, 0x09, 'v0,v5,z', 'I7'),
    ('cmlt', 0, 0x0a, 'v0,v5,z', 'I7'), ('abs', 0, 0x0b, 'v0,v5', 'I7'),
    ('rev32', 1, 0x00, 'v0,v5', 'R4'), ('uaddlp', 1, 0x02, 'p0,v5', 'I6'),
    ('usqadd', 1, 0x03, 'v0,v5', 'I7'), ('clz', 1, 0x04, 'v0,v5', 'I6'),
    ('uadalp', 1, 0x06, 'p0,v5', 'I6'), ('sqneg', 1, 0x07, 'v0,v5', 'I7'),
    ('cmge', 1, 0x08, 'v0,v5,z', 'I7'), ('cmle', 1, 0x09, 'v0,v5,z', 'I7'),
    ('neg', 1, 0x0b, 'v0,v5', 'I7'),
]
for name, u, opc, ops, arr in TWO_MISC:
    ln(name, ops, 0x0e200800 | (u << 29) | (opc << 12), arr)
# not (mvn) and rbit: size is part of the opcode
for name, size in (('not', 0), ('mvn', 0), ('rbit', 1)):
    ln(name, 'v0,v5', 0x2e205800 | (size << 22), 'B2')
# the narrowing ones (Vd.Tb, Vn.Ta) and shll (Vd.Ta, Vn.Tb, #esize)
for name, u, opc in (('xtn', 0, 0x12), ('sqxtn', 0, 0x14), ('sqxtun', 1, 0x12), ('uqxtn', 1, 0x14)):
    base = 0x0e200800 | (u << 29) | (opc << 12)
    ln(name, 'v0,w5', base, 'D3')
    ln(name + '2', 'v0,w5', base, 'Q3')
ln('shll', 'w0,v5,k', 0x2e213800, 'D3')
ln('shll2', 'w0,v5,k', 0x2e213800, 'Q3')

# Advanced SIMD across lanes: 0 Q U 01110 size 11000 opcode 10 Rn Rd; the
# result is a scalar of the element's size (`e`), or twice it (`f`)
ACROSS = [('saddlv', 0, 0x03, 'f0,v5'), ('smaxv', 0, 0x0a, 'e0,v5'), ('sminv', 0, 0x1a, 'e0,v5'),
          ('addv', 0, 0x1b, 'e0,v5'), ('uaddlv', 1, 0x03, 'f0,v5'), ('umaxv', 1, 0x0a, 'e0,v5'),
          ('uminv', 1, 0x1a, 'e0,v5')]
for name, u, opc, ops in ACROSS:
    ln(name, ops, 0x0e300800 | (u << 29) | (opc << 12), 'A5')

# Advanced SIMD shift by immediate: 0 Q U 011110 immh immb opcode 1 Rn Rd;
# immh:immb gives the element size and the shift (`l` left, `r` right)
SHIFTS = [
    ('sshr', 0, 0x00, 'r'), ('ssra', 0, 0x02, 'r'), ('srshr', 0, 0x04, 'r'), ('srsra', 0, 0x06, 'r'),
    ('shl', 0, 0x0a, 'l'), ('sqshl', 0, 0x0e, 'l'),
    ('ushr', 1, 0x00, 'r'), ('usra', 1, 0x02, 'r'), ('urshr', 1, 0x04, 'r'), ('ursra', 1, 0x06, 'r'),
    ('sri', 1, 0x08, 'r'), ('sli', 1, 0x0a, 'l'), ('sqshlu', 1, 0x0c, 'l'), ('uqshl', 1, 0x0e, 'l'),
]
for name, u, opc, k in SHIFTS:
    ln(name, 'v0,v5,' + k, 0x0f000400 | (u << 29) | (opc << 11), 'S7')
# the narrowing shifts (Vd.Tb, Vn.Ta, #1..esize) and the long ones (Vd.Ta, Vn.Tb, #0..esize-1)
NARROW_SH = [('shrn', 0, 0x10), ('rshrn', 0, 0x11), ('sqshrn', 0, 0x12), ('sqrshrn', 0, 0x13),
             ('sqshrun', 1, 0x10), ('sqrshrun', 1, 0x11), ('uqshrn', 1, 0x12), ('uqrshrn', 1, 0x13)]
for name, u, opc in NARROW_SH:
    base = 0x0f000400 | (u << 29) | (opc << 11)
    ln(name, 'v0,w5,r', base, 'SD3')
    ln(name + '2', 'v0,w5,r', base, 'SQ3')
for name, u in (('sshll', 0), ('ushll', 1)):
    base = 0x0f000400 | (u << 29) | (0x14 << 11)
    ln(name, 'w0,v5,l', base, 'SD3')
    ln(name + '2', 'w0,v5,l', base, 'SQ3')
# sxtl/uxtl: sshll/ushll #0 (immh is the element size: immb 0)
for name, u in (('sxtl', 0), ('uxtl', 1)):
    for es, a in ((8, 'b'), (16, 'h'), (32, 's')):
        base = 0x0f000400 | (u << 29) | (0x14 << 11) | (es << 16)
        ln(name, 'w0,v5', base, 'X' + a)
        ln(name + '2', 'w0,v5', base, 'X2' + a)
ARRS.update({'Xb': '8b:0', 'Xh': '4h:0', 'Xs': '2s:0',
             'X2b': '16b:%x' % Q, 'X2h': '8h:%x' % Q, 'X2s': '4s:%x' % Q})
# the fixed-point conversions (#fbits 1..esize, as a right shift)
for name, u, opc in (('scvtf', 0, 0x1c), ('fcvtzs', 0, 0x1f), ('ucvtf', 1, 0x1c), ('fcvtzu', 1, 0x1f)):
    ln(name, 'v0,v5,r', 0x0f000400 | (u << 29) | (opc << 11), 'SF')

# Advanced SIMD three same (floating point): 0 Q U 01110 a sz 1 Rm opcode 1 Rn Rd,
# and its half-precision form 0 Q U 01110 a 10 Rm 00 opcode<2:0> 1 Rn Rd
FP3 = [
    ('fmaxnm', 0, 0, 0x18), ('fmla', 0, 0, 0x19), ('fadd', 0, 0, 0x1a), ('fmulx', 0, 0, 0x1b),
    ('fcmeq', 0, 0, 0x1c), ('fmax', 0, 0, 0x1e), ('frecps', 0, 0, 0x1f),
    ('fminnm', 0, 1, 0x18), ('fmls', 0, 1, 0x19), ('fsub', 0, 1, 0x1a), ('fmin', 0, 1, 0x1e),
    ('frsqrts', 0, 1, 0x1f),
    ('fmaxnmp', 1, 0, 0x18), ('faddp', 1, 0, 0x1a), ('fmul', 1, 0, 0x1b), ('fcmge', 1, 0, 0x1c),
    ('facge', 1, 0, 0x1d), ('fmaxp', 1, 0, 0x1e), ('fdiv', 1, 0, 0x1f),
    ('fminnmp', 1, 1, 0x18), ('fabd', 1, 1, 0x1a), ('fcmgt', 1, 1, 0x1c), ('facgt', 1, 1, 0x1d),
    ('fminp', 1, 1, 0x1e),
]
for name, u, a, opc in FP3:
    ln(name, 'v0,v5,v16', 0x0e200400 | (u << 29) | (a << 23) | (opc << 11), 'F3')
    ln(name, 'v0,v5,v16', 0x0e400400 | (u << 29) | (a << 23) | ((opc & 7) << 11), 'FH')

# Advanced SIMD two-register miscellaneous (floating point): 0 Q U 01110 a sz
# 10000 opcode 10 Rn Rd; half precision 0 Q U 01110 a 1111000 opcode 10 Rn Rd
# (name, U, a, opcode, operands, half precision too)
FP2 = [
    ('frintn', 0, 0, 0x18, 'v0,v5', 1), ('frintm', 0, 0, 0x19, 'v0,v5', 1),
    ('fcvtns', 0, 0, 0x1a, 'v0,v5', 1), ('fcvtms', 0, 0, 0x1b, 'v0,v5', 1),
    ('fcvtas', 0, 0, 0x1c, 'v0,v5', 1), ('scvtf', 0, 0, 0x1d, 'v0,v5', 1),
    ('frint32z', 0, 0, 0x1e, 'v0,v5', 0), ('frint64z', 0, 0, 0x1f, 'v0,v5', 0),
    ('fcmgt', 0, 1, 0x0c, 'v0,v5,y', 1), ('fcmeq', 0, 1, 0x0d, 'v0,v5,y', 1),
    ('fcmlt', 0, 1, 0x0e, 'v0,v5,y', 1), ('fabs', 0, 1, 0x0f, 'v0,v5', 1),
    ('frintp', 0, 1, 0x18, 'v0,v5', 1), ('frintz', 0, 1, 0x19, 'v0,v5', 1),
    ('fcvtps', 0, 1, 0x1a, 'v0,v5', 1), ('fcvtzs', 0, 1, 0x1b, 'v0,v5', 1),
    ('frecpe', 0, 1, 0x1d, 'v0,v5', 1),
    ('frinta', 1, 0, 0x18, 'v0,v5', 1), ('frintx', 1, 0, 0x19, 'v0,v5', 1),
    ('fcvtnu', 1, 0, 0x1a, 'v0,v5', 1), ('fcvtmu', 1, 0, 0x1b, 'v0,v5', 1),
    ('fcvtau', 1, 0, 0x1c, 'v0,v5', 1), ('ucvtf', 1, 0, 0x1d, 'v0,v5', 1),
    ('frint32x', 1, 0, 0x1e, 'v0,v5', 0), ('frint64x', 1, 0, 0x1f, 'v0,v5', 0),
    ('fcmge', 1, 1, 0x0c, 'v0,v5,y', 1), ('fcmle', 1, 1, 0x0d, 'v0,v5,y', 1),
    ('fneg', 1, 1, 0x0f, 'v0,v5', 1), ('frinti', 1, 1, 0x19, 'v0,v5', 1),
    ('fcvtpu', 1, 1, 0x1a, 'v0,v5', 1), ('fcvtzu', 1, 1, 0x1b, 'v0,v5', 1),
    ('frsqrte', 1, 1, 0x1d, 'v0,v5', 1), ('fsqrt', 1, 1, 0x1f, 'v0,v5', 1),
]
for name, u, a, opc, ops, half in FP2:
    ln(name, ops, 0x0e200800 | (u << 29) | (a << 23) | (opc << 12), 'F3')
    if half:
        ln(name, ops, 0x0e780800 | (u << 29) | (a << 23) | (opc << 12), 'FH')
# the integer estimates (2s and 4s only): urecpe, ursqrte
ln('urecpe', 'v0,v5', 0x0ea1c800, 'FS2')
ln('ursqrte', 'v0,v5', 0x2ea1c800, 'FS2')
# fcvtn (Vd.Tb, Vn.Ta), fcvtl (Vd.Ta, Vn.Tb), fcvtxn (2s from 2d only)
ln('fcvtn', 'v0,w5', 0x0e216800, 'FN')
ln('fcvtn2', 'v0,w5', 0x0e216800, 'FN2')
ln('fcvtl', 'w0,v5', 0x0e217800, 'FN')
ln('fcvtl2', 'w0,v5', 0x0e217800, 'FN2')
ln('fcvtxn', 'v0,w5', 0x2e216800, 'FX')
ln('fcvtxn2', 'v0,w5', 0x2e216800, 'FX2')

# Advanced SIMD permute: 0 Q 001110 size 0 Rm 0 opcode 10 Rn Rd
for name, opc in (('uzp1', 1), ('trn1', 2), ('zip1', 3), ('uzp2', 5), ('trn2', 6), ('zip2', 7)):
    ln(name, 'v0,v5,v16', 0x0e000800 | (opc << 12), 'I7')
# Advanced SIMD table lookup: 0 Q 001110 000 Rm 0 len op 00 Rn Rd; `tN`: a list
# of N registers .16b at Rn (len = N - 1), op 1 for tbx
for name, op in (('tbl', 0), ('tbx', 1)):
    for n in range(1, 5):
        ln(name, 'v0,t%d5,v16' % n, 0x0e000000 | ((n - 1) << 13) | (op << 12), 'B2')

# Advanced SIMD vector x indexed element: 0 Q U 01111 size L M Rm opcode H 0 Rn
# Rd; `x16` the element Vm.Ts[index] (index in H:L:M as the element size says)
BYELEM = [('mul', 0, 0x8), ('sqdmulh', 0, 0xc), ('sqrdmulh', 0, 0xd), ('mla', 1, 0x0),
          ('mls', 1, 0x4), ('sqrdmlah', 1, 0xd), ('sqrdmlsh', 1, 0xf)]
for name, u, opc in BYELEM:
    ln(name, 'v0,v5,x16', 0x0f000000 | (u << 29) | (opc << 12), 'H4')
BYELEM_L = [('smlal', 0, 0x2), ('sqdmlal', 0, 0x3), ('smlsl', 0, 0x6), ('sqdmlsl', 0, 0x7),
            ('smull', 0, 0xa), ('sqdmull', 0, 0xb), ('umlal', 1, 0x2), ('umlsl', 1, 0x6),
            ('umull', 1, 0xa)]
ARRS.update({'EL': iset('4h', '2s'), 'EL2': iset('8h', '4s')})
for name, u, opc in BYELEM_L:
    base = 0x0f000000 | (u << 29) | (opc << 12)
    ln(name, 'w0,v5,x16', base, 'EL')
    ln(name + '2', 'w0,v5,x16', base, 'EL2')
# the floating-point ones: size 1x (sz at 22) for single and double, 00 for half
ARRS.update({'EF': '2s:%x 4s:%x 2d:%x' % (1 << 23, Q | (1 << 23), Q | (3 << 22)), 'EH': qset('4h', '8h')})
for name, u, opc in (('fmla', 0, 0x1), ('fmls', 0, 0x5), ('fmul', 0, 0x9), ('fmulx', 1, 0x9)):
    base = 0x0f000000 | (u << 29) | (opc << 12)
    ln(name, 'v0,v5,x16', base, 'EF')
    ln(name, 'v0,v5,x16', base, 'EH')

# Advanced SIMD three same extra (0 Q U 01110 size 0 Rm 1 opcode 1 Rn Rd) and
# the dot products, matrix multiplies and complex arithmetic around it.
# `q`: Vd of T's elements four times as wide; `u`: twice as wide, as many lanes.
ln('sqrdmlah', 'v0,v5,v16', 0x2e008400, 'H4')
ln('sqrdmlsh', 'v0,v5,v16', 0x2e008c00, 'H4')
ARRS.update({'DOT': qset('8b', '16b'), 'M16': '16b:0', 'M8H': '8h:0', 'HH': '2h:0 4h:%x' % Q,
             'CX': iset('4h', '8h', '2s', '4s', '2d'), 'CA': iset('4h', '8h', '2s', '4s', '2d'),
             'C4': iset('4h', '8h', '4s'), 'F4S': '4s:0'})
for name, base in (('sdot', 0x0e809400), ('udot', 0x2e809400), ('usdot', 0x0e809c00)):
    ln(name, 'q0,v5,v16', base, 'DOT')
for name, base in (('smmla', 0x4e80a400), ('ummla', 0x6e80a400), ('usmmla', 0x4e80ac00)):
    ln(name, 'q0,v5,v16', base, 'M16')
# fcmla Vd.T, Vn.T, Vm.T, #rot (rot at 11); fcadd ..., #90/#270 (rot at 12)
ln('fcmla', 'v0,v5,v16,o11', 0x2e00c400, 'CX')
ln('fcadd', 'v0,v5,v16,a12', 0x2e00e400, 'CA')
# BF16: bfdot (Vd.2s/4s, Vn.4h/8h), bfmmla, bfmlalb/t, bfcvtn
ln('bfdot', 'p0,v5,v16', 0x2e40fc00, 'FH')
ln('bfmmla', 'p0,v5,v16', 0x6e40ec00, 'M8H')
ln('bfmlalb', 'p0,v5,v16', 0x2ec0fc00, 'M8H')
ln('bfmlalt', 'p0,v5,v16', 0x6ec0fc00, 'M8H')
ln('bfcvtn', 'v0,w5', 0x0ea16800, 'FN0')
ln('bfcvtn2', 'v0,w5', 0x4ea16800, 'FN20')
ARRS.update({'FN0': '4h:0', 'FN20': '8h:0'})
# FEAT_FHM: fmlal Vd.2s/4s, Vn.2h/4h, Vm.2h/4h (and the elements' high halves: 2)
for name, base in (('fmlal', 0x0e20ec00), ('fmlal2', 0x2e20cc00), ('fmlsl', 0x0ea0ec00),
                   ('fmlsl2', 0x2ea0cc00)):
    ln(name, 'u0,v5,v16', base, 'HH')
# by element: the dot products' groups (`g` .4b[i], `G` .2h[i]), fcmla (`c`),
# fmlal and bfmlal (`x`: Vm.h[i])
for name, base in (('sdot', 0x0f80e000), ('udot', 0x2f80e000), ('usdot', 0x0f80f000),
                   ('sudot', 0x0f00f000)):
    ln(name, 'q0,v5,g16', base, 'DOT')
ln('bfdot', 'p0,v5,G16', 0x0f40f000, 'FH')
ln('fcmla', 'v0,v5,c16,o13', 0x2f001000, 'C4')
for name, base in (('fmlal', 0x0f800000), ('fmlal2', 0x2f808000), ('fmlsl', 0x0f804000),
                   ('fmlsl2', 0x2f80c000)):
    ln(name, 'u0,v5,x16', base, 'HH')
ln('bfmlalb', 'p0,v5,x16', 0x0fc0f000, 'M8H')
ln('bfmlalt', 'p0,v5,x16', 0x4fc0f000, 'M8H')

# floating-point across lanes: 4s (single) and 4h/8h (FEAT_FP16); the result a scalar
for name, base, hbase in (('fmaxnmv', 0x6e30c800, 0x0e30c800), ('fmaxv', 0x6e30f800, 0x0e30f800),
                          ('fminnmv', 0x6eb0c800, 0x0eb0c800), ('fminv', 0x6eb0f800, 0x0eb0f800)):
    ln(name, 'e0,v5', base, 'F4S')
    ln(name, 'e0,v5', hbase, 'FH')
# ext Vd.T, Vn.T, Vm.T, #index (bytes; imm4 at 11)
ln('ext', 'v0,v5,v16,j11', 0x2e000000, 'B2')

# ---------------------------------------------------------------------------
# Advanced SIMD scalar: `S` a scalar register (b h s d) whose element is T;
# the sets' members are element letters with the bits they set
# ---------------------------------------------------------------------------
def sset(*els, sz=False):
    out = []
    for e in els:
        k = 'bhsd'.index(e)
        if sz:
            out.append('%s:%x' % (e, SZ if e == 'd' else 0))
        else:
            out.append('%s:%x' % (e, k << 22))
    return ' '.join(out)


ARRS.update({'SBHSD': sset('b', 'h', 's', 'd'), 'SD': sset('d'), 'SHS': sset('h', 's'),
             'SBHS': sset('b', 'h', 's'), 'SFSD': sset('s', 'd', sz=True), 'SFH': 'h:0',
             # shifts by immediate: immh has the size
             'SID': 'd:0', 'SIBHSD': 'b:0 h:0 s:0 d:0', 'SIBHS': 'b:0 h:0 s:0', 'SIHSD': 'h:0 s:0 d:0',
             # by element: size 01 h, 10 s, 11 d; the floating ones size 00 h
             'SEHS': sset('h', 's'), 'SEF': 's:%x d:%x' % (2 << 22, 3 << 22), 'SEFH': 'h:0'})
# scalar three same: 01 U 11110 size 1 Rm opcode 1 Rn Rd
SC3 = [('sqadd', 0, 0x01, 'SBHSD'), ('sqsub', 0, 0x05, 'SBHSD'), ('cmgt', 0, 0x06, 'SD'),
       ('cmge', 0, 0x07, 'SD'), ('sshl', 0, 0x08, 'SD'), ('sqshl', 0, 0x09, 'SBHSD'),
       ('srshl', 0, 0x0a, 'SD'), ('sqrshl', 0, 0x0b, 'SBHSD'), ('add', 0, 0x10, 'SD'),
       ('cmtst', 0, 0x11, 'SD'), ('sqdmulh', 0, 0x16, 'SHS'),
       ('uqadd', 1, 0x01, 'SBHSD'), ('uqsub', 1, 0x05, 'SBHSD'), ('cmhi', 1, 0x06, 'SD'),
       ('cmhs', 1, 0x07, 'SD'), ('ushl', 1, 0x08, 'SD'), ('uqshl', 1, 0x09, 'SBHSD'),
       ('urshl', 1, 0x0a, 'SD'), ('uqrshl', 1, 0x0b, 'SBHSD'), ('sub', 1, 0x10, 'SD'),
       ('cmeq', 1, 0x11, 'SD'), ('sqrdmulh', 1, 0x16, 'SHS')]
for name, u, opc, arr in SC3:
    ln(name, 'S0,S5,S16', 0x5e200400 | (u << 29) | (opc << 11), arr)
ln('sqrdmlah', 'S0,S5,S16', 0x7e008400, 'SHS')
ln('sqrdmlsh', 'S0,S5,S16', 0x7e008c00, 'SHS')
# the floating-point ones (sz; FEAT_FP16's own encoding for h)
SCF3 = [('fmulx', 0, 0, 0x1b), ('fcmeq', 0, 0, 0x1c), ('frecps', 0, 0, 0x1f), ('frsqrts', 0, 1, 0x1f),
        ('fcmge', 1, 0, 0x1c), ('facge', 1, 0, 0x1d), ('fabd', 1, 1, 0x1a), ('fcmgt', 1, 1, 0x1c),
        ('facgt', 1, 1, 0x1d)]
for name, u, a, opc in SCF3:
    ln(name, 'S0,S5,S16', 0x5e200400 | (u << 29) | (a << 23) | (opc << 11), 'SFSD')
    ln(name, 'S0,S5,S16', 0x5e400400 | (u << 29) | (a << 23) | ((opc & 7) << 11), 'SFH')
# scalar three different (Vd twice T's element: `f`)
for name, opc in (('sqdmlal', 0x9), ('sqdmlsl', 0xb), ('sqdmull', 0xd)):
    ln(name, 'f0,S5,S16', 0x5e200000 | (opc << 12), 'SHS')
# scalar two-register miscellaneous: 01 U 11110 size 10000 opcode 10 Rn Rd
SC2 = [('suqadd', 0, 0x03, 'S0,S5', 'SBHSD'), ('sqabs', 0, 0x07, 'S0,S5', 'SBHSD'),
       ('cmgt', 0, 0x08, 'S0,S5,z', 'SD'), ('cmeq', 0, 0x09, 'S0,S5,z', 'SD'),
       ('cmlt', 0, 0x0a, 'S0,S5,z', 'SD'), ('abs', 0, 0x0b, 'S0,S5', 'SD'),
       ('sqxtn', 0, 0x14, 'S0,f5', 'SBHS'),
       ('usqadd', 1, 0x03, 'S0,S5', 'SBHSD'), ('sqneg', 1, 0x07, 'S0,S5', 'SBHSD'),
       ('cmge', 1, 0x08, 'S0,S5,z', 'SD'), ('cmle', 1, 0x09, 'S0,S5,z', 'SD'),
       ('neg', 1, 0x0b, 'S0,S5', 'SD'), ('sqxtun', 1, 0x12, 'S0,f5', 'SBHS'),
       ('uqxtn', 1, 0x14, 'S0,f5', 'SBHS')]
for name, u, opc, ops, arr in SC2:
    ln(name, ops, 0x5e200800 | (u << 29) | (opc << 12), arr)
SCF2 = [('fcvtns', 0, 0, 0x1a, 'S0,S5'), ('fcvtms', 0, 0, 0x1b, 'S0,S5'), ('fcvtas', 0, 0, 0x1c, 'S0,S5'),
        ('scvtf', 0, 0, 0x1d, 'S0,S5'), ('fcmgt', 0, 1, 0x0c, 'S0,S5,y'), ('fcmeq', 0, 1, 0x0d, 'S0,S5,y'),
        ('fcmlt', 0, 1, 0x0e, 'S0,S5,y'), ('fcvtps', 0, 1, 0x1a, 'S0,S5'), ('fcvtzs', 0, 1, 0x1b, 'S0,S5'),
        ('frecpe', 0, 1, 0x1d, 'S0,S5'), ('frecpx', 0, 1, 0x1f, 'S0,S5'),
        ('fcvtnu', 1, 0, 0x1a, 'S0,S5'), ('fcvtmu', 1, 0, 0x1b, 'S0,S5'), ('fcvtau', 1, 0, 0x1c, 'S0,S5'),
        ('ucvtf', 1, 0, 0x1d, 'S0,S5'), ('fcmge', 1, 1, 0x0c, 'S0,S5,y'), ('fcmle', 1, 1, 0x0d, 'S0,S5,y'),
        ('fcvtpu', 1, 1, 0x1a, 'S0,S5'), ('fcvtzu', 1, 1, 0x1b, 'S0,S5'), ('frsqrte', 1, 1, 0x1d, 'S0,S5')]
for name, u, a, opc, ops in SCF2:
    ln(name, ops, 0x5e200800 | (u << 29) | (a << 23) | (opc << 12), 'SFSD')
    ln(name, ops, 0x5e780800 | (u << 29) | (a << 23) | (opc << 12), 'SFH')
ln('fcvtxn', 'S0,f5', 0x7e616800, 'SFS0')
ARRS.update({'SFS0': 's:0'})
# scalar pairwise: 01 U 11110 size 11000 opcode 10 Rn Rd (`e`: T's element)
ARRS.update({'P2D': '2d:0', 'PF': '2s:0 2d:%x' % SZ, 'PH': '2h:0'})
ln('addp', 'e0,v5', 0x5ef1b800, 'P2D')
for name, base, hbase in (('faddp', 0x7e30d800, 0x5e30d800), ('fmaxnmp', 0x7e30c800, 0x5e30c800),
                          ('fmaxp', 0x7e30f800, 0x5e30f800), ('fminnmp', 0x7eb0c800, 0x5eb0c800),
                          ('fminp', 0x7eb0f800, 0x5eb0f800)):
    ln(name, 'e0,v5', base, 'PF')
    ln(name, 'e0,v5', hbase, 'PH')
# scalar shift by immediate: 01 U 111110 immh immb opcode 1 Rn Rd
SCSH = [('sshr', 0, 0x00, 'r', 'SID'), ('ssra', 0, 0x02, 'r', 'SID'), ('srshr', 0, 0x04, 'r', 'SID'),
        ('srsra', 0, 0x06, 'r', 'SID'), ('shl', 0, 0x0a, 'l', 'SID'), ('sqshl', 0, 0x0e, 'l', 'SIBHSD'),
        ('scvtf', 0, 0x1c, 'r', 'SIHSD'), ('fcvtzs', 0, 0x1f, 'r', 'SIHSD'),
        ('ushr', 1, 0x00, 'r', 'SID'), ('usra', 1, 0x02, 'r', 'SID'), ('urshr', 1, 0x04, 'r', 'SID'),
        ('ursra', 1, 0x06, 'r', 'SID'), ('sri', 1, 0x08, 'r', 'SID'), ('sli', 1, 0x0a, 'l', 'SID'),
        ('sqshlu', 1, 0x0c, 'l', 'SIBHSD'), ('uqshl', 1, 0x0e, 'l', 'SIBHSD'),
        ('ucvtf', 1, 0x1c, 'r', 'SIHSD'), ('fcvtzu', 1, 0x1f, 'r', 'SIHSD')]
for name, u, opc, k, arr in SCSH:
    ln(name, 'S0,S5,' + k, 0x5f000400 | (u << 29) | (opc << 11), arr)
for name, u, opc in (('sqshrn', 0, 0x12), ('sqrshrn', 0, 0x13), ('sqshrun', 1, 0x10),
                     ('sqrshrun', 1, 0x11), ('uqshrn', 1, 0x12), ('uqrshrn', 1, 0x13)):
    ln(name, 'S0,f5,r', 0x5f000400 | (u << 29) | (opc << 11), 'SIBHS')
# scalar x indexed element: 01 U 11111 size L M Rm opcode H 0 Rn Rd
for name, u, opc in (('sqdmulh', 0, 0xc), ('sqrdmulh', 0, 0xd), ('sqrdmlah', 1, 0xd), ('sqrdmlsh', 1, 0xf)):
    ln(name, 'S0,S5,x16', 0x5f000000 | (u << 29) | (opc << 12), 'SEHS')
for name, opc in (('sqdmlal', 0x3), ('sqdmlsl', 0x7), ('sqdmull', 0xb)):
    ln(name, 'f0,S5,x16', 0x5f000000 | (opc << 12), 'SEHS')
for name, u, opc in (('fmla', 0, 0x1), ('fmls', 0, 0x5), ('fmul', 0, 0x9), ('fmulx', 1, 0x9)):
    ln(name, 'S0,S5,x16', 0x5f000000 | (u << 29) | (opc << 12), 'SEF')
    ln(name, 'S0,S5,x16', 0x5f000000 | (u << 29) | (opc << 12), 'SEFH')

# the three-different ones of h and s only (saturating doubling) and pmull
# (8h from 8b; 1q from 1d with FEAT_PMULL)
for name, opc in (('sqdmlal', 0x9), ('sqdmlsl', 0xb), ('sqdmull', 0xd)):
    base = 0x0e200000 | (opc << 12)
    ln(name, 'w0,v5,v16', base, 'EL')
    ln(name + '2', 'w0,v5,v16', base, 'EL2')
ARRS.update({'PM': '8b:0 1d:%x' % (3 << 22), 'PM2': '16b:%x 2d:%x' % (Q, Q | (3 << 22))})
ln('pmull', 'w0,v5,v16', 0x0e20e000, 'PM')
ln('pmull2', 'w0,v5,v16', 0x0e20e000, 'PM2')
# bfcvt Hd, Sn (`e` would be T's element: the destination is half of it, `h`)
ln('bfcvt', 'h0,S5', 0x1e634000, 'SFS0')

# ---------------------------------------------------------------------------
# Cryptographic extensions: AES, SHA1, SHA-256, SHA-512, SHA3, SM3, SM4
# (`=q0`: a register of the fixed view q at bit 0, whatever the arrangement)
# ---------------------------------------------------------------------------
ARRS.update({'C4S': '4s:0', 'C2D': '2d:0'})
for name, opc in (('aese', 0x4), ('aesd', 0x5), ('aesmc', 0x6), ('aesimc', 0x7)):
    ln(name, 'v0,v5', 0x4e280800 | (opc << 12), 'M16')
# SHA three-register: 01011110 000 Rm 0 opcode 00 Rn Rd
for name, opc in (('sha1c', 0), ('sha1p', 1), ('sha1m', 2)):
    ln(name, '=q0,=s5,v16', 0x5e000000 | (opc << 12), 'C4S')
ln('sha1su0', 'v0,v5,v16', 0x5e003000, 'C4S')
ln('sha256h', '=q0,=q5,v16', 0x5e004000, 'C4S')
ln('sha256h2', '=q0,=q5,v16', 0x5e005000, 'C4S')
ln('sha256su1', 'v0,v5,v16', 0x5e006000, 'C4S')
# SHA two-register: 01011110 0010100 opcode 10 Rn Rd
ln('sha1h', '=s0,=s5', 0x5e280800, 'C4S')
ln('sha1su1', 'v0,v5', 0x5e281800, 'C4S')
ln('sha256su0', 'v0,v5', 0x5e282800, 'C4S')
# SHA-512 and SHA3: 11001110 011 Rm 1 0 00 opcode Rn Rd and the two-register forms
ln('sha512h', '=q0,=q5,v16', 0xce608000, 'C2D')
ln('sha512h2', '=q0,=q5,v16', 0xce608400, 'C2D')
ln('sha512su1', 'v0,v5,v16', 0xce608800, 'C2D')
ln('rax1', 'v0,v5,v16', 0xce608c00, 'C2D')
ln('sha512su0', 'v0,v5', 0xcec08000, 'C2D')
ln('eor3', 'v0,v5,v16,v10', 0xce000000, 'M16')
ln('bcax', 'v0,v5,v16,v10', 0xce200000, 'M16')
ln('xar', 'v0,v5,v16,m10', 0xce800000, 'C2D')
# SM3 and SM4
ln('sm3ss1', 'v0,v5,v16,v10', 0xce400000, 'C4S')
for name, opc in (('sm3tt1a', 0), ('sm3tt1b', 1), ('sm3tt2a', 2), ('sm3tt2b', 3)):
    ln(name, 'v0,v5,n16', 0xce408000 | (opc << 10), 'C4S')
ln('sm3partw1', 'v0,v5,v16', 0xce60c000, 'C4S')
ln('sm3partw2', 'v0,v5,v16', 0xce60c400, 'C4S')
ln('sm4ekey', 'v0,v5,v16', 0xce60c800, 'C4S')
ln('sm4e', 'v0,v5', 0xcec08400, 'C4S')

# ===========================================================================
# SVE and SVE2 (the Arm ARM's SVE encoding classes). A second table SV, with
# its element-size sets SVSZ (the size field, bits [23:22], for each element).
# The operand mini-language (asm/a64.ax skind): z/u sized/unsized Z, p/q
# sized/unsized P, g/m/e a 3-bit Pg (plain, /m, /z), h/n a 4-bit Pg (plain,
# /z), w/x/y a W/X/X-or-SP register, r a size-width register, d a Z equal to
# the destination; the immediate kinds A (pattern) R/L (unpred shift) S/T
# (predicated shift) B/C (signed imm, +lsl8) U/V (unsigned imm, +lsl8) F (FP8)
# M (bitmask) I (indexed element) Z (#0.0) O/P (rotations) D<lo>-<hi> (a range).
# Each token is a kind then its bit position (none for d/A/.. that are fixed).
# ===========================================================================
SV = []
SVSZ = {}
_SB = {'b': 0, 'h': 1, 's': 2, 'd': 3}


def _sz(*els):
    return ' '.join('%s:%x' % (e, _SB[e] << 22) for e in els)


SVSZ['BHSD'] = _sz('b', 'h', 's', 'd')
SVSZ['HSD'] = _sz('h', 's', 'd')
SVSZ['BHS'] = _sz('b', 'h', 's')
SVSZ['SD'] = _sz('s', 'd')
SVSZ['HS'] = _sz('h', 's')
SVSZ['B'] = _sz('b')
SVSZ['H'] = _sz('h')
SVSZ['S'] = _sz('s')
SVSZ['D'] = _sz('d')
# element-size fixed in the opcode: the member sets no bit
SVSZ['DX'] = 'd:0'
SVSZ['SX'] = 's:0'
SVSZ['BX'] = 'b:0'
SVSZ['QX'] = 'q:0'  # a fixed .q element (SVE2.1 revd): the size bits are in the opcode


def sv(name, ops, base, sz):
    SV.append('%s %s %08x %s' % (name, ops, base, sz))


# SVE integer add/subtract (vectors, unpredicated): 00000100 size 1 Zm 000 opc Zn Zd
for name, opc in (('add', 0), ('sub', 1), ('sqadd', 4), ('uqadd', 5), ('sqsub', 6), ('uqsub', 7)):
    sv(name, 'z0,z5,z16', 0x04200000 | (opc << 10), 'BHSD')
# the clamps (SVE2.1 / SME): Zd clamped to [Zn, Zm], three same-element vectors
sv('sclamp', 'z0,z5,z16', 0x4400c000, 'BHSD')
sv('uclamp', 'z0,z5,z16', 0x4400c400, 'BHSD')
sv('fclamp', 'z0,z5,z16', 0x64202400, 'HSD')
# the logical ones (unpredicated): element always .d, the two size bits select the op
for name, bits in (('and', 0), ('orr', 1), ('eor', 2), ('bic', 3)):
    sv(name, 'z0,z5,z16', 0x04203000 | (bits << 22), 'DX')
# SVE integer binary arithmetic (predicated): 00000100 size 0 op8 Pg Zdn Zm
PRED2 = [('add', 0x00, 'BHSD'), ('sub', 0x01, 'BHSD'), ('subr', 0x03, 'BHSD'), ('smax', 0x08, 'BHSD'),
         ('umax', 0x09, 'BHSD'), ('smin', 0x0a, 'BHSD'), ('umin', 0x0b, 'BHSD'), ('sabd', 0x0c, 'BHSD'),
         ('uabd', 0x0d, 'BHSD'), ('mul', 0x10, 'BHSD'), ('smulh', 0x12, 'BHSD'), ('umulh', 0x13, 'BHSD'),
         ('sdiv', 0x14, 'SD'), ('udiv', 0x15, 'SD'), ('sdivr', 0x16, 'SD'), ('udivr', 0x17, 'SD'),
         ('orr', 0x18, 'BHSD'), ('eor', 0x19, 'BHSD'), ('and', 0x1a, 'BHSD'), ('bic', 0x1b, 'BHSD')]
for name, op8, sz in PRED2:
    sv(name, 'z0,m10,d,z5', 0x04000000 | (op8 << 16), sz)
SVSZ['N'] = ''  # no element size (movprfx of whole registers)

# SVE integer unary (predicated): 00000100 size 1 0 opc(3) 101 Pg Zn Zd
UN = [('sxtb', 0x0, 'HSD'), ('uxtb', 0x1, 'HSD'), ('sxth', 0x2, 'SD'), ('uxth', 0x3, 'SD'),
      ('sxtw', 0x4, 'D'), ('uxtw', 0x5, 'D'), ('abs', 0x6, 'BHSD'), ('neg', 0x7, 'BHSD'),
      ('cls', 0x8, 'BHSD'), ('clz', 0x9, 'BHSD'), ('cnt', 0xa, 'BHSD'), ('cnot', 0xb, 'BHSD'),
      ('fabs', 0xc, 'HSD'), ('fneg', 0xd, 'HSD'), ('not', 0xe, 'BHSD')]
for name, opc, sz in UN:
    sv(name, 'z0,m10,z5', 0x0410a000 | (opc << 16), sz)
# the byte/halfword/word reverses and bit-reverse: 00000101 size 1 opc 100 Pg Zn Zd
for name, opc, sz in (('revb', 0x0, 'HSD'), ('revh', 0x1, 'SD'), ('revw', 0x2, 'D'), ('rbit', 0x3, 'BHSD')):
    sv(name, 'z0,m10,z5', 0x05248000 | (opc << 16), sz)
# REVD (SVE2.1): reverse 64-bit doublewords in 128-bit elements, .q only
sv('revd', 'z0,m10,z5', 0x052e8000, 'QX')

# MOVPRFX: unpredicated (whole register) and predicated (/m sets bit 16, /z clears)
sv('movprfx', 'u0,u5', 0x0420bc00, 'N')
sv('movprfx', 'z0,e10,z5', 0x04102000, 'BHSD')
sv('movprfx', 'z0,m10,z5', 0x04112000, 'BHSD')

# SVE shifts. The element size is carried in the tsz field, so the size set
# adds no size bits (TSZ): all of b h s d, no [23:22] contribution.
SVSZ['TSZ'] = 'b:0 h:0 s:0 d:0'
# unpredicated by immediate: tszh[23:22]:tszl[20:19]:imm3[18:16]
sv('asr', 'z0,z5,R', 0x04209000, 'TSZ')
sv('lsr', 'z0,z5,R', 0x04209400, 'TSZ')
sv('lsl', 'z0,z5,L', 0x04209c00, 'TSZ')
# predicated by immediate: tszh[23:22]:tszl[9:8]:imm3[7:5]
for name, opc in (('asr', 0), ('lsr', 1), ('lsl', 3), ('asrd', 4), ('sqshl', 6), ('uqshl', 7),
                  ('srshr', 0xc), ('urshr', 0xd), ('sqshlu', 0xf)):
    k = 'T' if name in ('lsl', 'sqshl', 'uqshl', 'sqshlu') else 'S'
    sv(name, 'z0,m10,d,' + k, 0x04008000 | (opc << 16), 'TSZ')
# predicated by vector (destructive): size in [23:22]
for name, opc in (('asr', 0x0), ('lsr', 0x1), ('lsl', 0x3), ('asrr', 0x4), ('lsrr', 0x5), ('lslr', 0x7)):
    sv(name, 'z0,m10,d,z5', 0x04108000 | (opc << 16), 'BHSD')
# predicated by wide element (Zm is .d), destructive; Zdn b/h/s
for name, opc in (('asr', 0x0), ('lsr', 0x1), ('lsl', 0x3)):
    sv(name, 'z0,m10,d,c5', 0x04188000 | (opc << 16), 'BHS')
# unpredicated by wide element (Zn b/h/s, Zm .d)
for name, opc in (('asr', 0x0), ('lsr', 0x1), ('lsl', 0x3)):
    sv(name, 'z0,z5,c16', 0x04208000 | (opc << 10), 'BHS')

# SVE integer compares (predicated, result a predicate). Pd[3:0], Pg/z[12:10],
# Zn[9:5], Zm[20:16]. The same-size vector forms:
for name, base in (('cmpeq', 0x2400a000), ('cmpne', 0x2400a010), ('cmpge', 0x24008000),
                   ('cmpgt', 0x24008010), ('cmphi', 0x24000010), ('cmphs', 0x24000000)):
    sv(name, 'p0,e10,z5,z16', base, 'BHSD')
# the wide forms (Zm is .d; Zn b/h/s):
for name, base in (('cmpeq', 0x24002000), ('cmpne', 0x24002010), ('cmpge', 0x24004000),
                   ('cmpgt', 0x24004010), ('cmphi', 0x2400c010), ('cmphs', 0x2400c000),
                   ('cmple', 0x24006010), ('cmplt', 0x24006000), ('cmplo', 0x2400e000), ('cmpls', 0x2400e010)):
    sv(name, 'p0,e10,z5,c16', base, 'BHS')
# the signed-immediate forms (-16..15 at bit 16):
for name, base in (('cmpeq', 0x25008000), ('cmpne', 0x25008010), ('cmpge', 0x25000000),
                   ('cmpgt', 0x25000010), ('cmplt', 0x25002000), ('cmple', 0x25002010)):
    sv(name, 'p0,e10,z5,K', base, 'BHSD')
# the unsigned-immediate forms (0..127 at bit 14):
for name, base in (('cmphi', 0x24200010), ('cmphs', 0x24200000), ('cmplo', 0x24202000), ('cmpls', 0x24202010)):
    sv(name, 'p0,e10,z5,G', base, 'BHSD')

# SVE integer reductions (predicated): a scalar of T's element size, Pg[12:10],
# Zn[9:5]. andv/orv/eorv and the min/max; saddv/uaddv widen to a d register.
for name, base in (('orv', 0x04182000), ('eorv', 0x04192000), ('andv', 0x041a2000), ('smaxv', 0x04082000),
                   ('sminv', 0x040a2000), ('umaxv', 0x04092000), ('uminv', 0x040b2000)):
    sv(name, 'v0,g10,z5', base, 'BHSD')
sv('saddv', 'a0,g10,z5', 0x04002000, 'BHS')
sv('uaddv', 'a0,g10,z5', 0x04012000, 'BHSD')

# INDEX: start and step from immediates (-16..15) or general registers.
sv('index', 'z0,i5,i16', 0x04204000, 'BHSD')
sv('index', 'z0,r5,i16', 0x04204400, 'BHSD')
sv('index', 'z0,i5,r16', 0x04204800, 'BHSD')
sv('index', 'z0,r5,r16', 0x04204c00, 'BHSD')

# SVE permutes: zip/uzp/trn on vectors and on predicates, rev of each.
for name, opc in (('zip1', 0), ('zip2', 1), ('uzp1', 2), ('uzp2', 3), ('trn1', 4), ('trn2', 5)):
    sv(name, 'z0,z5,z16', 0x05206000 | (opc << 10), 'BHSD')
    sv(name, 'p0,p5,p16', 0x05204000 | (opc << 10), 'BHSD')
sv('rev', 'z0,z5', 0x05383800, 'BHSD')
sv('rev', 'p0,p5', 0x05344000, 'BHSD')
# SEL (vectors): a 4-bit governing predicate, no qualifier.
sv('sel', 'z0,h10,z5,z16', 0x0520c000, 'BHSD')
# SPLICE (destructive), COMPACT (s/d), the vector CLASTA/CLASTB (destructive).
sv('splice', 'z0,g10,d,z5', 0x052c8000, 'BHSD')
sv('compact', 'z0,g10,z5', 0x05218000, 'SD')
sv('clasta', 'z0,g10,d,z5', 0x05288000, 'BHSD')
sv('clastb', 'z0,g10,d,z5', 0x05298000, 'BHSD')

# SVE predicate operations. The predicate logical ops (.b only): Pd[3:0],
# Pg/z[13:10], Pn[8:5], Pm[19:16].
for name, base in (('and', 0x25004000), ('orr', 0x25804000), ('eor', 0x25004200), ('bic', 0x25004010),
                   ('nand', 0x25804210), ('nor', 0x25804200), ('orn', 0x25804010), ('ands', 0x25404000),
                   ('bics', 0x25404010), ('eors', 0x25404200), ('orrs', 0x25c04000), ('nands', 0x25c04210),
                   ('nors', 0x25c04200), ('orns', 0x25c04010)):
    sv(name, 'p0,n10,p5,p16', base, 'B')
sv('sel', 'p0,h10,p5,p16', 0x25004210, 'B')
# BRKA/BRKB (predicated, /z or /m), and their flag-setting /z-only variants.
sv('brka', 'p0,n10,p5', 0x25104000, 'B')
sv('brka', 'p0,j10,p5', 0x25104010, 'B')
sv('brkb', 'p0,n10,p5', 0x25904000, 'B')
sv('brkb', 'p0,j10,p5', 0x25904010, 'B')
sv('brkas', 'p0,n10,p5', 0x25504000, 'B')
sv('brkbs', 'p0,n10,p5', 0x25d04000, 'B')
# BRKN/BRKNS (destructive): Pdm = Pd.
sv('brkn', 'p0,n10,p5,k', 0x25184000, 'B')
sv('brkns', 'p0,n10,p5,k', 0x25584000, 'B')
# BRKPA/BRKPB and their flag-setting variants: Pd, Pg/z, Pn, Pm.
sv('brkpa', 'p0,n10,p5,p16', 0x2500c000, 'B')
sv('brkpb', 'p0,n10,p5,p16', 0x2500c010, 'B')
sv('brkpas', 'p0,n10,p5,p16', 0x2540c000, 'B')
sv('brkpbs', 'p0,n10,p5,p16', 0x2540c010, 'B')
# CNTP Xd, Pg, Pn.T
sv('cntp', 'x0,h10,p5', 0x25208000, 'BHSD')
# PFIRST (.b, destructive), PNEXT (destructive): Pdn, Pg, Pdn.
sv('pfirst', 'p0,h5,k', 0x2558c000, 'B')
sv('pnext', 'p0,h5,k', 0x2519c400, 'BHSD')
# PTRUE/PTRUES: a pattern operand (defaulting to ALL when omitted).
sv('ptrue', 'p0', 0x2518e3e0, 'BHSD')
sv('ptrue', 'p0,A', 0x2518e000, 'BHSD')
sv('ptrues', 'p0', 0x2519e3e0, 'BHSD')
sv('ptrues', 'p0,A', 0x2519e000, 'BHSD')
sv('pfalse', 'p0', 0x2518e400, 'B')
# PTEST Pg, Pn.b
sv('ptest', 'h10,p5', 0x2550c000, 'B')
# RDFFR (unpredicated and predicated), RDFFRS, WRFFR.
sv('rdffr', 'p0', 0x2519f000, 'B')
sv('rdffr', 'p0,n5', 0x2518f000, 'B')
sv('rdffrs', 'p0,n5', 0x2558f000, 'B')
sv('wrffr', 'p5', 0x25289000, 'B')

# SVE floating point. Predicated binary arithmetic (destructive, h/s/d):
for name, op8 in (('fadd', 0x00), ('fsub', 0x01), ('fmul', 0x02), ('fsubr', 0x03), ('fmaxnm', 0x04),
                  ('fminnm', 0x05), ('fmax', 0x06), ('fmin', 0x07), ('fabd', 0x08), ('fscale', 0x09),
                  ('fmulx', 0x0a), ('fdivr', 0x0c), ('fdiv', 0x0d)):
    sv(name, 'z0,m10,d,z5', 0x65008000 | (op8 << 16), 'HSD')
# predicated binary with a one-bit floating immediate (the set per op):
for name, op, st in (('fadd', 0x18, 0), ('fsub', 0x19, 0), ('fmul', 0x1a, 1), ('fsubr', 0x1b, 0),
                     ('fmaxnm', 0x1c, 2), ('fminnm', 0x1d, 2), ('fmax', 0x1e, 2), ('fmin', 0x1f, 2)):
    sv(name, 'z0,m10,d,f%d' % st, 0x65008000 | (op << 16), 'HSD')
# unpredicated arithmetic:
for name, opc in (('fadd', 0x0), ('fsub', 0x1), ('fmul', 0x2), ('ftsmul', 0x3), ('frecps', 0x6), ('frsqrts', 0x7)):
    sv(name, 'z0,z5,z16', 0x65000000 | (opc << 10), 'HSD')
# the predicated fused multiply-adds (three sources):
for name, op in (('fmla', 0x0), ('fmls', 0x1), ('fnmla', 0x2), ('fnmls', 0x3), ('fmad', 0x4), ('fmsb', 0x5),
                 ('fnmad', 0x6), ('fnmsb', 0x7)):
    sv(name, 'z0,m10,z5,z16', 0x65200000 | (op << 13), 'HSD')
# the FP compares (result a predicate):
for name, base in (('fcmge', 0x65004000), ('fcmgt', 0x65004010), ('fcmeq', 0x65006000), ('fcmne', 0x65006010),
                   ('fcmuo', 0x6500c000), ('facge', 0x6500c010), ('facgt', 0x6500e010)):
    sv(name, 'p0,e10,z5,z16', base, 'HSD')
# the FP compares with zero:
for name, base in (('fcmge', 0x65102000), ('fcmgt', 0x65102010), ('fcmlt', 0x65112000), ('fcmle', 0x65112010),
                   ('fcmeq', 0x65122000), ('fcmne', 0x65132000)):
    sv(name, 'p0,e10,z5,Z', base, 'HSD')
# the predicated FP unary ops:
for name, opc in (('frintn', 0x0), ('frintp', 0x1), ('frintm', 0x2), ('frintz', 0x3), ('frinta', 0x4),
                  ('frintx', 0x6), ('frinti', 0x7), ('frecpx', 0xc), ('fsqrt', 0xd)):
    sv(name, 'z0,m10,z5', 0x6500a000 | (opc << 16), 'HSD')
# the unpredicated estimates, fexpa, ftssel:
sv('frecpe', 'z0,z5', 0x650e3000, 'HSD')
sv('frsqrte', 'z0,z5', 0x650f3000, 'HSD')
sv('fexpa', 'z0,z5', 0x0420b800, 'HSD')
sv('ftssel', 'z0,z5,z16', 0x0420b000, 'HSD')
# complex arithmetic: fcadd (#90/#270) and fcmla (#0/#90/#180/#270):
sv('fcadd', 'z0,m10,d,z5,P16', 0x64008000, 'HSD')
sv('fcmla', 'z0,m10,z5,z16,O13', 0x64000000, 'HSD')

# SVE integer multiply (unpredicated vectors) and the by-element forms. The
# by-element element size is carried entirely in the index field (set EHSD
# adds no size bits).
SVSZ['EHSD'] = 'h:0 s:800000 d:c00000'
sv('mul', 'z0,z5,z16', 0x04206000, 'BHSD')
sv('smulh', 'z0,z5,z16', 0x04206800, 'BHSD')
sv('umulh', 'z0,z5,z16', 0x04206c00, 'BHSD')
for name, base in (('mla', 0x44200800), ('mls', 0x44200c00), ('mul', 0x4420f800), ('sqdmulh', 0x4420f000),
                   ('sqrdmulh', 0x4420f400)):
    sv(name, 'z0,z5,I16', base, 'EHSD')
for name, base in (('fmla', 0x64200000), ('fmls', 0x64200400), ('fmul', 0x64202000)):
    sv(name, 'z0,z5,I16', base, 'EHSD')
# the predicated integer multiply-adds (Zda, Pg/m, Zn, Zm):
for name, base in (('mla', 0x04004000), ('mls', 0x04006000)):
    sv(name, 'z0,m10,z5,z16', base, 'BHSD')
# mad/msb: Zdn, Pg/m, Zm, Za -- the written Zm is at [9:5], Za at [20:16]
for name, base in (('mad', 0x0400c000), ('msb', 0x0400e000)):
    sv(name, 'z0,m10,z16,z5', base, 'BHSD')

# The dot products, the complex integer forms and SVE2. Operands of another
# element size than T: Y (half T: the widening sources), W (twice T: the
# narrowing sources), N (a quarter of T: the dot products' sources); the
# by-element operands Q (a quarter of T: .s i2 at [20:19] with Zm 0-7, .d i1
# at [20] with Zm 0-15), E (T itself in that layout one size down: .h, .s;
# fcmla, cmla) and X (half T: .s i3 at [20:19]:[11], .d i2 at [20]:[11]); l a
# widening left shift (an amount of half T's elements, in tsz:imm3).
SVSZ['DOT'] = 's:0 d:400000'     # the size bit 22 alone (the dot products)
SVSZ['NB'] = 'b:400000 h:800000 s:c00000'  # the narrowing forms: the size of Tb (twice T)
SVSZ['CPLX'] = 'h:0 s:400000'    # the by-element complex forms: bit 22
SVSZ['PMULL'] = 'h:400000 d:c00000 q:0'
SVSZ['BH'] = _sz('b', 'h')
SVSZ['FLOGB'] = 'h:20000 s:40000 d:60000'  # the size field at [18:17]
SVSZ['TSZ3'] = 'b:0 h:0 s:0'  # the narrowing shifts (T b h s, carried in tsz)
SVSZ['TSZW'] = 'h:0 s:0 d:0'  # the widening shifts
SVSZ['TSZN'] = 'b:80000 h:100000 s:400000'  # tsz one-hot at [22,20:19] (sqxtnb ...)
sv('sdot', 'z0,N5,N16', 0x44800000, 'DOT')
sv('udot', 'z0,N5,N16', 0x44800400, 'DOT')
sv('usdot', 'z0,N5,N16', 0x44807800, 'SX')
sv('sdot', 'z0,N5,Q', 0x44a00000, 'DOT')
sv('udot', 'z0,N5,Q', 0x44a00400, 'DOT')
sv('usdot', 'z0,N5,Q', 0x44a01800, 'SX')
sv('sudot', 'z0,N5,Q', 0x44a01c00, 'SX')
sv('fcmla', 'z0,z5,E,O10', 0x64a01000, 'CPLX')
sv('cdot', 'z0,N5,N16,O10', 0x44801000, 'DOT')
sv('cdot', 'z0,N5,Q,O10', 0x44a04000, 'DOT')
sv('cmla', 'z0,z5,z16,O10', 0x44002000, 'BHSD')
sv('sqrdcmlah', 'z0,z5,z16,O10', 0x44003000, 'BHSD')
sv('cmla', 'z0,z5,E,O10', 0x44a06000, 'CPLX')
sv('sqrdcmlah', 'z0,z5,E,O10', 0x44a07000, 'CPLX')
sv('cadd', 'z0,d,z5,P10', 0x4500d800, 'BHSD')
sv('sqcadd', 'z0,d,z5,P10', 0x4501d800, 'BHSD')
sv('saddlb', 'z0,Y5,Y16', 0x45000000, 'HSD')
sv('saddlt', 'z0,Y5,Y16', 0x45000400, 'HSD')
sv('uaddlb', 'z0,Y5,Y16', 0x45000800, 'HSD')
sv('uaddlt', 'z0,Y5,Y16', 0x45000c00, 'HSD')
sv('ssublb', 'z0,Y5,Y16', 0x45001000, 'HSD')
sv('ssublt', 'z0,Y5,Y16', 0x45001400, 'HSD')
sv('usublb', 'z0,Y5,Y16', 0x45001800, 'HSD')
sv('usublt', 'z0,Y5,Y16', 0x45001c00, 'HSD')
sv('sabdlb', 'z0,Y5,Y16', 0x45003000, 'HSD')
sv('sabdlt', 'z0,Y5,Y16', 0x45003400, 'HSD')
sv('uabdlb', 'z0,Y5,Y16', 0x45003800, 'HSD')
sv('uabdlt', 'z0,Y5,Y16', 0x45003c00, 'HSD')
sv('saddlbt', 'z0,Y5,Y16', 0x45008000, 'HSD')
sv('ssublbt', 'z0,Y5,Y16', 0x45008800, 'HSD')
sv('ssubltb', 'z0,Y5,Y16', 0x45008c00, 'HSD')
sv('saddwb', 'z0,z5,Y16', 0x45004000, 'HSD')
sv('saddwt', 'z0,z5,Y16', 0x45004400, 'HSD')
sv('uaddwb', 'z0,z5,Y16', 0x45004800, 'HSD')
sv('uaddwt', 'z0,z5,Y16', 0x45004c00, 'HSD')
sv('ssubwb', 'z0,z5,Y16', 0x45005000, 'HSD')
sv('ssubwt', 'z0,z5,Y16', 0x45005400, 'HSD')
sv('usubwb', 'z0,z5,Y16', 0x45005800, 'HSD')
sv('usubwt', 'z0,z5,Y16', 0x45005c00, 'HSD')
sv('smullb', 'z0,Y5,Y16', 0x45007000, 'HSD')
sv('smullt', 'z0,Y5,Y16', 0x45007400, 'HSD')
sv('umullb', 'z0,Y5,Y16', 0x45007800, 'HSD')
sv('umullt', 'z0,Y5,Y16', 0x45007c00, 'HSD')
sv('sqdmullb', 'z0,Y5,Y16', 0x45006000, 'HSD')
sv('sqdmullt', 'z0,Y5,Y16', 0x45006400, 'HSD')
sv('pmullb', 'z0,Y5,Y16', 0x45006800, 'PMULL')
sv('pmullt', 'z0,Y5,Y16', 0x45006c00, 'PMULL')
sv('smullb', 'z0,Y5,X', 0x44a0c000, 'DOT')
sv('smullt', 'z0,Y5,X', 0x44a0c400, 'DOT')
sv('umullb', 'z0,Y5,X', 0x44a0d000, 'DOT')
sv('umullt', 'z0,Y5,X', 0x44a0d400, 'DOT')
sv('sqdmullb', 'z0,Y5,X', 0x44a0e000, 'DOT')
sv('sqdmullt', 'z0,Y5,X', 0x44a0e400, 'DOT')
sv('smlalb', 'z0,Y5,Y16', 0x44004000, 'HSD')
sv('smlalt', 'z0,Y5,Y16', 0x44004400, 'HSD')
sv('umlalb', 'z0,Y5,Y16', 0x44004800, 'HSD')
sv('umlalt', 'z0,Y5,Y16', 0x44004c00, 'HSD')
sv('smlslb', 'z0,Y5,Y16', 0x44005000, 'HSD')
sv('smlslt', 'z0,Y5,Y16', 0x44005400, 'HSD')
sv('umlslb', 'z0,Y5,Y16', 0x44005800, 'HSD')
sv('umlslt', 'z0,Y5,Y16', 0x44005c00, 'HSD')
sv('sqdmlalb', 'z0,Y5,Y16', 0x44006000, 'HSD')
sv('sqdmlalt', 'z0,Y5,Y16', 0x44006400, 'HSD')
sv('sqdmlslb', 'z0,Y5,Y16', 0x44006800, 'HSD')
sv('sqdmlslt', 'z0,Y5,Y16', 0x44006c00, 'HSD')
sv('sqdmlalbt', 'z0,Y5,Y16', 0x44000800, 'HSD')
sv('sqdmlslbt', 'z0,Y5,Y16', 0x44000c00, 'HSD')
sv('smlalb', 'z0,Y5,X', 0x44a08000, 'DOT')
sv('smlalt', 'z0,Y5,X', 0x44a08400, 'DOT')
sv('umlalb', 'z0,Y5,X', 0x44a09000, 'DOT')
sv('umlalt', 'z0,Y5,X', 0x44a09400, 'DOT')
sv('smlslb', 'z0,Y5,X', 0x44a0a000, 'DOT')
sv('smlslt', 'z0,Y5,X', 0x44a0a400, 'DOT')
sv('umlslb', 'z0,Y5,X', 0x44a0b000, 'DOT')
sv('umlslt', 'z0,Y5,X', 0x44a0b400, 'DOT')
sv('sqdmlalb', 'z0,Y5,X', 0x44a02000, 'DOT')
sv('sqdmlalt', 'z0,Y5,X', 0x44a02400, 'DOT')
sv('sqdmlslb', 'z0,Y5,X', 0x44a03000, 'DOT')
sv('sqdmlslt', 'z0,Y5,X', 0x44a03400, 'DOT')
sv('fmlalb', 'z0,Y5,Y16', 0x64a08000, 'SX')
sv('fmlalt', 'z0,Y5,Y16', 0x64a08400, 'SX')
sv('fmlslb', 'z0,Y5,Y16', 0x64a0a000, 'SX')
sv('fmlslt', 'z0,Y5,Y16', 0x64a0a400, 'SX')
sv('fmlalb', 'z0,Y5,X', 0x64a04000, 'SX')
sv('fmlalt', 'z0,Y5,X', 0x64a04400, 'SX')
sv('fmlslb', 'z0,Y5,X', 0x64a06000, 'SX')
sv('fmlslt', 'z0,Y5,X', 0x64a06400, 'SX')
sv('saba', 'z0,z5,z16', 0x4500f800, 'BHSD')
sv('uaba', 'z0,z5,z16', 0x4500fc00, 'BHSD')
sv('sabalb', 'z0,Y5,Y16', 0x4500c000, 'HSD')
sv('sabalt', 'z0,Y5,Y16', 0x4500c400, 'HSD')
sv('uabalb', 'z0,Y5,Y16', 0x4500c800, 'HSD')
sv('uabalt', 'z0,Y5,Y16', 0x4500cc00, 'HSD')
sv('adclb', 'z0,z5,z16', 0x4500d000, 'DOT')
sv('adclt', 'z0,z5,z16', 0x4500d400, 'DOT')
sv('sbclb', 'z0,z5,z16', 0x4580d000, 'DOT')
sv('sbclt', 'z0,z5,z16', 0x4580d400, 'DOT')
sv('addhnb', 'z0,W5,W16', 0x45206000, 'NB')
sv('addhnt', 'z0,W5,W16', 0x45206400, 'NB')
sv('raddhnb', 'z0,W5,W16', 0x45206800, 'NB')
sv('raddhnt', 'z0,W5,W16', 0x45206c00, 'NB')
sv('subhnb', 'z0,W5,W16', 0x45207000, 'NB')
sv('subhnt', 'z0,W5,W16', 0x45207400, 'NB')
sv('rsubhnb', 'z0,W5,W16', 0x45207800, 'NB')
sv('rsubhnt', 'z0,W5,W16', 0x45207c00, 'NB')
sv('shrnb', 'z0,W5,R', 0x45201000, 'TSZ3')
sv('shrnt', 'z0,W5,R', 0x45201400, 'TSZ3')
sv('rshrnb', 'z0,W5,R', 0x45201800, 'TSZ3')
sv('rshrnt', 'z0,W5,R', 0x45201c00, 'TSZ3')
sv('sqshrnb', 'z0,W5,R', 0x45202000, 'TSZ3')
sv('sqshrnt', 'z0,W5,R', 0x45202400, 'TSZ3')
sv('uqshrnb', 'z0,W5,R', 0x45203000, 'TSZ3')
sv('uqshrnt', 'z0,W5,R', 0x45203400, 'TSZ3')
sv('sqrshrnb', 'z0,W5,R', 0x45202800, 'TSZ3')
sv('sqrshrnt', 'z0,W5,R', 0x45202c00, 'TSZ3')
sv('uqrshrnb', 'z0,W5,R', 0x45203800, 'TSZ3')
sv('uqrshrnt', 'z0,W5,R', 0x45203c00, 'TSZ3')
sv('sqshrunb', 'z0,W5,R', 0x45200000, 'TSZ3')
sv('sqshrunt', 'z0,W5,R', 0x45200400, 'TSZ3')
sv('sqrshrunb', 'z0,W5,R', 0x45200800, 'TSZ3')
sv('sqrshrunt', 'z0,W5,R', 0x45200c00, 'TSZ3')
sv('sqxtnb', 'z0,W5', 0x45204000, 'TSZN')
sv('sqxtnt', 'z0,W5', 0x45204400, 'TSZN')
sv('uqxtnb', 'z0,W5', 0x45204800, 'TSZN')
sv('uqxtnt', 'z0,W5', 0x45204c00, 'TSZN')
sv('sqxtunb', 'z0,W5', 0x45205000, 'TSZN')
sv('sqxtunt', 'z0,W5', 0x45205400, 'TSZN')
sv('sshllb', 'z0,Y5,l', 0x4500a000, 'TSZW')
sv('sshllt', 'z0,Y5,l', 0x4500a400, 'TSZW')
sv('ushllb', 'z0,Y5,l', 0x4500a800, 'TSZW')
sv('ushllt', 'z0,Y5,l', 0x4500ac00, 'TSZW')
sv('sli', 'z0,z5,L', 0x4500f400, 'TSZ')
sv('sri', 'z0,z5,R', 0x4500f000, 'TSZ')
sv('ssra', 'z0,z5,R', 0x4500e000, 'TSZ')
sv('usra', 'z0,z5,R', 0x4500e400, 'TSZ')
sv('srsra', 'z0,z5,R', 0x4500e800, 'TSZ')
sv('ursra', 'z0,z5,R', 0x4500ec00, 'TSZ')
sv('sqshl', 'z0,m10,d,z5', 0x44088000, 'BHSD')
sv('uqshl', 'z0,m10,d,z5', 0x44098000, 'BHSD')
sv('srshl', 'z0,m10,d,z5', 0x44028000, 'BHSD')
sv('urshl', 'z0,m10,d,z5', 0x44038000, 'BHSD')
sv('sqrshl', 'z0,m10,d,z5', 0x440a8000, 'BHSD')
sv('uqrshl', 'z0,m10,d,z5', 0x440b8000, 'BHSD')
sv('sqshlr', 'z0,m10,d,z5', 0x440c8000, 'BHSD')
sv('uqshlr', 'z0,m10,d,z5', 0x440d8000, 'BHSD')
sv('srshlr', 'z0,m10,d,z5', 0x44068000, 'BHSD')
sv('urshlr', 'z0,m10,d,z5', 0x44078000, 'BHSD')
sv('sqrshlr', 'z0,m10,d,z5', 0x440e8000, 'BHSD')
sv('uqrshlr', 'z0,m10,d,z5', 0x440f8000, 'BHSD')
sv('sqadd', 'z0,m10,d,z5', 0x44188000, 'BHSD')
sv('uqadd', 'z0,m10,d,z5', 0x44198000, 'BHSD')
sv('sqsub', 'z0,m10,d,z5', 0x441a8000, 'BHSD')
sv('uqsub', 'z0,m10,d,z5', 0x441b8000, 'BHSD')
sv('suqadd', 'z0,m10,d,z5', 0x441c8000, 'BHSD')
sv('usqadd', 'z0,m10,d,z5', 0x441d8000, 'BHSD')
sv('sqsubr', 'z0,m10,d,z5', 0x441e8000, 'BHSD')
sv('uqsubr', 'z0,m10,d,z5', 0x441f8000, 'BHSD')
sv('shadd', 'z0,m10,d,z5', 0x44108000, 'BHSD')
sv('uhadd', 'z0,m10,d,z5', 0x44118000, 'BHSD')
sv('shsub', 'z0,m10,d,z5', 0x44128000, 'BHSD')
sv('uhsub', 'z0,m10,d,z5', 0x44138000, 'BHSD')
sv('shsubr', 'z0,m10,d,z5', 0x44168000, 'BHSD')
sv('uhsubr', 'z0,m10,d,z5', 0x44178000, 'BHSD')
sv('srhadd', 'z0,m10,d,z5', 0x44148000, 'BHSD')
sv('urhadd', 'z0,m10,d,z5', 0x44158000, 'BHSD')
sv('addp', 'z0,m10,d,z5', 0x4411a000, 'BHSD')
sv('smaxp', 'z0,m10,d,z5', 0x4414a000, 'BHSD')
sv('umaxp', 'z0,m10,d,z5', 0x4415a000, 'BHSD')
sv('sminp', 'z0,m10,d,z5', 0x4416a000, 'BHSD')
sv('uminp', 'z0,m10,d,z5', 0x4417a000, 'BHSD')
sv('faddp', 'z0,m10,d,z5', 0x64108000, 'HSD')
sv('fmaxnmp', 'z0,m10,d,z5', 0x64148000, 'HSD')
sv('fminnmp', 'z0,m10,d,z5', 0x64158000, 'HSD')
sv('fmaxp', 'z0,m10,d,z5', 0x64168000, 'HSD')
sv('fminp', 'z0,m10,d,z5', 0x64178000, 'HSD')
sv('sadalp', 'z0,m10,Y5', 0x4404a000, 'HSD')
sv('uadalp', 'z0,m10,Y5', 0x4405a000, 'HSD')
sv('sqabs', 'z0,m10,z5', 0x4408a000, 'BHSD')
sv('sqneg', 'z0,m10,z5', 0x4409a000, 'BHSD')
sv('urecpe', 'z0,m10,z5', 0x4480a000, 'SX')
sv('ursqrte', 'z0,m10,z5', 0x4481a000, 'SX')
sv('flogb', 'z0,m10,z5', 0x6518a000, 'FLOGB')
sv('sqdmulh', 'z0,z5,z16', 0x04207000, 'BHSD')
sv('sqrdmulh', 'z0,z5,z16', 0x04207400, 'BHSD')
sv('pmul', 'z0,z5,z16', 0x04206400, 'BX')
sv('sqrdmlah', 'z0,z5,z16', 0x44007000, 'BHSD')
sv('sqrdmlsh', 'z0,z5,z16', 0x44007400, 'BHSD')
sv('sqrdmlah', 'z0,z5,I', 0x44201000, 'EHSD')
sv('sqrdmlsh', 'z0,z5,I', 0x44201400, 'EHSD')
sv('eor3', 'z0,d,z16,z5', 0x04203800, 'DX')
sv('bcax', 'z0,d,z16,z5', 0x04603800, 'DX')
sv('bsl', 'z0,d,z16,z5', 0x04203c00, 'DX')
sv('bsl1n', 'z0,d,z16,z5', 0x04603c00, 'DX')
sv('bsl2n', 'z0,d,z16,z5', 0x04a03c00, 'DX')
sv('nbsl', 'z0,d,z16,z5', 0x04e03c00, 'DX')
sv('xar', 'z0,d,z5,R', 0x04203400, 'TSZ')
sv('match', 'p0,e10,z5,z16', 0x45208000, 'BH')
sv('nmatch', 'p0,e10,z5,z16', 0x45208010, 'BH')
sv('histcnt', 'z0,e10,z5,z16', 0x45a0c000, 'DOT')
sv('histseg', 'z0,z5,z16', 0x4520a000, 'BX')
sv('bdep', 'z0,z5,z16', 0x4500b400, 'BHSD')
sv('bext', 'z0,z5,z16', 0x4500b000, 'BHSD')
sv('bgrp', 'z0,z5,z16', 0x4500b800, 'BHSD')
sv('eorbt', 'z0,z5,z16', 0x45009000, 'BHSD')
sv('eortb', 'z0,z5,z16', 0x45009400, 'BHSD')
sv('aese', 'z0,d,z5', 0x4522e000, 'BX')
sv('aesd', 'z0,d,z5', 0x4522e400, 'BX')
sv('aesmc', 'z0,d', 0x4520e000, 'BX')
sv('aesimc', 'z0,d', 0x4520e400, 'BX')
sv('sm4e', 'z0,d,z5', 0x4523e000, 'SX')
sv('sm4ekey', 'z0,z5,z16', 0x4520f000, 'SX')
sv('rax1', 'z0,z5,z16', 0x4520f400, 'DX')

# The FP reductions and the strictly ordered fadda (o: a scalar equal to the
# destination), the SVE2 and BF16 conversions, the matrix multiplies, BF16
# arithmetic (H: a half-T element in Q's layout), the 128-bit permutes, the
# unpacks (b: a P register .b), splice of a register pair (t: a list of two),
# the predicate-count increments (s: a W register numbered as the first
# operand), cterm, and cpy from a general or scalar register (_r: W or X of
# T's width, or sp).
SVSZ['CVTN'] = 'h:0 s:420000'
SVSZ['CVTL'] = 's:0 d:420000'
SVSZ['QX'] = 'q:0'
SVSZ['HX'] = 'h:0'
sv('fadda', 'v0,g10,o,z5', 0x65182000, 'HSD')
sv('faddv', 'v0,g10,z5', 0x65002000, 'HSD')
sv('fmaxv', 'v0,g10,z5', 0x65062000, 'HSD')
sv('fminv', 'v0,g10,z5', 0x65072000, 'HSD')
sv('fmaxnmv', 'v0,g10,z5', 0x65042000, 'HSD')
sv('fminnmv', 'v0,g10,z5', 0x65052000, 'HSD')
sv('fcvtlt', 'z0,m10,Y5', 0x6489a000, 'CVTL')
sv('fcvtnt', 'z0,m10,W5', 0x6488a000, 'CVTN')
sv('fcvtx', 'z0,m10,W5', 0x650aa000, 'SX')
sv('fcvtxnt', 'z0,m10,W5', 0x640aa000, 'SX')
sv('bfcvt', 'z0,m10,W5', 0x658aa000, 'HX')
sv('bfcvtnt', 'z0,m10,W5', 0x648aa000, 'HX')
sv('smmla', 'z0,N5,N16', 0x45009800, 'SX')
sv('ummla', 'z0,N5,N16', 0x45c09800, 'SX')
sv('usmmla', 'z0,N5,N16', 0x45809800, 'SX')
sv('fmmla', 'z0,z5,z16', 0x64a0e400, 'DOT')
sv('bfmmla', 'z0,Y5,Y16', 0x6460e400, 'SX')
sv('bfdot', 'z0,Y5,Y16', 0x64608000, 'SX')
sv('bfdot', 'z0,Y5,H', 0x64604000, 'SX')
sv('bfmlalb', 'z0,Y5,Y16', 0x64e08000, 'SX')
sv('bfmlalt', 'z0,Y5,Y16', 0x64e08400, 'SX')
sv('bfmlalb', 'z0,Y5,X', 0x64e04000, 'SX')
sv('bfmlalt', 'z0,Y5,X', 0x64e04400, 'SX')
sv('zip1', 'z0,z5,z16', 0x05a00000, 'QX')
sv('zip2', 'z0,z5,z16', 0x05a00400, 'QX')
sv('uzp1', 'z0,z5,z16', 0x05a00800, 'QX')
sv('uzp2', 'z0,z5,z16', 0x05a00c00, 'QX')
sv('trn1', 'z0,z5,z16', 0x05a01800, 'QX')
sv('trn2', 'z0,z5,z16', 0x05a01c00, 'QX')
sv('sunpklo', 'z0,Y5', 0x05303800, 'HSD')
sv('sunpkhi', 'z0,Y5', 0x05313800, 'HSD')
sv('uunpklo', 'z0,Y5', 0x05323800, 'HSD')
sv('uunpkhi', 'z0,Y5', 0x05333800, 'HSD')
sv('punpklo', 'p0,b5', 0x05304000, 'HX')
sv('punpkhi', 'p0,b5', 0x05314000, 'HX')
sv('splice', 'z0,g10,t5', 0x052d8000, 'BHSD')
sv('incp', 'x0,p5', 0x252c8800, 'BHSD')
sv('decp', 'x0,p5', 0x252d8800, 'BHSD')
sv('incp', 'z0,p5', 0x252c8000, 'HSD')
sv('decp', 'z0,p5', 0x252d8000, 'HSD')
sv('sqincp', 'x0,p5,s', 0x25288800, 'BHSD')
sv('sqdecp', 'x0,p5,s', 0x252a8800, 'BHSD')
sv('sqincp', 'x0,p5', 0x25288c00, 'BHSD')
sv('sqdecp', 'x0,p5', 0x252a8c00, 'BHSD')
sv('uqincp', 'w0,p5', 0x25298800, 'BHSD')
sv('uqdecp', 'w0,p5', 0x252b8800, 'BHSD')
sv('uqincp', 'x0,p5', 0x25298c00, 'BHSD')
sv('uqdecp', 'x0,p5', 0x252b8c00, 'BHSD')
sv('sqincp', 'z0,p5', 0x25288000, 'HSD')
sv('sqdecp', 'z0,p5', 0x252a8000, 'HSD')
sv('uqincp', 'z0,p5', 0x25298000, 'HSD')
sv('uqdecp', 'z0,p5', 0x252b8000, 'HSD')
sv('ctermeq', 'w5,w16', 0x25a02000, 'SX')
sv('ctermne', 'w5,w16', 0x25a02010, 'SX')
sv('ctermeq', 'x5,x16', 0x25e02000, 'DX')
sv('ctermne', 'x5,x16', 0x25e02010, 'DX')
sv('cpy', 'z0,m10,_r5', 0x0528a000, 'BHSD')
sv('mov', 'z0,m10,_r5', 0x0528a000, 'BHSD')
sv('cpy', 'z0,m10,v5', 0x05208000, 'BHSD')
sv('mov', 'z0,m10,v5', 0x05208000, 'BHSD')

# ===========================================================================
# The SVE loads, stores and prefetches (SVEMEM): a line is `mnemonic element
# mode base xs`: Zt's element (b h s d, or - for a prefetch, whose operation is
# at [3:0]), the addressing mode, the word with every field 0, and the bit an
# `sxtw` extend sets (the 32-bit offset forms). Zt at [4:0], Pg at [12:10],
# Rn/Zn at [9:5], Rm/Zm at [20:16]. The modes (asm/a64.ax sve_mem):
#   si  [Xn{, #imm, mul vl}]  imm4 at 16, in units of the registers loaded
#   sp  [Xn{, #imm, mul vl}]  imm6 at 16 (the prefetches)
#   sr  [Xn{, #imm}]          imm6 at 16 unsigned, scaled (ld1r*)
#   sq  [Xn{, #imm}]          imm4 at 16 in units of 16 bytes (ld1rq*); so of 32 (ld1ro*)
#   ss  [Xn, Xm{, lsl #msz}]  Xm not xzr; ssf the first-fault form, Xm optional (xzr)
#   su32 ss32 [Xn, Zm.S, uxtw|sxtw{ #msz}]  unscaled / scaled 32-bit offsets
#   su64 ss64 [Xn, Zm.D{, lsl #msz}]        64-bit offsets
#   sux ssx   [Xn, Zm.D, uxtw|sxtw{ #msz}]  64-bit elements, 32-bit offsets
#   vi32 vi64 [Zn.S|D{, #imm}]  imm5 at 16 unsigned, scaled
#   vs32 vs64 [Zn.S|D{, Xm}]    the SVE2 non-temporal forms, Xm default xzr
# The forms and base words are those of the Arm ARM's SVE memory encodings.
# ===========================================================================
SM = []


def sm(name, el, mode, base, xs):
    SM.append('%s %s %s %08x %x' % (name, el, mode, base, xs))


sm('ld1b', 'b', 'si', 0xa400a000, 0x0)
sm('ld1b', 'h', 'si', 0xa420a000, 0x0)
sm('ld1b', 's', 'si', 0xa440a000, 0x0)
sm('ld1b', 'd', 'si', 0xa460a000, 0x0)
sm('ld1h', 'h', 'si', 0xa4a0a000, 0x0)
sm('ld1h', 's', 'si', 0xa4c0a000, 0x0)
sm('ld1h', 'd', 'si', 0xa4e0a000, 0x0)
sm('ld1w', 's', 'si', 0xa540a000, 0x0)
sm('ld1w', 'd', 'si', 0xa560a000, 0x0)
sm('ld1d', 'd', 'si', 0xa5e0a000, 0x0)
sm('ld1sb', 'h', 'si', 0xa5c0a000, 0x0)
sm('ld1sb', 's', 'si', 0xa5a0a000, 0x0)
sm('ld1sb', 'd', 'si', 0xa580a000, 0x0)
sm('ld1sh', 's', 'si', 0xa520a000, 0x0)
sm('ld1sh', 'd', 'si', 0xa500a000, 0x0)
sm('ld1sw', 'd', 'si', 0xa480a000, 0x0)
sm('ldnf1b', 'b', 'si', 0xa410a000, 0x0)
sm('ldnf1b', 'h', 'si', 0xa430a000, 0x0)
sm('ldnf1b', 's', 'si', 0xa450a000, 0x0)
sm('ldnf1b', 'd', 'si', 0xa470a000, 0x0)
sm('ldnf1h', 'h', 'si', 0xa4b0a000, 0x0)
sm('ldnf1h', 's', 'si', 0xa4d0a000, 0x0)
sm('ldnf1h', 'd', 'si', 0xa4f0a000, 0x0)
sm('ldnf1w', 's', 'si', 0xa550a000, 0x0)
sm('ldnf1w', 'd', 'si', 0xa570a000, 0x0)
sm('ldnf1d', 'd', 'si', 0xa5f0a000, 0x0)
sm('ldnf1sb', 'h', 'si', 0xa5d0a000, 0x0)
sm('ldnf1sb', 's', 'si', 0xa5b0a000, 0x0)
sm('ldnf1sb', 'd', 'si', 0xa590a000, 0x0)
sm('ldnf1sh', 's', 'si', 0xa530a000, 0x0)
sm('ldnf1sh', 'd', 'si', 0xa510a000, 0x0)
sm('ldnf1sw', 'd', 'si', 0xa490a000, 0x0)
sm('ld1b', 'b', 'ss', 0xa4004000, 0x0)
sm('ldff1b', 'b', 'ssf', 0xa4006000, 0x0)
sm('ld1b', 'h', 'ss', 0xa4204000, 0x0)
sm('ldff1b', 'h', 'ssf', 0xa4206000, 0x0)
sm('ld1b', 's', 'ss', 0xa4404000, 0x0)
sm('ldff1b', 's', 'ssf', 0xa4406000, 0x0)
sm('ld1b', 'd', 'ss', 0xa4604000, 0x0)
sm('ldff1b', 'd', 'ssf', 0xa4606000, 0x0)
sm('ld1h', 'h', 'ss', 0xa4a04000, 0x0)
sm('ldff1h', 'h', 'ssf', 0xa4a06000, 0x0)
sm('ld1h', 's', 'ss', 0xa4c04000, 0x0)
sm('ldff1h', 's', 'ssf', 0xa4c06000, 0x0)
sm('ld1h', 'd', 'ss', 0xa4e04000, 0x0)
sm('ldff1h', 'd', 'ssf', 0xa4e06000, 0x0)
sm('ld1w', 's', 'ss', 0xa5404000, 0x0)
sm('ldff1w', 's', 'ssf', 0xa5406000, 0x0)
sm('ld1w', 'd', 'ss', 0xa5604000, 0x0)
sm('ldff1w', 'd', 'ssf', 0xa5606000, 0x0)
sm('ld1d', 'd', 'ss', 0xa5e04000, 0x0)
sm('ldff1d', 'd', 'ssf', 0xa5e06000, 0x0)
sm('ld1sb', 'h', 'ss', 0xa5c04000, 0x0)
sm('ldff1sb', 'h', 'ssf', 0xa5c06000, 0x0)
sm('ld1sb', 's', 'ss', 0xa5a04000, 0x0)
sm('ldff1sb', 's', 'ssf', 0xa5a06000, 0x0)
sm('ld1sb', 'd', 'ss', 0xa5804000, 0x0)
sm('ldff1sb', 'd', 'ssf', 0xa5806000, 0x0)
sm('ld1sh', 's', 'ss', 0xa5204000, 0x0)
sm('ldff1sh', 's', 'ssf', 0xa5206000, 0x0)
sm('ld1sh', 'd', 'ss', 0xa5004000, 0x0)
sm('ldff1sh', 'd', 'ssf', 0xa5006000, 0x0)
sm('ld1sw', 'd', 'ss', 0xa4804000, 0x0)
sm('ldff1sw', 'd', 'ssf', 0xa4806000, 0x0)
sm('st1b', 'b', 'si', 0xe400e000, 0x0)
sm('st1b', 'b', 'ss', 0xe4004000, 0x0)
sm('st1b', 'h', 'si', 0xe420e000, 0x0)
sm('st1b', 'h', 'ss', 0xe4204000, 0x0)
sm('st1b', 's', 'si', 0xe440e000, 0x0)
sm('st1b', 's', 'ss', 0xe4404000, 0x0)
sm('st1b', 'd', 'si', 0xe460e000, 0x0)
sm('st1b', 'd', 'ss', 0xe4604000, 0x0)
sm('st1h', 'h', 'si', 0xe4a0e000, 0x0)
sm('st1h', 'h', 'ss', 0xe4a04000, 0x0)
sm('st1h', 's', 'si', 0xe4c0e000, 0x0)
sm('st1h', 's', 'ss', 0xe4c04000, 0x0)
sm('st1h', 'd', 'si', 0xe4e0e000, 0x0)
sm('st1h', 'd', 'ss', 0xe4e04000, 0x0)
sm('st1w', 's', 'si', 0xe540e000, 0x0)
sm('st1w', 's', 'ss', 0xe5404000, 0x0)
sm('st1w', 'd', 'si', 0xe560e000, 0x0)
sm('st1w', 'd', 'ss', 0xe5604000, 0x0)
sm('st1d', 'd', 'si', 0xe5e0e000, 0x0)
sm('st1d', 'd', 'ss', 0xe5e04000, 0x0)
sm('ld1rb', 'b', 'sr', 0x84408000, 0x0)
sm('ld1rb', 'h', 'sr', 0x8440a000, 0x0)
sm('ld1rb', 's', 'sr', 0x8440c000, 0x0)
sm('ld1rb', 'd', 'sr', 0x8440e000, 0x0)
sm('ld1rh', 'h', 'sr', 0x84c0a000, 0x0)
sm('ld1rh', 's', 'sr', 0x84c0c000, 0x0)
sm('ld1rh', 'd', 'sr', 0x84c0e000, 0x0)
sm('ld1rw', 's', 'sr', 0x8540c000, 0x0)
sm('ld1rw', 'd', 'sr', 0x8540e000, 0x0)
sm('ld1rd', 'd', 'sr', 0x85c0e000, 0x0)
sm('ld1rsb', 'h', 'sr', 0x85c0c000, 0x0)
sm('ld1rsb', 's', 'sr', 0x85c0a000, 0x0)
sm('ld1rsb', 'd', 'sr', 0x85c08000, 0x0)
sm('ld1rsh', 's', 'sr', 0x8540a000, 0x0)
sm('ld1rsh', 'd', 'sr', 0x85408000, 0x0)
sm('ld1rsw', 'd', 'sr', 0x84c08000, 0x0)
sm('ld1rqb', 'b', 'sq', 0xa4002000, 0x0)
sm('ld1rqb', 'b', 'ss', 0xa4000000, 0x0)
sm('ld1rob', 'b', 'so', 0xa4202000, 0x0)
sm('ld1rob', 'b', 'ss', 0xa4200000, 0x0)
sm('ldnt1b', 'b', 'si', 0xa400e000, 0x0)
sm('ldnt1b', 'b', 'ss', 0xa400c000, 0x0)
sm('stnt1b', 'b', 'si', 0xe410e000, 0x0)
sm('stnt1b', 'b', 'ss', 0xe4006000, 0x0)
sm('ld2b', 'b', 'si', 0xa420e000, 0x0)
sm('ld2b', 'b', 'ss', 0xa420c000, 0x0)
sm('ld3b', 'b', 'si', 0xa440e000, 0x0)
sm('ld3b', 'b', 'ss', 0xa440c000, 0x0)
sm('ld4b', 'b', 'si', 0xa460e000, 0x0)
sm('ld4b', 'b', 'ss', 0xa460c000, 0x0)
sm('st2b', 'b', 'si', 0xe430e000, 0x0)
sm('st2b', 'b', 'ss', 0xe4206000, 0x0)
sm('st3b', 'b', 'si', 0xe450e000, 0x0)
sm('st3b', 'b', 'ss', 0xe4406000, 0x0)
sm('st4b', 'b', 'si', 0xe470e000, 0x0)
sm('st4b', 'b', 'ss', 0xe4606000, 0x0)
sm('ld1rqh', 'h', 'sq', 0xa4802000, 0x0)
sm('ld1rqh', 'h', 'ss', 0xa4800000, 0x0)
sm('ld1roh', 'h', 'so', 0xa4a02000, 0x0)
sm('ld1roh', 'h', 'ss', 0xa4a00000, 0x0)
sm('ldnt1h', 'h', 'si', 0xa480e000, 0x0)
sm('ldnt1h', 'h', 'ss', 0xa480c000, 0x0)
sm('stnt1h', 'h', 'si', 0xe490e000, 0x0)
sm('stnt1h', 'h', 'ss', 0xe4806000, 0x0)
sm('ld2h', 'h', 'si', 0xa4a0e000, 0x0)
sm('ld2h', 'h', 'ss', 0xa4a0c000, 0x0)
sm('ld3h', 'h', 'si', 0xa4c0e000, 0x0)
sm('ld3h', 'h', 'ss', 0xa4c0c000, 0x0)
sm('ld4h', 'h', 'si', 0xa4e0e000, 0x0)
sm('ld4h', 'h', 'ss', 0xa4e0c000, 0x0)
sm('st2h', 'h', 'si', 0xe4b0e000, 0x0)
sm('st2h', 'h', 'ss', 0xe4a06000, 0x0)
sm('st3h', 'h', 'si', 0xe4d0e000, 0x0)
sm('st3h', 'h', 'ss', 0xe4c06000, 0x0)
sm('st4h', 'h', 'si', 0xe4f0e000, 0x0)
sm('st4h', 'h', 'ss', 0xe4e06000, 0x0)
sm('ld1rqw', 's', 'sq', 0xa5002000, 0x0)
sm('ld1rqw', 's', 'ss', 0xa5000000, 0x0)
sm('ld1row', 's', 'so', 0xa5202000, 0x0)
sm('ld1row', 's', 'ss', 0xa5200000, 0x0)
sm('ldnt1w', 's', 'si', 0xa500e000, 0x0)
sm('ldnt1w', 's', 'ss', 0xa500c000, 0x0)
sm('stnt1w', 's', 'si', 0xe510e000, 0x0)
sm('stnt1w', 's', 'ss', 0xe5006000, 0x0)
sm('ld2w', 's', 'si', 0xa520e000, 0x0)
sm('ld2w', 's', 'ss', 0xa520c000, 0x0)
sm('ld3w', 's', 'si', 0xa540e000, 0x0)
sm('ld3w', 's', 'ss', 0xa540c000, 0x0)
sm('ld4w', 's', 'si', 0xa560e000, 0x0)
sm('ld4w', 's', 'ss', 0xa560c000, 0x0)
sm('st2w', 's', 'si', 0xe530e000, 0x0)
sm('st2w', 's', 'ss', 0xe5206000, 0x0)
sm('st3w', 's', 'si', 0xe550e000, 0x0)
sm('st3w', 's', 'ss', 0xe5406000, 0x0)
sm('st4w', 's', 'si', 0xe570e000, 0x0)
sm('st4w', 's', 'ss', 0xe5606000, 0x0)
sm('ld1rqd', 'd', 'sq', 0xa5802000, 0x0)
sm('ld1rqd', 'd', 'ss', 0xa5800000, 0x0)
sm('ld1rod', 'd', 'so', 0xa5a02000, 0x0)
sm('ld1rod', 'd', 'ss', 0xa5a00000, 0x0)
sm('ldnt1d', 'd', 'si', 0xa580e000, 0x0)
sm('ldnt1d', 'd', 'ss', 0xa580c000, 0x0)
sm('stnt1d', 'd', 'si', 0xe590e000, 0x0)
sm('stnt1d', 'd', 'ss', 0xe5806000, 0x0)
sm('ld2d', 'd', 'si', 0xa5a0e000, 0x0)
sm('ld2d', 'd', 'ss', 0xa5a0c000, 0x0)
sm('ld3d', 'd', 'si', 0xa5c0e000, 0x0)
sm('ld3d', 'd', 'ss', 0xa5c0c000, 0x0)
sm('ld4d', 'd', 'si', 0xa5e0e000, 0x0)
sm('ld4d', 'd', 'ss', 0xa5e0c000, 0x0)
sm('st2d', 'd', 'si', 0xe5b0e000, 0x0)
sm('st2d', 'd', 'ss', 0xe5a06000, 0x0)
sm('st3d', 'd', 'si', 0xe5d0e000, 0x0)
sm('st3d', 'd', 'ss', 0xe5c06000, 0x0)
sm('st4d', 'd', 'si', 0xe5f0e000, 0x0)
sm('st4d', 'd', 'ss', 0xe5e06000, 0x0)
sm('ld1b', 's', 'su32', 0x84004000, 0x400000)
sm('ld1b', 's', 'vi32', 0x8420c000, 0x0)
sm('ld1sb', 's', 'su32', 0x84000000, 0x400000)
sm('ld1sb', 's', 'vi32', 0x84208000, 0x0)
sm('ld1h', 's', 'su32', 0x84804000, 0x400000)
sm('ld1h', 's', 'vi32', 0x84a0c000, 0x0)
sm('ld1h', 's', 'ss32', 0x84a04000, 0x400000)
sm('ld1sh', 's', 'su32', 0x84800000, 0x400000)
sm('ld1sh', 's', 'vi32', 0x84a08000, 0x0)
sm('ld1sh', 's', 'ss32', 0x84a00000, 0x400000)
sm('ld1w', 's', 'su32', 0x85004000, 0x400000)
sm('ld1w', 's', 'vi32', 0x8520c000, 0x0)
sm('ld1w', 's', 'ss32', 0x85204000, 0x400000)
sm('ld1b', 'd', 'su64', 0xc440c000, 0x0)
sm('ld1b', 'd', 'sux', 0xc4004000, 0x400000)
sm('ld1b', 'd', 'vi64', 0xc420c000, 0x0)
sm('ld1sb', 'd', 'su64', 0xc4408000, 0x0)
sm('ld1sb', 'd', 'sux', 0xc4000000, 0x400000)
sm('ld1sb', 'd', 'vi64', 0xc4208000, 0x0)
sm('ld1h', 'd', 'su64', 0xc4c0c000, 0x0)
sm('ld1h', 'd', 'sux', 0xc4804000, 0x400000)
sm('ld1h', 'd', 'vi64', 0xc4a0c000, 0x0)
sm('ld1h', 'd', 'ss64', 0xc4e0c000, 0x0)
sm('ld1h', 'd', 'ssx', 0xc4a04000, 0x400000)
sm('ld1sh', 'd', 'su64', 0xc4c08000, 0x0)
sm('ld1sh', 'd', 'sux', 0xc4800000, 0x400000)
sm('ld1sh', 'd', 'vi64', 0xc4a08000, 0x0)
sm('ld1sh', 'd', 'ss64', 0xc4e08000, 0x0)
sm('ld1sh', 'd', 'ssx', 0xc4a00000, 0x400000)
sm('ld1w', 'd', 'su64', 0xc540c000, 0x0)
sm('ld1w', 'd', 'sux', 0xc5004000, 0x400000)
sm('ld1w', 'd', 'vi64', 0xc520c000, 0x0)
sm('ld1w', 'd', 'ss64', 0xc560c000, 0x0)
sm('ld1w', 'd', 'ssx', 0xc5204000, 0x400000)
sm('ld1sw', 'd', 'su64', 0xc5408000, 0x0)
sm('ld1sw', 'd', 'sux', 0xc5000000, 0x400000)
sm('ld1sw', 'd', 'vi64', 0xc5208000, 0x0)
sm('ld1sw', 'd', 'ss64', 0xc5608000, 0x0)
sm('ld1sw', 'd', 'ssx', 0xc5200000, 0x400000)
sm('ld1d', 'd', 'su64', 0xc5c0c000, 0x0)
sm('ld1d', 'd', 'sux', 0xc5804000, 0x400000)
sm('ld1d', 'd', 'vi64', 0xc5a0c000, 0x0)
sm('ld1d', 'd', 'ss64', 0xc5e0c000, 0x0)
sm('ld1d', 'd', 'ssx', 0xc5a04000, 0x400000)
sm('ldff1b', 's', 'su32', 0x84006000, 0x400000)
sm('ldff1b', 's', 'vi32', 0x8420e000, 0x0)
sm('ldff1sb', 's', 'su32', 0x84002000, 0x400000)
sm('ldff1sb', 's', 'vi32', 0x8420a000, 0x0)
sm('ldff1h', 's', 'su32', 0x84806000, 0x400000)
sm('ldff1h', 's', 'vi32', 0x84a0e000, 0x0)
sm('ldff1h', 's', 'ss32', 0x84a06000, 0x400000)
sm('ldff1sh', 's', 'su32', 0x84802000, 0x400000)
sm('ldff1sh', 's', 'vi32', 0x84a0a000, 0x0)
sm('ldff1sh', 's', 'ss32', 0x84a02000, 0x400000)
sm('ldff1w', 's', 'su32', 0x85006000, 0x400000)
sm('ldff1w', 's', 'vi32', 0x8520e000, 0x0)
sm('ldff1w', 's', 'ss32', 0x85206000, 0x400000)
sm('ldff1b', 'd', 'su64', 0xc440e000, 0x0)
sm('ldff1b', 'd', 'sux', 0xc4006000, 0x400000)
sm('ldff1b', 'd', 'vi64', 0xc420e000, 0x0)
sm('ldff1sb', 'd', 'su64', 0xc440a000, 0x0)
sm('ldff1sb', 'd', 'sux', 0xc4002000, 0x400000)
sm('ldff1sb', 'd', 'vi64', 0xc420a000, 0x0)
sm('ldff1h', 'd', 'su64', 0xc4c0e000, 0x0)
sm('ldff1h', 'd', 'sux', 0xc4806000, 0x400000)
sm('ldff1h', 'd', 'vi64', 0xc4a0e000, 0x0)
sm('ldff1h', 'd', 'ss64', 0xc4e0e000, 0x0)
sm('ldff1h', 'd', 'ssx', 0xc4a06000, 0x400000)
sm('ldff1sh', 'd', 'su64', 0xc4c0a000, 0x0)
sm('ldff1sh', 'd', 'sux', 0xc4802000, 0x400000)
sm('ldff1sh', 'd', 'vi64', 0xc4a0a000, 0x0)
sm('ldff1sh', 'd', 'ss64', 0xc4e0a000, 0x0)
sm('ldff1sh', 'd', 'ssx', 0xc4a02000, 0x400000)
sm('ldff1w', 'd', 'su64', 0xc540e000, 0x0)
sm('ldff1w', 'd', 'sux', 0xc5006000, 0x400000)
sm('ldff1w', 'd', 'vi64', 0xc520e000, 0x0)
sm('ldff1w', 'd', 'ss64', 0xc560e000, 0x0)
sm('ldff1w', 'd', 'ssx', 0xc5206000, 0x400000)
sm('ldff1sw', 'd', 'su64', 0xc540a000, 0x0)
sm('ldff1sw', 'd', 'sux', 0xc5002000, 0x400000)
sm('ldff1sw', 'd', 'vi64', 0xc520a000, 0x0)
sm('ldff1sw', 'd', 'ss64', 0xc560a000, 0x0)
sm('ldff1sw', 'd', 'ssx', 0xc5202000, 0x400000)
sm('ldff1d', 'd', 'su64', 0xc5c0e000, 0x0)
sm('ldff1d', 'd', 'sux', 0xc5806000, 0x400000)
sm('ldff1d', 'd', 'vi64', 0xc5a0e000, 0x0)
sm('ldff1d', 'd', 'ss64', 0xc5e0e000, 0x0)
sm('ldff1d', 'd', 'ssx', 0xc5a06000, 0x400000)
sm('st1b', 's', 'su32', 0xe4408000, 0x4000)
sm('st1b', 's', 'vi32', 0xe460a000, 0x0)
sm('st1h', 's', 'su32', 0xe4c08000, 0x4000)
sm('st1h', 's', 'vi32', 0xe4e0a000, 0x0)
sm('st1h', 's', 'ss32', 0xe4e08000, 0x4000)
sm('st1w', 's', 'su32', 0xe5408000, 0x4000)
sm('st1w', 's', 'vi32', 0xe560a000, 0x0)
sm('st1w', 's', 'ss32', 0xe5608000, 0x4000)
sm('st1b', 'd', 'su64', 0xe400a000, 0x0)
sm('st1b', 'd', 'sux', 0xe4008000, 0x4000)
sm('st1b', 'd', 'vi64', 0xe440a000, 0x0)
sm('st1h', 'd', 'su64', 0xe480a000, 0x0)
sm('st1h', 'd', 'sux', 0xe4808000, 0x4000)
sm('st1h', 'd', 'vi64', 0xe4c0a000, 0x0)
sm('st1h', 'd', 'ss64', 0xe4a0a000, 0x0)
sm('st1h', 'd', 'ssx', 0xe4a08000, 0x4000)
sm('st1w', 'd', 'su64', 0xe500a000, 0x0)
sm('st1w', 'd', 'sux', 0xe5008000, 0x4000)
sm('st1w', 'd', 'vi64', 0xe540a000, 0x0)
sm('st1w', 'd', 'ss64', 0xe520a000, 0x0)
sm('st1w', 'd', 'ssx', 0xe5208000, 0x4000)
sm('st1d', 'd', 'su64', 0xe580a000, 0x0)
sm('st1d', 'd', 'sux', 0xe5808000, 0x4000)
sm('st1d', 'd', 'vi64', 0xe5c0a000, 0x0)
sm('st1d', 'd', 'ss64', 0xe5a0a000, 0x0)
sm('st1d', 'd', 'ssx', 0xe5a08000, 0x4000)
sm('ldnt1b', 's', 'vs32', 0x8400a000, 0x0)
sm('ldnt1sb', 's', 'vs32', 0x84008000, 0x0)
sm('ldnt1h', 's', 'vs32', 0x8480a000, 0x0)
sm('ldnt1sh', 's', 'vs32', 0x84808000, 0x0)
sm('ldnt1w', 's', 'vs32', 0x8500a000, 0x0)
sm('ldnt1b', 'd', 'vs64', 0xc400c000, 0x0)
sm('ldnt1sb', 'd', 'vs64', 0xc4008000, 0x0)
sm('ldnt1h', 'd', 'vs64', 0xc480c000, 0x0)
sm('ldnt1sh', 'd', 'vs64', 0xc4808000, 0x0)
sm('ldnt1w', 'd', 'vs64', 0xc500c000, 0x0)
sm('ldnt1sw', 'd', 'vs64', 0xc5008000, 0x0)
sm('ldnt1d', 'd', 'vs64', 0xc580c000, 0x0)
sm('stnt1b', 's', 'vs32', 0xe4402000, 0x0)
sm('stnt1h', 's', 'vs32', 0xe4c02000, 0x0)
sm('stnt1w', 's', 'vs32', 0xe5402000, 0x0)
sm('stnt1b', 'd', 'vs64', 0xe4002000, 0x0)
sm('stnt1h', 'd', 'vs64', 0xe4802000, 0x0)
sm('stnt1w', 'd', 'vs64', 0xe5002000, 0x0)
sm('stnt1d', 'd', 'vs64', 0xe5802000, 0x0)
sm('prfb', '-', 'sp', 0x85c00000, 0x0)
sm('prfb', '-', 'ss', 0x8400c000, 0x0)
sm('prfb', '-', 'vi32', 0x8400e000, 0x0)
sm('prfb', '-', 'vi64', 0xc400e000, 0x0)
sm('prfb', '-', 'su32', 0x84200000, 0x400000)
sm('prfb', '-', 'su64', 0xc4608000, 0x0)
sm('prfb', '-', 'sux', 0xc4200000, 0x400000)
sm('prfh', '-', 'sp', 0x85c02000, 0x0)
sm('prfh', '-', 'ss', 0x8480c000, 0x0)
sm('prfh', '-', 'vi32', 0x8480e000, 0x0)
sm('prfh', '-', 'vi64', 0xc480e000, 0x0)
sm('prfh', '-', 'ss32', 0x84202000, 0x400000)
sm('prfh', '-', 'ss64', 0xc460a000, 0x0)
sm('prfh', '-', 'ssx', 0xc4202000, 0x400000)
sm('prfw', '-', 'sp', 0x85c04000, 0x0)
sm('prfw', '-', 'ss', 0x8500c000, 0x0)
sm('prfw', '-', 'vi32', 0x8500e000, 0x0)
sm('prfw', '-', 'vi64', 0xc500e000, 0x0)
sm('prfw', '-', 'ss32', 0x84204000, 0x400000)
sm('prfw', '-', 'ss64', 0xc460c000, 0x0)
sm('prfw', '-', 'ssx', 0xc4204000, 0x400000)
sm('prfd', '-', 'sp', 0x85c06000, 0x0)
sm('prfd', '-', 'ss', 0x8580c000, 0x0)
sm('prfd', '-', 'vi32', 0x8580e000, 0x0)
sm('prfd', '-', 'vi64', 0xc580e000, 0x0)
sm('prfd', '-', 'ss32', 0x84206000, 0x400000)
sm('prfd', '-', 'ss64', 0xc460e000, 0x0)
sm('prfd', '-', 'ssx', 0xc4206000, 0x400000)

print('''// =============================================================================
// asm/a64tab.ax - the table of the AArch64 assembler's instruction forms
// beyond those asm/a64.ax encodes by hand (asm/a64.ax, "The instruction
// table", says how a line reads). Generated from the Arm Architecture
// Reference Manual's A64 encodings by cmd/internal/tools/gen_a64_table.py:
// do not edit.
// =============================================================================''')
print('pub let ARRS: [%d][]u8 = [' % len(ARRS))
for k, v in ARRS.items():
    print('    "%s %s",' % (k, v))
print('];')
print('pub let TABLE: [%d][]u8 = [' % len(L))
for l in L:
    print('    "%s",' % l)
print('];')
print('pub let SVESZ: [%d][]u8 = [' % len(SVSZ))
for k, v in SVSZ.items():
    print('    "%s %s",' % (k, v))
print('];')
print('pub let SVETAB: [%d][]u8 = [' % len(SV))
for l in SV:
    print('    "%s",' % l)
print('];')
print('pub let SVEMEM: [%d][]u8 = [' % len(SM))
for l in SM:
    print('    "%s",' % l)
print('];')
