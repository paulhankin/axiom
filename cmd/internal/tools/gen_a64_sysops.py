#!/usr/bin/env python3
"""Generates cmd/axiom/src/asm/a64sysop.ax, the AArch64 assembler's table of
the operations of the SYS aliases (AT, DC, IC, TLBI and TLBIP, the
prediction-restriction and trace aliases, and BRB), from the lists below, written
from the Arm Architecture Reference Manual's "A64 System instructions"
encoding tables (each operation's op1, CRn, CRm, op2):

    python3 cmd/internal/tools/gen_a64_sysops.py > cmd/axiom/src/asm/a64sysop.ax && axiom fmt cmd/axiom/src/asm/a64sysop.ax

A table entry is `alias.operation`; its value holds op1<16:14> CRn<13:10>
CRm<9:6> op2<5:3> (the 14 bits SYS holds from bit 5, shifted by 3) and, in
its low bits, how the register operand is used: 0 none, 1 required, and
bit 2 set when TLBIP takes the operation too (with a register pair).
"""

NONE, REG, PAIR = 0, 1, 4
OPS = {}


def op(alias, name, op1, crn, crm, op2, how):
    OPS['%s.%s' % (alias, name)] = ((op1 << 11) | (crn << 7) | (crm << 3) | op2, how)


# AT: address translation (CRn 7; CRm 8 and 9)
for name, op1, crm, op2 in [
        ('s1e1r', 0, 8, 0), ('s1e1w', 0, 8, 1), ('s1e0r', 0, 8, 2), ('s1e0w', 0, 8, 3),
        ('s1e1rp', 0, 9, 0), ('s1e1wp', 0, 9, 1), ('s1e1a', 0, 9, 2),
        ('s1e2r', 4, 8, 0), ('s1e2w', 4, 8, 1), ('s12e1r', 4, 8, 4), ('s12e1w', 4, 8, 5),
        ('s12e0r', 4, 8, 6), ('s12e0w', 4, 8, 7), ('s1e2a', 4, 9, 2),
        ('s1e3r', 6, 8, 0), ('s1e3w', 6, 8, 1), ('s1e3a', 6, 9, 2)]:
    op('at', name, op1, 7, crm, op2, REG)

# DC: data cache maintenance (CRn 7), with the allocation-tag forms of FEAT_MTE
for name, op1, crm, op2 in [
        ('ivac', 0, 6, 1), ('isw', 0, 6, 2), ('igvac', 0, 6, 3), ('igsw', 0, 6, 4),
        ('igdvac', 0, 6, 5), ('igdsw', 0, 6, 6),
        ('csw', 0, 10, 2), ('cgsw', 0, 10, 4), ('cgdsw', 0, 10, 6),
        ('cisw', 0, 14, 2), ('cigsw', 0, 14, 4), ('cigdsw', 0, 14, 6),
        ('zva', 3, 4, 1), ('gva', 3, 4, 3), ('gzva', 3, 4, 4),
        ('cvac', 3, 10, 1), ('cgvac', 3, 10, 3), ('cgdvac', 3, 10, 5),
        ('cvau', 3, 11, 1),
        ('cvap', 3, 12, 1), ('cgvap', 3, 12, 3), ('cgdvap', 3, 12, 5),
        ('cvadp', 3, 13, 1), ('cgvadp', 3, 13, 3), ('cgdvadp', 3, 13, 5),
        ('civac', 3, 14, 1), ('cigvac', 3, 14, 3), ('cigdvac', 3, 14, 5),
        ('cipae', 4, 14, 0), ('cigdpae', 4, 14, 7),
        ('cipapa', 6, 14, 1), ('cigdpapa', 6, 14, 5)]:
    op('dc', name, op1, 7, crm, op2, REG)

# IC: instruction cache maintenance
op('ic', 'ialluis', 0, 7, 1, 0, NONE)
op('ic', 'iallu', 0, 7, 5, 0, NONE)
op('ic', 'ivau', 3, 7, 5, 1, REG)

