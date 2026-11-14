#!/usr/bin/env python3
"""Generates cmd/axiom/src/asm/a64sysreg.ax, the AArch64 assembler's table of
named system registers for MRS and MSR (and MRRS and MSRR), from the register
list below: each register's name, its op0, op1, CRn, CRm and op2, and whether
it may be read (r) and written (w), and whether it is a 128-bit register
(p: MRRS and MSRR name only those), as the Arm Architecture Reference
Manual's "System Register Encoding" tables and register descriptions give
them:

    python3 cmd/internal/tools/gen_a64_sysregs.py > cmd/axiom/src/asm/a64sysreg.ax && axiom fmt cmd/axiom/src/asm/a64sysreg.ax

The list was checked encoding by encoding (all 32768 of op0 2 and 3, read
and written) against the reference disassemblers of the test host: each name
is the one they print for its encoding, and a register only read (an ID
register, a status) or only written (OSLAR_EL1, the ICC_SGI registers) is
named only in that direction.

The assembler also accepts the generic S<op0>_<op1>_C<n>_C<m>_<op2> form, so
this table is only the names; a register it does not list is still reachable
by that form.  The value stored is op0<14:14> op1<13:11> CRn<10:7> CRm<6:3>
op2<2:0> with op0 reduced by 2 (as MRS/MSR encode it from bit 5), with bit 15
set when the register may be read, bit 16 when it may be written and bit 17
when it is a 128-bit register.
"""

