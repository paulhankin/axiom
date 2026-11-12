# Vector instructions for gen_x64_table.py: SSE2..SSE4.2 (legacy), AVX/AVX2,
# FMA, F16C, and AVX-512 (EVEX). Included by gen_x64_table.py, which defines
# ln() and the list L. Written from Intel SDM vol. 2's opcode tables. The
# helpers below generate the legacy, VEX and EVEX forms of one operation
# together, since they share an opcode and operand shape.

# pp codes
PP = {'np': 'np', '66': '66', 'f3': 'f3', 'f2': 'f2'}


def vec_rm(name, pp, mp, opc, w='wig', ew=None, tuple='fv', bc=None, legacy=True, vex=True, evex=True,
           lens='pq', mem=True, dq=False):
    """A packed op: dst, src/mem.  lens 'pq'=xmm+ymm+zmm, 'p'=xmm only etc."""
    msuf = '/m' if mem else ''
    if legacy:
        ln(name, 'RM', 'V,V%s' % msuf, 'L.%s.%s.w0' % (pp, mp), opc, '/r', 'lx')
    if vex:
        ln('v' + name, 'RM', 'V,V%s' % msuf, 'V.%s.%s.%s' % (pp, mp, w), opc, '/r', 'lxy')
    if evex:
        ew = ew or ('w1' if dq else 'w0')
        attrs = ['lxyz', tuple]
        if bc:
            attrs.append('b%d' % bc)
        attrs += ['z']
        ln('v' + name, 'RM', 'V,V%s' % msuf, 'E.%s.%s.%s' % (pp, mp, ew), opc, '/r', *attrs)


def vec_rvm(name, pp, mp, opc, w='wig', ew=None, tuple='fv', bc=None, legacy=True, vex=True, evex=True, dq=False):
    """A three-operand op: dst, src1(vvvv), src2/mem (the AVX form); the
    legacy form is dst(=src1), src2/mem."""
    if legacy:
        ln(name, 'RM', 'V,V/m', 'L.%s.%s.w0' % (pp, mp), opc, '/r', 'lx')
    if vex:
        ln('v' + name, 'RVM', 'V,V,V/m', 'V.%s.%s.%s' % (pp, mp, w), opc, '/r', 'lxy')
    if evex:
        ew = ew or ('w1' if dq else 'w0')
        attrs = ['lxyz', tuple]
        if bc:
            attrs.append('b%d' % bc)
        attrs.append('z')
        ln('v' + name, 'RVM', 'V,V,V/m', 'E.%s.%s.%s' % (pp, mp, ew), opc, '/r', *attrs)


def vec_rvmi(name, pp, mp, opc, w='wig', ew=None, tuple='fv', bc=None, legacy=True, vex=True, evex=True, dq=False):
    if legacy:
        ln(name, 'RMI', 'V,V/m,i', 'L.%s.%s.w0' % (pp, mp), opc, '/r', 'lx')
    if vex:
        ln('v' + name, 'RVMI', 'V,V,V/m,i', 'V.%s.%s.%s' % (pp, mp, w), opc, '/r', 'lxy')
    if evex:
        ew = ew or ('w1' if dq else 'w0')
        attrs = ['lxyz', tuple]
        if bc:
            attrs.append('b%d' % bc)
        attrs.append('z')
        ln('v' + name, 'RVMI', 'V,V,V/m,i', 'E.%s.%s.%s' % (pp, mp, ew), opc, '/r', *attrs)


def vec_rmi(name, pp, mp, opc, w='wig', ew=None, tuple='fv', bc=None, legacy=True, vex=True, evex=True, dq=False):
    if legacy:
        ln(name, 'RMI', 'V,V/m,i', 'L.%s.%s.w0' % (pp, mp), opc, '/r', 'lx')
    if vex:
        ln('v' + name, 'RMI', 'V,V/m,i', 'V.%s.%s.%s' % (pp, mp, w), opc, '/r', 'lxy')
    if evex:
        ew = ew or ('w1' if dq else 'w0')
        attrs = ['lxyz', tuple]
        if bc:
            attrs.append('b%d' % bc)
        attrs.append('z')
        ln('v' + name, 'RMI', 'V,V/m,i', 'E.%s.%s.%s' % (pp, mp, ew), opc, '/r', *attrs)


# -- packed integer arithmetic/logic (SSE2 integer, AVX2, AVX-512 BW/DQ) -----
# element width decides EVEX.W and masking element; grouped by what AVX-512 set
# supports them. For the differential tests we encode the common ones.
INT_BW = {  # byte/word element, AVX-512BW (w0), tuple fvm
    'paddb': 'fc', 'paddw': 'fd', 'paddsb': 'ec', 'paddsw': 'ed', 'paddusb': 'dc', 'paddusw': 'dd',
    'psubb': 'f8', 'psubw': 'f9', 'psubsb': 'e8', 'psubsw': 'e9', 'psubusb': 'd8', 'psubusw': 'd9',
    'pcmpeqb': '74', 'pcmpeqw': '75', 'pcmpgtb': '64', 'pcmpgtw': '65', 'pmaxsw': 'ee', 'pmaxub': 'de',
    'pminsw': 'ea', 'pminub': 'da', 'pavgb': 'e0', 'pavgw': 'e3', 'pmullw': 'd5', 'pmulhw': 'e5',
    'pmulhuw': 'e4', 'pmaddwd': 'f5', 'pmaddubsw': None, 'psadbw': 'f6', 'psubusbw': None,
    'punpckhbw': '68', 'punpckhwd': '69', 'punpcklbw': '60', 'punpcklwd': '61', 'packsswb': '63',
    'packuswb': '67', 'packssdw': '6b', 'phaddw': None, 'phaddd': None,
}
for n, c in INT_BW.items():
    if c is None:
        continue
    vec_rvm(n, '66', '0f', c, tuple='fvm')