# TLBI (CRn 8; the FEAT_XS nXS forms have CRn 9). The shareability is in
# CRm: 3 inner shareable (is), 1 outer (os), 7 neither; the range forms
# (FEAT_TLBIRANGE) have 2 (is), 5 (os), 6. (name, op1, op2, register use,
# whether a range form exists)
BY_VA = [
    # EL1&0
    ('vmalle1', 0, 0, NONE, False), ('vae1', 0, 1, REG | PAIR, True), ('aside1', 0, 2, REG, False),
    ('vaae1', 0, 3, REG | PAIR, True), ('vale1', 0, 5, REG | PAIR, True), ('vaale1', 0, 7, REG | PAIR, True),
    # EL2
    ('alle2', 4, 0, NONE, False), ('vae2', 4, 1, REG | PAIR, True), ('alle1', 4, 4, NONE, False),
    ('vale2', 4, 5, REG | PAIR, True), ('vmalls12e1', 4, 6, NONE, False),
    # EL3
    ('alle3', 6, 0, NONE, False), ('vae3', 6, 1, REG | PAIR, True), ('vale3', 6, 5, REG | PAIR, True),
]
TLBI = {}
for name, op1, op2, how, rng in BY_VA:
    for suffix, crm, rcrm in (('is', 3, 2), ('os', 1, 5), ('', 7, 6)):
        TLBI[name + suffix] = (op1, crm, op2, how)
        if rng:
            TLBI['r' + name + suffix] = (op1, rcrm, op2, how)
# stage 2 by IPA (CRm 0 inner shareable, 4 the others)
for name, crm, op2 in [
        ('ipas2e1is', 0, 1), ('ripas2e1is', 0, 2), ('ipas2le1is', 0, 5), ('ripas2le1is', 0, 6),
        ('ipas2e1os', 4, 0), ('ipas2e1', 4, 1), ('ripas2e1', 4, 2), ('ripas2e1os', 4, 3),
        ('ipas2le1os', 4, 4), ('ipas2le1', 4, 5), ('ripas2le1', 4, 6), ('ripas2le1os', 4, 7)]:
    TLBI[name] = (4, crm, op2, REG | PAIR)
# the granule-protection-table forms (FEAT_RME)
TLBI['paall'] = (6, 7, 4, NONE)
TLBI['paallos'] = (6, 1, 4, NONE)
TLBI['rpaos'] = (6, 4, 3, REG)
TLBI['rpalos'] = (6, 4, 7, REG)
# the stage-2 invalidations without the stage-1 entries (FEAT_TLBIW), in the
# range forms' CRm (no range of their own)
for suffix, crm in (('is', 2), ('os', 5), ('', 6)):
    TLBI['vmallws2e1' + suffix] = (4, crm, 2, NONE)
# each has its nXS form (FEAT_XS: CRn 9)
for name, (op1, crm, op2, how) in TLBI.items():
    op('tlbi', name, op1, 8, crm, op2, how)
    op('tlbi', name + 'nxs', op1, 9, crm, op2, how)

# the prediction restriction instructions (FEAT_SPECRES): cfp, dvp, cosp,
# cpp rctx, Xt (SYS #3, C7, C3, #4..7), and trcit Xt (FEAT_ITE)
for name, op2 in (('cfp', 4), ('dvp', 5), ('cosp', 6), ('cpp', 7)):
    op(name, 'rctx', 3, 7, 3, op2, REG)

# the branch record buffer's (FEAT_BRBE): brb iall, brb inj (SYS #1, C7, C2, #4 and #5)
op('brb', 'iall', 1, 7, 2, 4, NONE)
op('brb', 'inj', 1, 7, 2, 5, NONE)

names = sorted(OPS)
print('''// =============================================================================
// asm/a64sysop.ax - the operations of the AArch64 assembler's SYS aliases (at,
// dc, ic, tlbi, tlbip, cfp, dvp, cosp, cpp, brb) as `alias.operation`, with the
// value op1:CRn:CRm:op2 << 3 | how the register operand is used (0 none, 1
// required; bit 2: tlbip takes it too). Generated from the Arm Architecture
// Reference Manual's System instruction encoding tables by
// cmd/internal/tools/gen_a64_sysops.py: do not edit.
// =============================================================================''')
print('pub let NAMES: [%d][]u8 = [' % len(names))
for n in names:
    print('    "%s",' % n)
print('];')
print('pub let ENC: [%d]u32 = [' % len(names))
vals = ['0x%05x' % ((OPS[n][0] << 3) | OPS[n][1]) for n in names]
for i in range(0, len(vals), 10):
    print('    ' + ', '.join(vals[i:i + 10]) + ',')
print('];')