# name op0 op1 CRn CRm op2 directions, three to a line
LIST = """
    accdata_el1 3 0 13 0 5 rw | actlr_el1 3 0 1 0 1 rw | actlr_el2 3 4 1 0 1 rw
    actlr_el3 3 6 1 0 1 rw | afsr0_el1 3 0 5 1 0 rw | afsr0_el12 3 5 5 1 0 rw
    afsr0_el2 3 4 5 1 0 rw | afsr0_el3 3 6 5 1 0 rw | afsr1_el1 3 0 5 1 1 rw
    afsr1_el12 3 5 5 1 1 rw | afsr1_el2 3 4 5 1 1 rw | afsr1_el3 3 6 5 1 1 rw
    aidr_el1 3 1 0 0 7 r | allint 3 0 4 3 0 rw | amair2_el1 3 0 10 3 1 rw
    amair2_el12 3 5 10 3 1 rw | amair2_el2 3 4 10 3 1 rw | amair2_el3 3 6 10 3 1 rw
    amair_el1 3 0 10 3 0 rw | amair_el12 3 5 10 3 0 rw | amair_el2 3 4 10 3 0 rw
    amair_el3 3 6 10 3 0 rw | amcfgr_el0 3 3 13 2 1 r | amcg1idr_el0 3 3 13 2 6 r
    amcgcr_el0 3 3 13 2 2 r | amcntenclr0_el0 3 3 13 2 4 rw | amcntenclr1_el0 3 3 13 3 0 rw
    amcntenset0_el0 3 3 13 2 5 rw | amcntenset1_el0 3 3 13 3 1 rw | amcr_el0 3 3 13 2 0 rw
    amevcntr00_el0 3 3 13 4 0 rw | amevcntr01_el0 3 3 13 4 1 rw | amevcntr02_el0 3 3 13 4 2 rw
    amevcntr03_el0 3 3 13 4 3 rw | amevcntr10_el0 3 3 13 12 0 rw | amevcntr110_el0 3 3 13 13 2 rw
    amevcntr111_el0 3 3 13 13 3 rw | amevcntr112_el0 3 3 13 13 4 rw | amevcntr113_el0 3 3 13 13 5 rw
    amevcntr114_el0 3 3 13 13 6 rw | amevcntr115_el0 3 3 13 13 7 rw | amevcntr11_el0 3 3 13 12 1 rw
    amevcntr12_el0 3 3 13 12 2 rw | amevcntr13_el0 3 3 13 12 3 rw | amevcntr14_el0 3 3 13 12 4 rw
    amevcntr15_el0 3 3 13 12 5 rw | amevcntr16_el0 3 3 13 12 6 rw | amevcntr17_el0 3 3 13 12 7 rw
    amevcntr18_el0 3 3 13 13 0 rw | amevcntr19_el0 3 3 13 13 1 rw | amevcntvoff00_el2 3 4 13 8 0 rw
    amevcntvoff010_el2 3 4 13 9 2 rw | amevcntvoff011_el2 3 4 13 9 3 rw | amevcntvoff012_el2 3 4 13 9 4 rw
    amevcntvoff013_el2 3 4 13 9 5 rw | amevcntvoff014_el2 3 4 13 9 6 rw | amevcntvoff015_el2 3 4 13 9 7 rw
    amevcntvoff01_el2 3 4 13 8 1 rw | amevcntvoff02_el2 3 4 13 8 2 rw | amevcntvoff03_el2 3 4 13 8 3 rw
    amevcntvoff04_el2 3 4 13 8 4 rw | amevcntvoff05_el2 3 4 13 8 5 rw | amevcntvoff06_el2 3 4 13 8 6 rw
    amevcntvoff07_el2 3 4 13 8 7 rw | amevcntvoff08_el2 3 4 13 9 0 rw | amevcntvoff09_el2 3 4 13 9 1 rw
    amevcntvoff10_el2 3 4 13 10 0 rw | amevcntvoff110_el2 3 4 13 11 2 rw | amevcntvoff111_el2 3 4 13 11 3 rw
    amevcntvoff112_el2 3 4 13 11 4 rw | amevcntvoff113_el2 3 4 13 11 5 rw | amevcntvoff114_el2 3 4 13 11 6 rw
    amevcntvoff115_el2 3 4 13 11 7 rw | amevcntvoff11_el2 3 4 13 10 1 rw | amevcntvoff12_el2 3 4 13 10 2 rw
    amevcntvoff13_el2 3 4 13 10 3 rw | amevcntvoff14_el2 3 4 13 10 4 rw | amevcntvoff15_el2 3 4 13 10 5 rw
    amevcntvoff16_el2 3 4 13 10 6 rw | amevcntvoff17_el2 3 4 13 10 7 rw | amevcntvoff18_el2 3 4 13 11 0 rw
    amevcntvoff19_el2 3 4 13 11 1 rw | amevtyper00_el0 3 3 13 6 0 r | amevtyper01_el0 3 3 13 6 1 r
    amevtyper02_el0 3 3 13 6 2 r | amevtyper03_el0 3 3 13 6 3 r | amevtyper10_el0 3 3 13 14 0 rw
    amevtyper110_el0 3 3 13 15 2 rw | amevtyper111_el0 3 3 13 15 3 rw | amevtyper112_el0 3 3 13 15 4 rw
    amevtyper113_el0 3 3 13 15 5 rw | amevtyper114_el0 3 3 13 15 6 rw | amevtyper115_el0 3 3 13 15 7 rw
    amevtyper11_el0 3 3 13 14 1 rw | amevtyper12_el0 3 3 13 14 2 rw | amevtyper13_el0 3 3 13 14 3 rw
    amevtyper14_el0 3 3 13 14 4 rw | amevtyper15_el0 3 3 13 14 5 rw | amevtyper16_el0 3 3 13 14 6 rw
    amevtyper17_el0 3 3 13 14 7 rw | amevtyper18_el0 3 3 13 15 0 rw | amevtyper19_el0 3 3 13 15 1 rw
    amuserenr_el0 3 3 13 2 3 rw | apdakeyhi_el1 3 0 2 2 1 rw | apdakeylo_el1 3 0 2 2 0 rw
    apdbkeyhi_el1 3 0 2 2 3 rw | apdbkeylo_el1 3 0 2 2 2 rw | apgakeyhi_el1 3 0 2 3 1 rw
    apgakeylo_el1 3 0 2 3 0 rw | apiakeyhi_el1 3 0 2 1 1 rw | apiakeylo_el1 3 0 2 1 0 rw
    apibkeyhi_el1 3 0 2 1 3 rw | apibkeylo_el1 3 0 2 1 2 rw | brbcr_el1 2 1 9 0 0 rw
    brbcr_el12 2 5 9 0 0 rw | brbcr_el2 2 4 9 0 0 rw | brbfcr_el1 2 1 9 0 1 rw
    brbidr0_el1 2 1 9 2 0 r | brbinf0_el1 2 1 8 0 0 r | brbinf10_el1 2 1 8 10 0 r
    brbinf11_el1 2 1 8 11 0 r | brbinf12_el1 2 1 8 12 0 r | brbinf13_el1 2 1 8 13 0 r
    brbinf14_el1 2 1 8 14 0 r | brbinf15_el1 2 1 8 15 0 r | brbinf16_el1 2 1 8 0 4 r
    brbinf17_el1 2 1 8 1 4 r | brbinf18_el1 2 1 8 2 4 r | brbinf19_el1 2 1 8 3 4 r
    brbinf1_el1 2 1 8 1 0 r | brbinf20_el1 2 1 8 4 4 r | brbinf21_el1 2 1 8 5 4 r
    brbinf22_el1 2 1 8 6 4 r | brbinf23_el1 2 1 8 7 4 r | brbinf24_el1 2 1 8 8 4 r
    brbinf25_el1 2 1 8 9 4 r | brbinf26_el1 2 1 8 10 4 r | brbinf27_el1 2 1 8 11 4 r
    brbinf28_el1 2 1 8 12 4 r | brbinf29_el1 2 1 8 13 4 r | brbinf2_el1 2 1 8 2 0 r
    brbinf30_el1 2 1 8 14 4 r | brbinf31_el1 2 1 8 15 4 r | brbinf3_el1 2 1 8 3 0 r
    brbinf4_el1 2 1 8 4 0 r | brbinf5_el1 2 1 8 5 0 r | brbinf6_el1 2 1 8 6 0 r
    brbinf7_el1 2 1 8 7 0 r | brbinf8_el1 2 1 8 8 0 r | brbinf9_el1 2 1 8 9 0 r
    brbinfinj_el1 2 1 9 1 0 rw | brbsrc0_el1 2 1 8 0 1 r | brbsrc10_el1 2 1 8 10 1 r
    brbsrc11_el1 2 1 8 11 1 r | brbsrc12_el1 2 1 8 12 1 r | brbsrc13_el1 2 1 8 13 1 r
    brbsrc14_el1 2 1 8 14 1 r | brbsrc15_el1 2 1 8 15 1 r | brbsrc16_el1 2 1 8 0 5 r
    brbsrc17_el1 2 1 8 1 5 r | brbsrc18_el1 2 1 8 2 5 r | brbsrc19_el1 2 1 8 3 5 r
    brbsrc1_el1 2 1 8 1 1 r | brbsrc20_el1 2 1 8 4 5 r | brbsrc21_el1 2 1 8 5 5 r
    brbsrc22_el1 2 1 8 6 5 r | brbsrc23_el1 2 1 8 7 5 r | brbsrc24_el1 2 1 8 8 5 r
    brbsrc25_el1 2 1 8 9 5 r | brbsrc26_el1 2 1 8 10 5 r | brbsrc27_el1 2 1 8 11 5 r
    brbsrc28_el1 2 1 8 12 5 r | brbsrc29_el1 2 1 8 13 5 r | brbsrc2_el1 2 1 8 2 1 r
    brbsrc30_el1 2 1 8 14 5 r | brbsrc31_el1 2 1 8 15 5 r | brbsrc3_el1 2 1 8 3 1 r
    brbsrc4_el1 2 1 8 4 1 r | brbsrc5_el1 2 1 8 5 1 r | brbsrc6_el1 2 1 8 6 1 r
    brbsrc7_el1 2 1 8 7 1 r | brbsrc8_el1 2 1 8 8 1 r | brbsrc9_el1 2 1 8 9 1 r
    brbsrcinj_el1 2 1 9 1 1 rw | brbtgt0_el1 2 1 8 0 2 r | brbtgt10_el1 2 1 8 10 2 r
    brbtgt11_el1 2 1 8 11 2 r | brbtgt12_el1 2 1 8 12 2 r | brbtgt13_el1 2 1 8 13 2 r
    brbtgt14_el1 2 1 8 14 2 r | brbtgt15_el1 2 1 8 15 2 r | brbtgt16_el1 2 1 8 0 6 r
    brbtgt17_el1 2 1 8 1 6 r | brbtgt18_el1 2 1 8 2 6 r | brbtgt19_el1 2 1 8 3 6 r
    brbtgt1_el1 2 1 8 1 2 r | brbtgt20_el1 2 1 8 4 6 r | brbtgt21_el1 2 1 8 5 6 r
    brbtgt22_el1 2 1 8 6 6 r | brbtgt23_el1 2 1 8 7 6 r | brbtgt24_el1 2 1 8 8 6 r
    brbtgt25_el1 2 1 8 9 6 r | brbtgt26_el1 2 1 8 10 6 r | brbtgt27_el1 2 1 8 11 6 r
    brbtgt28_el1 2 1 8 12 6 r | brbtgt29_el1 2 1 8 13 6 r | brbtgt2_el1 2 1 8 2 2 r
    brbtgt30_el1 2 1 8 14 6 r | brbtgt31_el1 2 1 8 15 6 r | brbtgt3_el1 2 1 8 3 2 r
    brbtgt4_el1 2 1 8 4 2 r | brbtgt5_el1 2 1 8 5 2 r | brbtgt6_el1 2 1 8 6 2 r
    brbtgt7_el1 2 1 8 7 2 r | brbtgt8_el1 2 1 8 8 2 r | brbtgt9_el1 2 1 8 9 2 r
    brbtgtinj_el1 2 1 9 1 2 rw | brbts_el1 2 1 9 0 2 rw | ccsidr2_el1 3 1 0 0 2 r
    ccsidr_el1 3 1 0 0 0 r | clidr_el1 3 1 0 0 1 r | cntfrq_el0 3 3 14 0 0 rw
    cnthctl_el2 3 4 14 1 0 rw | cnthp_ctl_el2 3 4 14 2 1 rw | cnthp_cval_el2 3 4 14 2 2 rw
    cnthp_tval_el2 3 4 14 2 0 rw | cnthps_ctl_el2 3 4 14 5 1 rw | cnthps_cval_el2 3 4 14 5 2 rw
    cnthps_tval_el2 3 4 14 5 0 rw | cnthv_ctl_el2 3 4 14 3 1 rw | cnthv_cval_el2 3 4 14 3 2 rw
    cnthv_tval_el2 3 4 14 3 0 rw | cnthvs_ctl_el2 3 4 14 4 1 rw | cnthvs_cval_el2 3 4 14 4 2 rw
    cnthvs_tval_el2 3 4 14 4 0 rw | cntiscale_el2 3 4 14 0 5 rw | cntkctl_el1 3 0 14 1 0 rw
    cntkctl_el12 3 5 14 1 0 rw | cntp_ctl_el0 3 3 14 2 1 rw | cntp_ctl_el02 3 5 14 2 1 rw
    cntp_cval_el0 3 3 14 2 2 rw | cntp_cval_el02 3 5 14 2 2 rw | cntp_tval_el0 3 3 14 2 0 rw
    cntp_tval_el02 3 5 14 2 0 rw | cntpct_el0 3 3 14 0 1 r | cntpctss_el0 3 3 14 0 5 r
    cntpoff_el2 3 4 14 0 6 rw | cntps_ctl_el1 3 7 14 2 1 rw | cntps_cval_el1 3 7 14 2 2 rw
    cntps_tval_el1 3 7 14 2 0 rw | cntscale_el2 3 4 14 0 4 rw | cntv_ctl_el0 3 3 14 3 1 rw
    cntv_ctl_el02 3 5 14 3 1 rw | cntv_cval_el0 3 3 14 3 2 rw | cntv_cval_el02 3 5 14 3 2 rw
    cntv_tval_el0 3 3 14 3 0 rw | cntv_tval_el02 3 5 14 3 0 rw | cntvct_el0 3 3 14 0 2 r
    cntvctss_el0 3 3 14 0 6 r | cntvfrq_el2 3 4 14 0 7 rw | cntvoff_el2 3 4 14 0 3 rw
    contextidr_el1 3 0 13 0 1 rw | contextidr_el12 3 5 13 0 1 rw | contextidr_el2 3 4 13 0 1 rw
    cpacr_el1 3 0 1 0 2 rw | cpacr_el12 3 5 1 0 2 rw | cpm_ioacc_ctl_el3 3 7 15 2 0 rw
    cptr_el2 3 4 1 1 2 rw | cptr_el3 3 6 1 1 2 rw | csselr_el1 3 2 0 0 0 rw
    ctr_el0 3 3 0 0 1 r | currentel 3 0 4 2 2 r | dacr32_el2 3 4 3 0 0 rw
    daif 3 3 4 2 1 rw | dbgauthstatus_el1 2 0 7 14 6 r | dbgbcr0_el1 2 0 0 0 5 rw
    dbgbcr10_el1 2 0 0 10 5 rw | dbgbcr11_el1 2 0 0 11 5 rw | dbgbcr12_el1 2 0 0 12 5 rw
    dbgbcr13_el1 2 0 0 13 5 rw | dbgbcr14_el1 2 0 0 14 5 rw | dbgbcr15_el1 2 0 0 15 5 rw
    dbgbcr1_el1 2 0 0 1 5 rw | dbgbcr2_el1 2 0 0 2 5 rw | dbgbcr3_el1 2 0 0 3 5 rw
    dbgbcr4_el1 2 0 0 4 5 rw | dbgbcr5_el1 2 0 0 5 5 rw | dbgbcr6_el1 2 0 0 6 5 rw
    dbgbcr7_el1 2 0 0 7 5 rw | dbgbcr8_el1 2 0 0 8 5 rw | dbgbcr9_el1 2 0 0 9 5 rw
    dbgbvr0_el1 2 0 0 0 4 rw | dbgbvr10_el1 2 0 0 10 4 rw | dbgbvr11_el1 2 0 0 11 4 rw
    dbgbvr12_el1 2 0 0 12 4 rw | dbgbvr13_el1 2 0 0 13 4 rw | dbgbvr14_el1 2 0 0 14 4 rw
    dbgbvr15_el1 2 0 0 15 4 rw | dbgbvr1_el1 2 0 0 1 4 rw | dbgbvr2_el1 2 0 0 2 4 rw
    dbgbvr3_el1 2 0 0 3 4 rw | dbgbvr4_el1 2 0 0 4 4 rw | dbgbvr5_el1 2 0 0 5 4 rw
    dbgbvr6_el1 2 0 0 6 4 rw | dbgbvr7_el1 2 0 0 7 4 rw | dbgbvr8_el1 2 0 0 8 4 rw
    dbgbvr9_el1 2 0 0 9 4 rw | dbgclaimclr_el1 2 0 7 9 6 rw | dbgclaimset_el1 2 0 7 8 6 rw
    dbgdtr_el0 2 3 0 4 0 rw | dbgdtrrx_el0 2 3 0 5 0 r | dbgdtrtx_el0 2 3 0 5 0 w
    dbgprcr_el1 2 0 1 4 4 rw | dbgvcr32_el2 2 4 0 7 0 rw | dbgwcr0_el1 2 0 0 0 7 rw
    dbgwcr10_el1 2 0 0 10 7 rw | dbgwcr11_el1 2 0 0 11 7 rw | dbgwcr12_el1 2 0 0 12 7 rw
    dbgwcr13_el1 2 0 0 13 7 rw | dbgwcr14_el1 2 0 0 14 7 rw | dbgwcr15_el1 2 0 0 15 7 rw
    dbgwcr1_el1 2 0 0 1 7 rw | dbgwcr2_el1 2 0 0 2 7 rw | dbgwcr3_el1 2 0 0 3 7 rw
    dbgwcr4_el1 2 0 0 4 7 rw | dbgwcr5_el1 2 0 0 5 7 rw | dbgwcr6_el1 2 0 0 6 7 rw
    dbgwcr7_el1 2 0 0 7 7 rw | dbgwcr8_el1 2 0 0 8 7 rw | dbgwcr9_el1 2 0 0 9 7 rw
    dbgwvr0_el1 2 0 0 0 6 rw | dbgwvr10_el1 2 0 0 10 6 rw | dbgwvr11_el1 2 0 0 11 6 rw
    dbgwvr12_el1 2 0 0 12 6 rw | dbgwvr13_el1 2 0 0 13 6 rw | dbgwvr14_el1 2 0 0 14 6 rw
    dbgwvr15_el1 2 0 0 15 6 rw | dbgwvr1_el1 2 0 0 1 6 rw | dbgwvr2_el1 2 0 0 2 6 rw
    dbgwvr3_el1 2 0 0 3 6 rw | dbgwvr4_el1 2 0 0 4 6 rw | dbgwvr5_el1 2 0 0 5 6 rw
    dbgwvr6_el1 2 0 0 6 6 rw | dbgwvr7_el1 2 0 0 7 6 rw | dbgwvr8_el1 2 0 0 8 6 rw
    dbgwvr9_el1 2 0 0 9 6 rw | dczid_el0 3 3 0 0 7 r | disr_el1 3 0 12 1 1 rw
    dit 3 3 4 2 5 rw | dlr_el0 3 3 4 5 1 rw | dspsr_el0 3 3 4 5 0 rw
    elr_el1 3 0 4 0 1 rw | elr_el12 3 5 4 0 1 rw | elr_el2 3 4 4 0 1 rw
    elr_el3 3 6 4 0 1 rw | erridr_el1 3 0 5 3 0 r | errselr_el1 3 0 5 3 1 rw
    erxaddr_el1 3 0 5 4 3 rw | erxctlr_el1 3 0 5 4 1 rw | erxfr_el1 3 0 5 4 0 r
    erxgsr_el1 3 0 5 3 2 r | erxmisc0_el1 3 0 5 5 0 rw | erxmisc1_el1 3 0 5 5 1 rw
    erxmisc2_el1 3 0 5 5 2 rw | erxmisc3_el1 3 0 5 5 3 rw | erxpfgcdn_el1 3 0 5 4 6 rw
    erxpfgctl_el1 3 0 5 4 5 rw | erxpfgf_el1 3 0 5 4 4 r | erxstatus_el1 3 0 5 4 2 rw
    esr_el1 3 0 5 2 0 rw | esr_el12 3 5 5 2 0 rw | esr_el2 3 4 5 2 0 rw
    esr_el3 3 6 5 2 0 rw | far_el1 3 0 6 0 0 rw | far_el12 3 5 6 0 0 rw
    far_el2 3 4 6 0 0 rw | far_el3 3 6 6 0 0 rw | fgwte3_el3 3 6 1 1 5 rw
    fpcr 3 3 4 4 0 rw | fpexc32_el2 3 4 5 3 0 rw | fpmr 3 3 4 4 2 rw
    fpsr 3 3 4 4 1 rw | gcr_el1 3 0 1 0 6 rw | gcscr_el1 3 0 2 5 0 rw
    gcscr_el12 3 5 2 5 0 rw | gcscr_el2 3 4 2 5 0 rw | gcscr_el3 3 6 2 5 0 rw
    gcscre0_el1 3 0 2 5 2 rw | gcspr_el0 3 3 2 5 1 rw | gcspr_el1 3 0 2 5 1 rw
    gcspr_el12 3 5 2 5 1 rw | gcspr_el2 3 4 2 5 1 rw | gcspr_el3 3 6 2 5 1 rw
    gmid_el1 3 1 0 0 4 r | gpccr_el3 3 6 2 1 6 rw | gptbr_el3 3 6 2 1 4 rw
    hacdbsbr_el2 3 4 2 3 4 rw | hacdbscons_el2 3 4 2 3 5 rw | hacr_el2 3 4 1 1 7 rw
    hafgrtr_el2 3 4 3 1 6 rw | hcr_el2 3 4 1 1 0 rw | hcrx_el2 3 4 1 2 2 rw
    hdbssbr_el2 3 4 2 3 2 rw | hdbssprod_el2 3 4 2 3 3 rw | hdfgrtr2_el2 3 4 3 1 0 rw
    hdfgrtr_el2 3 4 3 1 4 rw | hdfgwtr2_el2 3 4 3 1 1 rw | hdfgwtr_el2 3 4 3 1 5 rw
    hfgitr2_el2 3 4 3 1 7 rw | hfgitr_el2 3 4 1 1 6 rw | hfgrtr2_el2 3 4 3 1 2 rw
    hfgrtr_el2 3 4 1 1 4 rw | hfgwtr2_el2 3 4 3 1 3 rw | hfgwtr_el2 3 4 1 1 5 rw
    hpfar_el2 3 4 6 0 4 rw | hstr_el2 3 4 1 1 3 rw | icc_ap0r0_el1 3 0 12 8 4 rw
    icc_ap0r1_el1 3 0 12 8 5 rw | icc_ap0r2_el1 3 0 12 8 6 rw | icc_ap0r3_el1 3 0 12 8 7 rw
    icc_ap1r0_el1 3 0 12 9 0 rw | icc_ap1r1_el1 3 0 12 9 1 rw | icc_ap1r2_el1 3 0 12 9 2 rw
    icc_ap1r3_el1 3 0 12 9 3 rw | icc_asgi1r_el1 3 0 12 11 6 w | icc_bpr0_el1 3 0 12 8 3 rw
    icc_bpr1_el1 3 0 12 12 3 rw | icc_ctlr_el1 3 0 12 12 4 rw | icc_ctlr_el3 3 6 12 12 4 rw
    icc_dir_el1 3 0 12 11 1 w | icc_eoir0_el1 3 0 12 8 1 w | icc_eoir1_el1 3 0 12 12 1 w
    icc_hppir0_el1 3 0 12 8 2 r | icc_hppir1_el1 3 0 12 12 2 r | icc_iar0_el1 3 0 12 8 0 r
    icc_iar1_el1 3 0 12 12 0 r | icc_igrpen0_el1 3 0 12 12 6 rw | icc_igrpen1_el1 3 0 12 12 7 rw
    icc_igrpen1_el3 3 6 12 12 7 rw | icc_nmiar1_el1 3 0 12 9 5 r | icc_pmr_el1 3 0 4 6 0 rw
    icc_rpr_el1 3 0 12 11 3 r | icc_sgi0r_el1 3 0 12 11 7 w | icc_sgi1r_el1 3 0 12 11 5 w
    icc_sre_el1 3 0 12 12 5 rw | icc_sre_el2 3 4 12 9 5 rw | icc_sre_el3 3 6 12 12 5 rw
    ich_ap0r0_el2 3 4 12 8 0 rw | ich_ap0r1_el2 3 4 12 8 1 rw | ich_ap0r2_el2 3 4 12 8 2 rw
    ich_ap0r3_el2 3 4 12 8 3 rw | ich_ap1r0_el2 3 4 12 9 0 rw | ich_ap1r1_el2 3 4 12 9 1 rw
    ich_ap1r2_el2 3 4 12 9 2 rw | ich_ap1r3_el2 3 4 12 9 3 rw | ich_eisr_el2 3 4 12 11 3 r
    ich_elrsr_el2 3 4 12 11 5 r | ich_hcr_el2 3 4 12 11 0 rw | ich_lr0_el2 3 4 12 12 0 rw
    ich_lr10_el2 3 4 12 13 2 rw | ich_lr11_el2 3 4 12 13 3 rw | ich_lr12_el2 3 4 12 13 4 rw
    ich_lr13_el2 3 4 12 13 5 rw | ich_lr14_el2 3 4 12 13 6 rw | ich_lr15_el2 3 4 12 13 7 rw
    ich_lr1_el2 3 4 12 12 1 rw | ich_lr2_el2 3 4 12 12 2 rw | ich_lr3_el2 3 4 12 12 3 rw
    ich_lr4_el2 3 4 12 12 4 rw | ich_lr5_el2 3 4 12 12 5 rw | ich_lr6_el2 3 4 12 12 6 rw
    ich_lr7_el2 3 4 12 12 7 rw | ich_lr8_el2 3 4 12 13 0 rw | ich_lr9_el2 3 4 12 13 1 rw
    ich_misr_el2 3 4 12 11 2 r | ich_vmcr_el2 3 4 12 11 7 rw | ich_vtr_el2 3 4 12 11 1 r
    id_aa64afr0_el1 3 0 0 5 4 r | id_aa64afr1_el1 3 0 0 5 5 r | id_aa64dfr0_el1 3 0 0 5 0 r
    id_aa64dfr1_el1 3 0 0 5 1 r | id_aa64dfr2_el1 3 0 0 5 2 r | id_aa64fpfr0_el1 3 0 0 4 7 r
    id_aa64isar0_el1 3 0 0 6 0 r | id_aa64isar1_el1 3 0 0 6 1 r | id_aa64isar2_el1 3 0 0 6 2 r
    id_aa64isar3_el1 3 0 0 6 3 r | id_aa64mmfr0_el1 3 0 0 7 0 r | id_aa64mmfr1_el1 3 0 0 7 1 r
    id_aa64mmfr2_el1 3 0 0 7 2 r | id_aa64mmfr3_el1 3 0 0 7 3 r | id_aa64mmfr4_el1 3 0 0 7 4 r
    id_aa64pfr0_el1 3 0 0 4 0 r | id_aa64pfr1_el1 3 0 0 4 1 r | id_aa64pfr2_el1 3 0 0 4 2 r
    id_aa64smfr0_el1 3 0 0 4 5 r | id_aa64zfr0_el1 3 0 0 4 4 r | id_afr0_el1 3 0 0 1 3 r
    id_dfr0_el1 3 0 0 1 2 r | id_dfr1_el1 3 0 0 3 5 r | id_isar0_el1 3 0 0 2 0 r
    id_isar1_el1 3 0 0 2 1 r | id_isar2_el1 3 0 0 2 2 r | id_isar3_el1 3 0 0 2 3 r
    id_isar4_el1 3 0 0 2 4 r | id_isar5_el1 3 0 0 2 5 r | id_isar6_el1 3 0 0 2 7 r
    id_mmfr0_el1 3 0 0 1 4 r | id_mmfr1_el1 3 0 0 1 5 r | id_mmfr2_el1 3 0 0 1 6 r
    id_mmfr3_el1 3 0 0 1 7 r | id_mmfr4_el1 3 0 0 2 6 r | id_mmfr5_el1 3 0 0 3 6 r
    id_pfr0_el1 3 0 0 1 0 r | id_pfr1_el1 3 0 0 1 1 r | id_pfr2_el1 3 0 0 3 4 r
    ifsr32_el2 3 4 5 0 1 rw | isr_el1 3 0 12 1 0 r | lorc_el1 3 0 10 4 3 rw
    lorea_el1 3 0 10 4 1 rw | lorid_el1 3 0 10 4 7 r | lorn_el1 3 0 10 4 2 rw
    lorsa_el1 3 0 10 4 0 rw | mair2_el1 3 0 10 2 1 rw | mair2_el12 3 5 10 2 1 rw
    mair2_el2 3 4 10 1 1 rw | mair2_el3 3 6 10 1 1 rw | mair_el1 3 0 10 2 0 rw
    mair_el12 3 5 10 2 0 rw | mair_el2 3 4 10 2 0 rw | mair_el3 3 6 10 2 0 rw
    mdccint_el1 2 0 0 2 0 rw | mdccsr_el0 2 3 0 1 0 r | mdcr_el2 3 4 1 1 1 rw
    mdcr_el3 3 6 1 3 1 rw | mdrar_el1 2 0 1 0 0 r | mdscr_el1 2 0 0 2 2 rw
    mdselr_el1 2 0 0 4 2 rw | mdstepop_el1 2 0 0 5 2 rw | mecid_a0_el2 3 4 10 8 1 rw
    mecid_a1_el2 3 4 10 8 3 rw | mecid_p0_el2 3 4 10 8 0 rw | mecid_p1_el2 3 4 10 8 2 rw
    mecid_rl_a_el3 3 6 10 10 1 rw | mecidr_el2 3 4 10 8 7 r | mfar_el3 3 6 6 0 5 rw
    midr_el1 3 0 0 0 0 r | mpam0_el1 3 0 10 5 1 rw | mpam1_el1 3 0 10 5 0 rw
    mpam1_el12 3 5 10 5 0 rw | mpam2_el2 3 4 10 5 0 rw | mpam3_el3 3 6 10 5 0 rw
    mpamhcr_el2 3 4 10 4 0 rw | mpamidr_el1 3 0 10 4 4 r | mpamsm_el1 3 0 10 5 3 rw
    mpamvpm0_el2 3 4 10 6 0 rw | mpamvpm1_el2 3 4 10 6 1 rw | mpamvpm2_el2 3 4 10 6 2 rw
    mpamvpm3_el2 3 4 10 6 3 rw | mpamvpm4_el2 3 4 10 6 4 rw | mpamvpm5_el2 3 4 10 6 5 rw
    mpamvpm6_el2 3 4 10 6 6 rw | mpamvpm7_el2 3 4 10 6 7 rw | mpamvpmv_el2 3 4 10 4 1 rw
    mpidr_el1 3 0 0 0 5 r | mpuir_el1 3 0 0 0 4 rw | mpuir_el2 3 4 0 0 4 rw
    mvfr0_el1 3 0 0 3 0 r | mvfr1_el1 3 0 0 3 1 r | mvfr2_el1 3 0 0 3 2 r
    nzcv 3 3 4 2 0 rw | osdlr_el1 2 0 1 3 4 rw | osdtrrx_el1 2 0 0 0 2 rw
    osdtrtx_el1 2 0 0 3 2 rw | oseccr_el1 2 0 0 6 2 rw | oslar_el1 2 0 1 0 4 w
    oslsr_el1 2 0 1 1 4 r | pan 3 0 4 2 3 rw | par_el1 3 0 7 4 0 rwp
    pfar_el1 3 0 6 0 5 rw | pfar_el12 3 5 6 0 5 rw | pfar_el2 3 4 6 0 5 rw
    pir_el1 3 0 10 2 3 rw | pir_el12 3 5 10 2 3 rw | pir_el2 3 4 10 2 3 rw
    pir_el3 3 6 10 2 3 rw | pire0_el1 3 0 10 2 2 rw | pire0_el12 3 5 10 2 2 rw
    pire0_el2 3 4 10 2 2 rw | pm 3 0 4 3 1 rw | pmbidr_el1 3 0 9 10 7 r
    pmblimitr_el1 3 0 9 10 0 rw | pmbptr_el1 3 0 9 10 1 rw | pmbsr_el1 3 0 9 10 3 rw
    pmccfiltr_el0 3 3 14 15 7 rw | pmccntr_el0 3 3 9 13 0 rw | pmccntsvr_el1 2 0 14 11 7 r
    pmceid0_el0 3 3 9 12 6 r | pmceid1_el0 3 3 9 12 7 r | pmcntenclr_el0 3 3 9 12 2 rw
    pmcntenset_el0 3 3 9 12 1 rw | pmcr_el0 3 3 9 12 0 rw | pmecr_el1 3 0 9 14 5 rw
    pmevcntr0_el0 3 3 14 8 0 rw | pmevcntr10_el0 3 3 14 9 2 rw | pmevcntr11_el0 3 3 14 9 3 rw
    pmevcntr12_el0 3 3 14 9 4 rw | pmevcntr13_el0 3 3 14 9 5 rw | pmevcntr14_el0 3 3 14 9 6 rw
    pmevcntr15_el0 3 3 14 9 7 rw | pmevcntr16_el0 3 3 14 10 0 rw | pmevcntr17_el0 3 3 14 10 1 rw
    pmevcntr18_el0 3 3 14 10 2 rw | pmevcntr19_el0 3 3 14 10 3 rw | pmevcntr1_el0 3 3 14 8 1 rw
    pmevcntr20_el0 3 3 14 10 4 rw | pmevcntr21_el0 3 3 14 10 5 rw | pmevcntr22_el0 3 3 14 10 6 rw
    pmevcntr23_el0 3 3 14 10 7 rw | pmevcntr24_el0 3 3 14 11 0 rw | pmevcntr25_el0 3 3 14 11 1 rw
    pmevcntr26_el0 3 3 14 11 2 rw | pmevcntr27_el0 3 3 14 11 3 rw | pmevcntr28_el0 3 3 14 11 4 rw
    pmevcntr29_el0 3 3 14 11 5 rw | pmevcntr2_el0 3 3 14 8 2 rw | pmevcntr30_el0 3 3 14 11 6 rw
    pmevcntr3_el0 3 3 14 8 3 rw | pmevcntr4_el0 3 3 14 8 4 rw | pmevcntr5_el0 3 3 14 8 5 rw
    pmevcntr6_el0 3 3 14 8 6 rw | pmevcntr7_el0 3 3 14 8 7 rw | pmevcntr8_el0 3 3 14 9 0 rw
    pmevcntr9_el0 3 3 14 9 1 rw | pmevcntsvr0_el1 2 0 14 8 0 r | pmevcntsvr10_el1 2 0 14 9 2 r
    pmevcntsvr11_el1 2 0 14 9 3 r | pmevcntsvr12_el1 2 0 14 9 4 r | pmevcntsvr13_el1 2 0 14 9 5 r
    pmevcntsvr14_el1 2 0 14 9 6 r | pmevcntsvr15_el1 2 0 14 9 7 r | pmevcntsvr16_el1 2 0 14 10 0 r
    pmevcntsvr17_el1 2 0 14 10 1 r | pmevcntsvr18_el1 2 0 14 10 2 r | pmevcntsvr19_el1 2 0 14 10 3 r
    pmevcntsvr1_el1 2 0 14 8 1 r | pmevcntsvr20_el1 2 0 14 10 4 r | pmevcntsvr21_el1 2 0 14 10 5 r
    pmevcntsvr22_el1 2 0 14 10 6 r | pmevcntsvr23_el1 2 0 14 10 7 r | pmevcntsvr24_el1 2 0 14 11 0 r
    pmevcntsvr25_el1 2 0 14 11 1 r | pmevcntsvr26_el1 2 0 14 11 2 r | pmevcntsvr27_el1 2 0 14 11 3 r
    pmevcntsvr28_el1 2 0 14 11 4 r | pmevcntsvr29_el1 2 0 14 11 5 r | pmevcntsvr2_el1 2 0 14 8 2 r
    pmevcntsvr30_el1 2 0 14 11 6 r | pmevcntsvr3_el1 2 0 14 8 3 r | pmevcntsvr4_el1 2 0 14 8 4 r
    pmevcntsvr5_el1 2 0 14 8 5 r | pmevcntsvr6_el1 2 0 14 8 6 r | pmevcntsvr7_el1 2 0 14 8 7 r
    pmevcntsvr8_el1 2 0 14 9 0 r | pmevcntsvr9_el1 2 0 14 9 1 r | pmevtyper0_el0 3 3 14 12 0 rw
    pmevtyper10_el0 3 3 14 13 2 rw | pmevtyper11_el0 3 3 14 13 3 rw | pmevtyper12_el0 3 3 14 13 4 rw
    pmevtyper13_el0 3 3 14 13 5 rw | pmevtyper14_el0 3 3 14 13 6 rw | pmevtyper15_el0 3 3 14 13 7 rw
    pmevtyper16_el0 3 3 14 14 0 rw | pmevtyper17_el0 3 3 14 14 1 rw | pmevtyper18_el0 3 3 14 14 2 rw
    pmevtyper19_el0 3 3 14 14 3 rw | pmevtyper1_el0 3 3 14 12 1 rw | pmevtyper20_el0 3 3 14 14 4 rw
    pmevtyper21_el0 3 3 14 14 5 rw | pmevtyper22_el0 3 3 14 14 6 rw | pmevtyper23_el0 3 3 14 14 7 rw
    pmevtyper24_el0 3 3 14 15 0 rw | pmevtyper25_el0 3 3 14 15 1 rw | pmevtyper26_el0 3 3 14 15 2 rw
    pmevtyper27_el0 3 3 14 15 3 rw | pmevtyper28_el0 3 3 14 15 4 rw | pmevtyper29_el0 3 3 14 15 5 rw
    pmevtyper2_el0 3 3 14 12 2 rw | pmevtyper30_el0 3 3 14 15 6 rw | pmevtyper3_el0 3 3 14 12 3 rw
    pmevtyper4_el0 3 3 14 12 4 rw | pmevtyper5_el0 3 3 14 12 5 rw | pmevtyper6_el0 3 3 14 12 6 rw
    pmevtyper7_el0 3 3 14 12 7 rw | pmevtyper8_el0 3 3 14 13 0 rw | pmevtyper9_el0 3 3 14 13 1 rw
    pmiar_el1 3 0 9 14 7 rw | pmicfiltr_el0 3 3 9 6 0 rw | pmicntr_el0 3 3 9 4 0 rw
    pmicntsvr_el1 2 0 14 12 0 r | pmintenclr_el1 3 0 9 14 2 rw | pmintenset_el1 3 0 9 14 1 rw
    pmmir_el1 3 0 9 14 6 r | pmovsclr_el0 3 3 9 12 3 rw | pmovsset_el0 3 3 9 14 3 rw
    pmscr_el1 3 0 9 9 0 rw | pmscr_el12 3 5 9 9 0 rw | pmscr_el2 3 4 9 9 0 rw
    pmsdsfr_el1 3 0 9 10 4 rw | pmselr_el0 3 3 9 12 5 rw | pmsevfr_el1 3 0 9 9 5 rw
    pmsfcr_el1 3 0 9 9 4 rw | pmsicr_el1 3 0 9 9 2 rw | pmsidr_el1 3 0 9 9 7 r
    pmsirr_el1 3 0 9 9 3 rw | pmslatfr_el1 3 0 9 9 6 rw | pmsnevfr_el1 3 0 9 9 1 rw
    pmsscr_el1 3 0 9 13 3 rw | pmswinc_el0 3 3 9 12 4 w | pmuacr_el1 3 0 9 14 4 rw
    pmuserenr_el0 3 3 9 14 0 rw | pmxevcntr_el0 3 3 9 13 2 rw | pmxevtyper_el0 3 3 9 13 1 rw
    pmzr_el0 3 3 9 13 4 w | por_el0 3 3 10 2 4 rw | por_el1 3 0 10 2 4 rw
    por_el12 3 5 10 2 4 rw | por_el2 3 4 10 2 4 rw | por_el3 3 6 10 2 4 rw
    prbar10_el1 3 0 6 13 0 rw | prbar10_el2 3 4 6 13 0 rw | prbar11_el1 3 0 6 13 4 rw
    prbar11_el2 3 4 6 13 4 rw | prbar12_el1 3 0 6 14 0 rw | prbar12_el2 3 4 6 14 0 rw
    prbar13_el1 3 0 6 14 4 rw | prbar13_el2 3 4 6 14 4 rw | prbar14_el1 3 0 6 15 0 rw
    prbar14_el2 3 4 6 15 0 rw | prbar15_el1 3 0 6 15 4 rw | prbar15_el2 3 4 6 15 4 rw
    prbar1_el1 3 0 6 8 4 rw | prbar1_el2 3 4 6 8 4 rw | prbar2_el1 3 0 6 9 0 rw
    prbar2_el2 3 4 6 9 0 rw | prbar3_el1 3 0 6 9 4 rw | prbar3_el2 3 4 6 9 4 rw
    prbar4_el1 3 0 6 10 0 rw | prbar4_el2 3 4 6 10 0 rw | prbar5_el1 3 0 6 10 4 rw
    prbar5_el2 3 4 6 10 4 rw | prbar6_el1 3 0 6 11 0 rw | prbar6_el2 3 4 6 11 0 rw
    prbar7_el1 3 0 6 11 4 rw | prbar7_el2 3 4 6 11 4 rw | prbar8_el1 3 0 6 12 0 rw
    prbar8_el2 3 4 6 12 0 rw | prbar9_el1 3 0 6 12 4 rw | prbar9_el2 3 4 6 12 4 rw
    prbar_el1 3 0 6 8 0 rw | prbar_el2 3 4 6 8 0 rw | prenr_el1 3 0 6 1 1 rw
    prenr_el2 3 4 6 1 1 rw | prlar10_el1 3 0 6 13 1 rw | prlar10_el2 3 4 6 13 1 rw
    prlar11_el1 3 0 6 13 5 rw | prlar11_el2 3 4 6 13 5 rw | prlar12_el1 3 0 6 14 1 rw
    prlar12_el2 3 4 6 14 1 rw | prlar13_el1 3 0 6 14 5 rw | prlar13_el2 3 4 6 14 5 rw
    prlar14_el1 3 0 6 15 1 rw | prlar14_el2 3 4 6 15 1 rw | prlar15_el1 3 0 6 15 5 rw
    prlar15_el2 3 4 6 15 5 rw | prlar1_el1 3 0 6 8 5 rw | prlar1_el2 3 4 6 8 5 rw
    prlar2_el1 3 0 6 9 1 rw | prlar2_el2 3 4 6 9 1 rw | prlar3_el1 3 0 6 9 5 rw
    prlar3_el2 3 4 6 9 5 rw | prlar4_el1 3 0 6 10 1 rw | prlar4_el2 3 4 6 10 1 rw
    prlar5_el1 3 0 6 10 5 rw | prlar5_el2 3 4 6 10 5 rw | prlar6_el1 3 0 6 11 1 rw
    prlar6_el2 3 4 6 11 1 rw | prlar7_el1 3 0 6 11 5 rw | prlar7_el2 3 4 6 11 5 rw
    prlar8_el1 3 0 6 12 1 rw | prlar8_el2 3 4 6 12 1 rw | prlar9_el1 3 0 6 12 5 rw
    prlar9_el2 3 4 6 12 5 rw | prlar_el1 3 0 6 8 1 rw | prlar_el2 3 4 6 8 1 rw
    prselr_el1 3 0 6 2 1 rw | prselr_el2 3 4 6 2 1 rw | rcwmask_el1 3 0 13 0 6 rwp
    rcwsmask_el1 3 0 13 0 3 rwp | revidr_el1 3 0 0 0 6 r | rgsr_el1 3 0 1 0 5 rw
    rmr_el1 3 0 12 0 2 rw | rmr_el2 3 4 12 0 2 rw | rmr_el3 3 6 12 0 2 rw
    rndr 3 3 2 4 0 r | rndrrs 3 3 2 4 1 r | rvbar_el1 3 0 12 0 1 r
    rvbar_el2 3 4 12 0 1 r | rvbar_el3 3 6 12 0 1 r | s2pir_el2 3 4 10 2 5 rw
    s2por_el1 3 0 10 2 5 rw | scr_el3 3 6 1 1 0 rw | sctlr2_el1 3 0 1 0 3 rw
    sctlr2_el12 3 5 1 0 3 rw | sctlr2_el2 3 4 1 0 3 rw | sctlr2_el3 3 6 1 0 3 rw
    sctlr_el1 3 0 1 0 0 rw | sctlr_el12 3 5 1 0 0 rw | sctlr_el2 3 4 1 0 0 rw
    sctlr_el3 3 6 1 0 0 rw | scxtnum_el0 3 3 13 0 7 rw | scxtnum_el1 3 0 13 0 7 rw
    scxtnum_el12 3 5 13 0 7 rw | scxtnum_el2 3 4 13 0 7 rw | scxtnum_el3 3 6 13 0 7 rw
    sder32_el2 3 4 1 3 1 rw | sder32_el3 3 6 1 1 1 rw | smcr_el1 3 0 1 2 6 rw
    smcr_el12 3 5 1 2 6 rw | smcr_el2 3 4 1 2 6 rw | smcr_el3 3 6 1 2 6 rw
    smidr_el1 3 1 0 0 6 r | smpri_el1 3 0 1 2 4 rw | smprimap_el2 3 4 1 2 5 rw
    sp_el0 3 0 4 1 0 rw | sp_el1 3 4 4 1 0 rw | sp_el2 3 6 4 1 0 rw
    spmaccessr_el1 2 0 9 13 3 rw | spmaccessr_el12 2 5 9 13 3 rw | spmaccessr_el2 2 4 9 13 3 rw
    spmaccessr_el3 2 6 9 13 3 rw | spmcfgr_el1 2 0 9 13 7 r | spmcgcr0_el1 2 0 9 13 0 r
    spmcgcr1_el1 2 0 9 13 1 r | spmcntenclr_el0 2 3 9 12 2 rw | spmcntenset_el0 2 3 9 12 1 rw
    spmcr_el0 2 3 9 12 0 rw | spmdevaff_el1 2 0 9 13 6 r | spmdevarch_el1 2 0 9 13 5 r
    spmevcntr0_el0 2 3 14 0 0 rw | spmevcntr10_el0 2 3 14 1 2 rw | spmevcntr11_el0 2 3 14 1 3 rw
    spmevcntr12_el0 2 3 14 1 4 rw | spmevcntr13_el0 2 3 14 1 5 rw | spmevcntr14_el0 2 3 14 1 6 rw
    spmevcntr15_el0 2 3 14 1 7 rw | spmevcntr1_el0 2 3 14 0 1 rw | spmevcntr2_el0 2 3 14 0 2 rw
    spmevcntr3_el0 2 3 14 0 3 rw | spmevcntr4_el0 2 3 14 0 4 rw | spmevcntr5_el0 2 3 14 0 5 rw
    spmevcntr6_el0 2 3 14 0 6 rw | spmevcntr7_el0 2 3 14 0 7 rw | spmevcntr8_el0 2 3 14 1 0 rw
    spmevcntr9_el0 2 3 14 1 1 rw | spmevfilt2r0_el0 2 3 14 6 0 rw | spmevfilt2r10_el0 2 3 14 7 2 rw
    spmevfilt2r11_el0 2 3 14 7 3 rw | spmevfilt2r12_el0 2 3 14 7 4 rw | spmevfilt2r13_el0 2 3 14 7 5 rw
    spmevfilt2r14_el0 2 3 14 7 6 rw | spmevfilt2r15_el0 2 3 14 7 7 rw | spmevfilt2r1_el0 2 3 14 6 1 rw
    spmevfilt2r2_el0 2 3 14 6 2 rw | spmevfilt2r3_el0 2 3 14 6 3 rw | spmevfilt2r4_el0 2 3 14 6 4 rw
    spmevfilt2r5_el0 2 3 14 6 5 rw | spmevfilt2r6_el0 2 3 14 6 6 rw | spmevfilt2r7_el0 2 3 14 6 7 rw
    spmevfilt2r8_el0 2 3 14 7 0 rw | spmevfilt2r9_el0 2 3 14 7 1 rw | spmevfiltr0_el0 2 3 14 4 0 rw
    spmevfiltr10_el0 2 3 14 5 2 rw | spmevfiltr11_el0 2 3 14 5 3 rw | spmevfiltr12_el0 2 3 14 5 4 rw
    spmevfiltr13_el0 2 3 14 5 5 rw | spmevfiltr14_el0 2 3 14 5 6 rw | spmevfiltr15_el0 2 3 14 5 7 rw
    spmevfiltr1_el0 2 3 14 4 1 rw | spmevfiltr2_el0 2 3 14 4 2 rw | spmevfiltr3_el0 2 3 14 4 3 rw
    spmevfiltr4_el0 2 3 14 4 4 rw | spmevfiltr5_el0 2 3 14 4 5 rw | spmevfiltr6_el0 2 3 14 4 6 rw
    spmevfiltr7_el0 2 3 14 4 7 rw | spmevfiltr8_el0 2 3 14 5 0 rw | spmevfiltr9_el0 2 3 14 5 1 rw
    spmevtyper0_el0 2 3 14 2 0 rw | spmevtyper10_el0 2 3 14 3 2 rw | spmevtyper11_el0 2 3 14 3 3 rw
    spmevtyper12_el0 2 3 14 3 4 rw | spmevtyper13_el0 2 3 14 3 5 rw | spmevtyper14_el0 2 3 14 3 6 rw
    spmevtyper15_el0 2 3 14 3 7 rw | spmevtyper1_el0 2 3 14 2 1 rw | spmevtyper2_el0 2 3 14 2 2 rw
    spmevtyper3_el0 2 3 14 2 3 rw | spmevtyper4_el0 2 3 14 2 4 rw | spmevtyper5_el0 2 3 14 2 5 rw
    spmevtyper6_el0 2 3 14 2 6 rw | spmevtyper7_el0 2 3 14 2 7 rw | spmevtyper8_el0 2 3 14 3 0 rw
    spmevtyper9_el0 2 3 14 3 1 rw | spmiidr_el1 2 0 9 13 4 r | spmintenclr_el1 2 0 9 14 2 rw
    spmintenset_el1 2 0 9 14 1 rw | spmovsclr_el0 2 3 9 12 3 rw | spmovsset_el0 2 3 9 14 3 rw
    spmrootcr_el3 2 6 9 14 7 rw | spmscr_el1 2 7 9 14 7 rw | spmselr_el0 2 3 9 12 5 rw
    spmzr_el0 2 3 9 12 4 w | spsel 3 0 4 2 0 rw | spsr_abt 3 4 4 3 1 rw
    spsr_el1 3 0 4 0 0 rw | spsr_el12 3 5 4 0 0 rw | spsr_el2 3 4 4 0 0 rw
    spsr_el3 3 6 4 0 0 rw | spsr_fiq 3 4 4 3 3 rw | spsr_irq 3 4 4 3 0 rw
    spsr_und 3 4 4 3 2 rw | ssbs 3 3 4 2 6 rw | svcr 3 3 4 2 2 rw
    tco 3 3 4 2 7 rw | tcr2_el1 3 0 2 0 3 rw | tcr2_el12 3 5 2 0 3 rw
    tcr2_el2 3 4 2 0 3 rw | tcr_el1 3 0 2 0 2 rw | tcr_el12 3 5 2 0 2 rw
    tcr_el2 3 4 2 0 2 rw | tcr_el3 3 6 2 0 2 rw | teecr32_el1 2 2 0 0 0 rw
    teehbr32_el1 2 2 1 0 0 rw | tfsr_el1 3 0 5 6 0 rw | tfsr_el12 3 5 5 6 0 rw
    tfsr_el2 3 4 5 6 0 rw | tfsr_el3 3 6 5 6 0 rw | tfsre0_el1 3 0 5 6 1 rw
    tpidr2_el0 3 3 13 0 5 rw | tpidr_el0 3 3 13 0 2 rw | tpidr_el1 3 0 13 0 4 rw
    tpidr_el2 3 4 13 0 2 rw | tpidr_el3 3 6 13 0 2 rw | tpidrro_el0 3 3 13 0 3 rw
    trbbaser_el1 3 0 9 11 2 rw | trbidr_el1 3 0 9 11 7 r | trblimitr_el1 3 0 9 11 0 rw
    trbmar_el1 3 0 9 11 4 rw | trbptr_el1 3 0 9 11 1 rw | trbsr_el1 3 0 9 11 3 rw
    trbtrg_el1 3 0 9 11 6 rw | trcacatr0 2 1 2 0 2 rw | trcacatr1 2 1 2 2 2 rw
    trcacatr10 2 1 2 4 3 rw | trcacatr11 2 1 2 6 3 rw | trcacatr12 2 1 2 8 3 rw
    trcacatr13 2 1 2 10 3 rw | trcacatr14 2 1 2 12 3 rw | trcacatr15 2 1 2 14 3 rw
    trcacatr2 2 1 2 4 2 rw | trcacatr3 2 1 2 6 2 rw | trcacatr4 2 1 2 8 2 rw
    trcacatr5 2 1 2 10 2 rw | trcacatr6 2 1 2 12 2 rw | trcacatr7 2 1 2 14 2 rw
    trcacatr8 2 1 2 0 3 rw | trcacatr9 2 1 2 2 3 rw | trcacvr0 2 1 2 0 0 rw
    trcacvr1 2 1 2 2 0 rw | trcacvr10 2 1 2 4 1 rw | trcacvr11 2 1 2 6 1 rw
    trcacvr12 2 1 2 8 1 rw | trcacvr13 2 1 2 10 1 rw | trcacvr14 2 1 2 12 1 rw
    trcacvr15 2 1 2 14 1 rw | trcacvr2 2 1 2 4 0 rw | trcacvr3 2 1 2 6 0 rw
    trcacvr4 2 1 2 8 0 rw | trcacvr5 2 1 2 10 0 rw | trcacvr6 2 1 2 12 0 rw
    trcacvr7 2 1 2 14 0 rw | trcacvr8 2 1 2 0 1 rw | trcacvr9 2 1 2 2 1 rw
    trcauthstatus 2 1 7 14 6 r | trcauxctlr 2 1 0 6 0 rw | trcbbctlr 2 1 0 15 0 rw
    trcccctlr 2 1 0 14 0 rw | trccidcctlr0 2 1 3 0 2 rw | trccidcctlr1 2 1 3 1 2 rw
    trccidcvr0 2 1 3 0 0 rw | trccidcvr1 2 1 3 2 0 rw | trccidcvr2 2 1 3 4 0 rw
    trccidcvr3 2 1 3 6 0 rw | trccidcvr4 2 1 3 8 0 rw | trccidcvr5 2 1 3 10 0 rw
    trccidcvr6 2 1 3 12 0 rw | trccidcvr7 2 1 3 14 0 rw | trccidr0 2 1 7 12 7 r
    trccidr1 2 1 7 13 7 r | trccidr2 2 1 7 14 7 r | trccidr3 2 1 7 15 7 r
    trcclaimclr 2 1 7 9 6 rw | trcclaimset 2 1 7 8 6 rw | trccntctlr0 2 1 0 4 5 rw
    trccntctlr1 2 1 0 5 5 rw | trccntctlr2 2 1 0 6 5 rw | trccntctlr3 2 1 0 7 5 rw
    trccntrldvr0 2 1 0 0 5 rw | trccntrldvr1 2 1 0 1 5 rw | trccntrldvr2 2 1 0 2 5 rw
    trccntrldvr3 2 1 0 3 5 rw | trccntvr0 2 1 0 8 5 rw | trccntvr1 2 1 0 9 5 rw
    trccntvr2 2 1 0 10 5 rw | trccntvr3 2 1 0 11 5 rw | trcconfigr 2 1 0 4 0 rw
    trcdevaff0 2 1 7 10 6 r | trcdevaff1 2 1 7 11 6 r | trcdevarch 2 1 7 15 6 r
    trcdevid 2 1 7 2 7 r | trcdevtype 2 1 7 3 7 r | trcdvcmr0 2 1 2 0 6 rw
    trcdvcmr1 2 1 2 4 6 rw | trcdvcmr2 2 1 2 8 6 rw | trcdvcmr3 2 1 2 12 6 rw
    trcdvcmr4 2 1 2 0 7 rw | trcdvcmr5 2 1 2 4 7 rw | trcdvcmr6 2 1 2 8 7 rw
    trcdvcmr7 2 1 2 12 7 rw | trcdvcvr0 2 1 2 0 4 rw | trcdvcvr1 2 1 2 4 4 rw
    trcdvcvr2 2 1 2 8 4 rw | trcdvcvr3 2 1 2 12 4 rw | trcdvcvr4 2 1 2 0 5 rw
    trcdvcvr5 2 1 2 4 5 rw | trcdvcvr6 2 1 2 8 5 rw | trcdvcvr7 2 1 2 12 5 rw
    trceventctl0r 2 1 0 8 0 rw | trceventctl1r 2 1 0 9 0 rw | trcextinselr1 2 1 0 9 4 rw
    trcextinselr2 2 1 0 10 4 rw | trcextinselr3 2 1 0 11 4 rw | trcidr0 2 1 0 8 7 r
    trcidr1 2 1 0 9 7 r | trcidr10 2 1 0 2 6 r | trcidr11 2 1 0 3 6 r
    trcidr12 2 1 0 4 6 r | trcidr13 2 1 0 5 6 r | trcidr2 2 1 0 10 7 r
    trcidr3 2 1 0 11 7 r | trcidr4 2 1 0 12 7 r | trcidr5 2 1 0 13 7 r
    trcidr6 2 1 0 14 7 r | trcidr7 2 1 0 15 7 r | trcidr8 2 1 0 0 6 r
    trcidr9 2 1 0 1 6 r | trcimspec0 2 1 0 0 7 rw | trcimspec1 2 1 0 1 7 rw
    trcimspec2 2 1 0 2 7 rw | trcimspec3 2 1 0 3 7 rw | trcimspec4 2 1 0 4 7 rw
    trcimspec5 2 1 0 5 7 rw | trcimspec6 2 1 0 6 7 rw | trcimspec7 2 1 0 7 7 rw
    trcitctrl 2 1 7 0 4 rw | trcitecr_el1 3 0 1 2 3 rw | trcitecr_el12 3 5 1 2 3 rw
    trcitecr_el2 3 4 1 2 3 rw | trciteedcr 2 1 0 2 1 rw | trclar 2 1 7 12 6 w
    trclsr 2 1 7 13 6 r | trcoslar 2 1 1 0 4 w | trcoslsr 2 1 1 1 4 r
    trcpdcr 2 1 1 4 4 rw | trcpdsr 2 1 1 5 4 r | trcpidr0 2 1 7 8 7 r
    trcpidr1 2 1 7 9 7 r | trcpidr2 2 1 7 10 7 r | trcpidr3 2 1 7 11 7 r
    trcpidr4 2 1 7 4 7 r | trcpidr5 2 1 7 5 7 r | trcpidr6 2 1 7 6 7 r
    trcpidr7 2 1 7 7 7 r | trcprgctlr 2 1 0 1 0 rw | trcprocselr 2 1 0 2 0 rw
    trcqctlr 2 1 0 1 1 rw | trcrsctlr10 2 1 1 10 0 rw | trcrsctlr11 2 1 1 11 0 rw
    trcrsctlr12 2 1 1 12 0 rw | trcrsctlr13 2 1 1 13 0 rw | trcrsctlr14 2 1 1 14 0 rw
    trcrsctlr15 2 1 1 15 0 rw | trcrsctlr16 2 1 1 0 1 rw | trcrsctlr17 2 1 1 1 1 rw
    trcrsctlr18 2 1 1 2 1 rw | trcrsctlr19 2 1 1 3 1 rw | trcrsctlr2 2 1 1 2 0 rw
    trcrsctlr20 2 1 1 4 1 rw | trcrsctlr21 2 1 1 5 1 rw | trcrsctlr22 2 1 1 6 1 rw
    trcrsctlr23 2 1 1 7 1 rw | trcrsctlr24 2 1 1 8 1 rw | trcrsctlr25 2 1 1 9 1 rw
    trcrsctlr26 2 1 1 10 1 rw | trcrsctlr27 2 1 1 11 1 rw | trcrsctlr28 2 1 1 12 1 rw
    trcrsctlr29 2 1 1 13 1 rw | trcrsctlr3 2 1 1 3 0 rw | trcrsctlr30 2 1 1 14 1 rw
    trcrsctlr31 2 1 1 15 1 rw | trcrsctlr4 2 1 1 4 0 rw | trcrsctlr5 2 1 1 5 0 rw
    trcrsctlr6 2 1 1 6 0 rw | trcrsctlr7 2 1 1 7 0 rw | trcrsctlr8 2 1 1 8 0 rw
    trcrsctlr9 2 1 1 9 0 rw | trcrsr 2 1 0 10 0 rw | trcseqevr0 2 1 0 0 4 rw
    trcseqevr1 2 1 0 1 4 rw | trcseqevr2 2 1 0 2 4 rw | trcseqrstevr 2 1 0 6 4 rw
    trcseqstr 2 1 0 7 4 rw | trcssccr0 2 1 1 0 2 rw | trcssccr1 2 1 1 1 2 rw
    trcssccr2 2 1 1 2 2 rw | trcssccr3 2 1 1 3 2 rw | trcssccr4 2 1 1 4 2 rw
    trcssccr5 2 1 1 5 2 rw | trcssccr6 2 1 1 6 2 rw | trcssccr7 2 1 1 7 2 rw
    trcsscsr0 2 1 1 8 2 rw | trcsscsr1 2 1 1 9 2 rw | trcsscsr2 2 1 1 10 2 rw
    trcsscsr3 2 1 1 11 2 rw | trcsscsr4 2 1 1 12 2 rw | trcsscsr5 2 1 1 13 2 rw
    trcsscsr6 2 1 1 14 2 rw | trcsscsr7 2 1 1 15 2 rw | trcsspcicr0 2 1 1 0 3 rw
    trcsspcicr1 2 1 1 1 3 rw | trcsspcicr2 2 1 1 2 3 rw | trcsspcicr3 2 1 1 3 3 rw
    trcsspcicr4 2 1 1 4 3 rw | trcsspcicr5 2 1 1 5 3 rw | trcsspcicr6 2 1 1 6 3 rw
    trcsspcicr7 2 1 1 7 3 rw | trcstallctlr 2 1 0 11 0 rw | trcstatr 2 1 0 3 0 r
    trcsyncpr 2 1 0 13 0 rw | trctraceidr 2 1 0 0 1 rw | trctsctlr 2 1 0 12 0 rw
    trcvdarcctlr 2 1 0 10 2 rw | trcvdctlr 2 1 0 8 2 rw | trcvdsacctlr 2 1 0 9 2 rw
    trcvictlr 2 1 0 0 2 rw | trcviiectlr 2 1 0 1 2 rw | trcvipcssctlr 2 1 0 3 2 rw
    trcvissctlr 2 1 0 2 2 rw | trcvmidcctlr0 2 1 3 2 2 rw | trcvmidcctlr1 2 1 3 3 2 rw
    trcvmidcvr0 2 1 3 0 1 rw | trcvmidcvr1 2 1 3 2 1 rw | trcvmidcvr2 2 1 3 4 1 rw
    trcvmidcvr3 2 1 3 6 1 rw | trcvmidcvr4 2 1 3 8 1 rw | trcvmidcvr5 2 1 3 10 1 rw
    trcvmidcvr6 2 1 3 12 1 rw | trcvmidcvr7 2 1 3 14 1 rw | trfcr_el1 3 0 1 2 1 rw
    trfcr_el12 3 5 1 2 1 rw | trfcr_el2 3 4 1 2 1 rw | ttbr0_el1 3 0 2 0 0 rwp
    ttbr0_el12 3 5 2 0 0 rwp | ttbr0_el2 3 4 2 0 0 rwp | ttbr0_el3 3 6 2 0 0 rw
    ttbr1_el1 3 0 2 0 1 rwp | ttbr1_el12 3 5 2 0 1 rwp | ttbr1_el2 3 4 2 0 1 rwp
    uao 3 0 4 2 4 rw | vbar_el1 3 0 12 0 0 rw | vbar_el12 3 5 12 0 0 rw
    vbar_el2 3 4 12 0 0 rw | vbar_el3 3 6 12 0 0 rw | vdisr_el2 3 4 12 1 1 rw
    vdisr_el3 3 6 12 1 1 rw | vmecid_a_el2 3 4 10 9 1 rw | vmecid_p_el2 3 4 10 9 0 rw
    vmpidr_el2 3 4 0 0 5 rw | vncr_el2 3 4 2 2 0 rw | vpidr_el2 3 4 0 0 0 rw
    vsesr_el2 3 4 5 2 3 rw | vsesr_el3 3 6 5 2 3 rw | vstcr_el2 3 4 2 6 2 rw
    vsttbr_el2 3 4 2 6 0 rw | vtcr_el2 3 4 2 1 2 rw | vttbr_el2 3 4 2 1 0 rwp
    zcr_el1 3 0 1 2 0 rw | zcr_el12 3 5 1 2 0 rw | zcr_el2 3 4 1 2 0 rw
    zcr_el3 3 6 1 2 0 rw
"""