# dword/qword element (AVX-512F, w0 for d, w1 for q), tuple fv, broadcast
INT_D = {'paddd': 'fe', 'psubd': 'fa', 'pcmpeqd': '76', 'pcmpgtd': '66', 'pmaxsd': ('38', '3d'),
         'pminsd': ('38', '39'), 'pmaxud': ('38', '3f'), 'pminud': ('38', '3b'), 'pmulld': ('38', '40'),
         'pmuludq': 'f4', 'punpckhdq': '6a', 'punpckldq': '62', 'punpckhqdq': '6d', 'punpcklqdq': '6c'}
for n, c in INT_D.items():
    q = n.endswith('q') or n in ('pmuludq', 'punpckhqdq', 'punpcklqdq')
    if isinstance(c, tuple):
        vec_rvm(n, '66', '0f38', c[1], bc=8 if q else 4, dq=q)
    else:
        vec_rvm(n, '66', '0f', c, bc=8 if q else 4, dq=q)
vec_rvm('paddq', '66', '0f', 'd4', bc=8, dq=True)
vec_rvm('psubq', '66', '0f', 'fb', bc=8, dq=True)
vec_rvm('pmuldq', '66', '0f38', '28', bc=8, dq=True)

# logic: legacy/VEX use 0F EB etc.; EVEX pandd/pandq split
for n, c in [('pand', 'db'), ('pandn', 'df'), ('por', 'eb'), ('pxor', 'ef')]:
    ln(n, 'RM', 'V,V/m', 'L.66.0f.w0', c, '/r', 'lx')
    ln('v' + n, 'RVM', 'V,V,V/m', 'V.66.0f.wig', c, '/r', 'lxy')
    for suf, ew in [('d', 'w0'), ('q', 'w1')]:
        ln('v' + n + suf, 'RVM', 'V,V,V/m', 'E.66.0f.%s' % ew, c, '/r', 'lxyz', 'fv', 'b%d' % (4 if suf == 'd' else 8),
           'z')

# -- SSE/SSE2 float arithmetic (the scalar/packed single/double not in x64.ax's
# arith4: those are handled there for legacy; add VEX+EVEX forms) ------------
FLT = [('add', '58'), ('sub', '5c'), ('mul', '59'), ('div', '5e'), ('min', '5d'), ('max', '5f')]
for n, c in FLT:
    # packed single (np.0f), packed double (66.0f): VEX+EVEX
    ln('v' + n + 'ps', 'RVM', 'V,V,V/m', 'V.np.0f.wig', c, '/r', 'lxy')
    ln('v' + n + 'ps', 'RVM', 'V,V,V/m', 'E.np.0f.w0', c, '/r', 'lxyz', 'fv', 'b4', 'z', 'er')
    ln('v' + n + 'pd', 'RVM', 'V,V,V/m', 'V.66.0f.wig', c, '/r', 'lxy')
    ln('v' + n + 'pd', 'RVM', 'V,V,V/m', 'E.66.0f.w1', c, '/r', 'lxyz', 'fv', 'b8', 'z', 'er')
    # scalar single/double
    ln('v' + n + 'ss', 'RVM', 'x,x,x/m', 'V.f3.0f.wig', c, '/r', 'lig')
    ln('v' + n + 'ss', 'RVM', 'x,x,x/m', 'E.f3.0f.w0', c, '/r', 'lig', 'n4', 'z', 'er')
    ln('v' + n + 'sd', 'RVM', 'x,x,x/m', 'V.f2.0f.wig', c, '/r', 'lig')
    ln('v' + n + 'sd', 'RVM', 'x,x,x/m', 'E.f2.0f.w1', c, '/r', 'lig', 'n8', 'z', 'er')
for n, c in [('sqrt', '51')]:
    ln('v' + n + 'ps', 'RM', 'V,V/m', 'V.np.0f.wig', c, '/r', 'lxy')
    ln('v' + n + 'ps', 'RM', 'V,V/m', 'E.np.0f.w0', c, '/r', 'lxyz', 'fv', 'b4', 'z', 'er')
    ln('v' + n + 'pd', 'RM', 'V,V/m', 'V.66.0f.wig', c, '/r', 'lxy')
    ln('v' + n + 'pd', 'RM', 'V,V/m', 'E.66.0f.w1', c, '/r', 'lxyz', 'fv', 'b8', 'z', 'er')
    ln('v' + n + 'ss', 'RVM', 'x,x,x/m', 'V.f3.0f.wig', c, '/r', 'lig')
    ln('v' + n + 'ss', 'RVM', 'x,x,x/m', 'E.f3.0f.w0', c, '/r', 'lig', 'n4', 'z', 'er')
    ln('v' + n + 'sd', 'RVM', 'x,x,x/m', 'V.f2.0f.wig', c, '/r', 'lig')
    ln('v' + n + 'sd', 'RVM', 'x,x,x/m', 'E.f2.0f.w1', c, '/r', 'lig', 'n8', 'z', 'er')
