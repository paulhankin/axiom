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