REGS = {}
for line in LIST.strip().split('\n'):
    for ent in line.split('|'):
        f = ent.split()
        REGS[f[0]] = (tuple(int(x) for x in f[1:6]), f[6])


def enc(op0, op1, crn, crm, op2):
    return (((op0 - 2) << 14) | (op1 << 11) | (crn << 7) | (crm << 3) | op2)


names = sorted(REGS)
print('''// =============================================================================
// asm/a64sysreg.ax - the AArch64 assembler's named system registers for MRS
// and MSR, with the 15-bit encoding (op0 op1 CRn CRm op2) that MRS/MSR hold
// from bit 5; bit 15 is set for a register that may be read, bit 16 for one
// that may be written and bit 17 for a 128-bit one (of MRRS and MSRR).
// Generated from the Arm Architecture Reference Manual's System Register
// Encoding tables by cmd/internal/tools/gen_a64_sysregs.py: do not edit.  A
// register not listed here is still reachable by the generic
// S<op0>_<op1>_C<n>_C<m>_<op2> form.  The names are sorted (the assembler
// searches them by halves).
// =============================================================================''')
print('pub let NAMES: [%d][]u8 = [' % len(names))
for n in names:
    print('    "%s",' % n)
print('];')
print('pub let ENC: [%d]u64 = [' % len(names))
vals = []
for n in names:
    e, d = REGS[n]
    vals.append('0x%05x' % (enc(*e) | ((1 << 15) if 'r' in d else 0) | ((1 << 16) if 'w' in d else 0) |
                ((1 << 17) if 'p' in d else 0)))
for i in range(0, len(vals), 10):
    print('    ' + ', '.join(vals[i:i + 10]) + ',')
print('];')