# andps/andpd/orps/xorps/andnps (VEX; EVEX DQ)
for n, c in [('and', '54'), ('andn', '55'), ('or', '56'), ('xor', '57')]:
    ln('v' + n + 'ps', 'RVM', 'V,V,V/m', 'V.np.0f.wig', c, '/r', 'lxy')
    ln('v' + n + 'ps', 'RVM', 'V,V,V/m', 'E.np.0f.w0', c, '/r', 'lxyz', 'fv', 'b4', 'z')
    ln('v' + n + 'pd', 'RVM', 'V,V,V/m', 'V.66.0f.wig', c, '/r', 'lxy')
    ln('v' + n + 'pd', 'RVM', 'V,V,V/m', 'E.66.0f.w1', c, '/r', 'lxyz', 'fv', 'b8', 'z')
# unpck/movhl: VEX+EVEX
for n, c, pp in [('unpcklps', '14', 'np'), ('unpckhps', '15', 'np'), ('unpcklpd', '14', '66'),
                 ('unpckhpd', '15', '66')]:
    ew = 'w1' if pp == '66' else 'w0'
    bc = 8 if pp == '66' else 4
    ln('v' + n, 'RVM', 'V,V,V/m', 'V.%s.0f.wig' % pp, c, '/r', 'lxy')
    ln('v' + n, 'RVM', 'V,V,V/m', 'E.%s.0f.%s' % (pp, ew), c, '/r', 'lxyz', 'fv', 'b%d' % bc, 'z')

# cmpps/cmppd/cmpss/cmpsd with imm predicate (VEX+EVEX produce a mask for EVEX)
ln('vcmpps', 'RVMI', 'V,V,V/m,i', 'V.np.0f.wig', 'c2', '/r', 'lxy')
ln('vcmppd', 'RVMI', 'V,V,V/m,i', 'V.66.0f.wig', 'c2', '/r', 'lxy')
ln('vcmpss', 'RVMI', 'x,x,x/m,i', 'V.f3.0f.wig', 'c2', '/r', 'lig')
ln('vcmpsd', 'RVMI', 'x,x,x/m,i', 'V.f2.0f.wig', 'c2', '/r', 'lig')
ln('vcmpps', 'RVMI', 'k,V,V/m,i', 'E.np.0f.w0', 'c2', '/r', 'lxyz', 'fv', 'b4', 'sae')
ln('vcmppd', 'RVMI', 'k,V,V/m,i', 'E.66.0f.w1', 'c2', '/r', 'lxyz', 'fv', 'b8', 'sae')
ln('vcmpss', 'RVMI', 'k,x,x/m,i', 'E.f3.0f.w0', 'c2', '/r', 'lig', 'n4', 'sae')
ln('vcmpsd', 'RVMI', 'k,x,x/m,i', 'E.f2.0f.w1', 'c2', '/r', 'lig', 'n8', 'sae')

# comiss/ucomiss (scalar, set flags)
for n, c, pp in [('comiss', '2f', 'np'), ('ucomiss', '2e', 'np'), ('comisd', '2f', '66'), ('ucomisd', '2e', '66')]:
    ew = 'w1' if pp == '66' else 'w0'
    tn = 'n8' if pp == '66' else 'n4'
    ln('v' + n, 'RM', 'x,x/m', 'V.%s.0f.wig' % pp, c, '/r', 'lig')
    ln('v' + n, 'RM', 'x,x/m', 'E.%s.0f.%s' % (pp, ew), c, '/r', 'lig', tn, 'sae')

# shuffle/blend
ln('vshufps', 'RVMI', 'V,V,V/m,i', 'V.np.0f.wig', 'c6', '/r', 'lxy')
ln('vshufpd', 'RVMI', 'V,V,V/m,i', 'V.66.0f.wig', 'c6', '/r', 'lxy')
ln('vshufps', 'RVMI', 'V,V,V/m,i', 'E.np.0f.w0', 'c6', '/r', 'lxyz', 'fv', 'b4', 'z')
ln('vshufpd', 'RVMI', 'V,V,V/m,i', 'E.66.0f.w1', 'c6', '/r', 'lxyz', 'fv', 'b8', 'z')

# movaps/movups/movdqa/movdqu etc. (load and store)
MOV = [('movaps', 'np', '0f', '28', '29', 'fvm', None), ('movups', 'np', '0f', '10', '11', 'fvm', None),
       ('movapd', '66', '0f', '28', '29', 'fvm', 'w1'), ('movupd', '66', '0f', '10', '11', 'fvm', 'w1')]
for n, pp, mp, ld, st, tp, ew in MOV:
    ln('v' + n, 'RM', 'V,V/m', 'V.%s.%s.wig' % (pp, mp), ld, '/r', 'lxy')
    ln('v' + n, 'MR', 'V/m,V', 'V.%s.%s.wig' % (pp, mp), st, '/r', 'lxy')
    ewx = ew or 'w0'
    ln('v' + n, 'RM', 'V,V/m', 'E.%s.%s.%s' % (pp, mp, ewx), ld, '/r', 'lxyz', tp, 'z')
    ln('v' + n, 'MR', 'V/m,V', 'E.%s.%s.%s' % (pp, mp, ewx), st, '/r', 'lxyz', tp, 'z')
# movdqa/movdqu: EVEX split into 32/64
ln('vmovdqa', 'RM', 'V,V/m', 'V.66.0f.wig', '6f', '/r', 'lxy')
ln('vmovdqa', 'MR', 'V/m,V', 'V.66.0f.wig', '7f', '/r', 'lxy')
ln('vmovdqu', 'RM', 'V,V/m', 'V.f3.0f.wig', '6f', '/r', 'lxy')
ln('vmovdqu', 'MR', 'V/m,V', 'V.f3.0f.wig', '7f', '/r', 'lxy')
for suf, pp, ew in [('32', 'f3', 'w0'), ('64', 'f3', 'w1')]:
    ln('vmovdqu' + suf, 'RM', 'V,V/m', 'E.%s.0f.%s' % (pp, ew), '6f', '/r', 'lxyz', 'fvm', 'z')
    ln('vmovdqu' + suf, 'MR', 'V/m,V', 'E.%s.0f.%s' % (pp, ew), '7f', '/r', 'lxyz', 'fvm', 'z')
for suf, pp, ew in [('32', '66', 'w0'), ('64', '66', 'w1')]:
    ln('vmovdqa' + suf, 'RM', 'V,V/m', 'E.%s.0f.%s' % (pp, ew), '6f', '/r', 'lxyz', 'fvm', 'z')
    ln('vmovdqa' + suf, 'MR', 'V/m,V', 'E.%s.0f.%s' % (pp, ew), '7f', '/r', 'lxyz', 'fvm', 'z')
for suf, pp, ew in [('8', 'f2', 'w0'), ('16', 'f2', 'w1')]:
    ln('vmovdqu' + suf, 'RM', 'V,V/m', 'E.%s.0f.%s' % (pp, ew), '6f', '/r', 'lxyz', 'fvm', 'z')
    ln('vmovdqu' + suf, 'MR', 'V/m,V', 'E.%s.0f.%s' % (pp, ew), '7f', '/r', 'lxyz', 'fvm', 'z')

# broadcasts (AVX2 + AVX-512)
ln('vbroadcastss', 'RM', 'V,x/m', 'V.66.0f38.w0', '18', '/r', 'lxy')
ln('vbroadcastss', 'RM', 'V,x/m', 'E.66.0f38.w0', '18', '/r', 'lxyz', 'n4', 'z')
ln('vbroadcastsd', 'RM', 'y,x/m', 'V.66.0f38.w0', '19', '/r', 'ly')
ln('vbroadcastsd', 'RM', 'V,x/m', 'E.66.0f38.w1', '19', '/r', 'lyz', 'n8', 'z')
ln('vpbroadcastb', 'RM', 'V,x/m', 'V.66.0f38.w0', '78', '/r', 'lxy')
ln('vpbroadcastw', 'RM', 'V,x/m', 'V.66.0f38.w0', '79', '/r', 'lxy')
ln('vpbroadcastd', 'RM', 'V,x/m', 'V.66.0f38.w0', '58', '/r', 'lxy')
ln('vpbroadcastq', 'RM', 'V,x/m', 'V.66.0f38.w0', '59', '/r', 'lxy')
ln('vpbroadcastb', 'RM', 'V,x/m', 'E.66.0f38.w0', '78', '/r', 'lxyz', 'n1', 'z')
ln('vpbroadcastw', 'RM', 'V,x/m', 'E.66.0f38.w1', '79', '/r', 'lxyz', 'n2', 'z')
ln('vpbroadcastd', 'RM', 'V,x/m', 'E.66.0f38.w0', '58', '/r', 'lxyz', 'n4', 'z')
ln('vpbroadcastq', 'RM', 'V,x/m', 'E.66.0f38.w1', '59', '/r', 'lxyz', 'n8', 'z')

# -- FMA (VEX.66.0F38, EVEX) -------------------------------------------------
FMA_PS = {'132': '98', '213': 'a8', '231': 'b8'}  # ..., and 'add'/'sub' base
FMA_OPS = [('madd', 0), ('msub', 0x0a - 0x08), ('nmadd', 0x0c - 0x08), ('nmsub', 0x0e - 0x08),
           ('maddsub', 0x06 - 0x08), ('msubadd', 0x07 - 0x08)]
# base opcodes by order and width: 132->90s,213->a0s,231->b0s
ORD = {'132': 0x98, '213': 0xa8, '231': 0xb8}
OPADD = {'madd': 0, 'msub': 0x02, 'nmadd': 0x04, 'nmsub': 0x06, 'maddsub': -2, 'msubadd': -1}
for base, add in OPADD.items():
    for order, o in ORD.items():
        if base in ('maddsub', 'msubadd') and order:
            pass
        opc_ps = o + (add if add >= 0 else (0x06 if base == 'maddsub' else 0x07))
        # ps
        for suf, ew, bc, pp in [('ps', 'w0', 4, '66'), ('pd', 'w1', 8, '66')]:
            code = o + ({'madd': 0, 'msub': 2, 'nmadd': 4, 'nmsub': 6, 'maddsub': -2, 'msubadd': -1}[base])
            if base == 'maddsub':
                code = o + 6
            elif base == 'msubadd':
                code = o + 7
            nm = 'vf%s%s%s' % (base, order, suf)
            ln(nm, 'RVM', 'V,V,V/m', 'V.66.0f38.%s' % ('w0' if suf == 'ps' else 'w1'), '%02x' % code, '/r', 'lxy')
            ln(nm, 'RVM', 'V,V,V/m', 'E.66.0f38.%s' % ew, '%02x' % code, '/r', 'lxyz', 'fv', 'b%d' % bc, 'z', 'er')
        for suf, ew, pp, tn in [('ss', 'w0', '66', 'n4'), ('sd', 'w1', '66', 'n8')]:
            if base in ('maddsub', 'msubadd'):
                continue
            code = o + {'madd': 1, 'msub': 3, 'nmadd': 5, 'nmsub': 7}[base]
            nm = 'vf%s%s%s' % (base, order, suf)
            ln(nm, 'RVM', 'x,x,x/m', 'V.66.0f38.%s' % ew, '%02x' % code, '/r', 'lig')
            ln(nm, 'RVM', 'x,x,x/m', 'E.66.0f38.%s' % ew, '%02x' % code, '/r', 'lig', tn, 'z', 'er')

# -- F16C --------------------------------------------------------------------
ln('vcvtph2ps', 'RM', 'V,H/m', 'V.66.0f38.w0', '13', '/r', 'lxy')
ln('vcvtps2ph', 'MRI', 'H/m,V,i', 'V.66.0f3a.w0', '1d', '/r', 'lxy')
ln('vcvtph2ps', 'RM', 'V,H/m', 'E.66.0f38.w0', '13', '/r', 'lxyz', 'hvm', 'z', 'sae')
ln('vcvtps2ph', 'MRI', 'H/m,V,i', 'E.66.0f3a.w0', '1d', '/r', 'lxyz', 'hvm', 'z', 'sae')

# -- conversions (int<->float) ----------------------------------------------
ln('vcvtdq2ps', 'RM', 'V,V/m', 'V.np.0f.wig', '5b', '/r', 'lxy')
ln('vcvtdq2ps', 'RM', 'V,V/m', 'E.np.0f.w0', '5b', '/r', 'lxyz', 'fv', 'b4', 'z', 'er')
ln('vcvtps2dq', 'RM', 'V,V/m', 'V.66.0f.wig', '5b', '/r', 'lxy')
ln('vcvtps2dq', 'RM', 'V,V/m', 'E.66.0f.w0', '5b', '/r', 'lxyz', 'fv', 'b4', 'z', 'er')
ln('vcvttps2dq', 'RM', 'V,V/m', 'V.f3.0f.wig', '5b', '/r', 'lxy')
ln('vcvttps2dq', 'RM', 'V,V/m', 'E.f3.0f.w0', '5b', '/r', 'lxyz', 'fv', 'b4', 'z', 'sae')
ln('vcvtpd2ps', 'RM', 'V,V/m', 'V.66.0f.wig', '5a', '/r', 'lxy')
ln('vcvtps2pd', 'RM', 'V,H/m', 'V.np.0f.wig', '5a', '/r', 'lxy')
ln('vcvtps2pd', 'RM', 'V,H/m', 'E.np.0f.w0', '5a', '/r', 'lxyz', 'hv', 'b4', 'z', 'sae')
ln('vcvtsd2ss', 'RVM', 'x,x,x/m', 'V.f2.0f.wig', '5a', '/r', 'lig')
ln('vcvtss2sd', 'RVM', 'x,x,x/m', 'V.f3.0f.wig', '5a', '/r', 'lig')
ln('vcvtsd2ss', 'RVM', 'x,x,x/m', 'E.f2.0f.w1', '5a', '/r', 'lig', 'n8', 'z', 'er')
ln('vcvtss2sd', 'RVM', 'x,x,x/m', 'E.f3.0f.w0', '5a', '/r', 'lig', 'n4', 'z', 'sae')
ln('vcvtsi2ss', 'RVM', 'x,x,r/m', 'V.f3.0f.wr', '2a', '/r', 'lig')
ln('vcvtsi2sd', 'RVM', 'x,x,r/m', 'V.f2.0f.wr', '2a', '/r', 'lig')
ln('vcvtss2si', 'RM', 'r,x/m', 'V.f3.0f.wr', '2d', '/r', 'lig')
ln('vcvtsd2si', 'RM', 'r,x/m', 'V.f2.0f.wr', '2d', '/r', 'lig')
ln('vcvttss2si', 'RM', 'r,x/m', 'V.f3.0f.wr', '2c', '/r', 'lig')
ln('vcvttsd2si', 'RM', 'r,x/m', 'V.f2.0f.wr', '2c', '/r', 'lig')

# -- movss/movsd scalar moves -----------------------------------------------
ln('vmovss', 'RVM', 'x,x,x', 'V.f3.0f.wig', '10', '/r', 'lig')
ln('vmovss', 'MVR', 'x,x,x', 'V.f3.0f.wig', '11', '/r', 'lig')
ln('vmovss', 'RM', 'x,m', 'V.f3.0f.wig', '10', '/r', 'lig')
ln('vmovss', 'MR', 'm,x', 'V.f3.0f.wig', '11', '/r', 'lig')
ln('vmovsd', 'RVM', 'x,x,x', 'V.f2.0f.wig', '10', '/r', 'lig')
ln('vmovsd', 'MVR', 'x,x,x', 'V.f2.0f.wig', '11', '/r', 'lig')
ln('vmovsd', 'RM', 'x,m', 'V.f2.0f.wig', '10', '/r', 'lig')
ln('vmovsd', 'MR', 'm,x', 'V.f2.0f.wig', '11', '/r', 'lig')

# movd/movq via VEX+EVEX
ln('vmovd', 'RM', 'x,r32/m', 'V.66.0f.w0', '6e', '/r', 'lig')
ln('vmovd', 'MR', 'r32/m,x', 'V.66.0f.w0', '7e', '/r', 'lig')
ln('vmovq', 'RM', 'x,r64/m', 'V.66.0f.w1', '6e', '/r', 'lig')
ln('vmovq', 'MR', 'r64/m,x', 'V.66.0f.w1', '7e', '/r', 'lig')
ln('vmovq', 'RM', 'x,x/m', 'V.f3.0f.wig', '7e', '/r', 'lig')
ln('vmovq', 'MR', 'x/m,x', 'V.66.0f.wig', 'd6', '/r', 'lig')
ln('vmovd', 'RM', 'x,r32/m', 'E.66.0f.w0', '6e', '/r', 'lig', 'n4')
ln('vmovd', 'MR', 'r32/m,x', 'E.66.0f.w0', '7e', '/r', 'lig', 'n4')
ln('vmovq', 'RM', 'x,r64/m', 'E.66.0f.w1', '6e', '/r', 'lig', 'n8')
ln('vmovq', 'MR', 'r64/m,x', 'E.66.0f.w1', '7e', '/r', 'lig', 'n8')

# pmovmskb, movmskps/pd
ln('vpmovmskb', 'RM', 'r32/r64,V', 'V.66.0f.wig', 'd7', '/r', 'lxy')
ln('vmovmskps', 'RM', 'r32/r64,V', 'V.np.0f.wig', '50', '/r', 'lxy')
ln('vmovmskpd', 'RM', 'r32/r64,V', 'V.66.0f.wig', '50', '/r', 'lxy')

# pshufd/pshuflw/pshufhw
ln('vpshufd', 'RMI', 'V,V/m,i', 'V.66.0f.wig', '70', '/r', 'lxy')
ln('vpshufd', 'RMI', 'V,V/m,i', 'E.66.0f.w0', '70', '/r', 'lxyz', 'fv', 'b4', 'z')
ln('vpshuflw', 'RMI', 'V,V/m,i', 'V.f2.0f.wig', '70', '/r', 'lxy')
ln('vpshufhw', 'RMI', 'V,V/m,i', 'V.f3.0f.wig', '70', '/r', 'lxy')
ln('vpshufb', 'RVM', 'V,V,V/m', 'V.66.0f38.wig', '00', '/r', 'lxy')
ln('vpshufb', 'RVM', 'V,V,V/m', 'E.66.0f38.w0', '00', '/r', 'lxyz', 'fvm', 'z')

# pslld/psrld/psraw etc. immediate forms (VEX: vvvv is dst; EVEX too)
for n, c, d in [('psrlw', '71', 2), ('psraw', '71', 4), ('psllw', '71', 6), ('psrld', '72', 2),
                ('psrad', '72', 4), ('pslld', '72', 6), ('psrlq', '73', 2), ('psllq', '73', 6),
                ('psrldq', '73', 3), ('pslldq', '73', 7)]:
    ln('v' + n, 'VMI', 'V,V/m,i', 'V.66.0f.wig', c, '/%d' % d, 'lxy')
# shift by xmm
for n, c in [('psrlw', 'd1'), ('psrld', 'd2'), ('psrlq', 'd3'), ('psraw', 'e1'), ('psrad', 'e2'),
             ('psllw', 'f1'), ('pslld', 'f2'), ('psllq', 'f3')]:
    ln('v' + n, 'RVM', 'V,V,x/m', 'V.66.0f.wig', c, '/r', 'lxy')

# insert/extract
ln('vpinsrb', 'RVMI', 'x,x,r32/m,i', 'V.66.0f3a.w0', '20', '/r', 'lig')
ln('vpinsrw', 'RVMI', 'x,x,r32/m,i', 'V.66.0f.w0', 'c4', '/r', 'lig')
ln('vpinsrd', 'RVMI', 'x,x,r32/m,i', 'V.66.0f3a.w0', '22', '/r', 'lig')
ln('vpinsrq', 'RVMI', 'x,x,r64/m,i', 'V.66.0f3a.w1', '22', '/r', 'lig')
ln('vpextrb', 'MRI', 'r32/m,x,i', 'V.66.0f3a.w0', '14', '/r', 'lig')
ln('vpextrw', 'RMI', 'r32,x,i', 'V.66.0f.w0', 'c5', '/r', 'lig')
ln('vpextrd', 'MRI', 'r32/m,x,i', 'V.66.0f3a.w0', '16', '/r', 'lig')
ln('vpextrq', 'MRI', 'r64/m,x,i', 'V.66.0f3a.w1', '16', '/r', 'lig')
ln('vinsertps', 'RVMI', 'x,x,x/m,i', 'V.66.0f3a.wig', '21', '/r', 'lig')
ln('vextractps', 'MRI', 'r32/m,x,i', 'V.66.0f3a.wig', '17', '/r', 'lig')

# blend/blendv (SSE4.1 handled by sse() in x64.ax for legacy; add VEX)
ln('vblendps', 'RVMI', 'V,V,V/m,i', 'V.66.0f3a.wig', '0c', '/r', 'lxy')
ln('vblendpd', 'RVMI', 'V,V,V/m,i', 'V.66.0f3a.wig', '0d', '/r', 'lxy')
ln('vpblendw', 'RVMI', 'V,V,V/m,i', 'V.66.0f3a.wig', '0e', '/r', 'lxy')
ln('vpblendd', 'RVMI', 'V,V,V/m,i', 'V.66.0f3a.w0', '02', '/r', 'lxy')
ln('vblendvps', 'RVMS', 'V,V,V/m,V', 'V.66.0f3a.w0', '4a', '/r', 'lxy')
ln('vblendvpd', 'RVMS', 'V,V,V/m,V', 'V.66.0f3a.w0', '4b', '/r', 'lxy')
ln('vpblendvb', 'RVMS', 'V,V,V/m,V', 'V.66.0f3a.w0', '4c', '/r', 'lxy')
ln('vdpps', 'RVMI', 'V,V,V/m,i', 'V.66.0f3a.wig', '40', '/r', 'lxy')
ln('vdppd', 'RVMI', 'x,x,x/m,i', 'V.66.0f3a.wig', '41', '/r', 'lig')

# perm (AVX2/AVX-512)
ln('vpermq', 'RMI', 'y,y/m,i', 'V.66.0f3a.w1', '00', '/r', 'ly')
ln('vpermpd', 'RMI', 'y,y/m,i', 'V.66.0f3a.w1', '01', '/r', 'ly')
ln('vperm2f128', 'RVMI', 'y,y,y/m,i', 'V.66.0f3a.w0', '06', '/r', 'ly')
ln('vperm2i128', 'RVMI', 'y,y,y/m,i', 'V.66.0f3a.w0', '46', '/r', 'ly')
ln('vpermd', 'RVM', 'y,y,y/m', 'V.66.0f38.w0', '36', '/r', 'ly')
ln('vpermps', 'RVM', 'y,y,y/m', 'V.66.0f38.w0', '16', '/r', 'ly')
ln('vpermilps', 'RVM', 'V,V,V/m', 'V.66.0f38.w0', '0c', '/r', 'lxy')
ln('vpermilpd', 'RVM', 'V,V,V/m', 'V.66.0f38.w0', '0d', '/r', 'lxy')
ln('vpermilps', 'RMI', 'V,V/m,i', 'V.66.0f3a.w0', '04', '/r', 'lxy')
ln('vpermilpd', 'RMI', 'V,V/m,i', 'V.66.0f3a.w0', '05', '/r', 'lxy')

# insert/extract 128
ln('vinsertf128', 'RVMI', 'y,y,x/m,i', 'V.66.0f3a.w0', '18', '/r', 'ly')
ln('vextractf128', 'MRI', 'x/m,y,i', 'V.66.0f3a.w0', '19', '/r', 'ly')
ln('vinserti128', 'RVMI', 'y,y,x/m,i', 'V.66.0f3a.w0', '38', '/r', 'ly')
ln('vextracti128', 'MRI', 'x/m,y,i', 'V.66.0f3a.w0', '39', '/r', 'ly')

# AVX-512 insert/extract
for nm, opc, w, tp in [('vinsertf32x4', '18', 'w0', 'n16'), ('vinsertf64x2', '18', 'w1', 'n16'),
                       ('vinserti32x4', '38', 'w0', 'n16'), ('vinserti64x2', '38', 'w1', 'n16')]:
    ln(nm, 'RVMI', 'V,V,x/m,i', 'E.66.0f3a.%s' % w, opc, '/r', 'lyz', tp, 'z')
for nm, opc, w, tp in [('vinsertf32x8', '1a', 'w0', 'n32'), ('vinsertf64x4', '1a', 'w1', 'n32'),
                       ('vinserti32x8', '3a', 'w0', 'n32'), ('vinserti64x4', '3a', 'w1', 'n32')]:
    ln(nm, 'RVMI', 'z,z,y/m,i', 'E.66.0f3a.%s' % w, opc, '/r', 'lz', tp, 'z')
for nm, opc, w, tp in [('vextractf32x4', '19', 'w0', 'n16'), ('vextractf64x2', '19', 'w1', 'n16'),
                       ('vextracti32x4', '39', 'w0', 'n16'), ('vextracti64x2', '39', 'w1', 'n16')]:
    ln(nm, 'MRI', 'x/m,V,i', 'E.66.0f3a.%s' % w, opc, '/r', 'lyz', tp, 'z')
for nm, opc, w, tp in [('vextractf32x8', '1b', 'w0', 'n32'), ('vextractf64x4', '1b', 'w1', 'n32'),
                       ('vextracti32x8', '3b', 'w0', 'n32'), ('vextracti64x4', '3b', 'w1', 'n32')]:
    ln(nm, 'MRI', 'y/m,z,i', 'E.66.0f3a.%s' % w, opc, '/r', 'lz', tp, 'z')

# AVX-512 ternary logic, blends by mask, compares to mask
ln('vpternlogd', 'RVMI', 'V,V,V/m,i', 'E.66.0f3a.w0', '25', '/r', 'lxyz', 'fv', 'b4', 'z')
ln('vpternlogq', 'RVMI', 'V,V,V/m,i', 'E.66.0f3a.w1', '25', '/r', 'lxyz', 'fv', 'b8', 'z')
for suf, ew, bc, pp in [('d', 'w0', 4, '66'), ('q', 'w1', 8, '66')]:
    ln('vpcmp' + suf, 'RVMI', 'k,V,V/m,i', 'E.66.0f3a.%s' % ew, '1f', '/r', 'lxyz', 'fv', 'b%d' % bc)
    ln('vpcmpu' + suf, 'RVMI', 'k,V,V/m,i', 'E.66.0f3a.%s' % ew, '1e', '/r', 'lxyz', 'fv', 'b%d' % bc)
ln('vpcmpb', 'RVMI', 'k,V,V/m,i', 'E.66.0f3a.w0', '3f', '/r', 'lxyz', 'fvm')
ln('vpcmpw', 'RVMI', 'k,V,V/m,i', 'E.66.0f3a.w1', '3f', '/r', 'lxyz', 'fvm')
ln('vpcmpub', 'RVMI', 'k,V,V/m,i', 'E.66.0f3a.w0', '3e', '/r', 'lxyz', 'fvm')
ln('vpcmpuw', 'RVMI', 'k,V,V/m,i', 'E.66.0f3a.w1', '3e', '/r', 'lxyz', 'fvm')

# mask register ops (AVX-512 F/BW/DQ)
for suf, pp, w in [('b', '66', 'w0'), ('w', 'np', 'w0'), ('d', '66', 'w1'), ('q', 'np', 'w1')]:
    ln('kmov' + suf, 'RM', 'k,k/m', 'V.%s.0f.%s' % (pp, w), '90', '/r', 'lz')
    ln('kmov' + suf, 'MR', 'k/m,k', 'V.%s.0f.%s' % (pp, w), '91', '/r', 'lz')
ln('kmovb', 'RM', 'k,r32', 'V.66.0f.w0', '92', '/r', 'lz')
ln('kmovw', 'RM', 'k,r32', 'V.np.0f.w0', '92', '/r', 'lz')
ln('kmovd', 'RM', 'k,r32', 'V.f2.0f.w0', '92', '/r', 'lz')
ln('kmovq', 'RM', 'k,r64', 'V.f2.0f.w1', '92', '/r', 'lz')
ln('kmovb', 'RM', 'r32,k', 'V.66.0f.w0', '93', '/r', 'lz')
ln('kmovw', 'RM', 'r32,k', 'V.np.0f.w0', '93', '/r', 'lz')
ln('kmovd', 'RM', 'r32,k', 'V.f2.0f.w0', '93', '/r', 'lz')
ln('kmovq', 'RM', 'r64,k', 'V.f2.0f.w1', '93', '/r', 'lz')
for suf, pp, w in [('b', '66', 'w0'), ('w', 'np', 'w0'), ('d', '66', 'w1'), ('q', 'np', 'w1')]:
    for op, c in [('kand', '41'), ('kandn', '42'), ('kor', '45'), ('kxnor', '46'), ('kxor', '47'),
                  ('knot', '44'), ('kadd', '4a'), ('kortest', '98'), ('ktest', '99')]:
        if op in ('knot', 'kortest', 'ktest'):
            ln(op + suf, 'RM', 'k,k', 'V.%s.0f.%s' % (pp, w), c, '/r', 'lz')
        else:
            ln(op + suf, 'RVM', 'k,k,k', 'V.%s.0f.%s' % (pp, w), c, '/r', 'ly')

# -- system / privileged (legacy; plain encodings) ---------------------------
none('cli', 'L.np.-.w0', 'fa')
none('sti', 'L.np.-.w0', 'fb')
none('hlt', 'L.np.-.w0', 'f4')
none('swapgs', 'L.np.0f.w0', '01 f8')
none('iret', 'L.np.-.w0', 'cf')
none('iretq', 'L.np.-.w1', 'cf')
none('iretd', 'L.np.-.w0', 'cf')
none('rsm', 'L.np.0f.w0', 'aa')
none('wrmsr', 'L.np.0f.w0', '30')
none('rdmsr', 'L.np.0f.w0', '32')
none('wbinvd', 'L.np.0f.w0', '09')
none('invd', 'L.np.0f.w0', '08')
none('monitor', 'L.np.0f.w0', '01 c8')
none('mwait', 'L.np.0f.w0', '01 c9')
none('clts', 'L.np.0f.w0', '06')
none('sysret', 'L.np.0f.w0', '07')
none('sysretq', 'L.np.0f.w1', '07')
none('sysenter', 'L.np.0f.w0', '34')
none('sysexit', 'L.np.0f.w0', '35')
none('rdpid', '', '')  # handled in x64.ax (removed below)
L[:] = [x for x in L if not x.startswith('rdpid ')]
for n, d in [('invlpg', 7), ('lgdt', 2), ('lidt', 3), ('sgdt', 0), ('sidt', 1), ('lldt', None),
             ('sldt', None), ('ltr', 3), ('str', 1), ('lmsw', 6), ('smsw', 4), ('verr', 4), ('verw', 5)]:
    if n in ('invlpg', 'lgdt', 'lidt', 'sgdt', 'sidt', 'lmsw', 'smsw'):
        ln(n, 'M', 'm', 'L.np.0f.w0', '01', '/%d' % d)
for n, d in [('lldt', 2), ('sldt', 0), ('ltr', 3), ('str', 1), ('verr', 4), ('verw', 5)]:
    ln(n, 'M', 'r16/m', 'L.np.0f.w0', '00', '/%d' % d)
# mov to/from control and debug registers
ln('mov', 'MR', 'r64,cr', 'L.np.0f.w0', '20', '/r')
ln('mov', 'RM', 'cr,r64', 'L.np.0f.w0', '22', '/r')
ln('mov', 'MR', 'r64,dr', 'L.np.0f.w0', '21', '/r')
ln('mov', 'RM', 'dr,r64', 'L.np.0f.w0', '23', '/r')
# -- control-flow extensions -------------------------------------------------
none('endbr32', 'L.f3.0f.w0', '1e fb')
none('cldemote', '', '')
L[:] = [x for x in L if not x.startswith('cldemote ')]
ln('cldemote', 'M', 'm', 'L.np.0f.w0', '1c', '/0')
ln('clrssbsy', 'M', 'm', 'L.f3.0f.w0', 'ae', '/6')
ln('wrssd', 'MR', 'm,r32', 'L.np.0f38.w0', 'f6', '/r')
ln('wrssq', 'MR', 'm,r64', 'L.np.0f38.w1', 'f6', '/r')
ln('wrussd', 'MR', 'm,r32', 'L.66.0f38.w0', 'f5', '/r')
ln('wrussq', 'MR', 'm,r64', 'L.66.0f38.w1', 'f5', '/r')
ln('rstorssp', 'M', 'm', 'L.f3.0f.w0', '01', '/5')
none('setssbsy', 'L.f3.0f.w0', '01 e8')
none('saveprevssp', 'L.f3.0f.w0', '01 ea')
