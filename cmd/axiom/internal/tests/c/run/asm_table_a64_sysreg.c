/* flags: --c-std=gnu17 */
/* native-only */
/* The AArch64 assembler's named system registers (the generated table, cmd/internal/tools/gen_a64_sysregs.py) against a byte table: every register once, read with mrs where it may be read and else written with msr, and the 128-bit ones with mrrs and msrr; the bytes were checked against the system assembler in development. */
#include "host.h"

#ifdef __AXIOM_ASM_AARCH64__
__asm__(".section .rodata.axiom_asm_table, \"a\"\n"
        ".globl axiom_asm_table\n"
        "axiom_asm_table:\n"
        "mrs x10, accdata_el1\n"
        "mrs x30, actlr_el1\n"
        "mrs x4, actlr_el2\n"
        "mrs x12, actlr_el3\n"
        "mrs x20, afsr0_el1\n"
        "mrs x1, afsr0_el12\n"
        "mrs x2, afsr0_el2\n"
        "mrs x26, afsr0_el3\n"
        "mrs x17, afsr1_el1\n"
        "mrs x3, afsr1_el12\n"
        "mrs x11, afsr1_el2\n"
        "mrs x18, afsr1_el3\n"
        "mrs x1, aidr_el1\n"
        "mrs x29, allint\n"
        "mrs x16, amair2_el1\n"
        "mrs x6, amair2_el12\n"
        "mrs x1, amair2_el2\n"
        "mrs x2, amair2_el3\n"
        "mrs x13, amair_el1\n"
        "mrs x13, amair_el12\n"
        "mrs x2, amair_el2\n"
        "mrs x7, amair_el3\n"
        "mrs x2, amcfgr_el0\n"
        "mrs x17, amcg1idr_el0\n"
        "mrs x13, amcgcr_el0\n"
        "mrs x1, amcntenclr0_el0\n"
        "mrs x26, amcntenclr1_el0\n"
        "mrs x18, amcntenset0_el0\n"
        "mrs x3, amcntenset1_el0\n"
        "mrs x30, amcr_el0\n"
        "mrs x7, amevcntr00_el0\n"
        "mrs x20, amevcntr01_el0\n"
        "mrs x20, amevcntr02_el0\n"
        "mrs x18, amevcntr03_el0\n"
        "mrs x30, amevcntr10_el0\n"
        "mrs x1, amevcntr110_el0\n"
        "mrs x18, amevcntr111_el0\n"
        "mrs x18, amevcntr112_el0\n"
        "mrs x12, amevcntr113_el0\n"
        "mrs x1, amevcntr114_el0\n"
        "mrs x7, amevcntr115_el0\n"
        "mrs x1, amevcntr11_el0\n"
        "mrs x17, amevcntr12_el0\n"
        "mrs x27, amevcntr13_el0\n"
        "mrs x4, amevcntr14_el0\n"
        "mrs x9, amevcntr15_el0\n"
        "mrs x13, amevcntr16_el0\n"
        "mrs x4, amevcntr17_el0\n"
        "mrs x17, amevcntr18_el0\n"
        "mrs x3, amevcntr19_el0\n"
        "mrs x18, amevcntvoff00_el2\n"
        "mrs x9, amevcntvoff010_el2\n"
        "mrs x17, amevcntvoff011_el2\n"
        "mrs x26, amevcntvoff012_el2\n"
        "mrs x21, amevcntvoff013_el2\n"
        "mrs x5, amevcntvoff014_el2\n"
        "mrs x3, amevcntvoff015_el2\n"
        "mrs x18, amevcntvoff01_el2\n"
        "mrs x18, amevcntvoff02_el2\n"
        "mrs x20, amevcntvoff03_el2\n"
        "mrs x6, amevcntvoff04_el2\n"
        "mrs x11, amevcntvoff05_el2\n"
        "mrs x3, amevcntvoff06_el2\n"
        "mrs x17, amevcntvoff07_el2\n"
        "mrs x22, amevcntvoff08_el2\n"
        "mrs x2, amevcntvoff09_el2\n"
        "mrs x18, amevcntvoff10_el2\n"
        "mrs x1, amevcntvoff110_el2\n"
        "mrs x19, amevcntvoff111_el2\n"
        "mrs x6, amevcntvoff112_el2\n"
        "mrs x15, amevcntvoff113_el2\n"
        "mrs x21, amevcntvoff114_el2\n"
        "mrs x17, amevcntvoff115_el2\n"
        "mrs x13, amevcntvoff11_el2\n"
        "mrs x24, amevcntvoff12_el2\n"
        "mrs x10, amevcntvoff13_el2\n"
        "mrs x14, amevcntvoff14_el2\n"
        "mrs x18, amevcntvoff15_el2\n"
        "mrs x29, amevcntvoff16_el2\n"
        "mrs x14, amevcntvoff17_el2\n"
        "mrs x11, amevcntvoff18_el2\n"
        "mrs x9, amevcntvoff19_el2\n"
        "mrs x7, amevtyper00_el0\n"
        "mrs x25, amevtyper01_el0\n"
        "mrs x5, amevtyper02_el0\n"
        "mrs x22, amevtyper03_el0\n"
        "mrs x24, amevtyper10_el0\n"
        "mrs x7, amevtyper110_el0\n"
        "mrs x2, amevtyper111_el0\n"
        "mrs x18, amevtyper112_el0\n"
        "mrs x9, amevtyper113_el0\n"
        "mrs x16, amevtyper114_el0\n"
        "mrs x15, amevtyper115_el0\n"
        "mrs x28, amevtyper11_el0\n"
        "mrs x10, amevtyper12_el0\n"
        "mrs x23, amevtyper13_el0\n"
        "mrs x14, amevtyper14_el0\n"
        "mrs x9, amevtyper15_el0\n"
        "mrs x19, amevtyper16_el0\n"
        "mrs x2, amevtyper17_el0\n"
        "mrs x3, amevtyper18_el0\n"
        "mrs x16, amevtyper19_el0\n"
        "mrs x13, amuserenr_el0\n"
        "mrs x5, apdakeyhi_el1\n"
        "mrs x24, apdakeylo_el1\n"
        "mrs x10, apdbkeyhi_el1\n"
        "mrs x4, apdbkeylo_el1\n"
        "mrs x29, apgakeyhi_el1\n"
        "mrs x15, apgakeylo_el1\n"
        "mrs x13, apiakeyhi_el1\n"
        "mrs x1, apiakeylo_el1\n"
        "mrs x30, apibkeyhi_el1\n"
        "mrs x21, apibkeylo_el1\n"
        "mrs x2, brbcr_el1\n"
        "mrs x24, brbcr_el12\n"
        "mrs x17, brbcr_el2\n"
        "mrs x18, brbfcr_el1\n"
        "mrs x25, brbidr0_el1\n"
        "mrs x28, brbinf0_el1\n"
        "mrs x26, brbinf10_el1\n"
        "mrs x10, brbinf11_el1\n"
        "mrs x10, brbinf12_el1\n"
        "mrs x22, brbinf13_el1\n"
        "mrs x11, brbinf14_el1\n"
        "mrs x19, brbinf15_el1\n"
        "mrs x15, brbinf16_el1\n"
        "mrs x18, brbinf17_el1\n"
        "mrs x25, brbinf18_el1\n"
        "mrs x14, brbinf19_el1\n"
        "mrs x2, brbinf1_el1\n"
        "mrs x26, brbinf20_el1\n"
        "mrs x2, brbinf21_el1\n"
        "mrs x30, brbinf22_el1\n"
        "mrs x8, brbinf23_el1\n"
        "mrs x15, brbinf24_el1\n"
        "mrs x22, brbinf25_el1\n"
        "mrs x21, brbinf26_el1\n"
        "mrs x2, brbinf27_el1\n"
        "mrs x1, brbinf28_el1\n"
        "mrs x23, brbinf29_el1\n"
        "mrs x22, brbinf2_el1\n"
        "mrs x9, brbinf30_el1\n"
        "mrs x20, brbinf31_el1\n"
        "mrs x18, brbinf3_el1\n"
        "mrs x21, brbinf4_el1\n"
        "mrs x26, brbinf5_el1\n"
        "mrs x14, brbinf6_el1\n"
        "mrs x9, brbinf7_el1\n"
        "mrs x22, brbinf8_el1\n"
        "mrs x12, brbinf9_el1\n"
        "mrs x28, brbinfinj_el1\n"
        "mrs x21, brbsrc0_el1\n"
        "mrs x11, brbsrc10_el1\n"
        "mrs x0, brbsrc11_el1\n"
        "mrs x30, brbsrc12_el1\n"
        "mrs x14, brbsrc13_el1\n"
        "mrs x11, brbsrc14_el1\n"
        "mrs x5, brbsrc15_el1\n"
        "mrs x19, brbsrc16_el1\n"
        "mrs x3, brbsrc17_el1\n"
        "mrs x15, brbsrc18_el1\n"
        "mrs x1, brbsrc19_el1\n"
        "mrs x6, brbsrc1_el1\n"
        "mrs x24, brbsrc20_el1\n"
        "mrs x9, brbsrc21_el1\n"
        "mrs x4, brbsrc22_el1\n"
        "mrs x23, brbsrc23_el1\n"
        "mrs x7, brbsrc24_el1\n"
        "mrs x12, brbsrc25_el1\n"
        "mrs x12, brbsrc26_el1\n"
        "mrs x29, brbsrc27_el1\n"
        "mrs x27, brbsrc28_el1\n"
        "mrs x15, brbsrc29_el1\n"
        "mrs x2, brbsrc2_el1\n"
        "mrs x5, brbsrc30_el1\n"
        "mrs x14, brbsrc31_el1\n"
        "mrs x12, brbsrc3_el1\n"
        "mrs x17, brbsrc4_el1\n"
        "mrs x8, brbsrc5_el1\n"
        "mrs x28, brbsrc6_el1\n"
        "mrs x4, brbsrc7_el1\n"
        "mrs x26, brbsrc8_el1\n"
        "mrs x13, brbsrc9_el1\n"
        "mrs x27, brbsrcinj_el1\n"
        "mrs x17, brbtgt0_el1\n"
        "mrs x8, brbtgt10_el1\n"
        "mrs x22, brbtgt11_el1\n"
        "mrs x13, brbtgt12_el1\n"
        "mrs x11, brbtgt13_el1\n"
        "mrs x21, brbtgt14_el1\n"
        "mrs x28, brbtgt15_el1\n"
        "mrs x12, brbtgt16_el1\n"
        "mrs x30, brbtgt17_el1\n"
        "mrs x7, brbtgt18_el1\n"
        "mrs x4, brbtgt19_el1\n"
        "mrs x2, brbtgt1_el1\n"
        "mrs x5, brbtgt20_el1\n"
        "mrs x4, brbtgt21_el1\n"
        "mrs x7, brbtgt22_el1\n"
        "mrs x21, brbtgt23_el1\n"
        "mrs x7, brbtgt24_el1\n"
        "mrs x0, brbtgt25_el1\n"
        "mrs x15, brbtgt26_el1\n"
        "mrs x26, brbtgt27_el1\n"
        "mrs x18, brbtgt28_el1\n"
        "mrs x5, brbtgt29_el1\n"
        "mrs x8, brbtgt2_el1\n"
        "mrs x9, brbtgt30_el1\n"
        "mrs x0, brbtgt31_el1\n"
        "mrs x4, brbtgt3_el1\n"
        "mrs x13, brbtgt4_el1\n"
        "mrs x17, brbtgt5_el1\n"
        "mrs x11, brbtgt6_el1\n"
        "mrs x19, brbtgt7_el1\n"
        "mrs x18, brbtgt8_el1\n"
        "mrs x10, brbtgt9_el1\n"
        "mrs x30, brbtgtinj_el1\n"
        "mrs x4, brbts_el1\n"
        "mrs x22, ccsidr2_el1\n"
        "mrs x27, ccsidr_el1\n"
        "mrs x16, clidr_el1\n"
        "mrs x30, cntfrq_el0\n"
        "mrs x19, cnthctl_el2\n"
        "mrs x20, cnthp_ctl_el2\n"
        "mrs x21, cnthp_cval_el2\n"
        "mrs x23, cnthp_tval_el2\n"
        "mrs x1, cnthps_ctl_el2\n"
        "mrs x14, cnthps_cval_el2\n"
        "mrs x28, cnthps_tval_el2\n"
        "mrs x27, cnthv_ctl_el2\n"
        "mrs x24, cnthv_cval_el2\n"
        "mrs x30, cnthv_tval_el2\n"
        "mrs x27, cnthvs_ctl_el2\n"
        "mrs x21, cnthvs_cval_el2\n"
        "mrs x25, cnthvs_tval_el2\n"
        "mrs x17, cntiscale_el2\n"
        "mrs x12, cntkctl_el1\n"
        "mrs x12, cntkctl_el12\n"
        "mrs x12, cntp_ctl_el0\n"
        "mrs x12, cntp_ctl_el02\n"
        "mrs x3, cntp_cval_el0\n"
        "mrs x15, cntp_cval_el02\n"
        "mrs x20, cntp_tval_el0\n"
        "mrs x12, cntp_tval_el02\n"
        "mrs x1, cntpct_el0\n"
        "mrs x6, cntpctss_el0\n"
        "mrs x2, cntpoff_el2\n"
        "mrs x6, cntps_ctl_el1\n"
        "mrs x14, cntps_cval_el1\n"
        "mrs x5, cntps_tval_el1\n"
        "mrs x3, cntscale_el2\n"
        "mrs x10, cntv_ctl_el0\n"
        "mrs x19, cntv_ctl_el02\n"
        "mrs x1, cntv_cval_el0\n"
        "mrs x3, cntv_cval_el02\n"
        "mrs x0, cntv_tval_el0\n"
        "mrs x18, cntv_tval_el02\n"
        "mrs x4, cntvct_el0\n"
        "mrs x17, cntvctss_el0\n"
        "mrs x3, cntvfrq_el2\n"
        "mrs x30, cntvoff_el2\n"
        "mrs x11, contextidr_el1\n"
        "mrs x19, contextidr_el12\n"
        "mrs x0, contextidr_el2\n"
        "mrs x2, cpacr_el1\n"
        "mrs x27, cpacr_el12\n"
        "mrs x6, cpm_ioacc_ctl_el3\n"
        "mrs x19, cptr_el2\n"
        "mrs x12, cptr_el3\n"
        "mrs x4, csselr_el1\n"
        "mrs x20, ctr_el0\n"
        "mrs x8, currentel\n"
        "mrs x30, dacr32_el2\n"
        "mrs x11, daif\n"
        "mrs x19, dbgauthstatus_el1\n"
        "mrs x11, dbgbcr0_el1\n"
        "mrs x15, dbgbcr10_el1\n"
        "mrs x3, dbgbcr11_el1\n"
        "mrs x3, dbgbcr12_el1\n"
        "mrs x27, dbgbcr13_el1\n"
        "mrs x15, dbgbcr14_el1\n"
        "mrs x14, dbgbcr15_el1\n"
        "mrs x15, dbgbcr1_el1\n"
        "mrs x15, dbgbcr2_el1\n"
        "mrs x9, dbgbcr3_el1\n"
        "mrs x2, dbgbcr4_el1\n"
        "mrs x4, dbgbcr5_el1\n"
        "mrs x3, dbgbcr6_el1\n"
        "mrs x23, dbgbcr7_el1\n"
        "mrs x10, dbgbcr8_el1\n"
        "mrs x23, dbgbcr9_el1\n"
        "mrs x8, dbgbvr0_el1\n"
        "mrs x15, dbgbvr10_el1\n"
        "mrs x26, dbgbvr11_el1\n"
        "mrs x22, dbgbvr12_el1\n"
        "mrs x5, dbgbvr13_el1\n"
        "mrs x16, dbgbvr14_el1\n"
        "mrs x0, dbgbvr15_el1\n"
        "mrs x6, dbgbvr1_el1\n"
        "mrs x30, dbgbvr2_el1\n"
        "mrs x30, dbgbvr3_el1\n"
        "mrs x16, dbgbvr4_el1\n"
        "mrs x11, dbgbvr5_el1\n"
        "mrs x4, dbgbvr6_el1\n"
        "mrs x22, dbgbvr7_el1\n"
        "mrs x17, dbgbvr8_el1\n"
        "mrs x29, dbgbvr9_el1\n"
        "mrs x0, dbgclaimclr_el1\n"
        "mrs x24, dbgclaimset_el1\n"
        "mrs x16, dbgdtr_el0\n"
        "mrs x9, dbgdtrrx_el0\n"
        "msr dbgdtrtx_el0, x20\n"
        "mrs x27, dbgprcr_el1\n"
        "mrs x2, dbgvcr32_el2\n"
        "mrs x22, dbgwcr0_el1\n"
        "mrs x27, dbgwcr10_el1\n"
        "mrs x8, dbgwcr11_el1\n"
        "mrs x16, dbgwcr12_el1\n"
        "mrs x11, dbgwcr13_el1\n"
        "mrs x29, dbgwcr14_el1\n"
        "mrs x5, dbgwcr15_el1\n"
        "mrs x11, dbgwcr1_el1\n"
        "mrs x24, dbgwcr2_el1\n"
        "mrs x7, dbgwcr3_el1\n"
        "mrs x17, dbgwcr4_el1\n"
        "mrs x17, dbgwcr5_el1\n"
        "mrs x24, dbgwcr6_el1\n"
        "mrs x16, dbgwcr7_el1\n"
        "mrs x10, dbgwcr8_el1\n"
        "mrs x20, dbgwcr9_el1\n"
        "mrs x7, dbgwvr0_el1\n"
        "mrs x19, dbgwvr10_el1\n"
        "mrs x25, dbgwvr11_el1\n"
        "mrs x25, dbgwvr12_el1\n"
        "mrs x24, dbgwvr13_el1\n"
        "mrs x27, dbgwvr14_el1\n"
        "mrs x6, dbgwvr15_el1\n"
        "mrs x25, dbgwvr1_el1\n"
        "mrs x7, dbgwvr2_el1\n"
        "mrs x26, dbgwvr3_el1\n"
        "mrs x12, dbgwvr4_el1\n"
        "mrs x23, dbgwvr5_el1\n"
        "mrs x25, dbgwvr6_el1\n"
        "mrs x7, dbgwvr7_el1\n"
        "mrs x6, dbgwvr8_el1\n"
        "mrs x16, dbgwvr9_el1\n"
        "mrs x15, dczid_el0\n"
        "mrs x11, disr_el1\n"
        "mrs x23, dit\n"
        "mrs x0, dlr_el0\n"
        "mrs x0, dspsr_el0\n"
        "mrs x25, elr_el1\n"
        "mrs x8, elr_el12\n"
        "mrs x15, elr_el2\n"
        "mrs x8, elr_el3\n"
        "mrs x6, erridr_el1\n"
        "mrs x22, errselr_el1\n"
        "mrs x19, erxaddr_el1\n"
        "mrs x30, erxctlr_el1\n"
        "mrs x11, erxfr_el1\n"
        "mrs x14, erxgsr_el1\n"
        "mrs x25, erxmisc0_el1\n"
        "mrs x29, erxmisc1_el1\n"
        "mrs x23, erxmisc2_el1\n"
        "mrs x11, erxmisc3_el1\n"
        "mrs x30, erxpfgcdn_el1\n"
        "mrs x11, erxpfgctl_el1\n"
        "mrs x2, erxpfgf_el1\n"
        "mrs x7, erxstatus_el1\n"
        "mrs x3, esr_el1\n"
        "mrs x7, esr_el12\n"
        "mrs x15, esr_el2\n"
        "mrs x6, esr_el3\n"
        "mrs x10, far_el1\n"
        "mrs x6, far_el12\n"
        "mrs x15, far_el2\n"
        "mrs x19, far_el3\n"
        "mrs x28, fgwte3_el3\n"
        "mrs x19, fpcr\n"
        "mrs x26, fpexc32_el2\n"
        "mrs x0, fpmr\n"
        "mrs x15, fpsr\n"
        "mrs x29, gcr_el1\n"
        "mrs x20, gcscr_el1\n"
        "mrs x11, gcscr_el12\n"
        "mrs x25, gcscr_el2\n"
        "mrs x20, gcscr_el3\n"
        "mrs x2, gcscre0_el1\n"
        "mrs x26, gcspr_el0\n"
        "mrs x21, gcspr_el1\n"
        "mrs x3, gcspr_el12\n"
        "mrs x29, gcspr_el2\n"
        "mrs x12, gcspr_el3\n"
        "mrs x25, gmid_el1\n"
        "mrs x22, gpccr_el3\n"
        "mrs x24, gptbr_el3\n"
        "mrs x6, hacdbsbr_el2\n"
        "mrs x15, hacdbscons_el2\n"
        "mrs x28, hacr_el2\n"
        "mrs x5, hafgrtr_el2\n"
        "mrs x13, hcr_el2\n"
        "mrs x25, hcrx_el2\n"
        "mrs x20, hdbssbr_el2\n"
        "mrs x10, hdbssprod_el2\n"
        "mrs x2, hdfgrtr2_el2\n"
        "mrs x25, hdfgrtr_el2\n"
        "mrs x30, hdfgwtr2_el2\n"
        "mrs x23, hdfgwtr_el2\n"
        "mrs x12, hfgitr2_el2\n"
        "mrs x14, hfgitr_el2\n"
        "mrs x12, hfgrtr2_el2\n"
        "mrs x23, hfgrtr_el2\n"
        "mrs x30, hfgwtr2_el2\n"
        "mrs x2, hfgwtr_el2\n"
        "mrs x23, hpfar_el2\n"
        "mrs x5, hstr_el2\n"
        "mrs x5, icc_ap0r0_el1\n"
        "mrs x4, icc_ap0r1_el1\n"
        "mrs x0, icc_ap0r2_el1\n"
        "mrs x4, icc_ap0r3_el1\n"
        "mrs x18, icc_ap1r0_el1\n"
        "mrs x28, icc_ap1r1_el1\n"
        "mrs x14, icc_ap1r2_el1\n"
        "mrs x25, icc_ap1r3_el1\n"
        "msr icc_asgi1r_el1, x20\n"
        "mrs x4, icc_bpr0_el1\n"
        "mrs x19, icc_bpr1_el1\n"
        "mrs x26, icc_ctlr_el1\n"
        "mrs x19, icc_ctlr_el3\n"
        "msr icc_dir_el1, x15\n"
        "msr icc_eoir0_el1, x21\n"
        "msr icc_eoir1_el1, x29\n"
        "mrs x11, icc_hppir0_el1\n"
        "mrs x4, icc_hppir1_el1\n"
        "mrs x17, icc_iar0_el1\n"
        "mrs x17, icc_iar1_el1\n"
        "mrs x4, icc_igrpen0_el1\n"
        "mrs x0, icc_igrpen1_el1\n"
        "mrs x0, icc_igrpen1_el3\n"
        "mrs x25, icc_nmiar1_el1\n"
        "mrs x23, icc_pmr_el1\n"
        "mrs x20, icc_rpr_el1\n"
        "msr icc_sgi0r_el1, x3\n"
        "msr icc_sgi1r_el1, x16\n"
        "mrs x23, icc_sre_el1\n"
        "mrs x29, icc_sre_el2\n"
        "mrs x4, icc_sre_el3\n"
        "mrs x13, ich_ap0r0_el2\n"
        "mrs x27, ich_ap0r1_el2\n"
        "mrs x6, ich_ap0r2_el2\n"
        "mrs x26, ich_ap0r3_el2\n"
        "mrs x27, ich_ap1r0_el2\n"
        "mrs x6, ich_ap1r1_el2\n"
        "mrs x0, ich_ap1r2_el2\n"
        "mrs x8, ich_ap1r3_el2\n"
        "mrs x6, ich_eisr_el2\n"
        "mrs x9, ich_elrsr_el2\n"
        "mrs x16, ich_hcr_el2\n"
        "mrs x7, ich_lr0_el2\n"
        "mrs x24, ich_lr10_el2\n"
        "mrs x18, ich_lr11_el2\n"
        "mrs x10, ich_lr12_el2\n"
        "mrs x8, ich_lr13_el2\n"
        "mrs x17, ich_lr14_el2\n"
        "mrs x13, ich_lr15_el2\n"
        "mrs x26, ich_lr1_el2\n"
        "mrs x4, ich_lr2_el2\n"
        "mrs x1, ich_lr3_el2\n"
        "mrs x29, ich_lr4_el2\n"
        "mrs x23, ich_lr5_el2\n"
        "mrs x11, ich_lr6_el2\n"
        "mrs x28, ich_lr7_el2\n"
        "mrs x14, ich_lr8_el2\n"
        "mrs x21, ich_lr9_el2\n"
        "mrs x18, ich_misr_el2\n"
        "mrs x26, ich_vmcr_el2\n"
        "mrs x28, ich_vtr_el2\n"
        "mrs x16, id_aa64afr0_el1\n"
        "mrs x13, id_aa64afr1_el1\n"
        "mrs x26, id_aa64dfr0_el1\n"
        "mrs x29, id_aa64dfr1_el1\n"
        "mrs x28, id_aa64dfr2_el1\n"
        "mrs x16, id_aa64fpfr0_el1\n"
        "mrs x4, id_aa64isar0_el1\n"
        "mrs x17, id_aa64isar1_el1\n"
        "mrs x4, id_aa64isar2_el1\n"
        "mrs x16, id_aa64isar3_el1\n"
        "mrs x16, id_aa64mmfr0_el1\n"
        "mrs x0, id_aa64mmfr1_el1\n"
        "mrs x27, id_aa64mmfr2_el1\n"
        "mrs x14, id_aa64mmfr3_el1\n"
        "mrs x24, id_aa64mmfr4_el1\n"
        "mrs x5, id_aa64pfr0_el1\n"
        "mrs x19, id_aa64pfr1_el1\n"
        "mrs x0, id_aa64pfr2_el1\n"
        "mrs x24, id_aa64smfr0_el1\n"
        "mrs x25, id_aa64zfr0_el1\n"
        "mrs x4, id_afr0_el1\n"
        "mrs x5, id_dfr0_el1\n"
        "mrs x4, id_dfr1_el1\n"
        "mrs x15, id_isar0_el1\n"
        "mrs x19, id_isar1_el1\n"
        "mrs x23, id_isar2_el1\n"
        "mrs x3, id_isar3_el1\n"
        "mrs x17, id_isar4_el1\n"
        "mrs x1, id_isar5_el1\n"
        "mrs x10, id_isar6_el1\n"
        "mrs x21, id_mmfr0_el1\n"
        "mrs x16, id_mmfr1_el1\n"
        "mrs x16, id_mmfr2_el1\n"
        "mrs x17, id_mmfr3_el1\n"
        "mrs x15, id_mmfr4_el1\n"
        "mrs x25, id_mmfr5_el1\n"
        "mrs x24, id_pfr0_el1\n"
        "mrs x3, id_pfr1_el1\n"
        "mrs x28, id_pfr2_el1\n"
        "mrs x17, ifsr32_el2\n"
        "mrs x1, isr_el1\n"
        "mrs x7, lorc_el1\n"
        "mrs x6, lorea_el1\n"
        "mrs x8, lorid_el1\n"
        "mrs x1, lorn_el1\n"
        "mrs x24, lorsa_el1\n"
        "mrs x3, mair2_el1\n"
        "mrs x16, mair2_el12\n"
        "mrs x14, mair2_el2\n"
        "mrs x17, mair2_el3\n"
        "mrs x0, mair_el1\n"
        "mrs x24, mair_el12\n"
        "mrs x28, mair_el2\n"
        "mrs x29, mair_el3\n"
        "mrs x2, mdccint_el1\n"
        "mrs x14, mdccsr_el0\n"
        "mrs x10, mdcr_el2\n"
        "mrs x19, mdcr_el3\n"
        "mrs x16, mdrar_el1\n"
        "mrs x19, mdscr_el1\n"
        "mrs x16, mdselr_el1\n"
        "mrs x6, mdstepop_el1\n"
        "mrs x22, mecid_a0_el2\n"
        "mrs x8, mecid_a1_el2\n"
        "mrs x14, mecid_p0_el2\n"
        "mrs x16, mecid_p1_el2\n"
        "mrs x17, mecid_rl_a_el3\n"
        "mrs x25, mecidr_el2\n"
        "mrs x15, mfar_el3\n"
        "mrs x16, midr_el1\n"
        "mrs x30, mpam0_el1\n"
        "mrs x7, mpam1_el1\n"
        "mrs x22, mpam1_el12\n"
        "mrs x16, mpam2_el2\n"
        "mrs x28, mpam3_el3\n"
        "mrs x28, mpamhcr_el2\n"
        "mrs x30, mpamidr_el1\n"
        "mrs x29, mpamsm_el1\n"
        "mrs x8, mpamvpm0_el2\n"
        "mrs x29, mpamvpm1_el2\n"
        "mrs x17, mpamvpm2_el2\n"
        "mrs x28, mpamvpm3_el2\n"
        "mrs x30, mpamvpm4_el2\n"
        "mrs x6, mpamvpm5_el2\n"
        "mrs x26, mpamvpm6_el2\n"
        "mrs x14, mpamvpm7_el2\n"
        "mrs x4, mpamvpmv_el2\n"
        "mrs x13, mpidr_el1\n"
        "mrs x3, mpuir_el1\n"
        "mrs x12, mpuir_el2\n"
        "mrs x14, mvfr0_el1\n"
        "mrs x10, mvfr1_el1\n"
        "mrs x2, mvfr2_el1\n"
        "mrs x21, nzcv\n"
        "mrs x7, osdlr_el1\n"
        "mrs x13, osdtrrx_el1\n"
        "mrs x2, osdtrtx_el1\n"
        "mrs x6, oseccr_el1\n"
        "msr oslar_el1, x21\n"
        "mrs x9, oslsr_el1\n"
        "mrs x25, pan\n"
        "mrs x3, par_el1\n"
        "mrs x28, pfar_el1\n"
        "mrs x24, pfar_el12\n"
        "mrs x4, pfar_el2\n"
        "mrs x30, pir_el1\n"
        "mrs x22, pir_el12\n"
        "mrs x20, pir_el2\n"
        "mrs x21, pir_el3\n"
        "mrs x11, pire0_el1\n"
        "mrs x4, pire0_el12\n"
        "mrs x8, pire0_el2\n"
        "mrs x28, pm\n"
        "mrs x4, pmbidr_el1\n"
        "mrs x30, pmblimitr_el1\n"
        "mrs x14, pmbptr_el1\n"
        "mrs x7, pmbsr_el1\n"
        "mrs x23, pmccfiltr_el0\n"
        "mrs x30, pmccntr_el0\n"
        "mrs x3, pmccntsvr_el1\n"
        "mrs x12, pmceid0_el0\n"
        "mrs x28, pmceid1_el0\n"
        "mrs x15, pmcntenclr_el0\n"
        "mrs x5, pmcntenset_el0\n"
        "mrs x21, pmcr_el0\n"
        "mrs x26, pmecr_el1\n"
        "mrs x7, pmevcntr0_el0\n"
        "mrs x5, pmevcntr10_el0\n"
        "mrs x22, pmevcntr11_el0\n"
        "mrs x13, pmevcntr12_el0\n"
        "mrs x16, pmevcntr13_el0\n"
        "mrs x12, pmevcntr14_el0\n"
        "mrs x10, pmevcntr15_el0\n"
        "mrs x13, pmevcntr16_el0\n"
        "mrs x6, pmevcntr17_el0\n"
        "mrs x11, pmevcntr18_el0\n"
        "mrs x10, pmevcntr19_el0\n"
        "mrs x2, pmevcntr1_el0\n"
        "mrs x23, pmevcntr20_el0\n"
        "mrs x11, pmevcntr21_el0\n"
        "mrs x0, pmevcntr22_el0\n"
        "mrs x10, pmevcntr23_el0\n"
        "mrs x17, pmevcntr24_el0\n"
        "mrs x14, pmevcntr25_el0\n"
        "mrs x14, pmevcntr26_el0\n"
        "mrs x22, pmevcntr27_el0\n"
        "mrs x0, pmevcntr28_el0\n"
        "mrs x12, pmevcntr29_el0\n"
        "mrs x10, pmevcntr2_el0\n"
        "mrs x16, pmevcntr30_el0\n"
        "mrs x19, pmevcntr3_el0\n"
        "mrs x9, pmevcntr4_el0\n"
        "mrs x16, pmevcntr5_el0\n"
        "mrs x30, pmevcntr6_el0\n"
        "mrs x2, pmevcntr7_el0\n"
        "mrs x3, pmevcntr8_el0\n"
        "mrs x29, pmevcntr9_el0\n"
        "mrs x25, pmevcntsvr0_el1\n"
        "mrs x7, pmevcntsvr10_el1\n"
        "mrs x28, pmevcntsvr11_el1\n"
        "mrs x3, pmevcntsvr12_el1\n"
        "mrs x2, pmevcntsvr13_el1\n"
        "mrs x8, pmevcntsvr14_el1\n"
        "mrs x8, pmevcntsvr15_el1\n"
        "mrs x1, pmevcntsvr16_el1\n"
        "mrs x28, pmevcntsvr17_el1\n"
        "mrs x24, pmevcntsvr18_el1\n"
        "mrs x5, pmevcntsvr19_el1\n"
        "mrs x8, pmevcntsvr1_el1\n"
        "mrs x24, pmevcntsvr20_el1\n"
        "mrs x4, pmevcntsvr21_el1\n"
        "mrs x26, pmevcntsvr22_el1\n"
        "mrs x13, pmevcntsvr23_el1\n"
        "mrs x27, pmevcntsvr24_el1\n"
        "mrs x29, pmevcntsvr25_el1\n"
        "mrs x21, pmevcntsvr26_el1\n"
        "mrs x26, pmevcntsvr27_el1\n"
        "mrs x30, pmevcntsvr28_el1\n"
        "mrs x8, pmevcntsvr29_el1\n"
        "mrs x12, pmevcntsvr2_el1\n"
        "mrs x4, pmevcntsvr30_el1\n"
        "mrs x17, pmevcntsvr3_el1\n"
        "mrs x29, pmevcntsvr4_el1\n"
        "mrs x16, pmevcntsvr5_el1\n"
        "mrs x18, pmevcntsvr6_el1\n"
        "mrs x15, pmevcntsvr7_el1\n"
        "mrs x22, pmevcntsvr8_el1\n"
        "mrs x10, pmevcntsvr9_el1\n"
        "mrs x2, pmevtyper0_el0\n"
        "mrs x8, pmevtyper10_el0\n"
        "mrs x1, pmevtyper11_el0\n"
        "mrs x25, pmevtyper12_el0\n"
        "mrs x22, pmevtyper13_el0\n"
        "mrs x5, pmevtyper14_el0\n"
        "mrs x13, pmevtyper15_el0\n"
        "mrs x28, pmevtyper16_el0\n"
        "mrs x2, pmevtyper17_el0\n"
        "mrs x8, pmevtyper18_el0\n"
        "mrs x30, pmevtyper19_el0\n"
        "mrs x0, pmevtyper1_el0\n"
        "mrs x20, pmevtyper20_el0\n"
        "mrs x2, pmevtyper21_el0\n"
        "mrs x25, pmevtyper22_el0\n"
        "mrs x8, pmevtyper23_el0\n"
        "mrs x2, pmevtyper24_el0\n"
        "mrs x19, pmevtyper25_el0\n"
        "mrs x27, pmevtyper26_el0\n"
        "mrs x7, pmevtyper27_el0\n"
        "mrs x2, pmevtyper28_el0\n"
        "mrs x8, pmevtyper29_el0\n"
        "mrs x27, pmevtyper2_el0\n"
        "mrs x3, pmevtyper30_el0\n"
        "mrs x14, pmevtyper3_el0\n"
        "mrs x0, pmevtyper4_el0\n"
        "mrs x10, pmevtyper5_el0\n"
        "mrs x17, pmevtyper6_el0\n"
        "mrs x13, pmevtyper7_el0\n"
        "mrs x29, pmevtyper8_el0\n"
        "mrs x29, pmevtyper9_el0\n"
        "mrs x8, pmiar_el1\n"
        "mrs x19, pmicfiltr_el0\n"
        "mrs x4, pmicntr_el0\n"
        "mrs x1, pmicntsvr_el1\n"
        "mrs x16, pmintenclr_el1\n"
        "mrs x22, pmintenset_el1\n"
        "mrs x7, pmmir_el1\n"
        "mrs x30, pmovsclr_el0\n"
        "mrs x3, pmovsset_el0\n"
        "mrs x5, pmscr_el1\n"
        "mrs x8, pmscr_el12\n"
        "mrs x1, pmscr_el2\n"
        "mrs x5, pmsdsfr_el1\n"
        "mrs x6, pmselr_el0\n"
        "mrs x29, pmsevfr_el1\n"
        "mrs x9, pmsfcr_el1\n"
        "mrs x20, pmsicr_el1\n"
        "mrs x9, pmsidr_el1\n"
        "mrs x16, pmsirr_el1\n"
        "mrs x24, pmslatfr_el1\n"
        "mrs x6, pmsnevfr_el1\n"
        "mrs x9, pmsscr_el1\n"
        "msr pmswinc_el0, x14\n"
        "mrs x16, pmuacr_el1\n"
        "mrs x21, pmuserenr_el0\n"
        "mrs x5, pmxevcntr_el0\n"
        "mrs x8, pmxevtyper_el0\n"
        "msr pmzr_el0, x11\n"
        "mrs x25, por_el0\n"
        "mrs x0, por_el1\n"
        "mrs x8, por_el12\n"
        "mrs x1, por_el2\n"
        "mrs x0, por_el3\n"
        "mrs x0, prbar10_el1\n"
        "mrs x23, prbar10_el2\n"
        "mrs x16, prbar11_el1\n"
        "mrs x17, prbar11_el2\n"
        "mrs x6, prbar12_el1\n"
        "mrs x16, prbar12_el2\n"
        "mrs x15, prbar13_el1\n"
        "mrs x7, prbar13_el2\n"
        "mrs x29, prbar14_el1\n"
        "mrs x14, prbar14_el2\n"
        "mrs x3, prbar15_el1\n"
        "mrs x21, prbar15_el2\n"
        "mrs x26, prbar1_el1\n"
        "mrs x20, prbar1_el2\n"
        "mrs x13, prbar2_el1\n"
        "mrs x21, prbar2_el2\n"
        "mrs x15, prbar3_el1\n"
        "mrs x17, prbar3_el2\n"
        "mrs x26, prbar4_el1\n"
        "mrs x28, prbar4_el2\n"
        "mrs x12, prbar5_el1\n"
        "mrs x16, prbar5_el2\n"
        "mrs x9, prbar6_el1\n"
        "mrs x22, prbar6_el2\n"
        "mrs x6, prbar7_el1\n"
        "mrs x7, prbar7_el2\n"
        "mrs x10, prbar8_el1\n"
        "mrs x6, prbar8_el2\n"
        "mrs x26, prbar9_el1\n"
        "mrs x28, prbar9_el2\n"
        "mrs x22, prbar_el1\n"
        "mrs x23, prbar_el2\n"
        "mrs x20, prenr_el1\n"
        "mrs x4, prenr_el2\n"
        "mrs x12, prlar10_el1\n"
        "mrs x11, prlar10_el2\n"
        "mrs x1, prlar11_el1\n"
        "mrs x26, prlar11_el2\n"
        "mrs x4, prlar12_el1\n"
        "mrs x0, prlar12_el2\n"
        "mrs x2, prlar13_el1\n"
        "mrs x20, prlar13_el2\n"
        "mrs x23, prlar14_el1\n"
        "mrs x28, prlar14_el2\n"
        "mrs x8, prlar15_el1\n"
        "mrs x13, prlar15_el2\n"
        "mrs x5, prlar1_el1\n"
        "mrs x1, prlar1_el2\n"
        "mrs x2, prlar2_el1\n"
        "mrs x21, prlar2_el2\n"
        "mrs x26, prlar3_el1\n"
        "mrs x12, prlar3_el2\n"
        "mrs x27, prlar4_el1\n"
        "mrs x16, prlar4_el2\n"
        "mrs x21, prlar5_el1\n"
        "mrs x9, prlar5_el2\n"
        "mrs x19, prlar6_el1\n"
        "mrs x7, prlar6_el2\n"
        "mrs x22, prlar7_el1\n"
        "mrs x9, prlar7_el2\n"
        "mrs x1, prlar8_el1\n"
        "mrs x14, prlar8_el2\n"
        "mrs x5, prlar9_el1\n"
        "mrs x5, prlar9_el2\n"
        "mrs x8, prlar_el1\n"
        "mrs x14, prlar_el2\n"
        "mrs x0, prselr_el1\n"
        "mrs x8, prselr_el2\n"
        "mrs x11, rcwmask_el1\n"
        "mrs x30, rcwsmask_el1\n"
        "mrs x10, revidr_el1\n"
        "mrs x17, rgsr_el1\n"
        "mrs x10, rmr_el1\n"
        "mrs x7, rmr_el2\n"
        "mrs x1, rmr_el3\n"
        "mrs x30, rndr\n"
        "mrs x28, rndrrs\n"
        "mrs x9, rvbar_el1\n"
        "mrs x6, rvbar_el2\n"
        "mrs x11, rvbar_el3\n"
        "mrs x5, s2pir_el2\n"
        "mrs x0, s2por_el1\n"
        "mrs x10, scr_el3\n"
        "mrs x12, sctlr2_el1\n"
        "mrs x2, sctlr2_el12\n"
        "mrs x15, sctlr2_el2\n"
        "mrs x8, sctlr2_el3\n"
        "mrs x16, sctlr_el1\n"
        "mrs x20, sctlr_el12\n"
        "mrs x6, sctlr_el2\n"
        "mrs x7, sctlr_el3\n"
        "mrs x16, scxtnum_el0\n"
        "mrs x24, scxtnum_el1\n"
        "mrs x0, scxtnum_el12\n"
        "mrs x2, scxtnum_el2\n"
        "mrs x8, scxtnum_el3\n"
        "mrs x26, sder32_el2\n"
        "mrs x2, sder32_el3\n"
        "mrs x4, smcr_el1\n"
        "mrs x12, smcr_el12\n"
        "mrs x18, smcr_el2\n"
        "mrs x1, smcr_el3\n"
        "mrs x12, smidr_el1\n"
        "mrs x0, smpri_el1\n"
        "mrs x9, smprimap_el2\n"
        "mrs x9, sp_el0\n"
        "mrs x20, sp_el1\n"
        "mrs x7, sp_el2\n"
        "mrs x2, spmaccessr_el1\n"
        "mrs x18, spmaccessr_el12\n"
        "mrs x30, spmaccessr_el2\n"
        "mrs x16, spmaccessr_el3\n"
        "mrs x27, spmcfgr_el1\n"
        "mrs x24, spmcgcr0_el1\n"
        "mrs x4, spmcgcr1_el1\n"
        "mrs x21, spmcntenclr_el0\n"
        "mrs x28, spmcntenset_el0\n"
        "mrs x22, spmcr_el0\n"
        "mrs x25, spmdevaff_el1\n"
        "mrs x28, spmdevarch_el1\n"
        "mrs x19, spmevcntr0_el0\n"
        "mrs x12, spmevcntr10_el0\n"
        "mrs x24, spmevcntr11_el0\n"
        "mrs x10, spmevcntr12_el0\n"
        "mrs x23, spmevcntr13_el0\n"
        "mrs x15, spmevcntr14_el0\n"
        "mrs x4, spmevcntr15_el0\n"
        "mrs x9, spmevcntr1_el0\n"
        "mrs x23, spmevcntr2_el0\n"
        "mrs x19, spmevcntr3_el0\n"
        "mrs x20, spmevcntr4_el0\n"
        "mrs x4, spmevcntr5_el0\n"
        "mrs x1, spmevcntr6_el0\n"
        "mrs x26, spmevcntr7_el0\n"
        "mrs x26, spmevcntr8_el0\n"
        "mrs x22, spmevcntr9_el0\n"
        "mrs x28, spmevfilt2r0_el0\n"
        "mrs x16, spmevfilt2r10_el0\n"
        "mrs x20, spmevfilt2r11_el0\n"
        "mrs x13, spmevfilt2r12_el0\n"
        "mrs x23, spmevfilt2r13_el0\n"
        "mrs x22, spmevfilt2r14_el0\n"
        "mrs x25, spmevfilt2r15_el0\n"
        "mrs x16, spmevfilt2r1_el0\n"
        "mrs x4, spmevfilt2r2_el0\n"
        "mrs x29, spmevfilt2r3_el0\n"
        "mrs x16, spmevfilt2r4_el0\n"
        "mrs x24, spmevfilt2r5_el0\n"
        "mrs x16, spmevfilt2r6_el0\n"
        "mrs x18, spmevfilt2r7_el0\n"
        "mrs x26, spmevfilt2r8_el0\n"
        "mrs x26, spmevfilt2r9_el0\n"
        "mrs x25, spmevfiltr0_el0\n"
        "mrs x0, spmevfiltr10_el0\n"
        "mrs x26, spmevfiltr11_el0\n"
        "mrs x21, spmevfiltr12_el0\n"
        "mrs x18, spmevfiltr13_el0\n"
        "mrs x25, spmevfiltr14_el0\n"
        "mrs x28, spmevfiltr15_el0\n"
        "mrs x22, spmevfiltr1_el0\n"
        "mrs x21, spmevfiltr2_el0\n"
        "mrs x30, spmevfiltr3_el0\n"
        "mrs x22, spmevfiltr4_el0\n"
        "mrs x20, spmevfiltr5_el0\n"
        "mrs x7, spmevfiltr6_el0\n"
        "mrs x2, spmevfiltr7_el0\n"
        "mrs x0, spmevfiltr8_el0\n"
        "mrs x1, spmevfiltr9_el0\n"
        "mrs x4, spmevtyper0_el0\n"
        "mrs x20, spmevtyper10_el0\n"
        "mrs x11, spmevtyper11_el0\n"
        "mrs x30, spmevtyper12_el0\n"
        "mrs x3, spmevtyper13_el0\n"
        "mrs x12, spmevtyper14_el0\n"
        "mrs x26, spmevtyper15_el0\n"
        "mrs x14, spmevtyper1_el0\n"
        "mrs x17, spmevtyper2_el0\n"
        "mrs x1, spmevtyper3_el0\n"
        "mrs x20, spmevtyper4_el0\n"
        "mrs x0, spmevtyper5_el0\n"
        "mrs x20, spmevtyper6_el0\n"
        "mrs x17, spmevtyper7_el0\n"
        "mrs x21, spmevtyper8_el0\n"
        "mrs x7, spmevtyper9_el0\n"
        "mrs x15, spmiidr_el1\n"
        "mrs x8, spmintenclr_el1\n"
        "mrs x0, spmintenset_el1\n"
        "mrs x14, spmovsclr_el0\n"
        "mrs x25, spmovsset_el0\n"
        "mrs x2, spmrootcr_el3\n"
        "mrs x23, spmscr_el1\n"
        "mrs x29, spmselr_el0\n"
        "msr spmzr_el0, x16\n"
        "mrs x28, spsel\n"
        "mrs x17, spsr_abt\n"
        "mrs x2, spsr_el1\n"
        "mrs x21, spsr_el12\n"
        "mrs x16, spsr_el2\n"
        "mrs x2, spsr_el3\n"
        "mrs x23, spsr_fiq\n"
        "mrs x23, spsr_irq\n"
        "mrs x15, spsr_und\n"
        "mrs x8, ssbs\n"
        "mrs x25, svcr\n"
        "mrs x2, tco\n"
        "mrs x27, tcr2_el1\n"
        "mrs x8, tcr2_el12\n"
        "mrs x7, tcr2_el2\n"
        "mrs x23, tcr_el1\n"
        "mrs x24, tcr_el12\n"
        "mrs x6, tcr_el2\n"
        "mrs x7, tcr_el3\n"
        "mrs x23, teecr32_el1\n"
        "mrs x20, teehbr32_el1\n"
        "mrs x14, tfsr_el1\n"
        "mrs x15, tfsr_el12\n"
        "mrs x27, tfsr_el2\n"
        "mrs x12, tfsr_el3\n"
        "mrs x2, tfsre0_el1\n"
        "mrs x15, tpidr2_el0\n"
        "mrs x29, tpidr_el0\n"
        "mrs x21, tpidr_el1\n"
        "mrs x9, tpidr_el2\n"
        "mrs x24, tpidr_el3\n"
        "mrs x1, tpidrro_el0\n"
        "mrs x19, trbbaser_el1\n"
        "mrs x20, trbidr_el1\n"
        "mrs x20, trblimitr_el1\n"
        "mrs x6, trbmar_el1\n"
        "mrs x2, trbptr_el1\n"
        "mrs x19, trbsr_el1\n"
        "mrs x4, trbtrg_el1\n"
        "mrs x10, trcacatr0\n"
        "mrs x8, trcacatr1\n"
        "mrs x20, trcacatr10\n"
        "mrs x23, trcacatr11\n"
        "mrs x22, trcacatr12\n"
        "mrs x9, trcacatr13\n"
        "mrs x19, trcacatr14\n"
        "mrs x18, trcacatr15\n"
        "mrs x4, trcacatr2\n"
        "mrs x0, trcacatr3\n"
        "mrs x15, trcacatr4\n"
        "mrs x1, trcacatr5\n"
        "mrs x15, trcacatr6\n"
        "mrs x8, trcacatr7\n"
        "mrs x21, trcacatr8\n"
        "mrs x3, trcacatr9\n"
        "mrs x22, trcacvr0\n"
        "mrs x6, trcacvr1\n"
        "mrs x21, trcacvr10\n"
        "mrs x15, trcacvr11\n"
        "mrs x9, trcacvr12\n"
        "mrs x22, trcacvr13\n"
        "mrs x16, trcacvr14\n"
        "mrs x9, trcacvr15\n"
        "mrs x14, trcacvr2\n"
        "mrs x14, trcacvr3\n"
        "mrs x14, trcacvr4\n"
        "mrs x24, trcacvr5\n"
        "mrs x3, trcacvr6\n"
        "mrs x28, trcacvr7\n"
        "mrs x17, trcacvr8\n"
        "mrs x6, trcacvr9\n"
        "mrs x9, trcauthstatus\n"
        "mrs x2, trcauxctlr\n"
        "mrs x29, trcbbctlr\n"
        "mrs x15, trcccctlr\n"
        "mrs x0, trccidcctlr0\n"
        "mrs x9, trccidcctlr1\n"
        "mrs x14, trccidcvr0\n"
        "mrs x2, trccidcvr1\n"
        "mrs x26, trccidcvr2\n"
        "mrs x16, trccidcvr3\n"
        "mrs x30, trccidcvr4\n"
        "mrs x14, trccidcvr5\n"
        "mrs x8, trccidcvr6\n"
        "mrs x12, trccidcvr7\n"
        "mrs x6, trccidr0\n"
        "mrs x29, trccidr1\n"
        "mrs x30, trccidr2\n"
        "mrs x29, trccidr3\n"
        "mrs x6, trcclaimclr\n"
        "mrs x2, trcclaimset\n"
        "mrs x18, trccntctlr0\n"
        "mrs x2, trccntctlr1\n"
        "mrs x4, trccntctlr2\n"
        "mrs x23, trccntctlr3\n"
        "mrs x16, trccntrldvr0\n"
        "mrs x8, trccntrldvr1\n"
        "mrs x30, trccntrldvr2\n"
        "mrs x11, trccntrldvr3\n"
        "mrs x4, trccntvr0\n"
        "mrs x19, trccntvr1\n"
        "mrs x26, trccntvr2\n"
        "mrs x20, trccntvr3\n"
        "mrs x16, trcconfigr\n"
        "mrs x8, trcdevaff0\n"
        "mrs x28, trcdevaff1\n"
        "mrs x3, trcdevarch\n"
        "mrs x22, trcdevid\n"
        "mrs x11, trcdevtype\n"
        "mrs x7, trcdvcmr0\n"
        "mrs x15, trcdvcmr1\n"
        "mrs x28, trcdvcmr2\n"
        "mrs x28, trcdvcmr3\n"
        "mrs x15, trcdvcmr4\n"
        "mrs x12, trcdvcmr5\n"
        "mrs x0, trcdvcmr6\n"
        "mrs x5, trcdvcmr7\n"
        "mrs x0, trcdvcvr0\n"
        "mrs x30, trcdvcvr1\n"
        "mrs x15, trcdvcvr2\n"
        "mrs x21, trcdvcvr3\n"
        "mrs x14, trcdvcvr4\n"
        "mrs x12, trcdvcvr5\n"
        "mrs x9, trcdvcvr6\n"
        "mrs x23, trcdvcvr7\n"
        "mrs x4, trceventctl0r\n"
        "mrs x13, trceventctl1r\n"
        "mrs x11, trcextinselr1\n"
        "mrs x12, trcextinselr2\n"
        "mrs x10, trcextinselr3\n"
        "mrs x3, trcidr0\n"
        "mrs x26, trcidr1\n"
        "mrs x10, trcidr10\n"
        "mrs x0, trcidr11\n"
        "mrs x10, trcidr12\n"
        "mrs x24, trcidr13\n"
        "mrs x10, trcidr2\n"
        "mrs x26, trcidr3\n"
        "mrs x12, trcidr4\n"
        "mrs x3, trcidr5\n"
        "mrs x30, trcidr6\n"
        "mrs x29, trcidr7\n"
        "mrs x6, trcidr8\n"
        "mrs x22, trcidr9\n"
        "mrs x0, trcimspec0\n"
        "mrs x28, trcimspec1\n"
        "mrs x23, trcimspec2\n"
        "mrs x9, trcimspec3\n"
        "mrs x8, trcimspec4\n"
        "mrs x11, trcimspec5\n"
        "mrs x2, trcimspec6\n"
        "mrs x12, trcimspec7\n"
        "mrs x12, trcitctrl\n"
        "mrs x27, trcitecr_el1\n"
        "mrs x18, trcitecr_el12\n"
        "mrs x2, trcitecr_el2\n"
        "mrs x11, trciteedcr\n"
        "msr trclar, x29\n"
        "mrs x13, trclsr\n"
        "msr trcoslar, x24\n"
        "mrs x8, trcoslsr\n"
        "mrs x27, trcpdcr\n"
        "mrs x1, trcpdsr\n"
        "mrs x8, trcpidr0\n"
        "mrs x3, trcpidr1\n"
        "mrs x1, trcpidr2\n"
        "mrs x26, trcpidr3\n"
        "mrs x21, trcpidr4\n"
        "mrs x9, trcpidr5\n"
        "mrs x20, trcpidr6\n"
        "mrs x29, trcpidr7\n"
        "mrs x4, trcprgctlr\n"
        "mrs x7, trcprocselr\n"
        "mrs x8, trcqctlr\n"
        "mrs x13, trcrsctlr10\n"
        "mrs x16, trcrsctlr11\n"
        "mrs x10, trcrsctlr12\n"
        "mrs x6, trcrsctlr13\n"
        "mrs x24, trcrsctlr14\n"
        "mrs x11, trcrsctlr15\n"
        "mrs x25, trcrsctlr16\n"
        "mrs x30, trcrsctlr17\n"
        "mrs x13, trcrsctlr18\n"
        "mrs x28, trcrsctlr19\n"
        "mrs x0, trcrsctlr2\n"
        "mrs x25, trcrsctlr20\n"
        "mrs x24, trcrsctlr21\n"
        "mrs x20, trcrsctlr22\n"
        "mrs x12, trcrsctlr23\n"
        "mrs x29, trcrsctlr24\n"
        "mrs x28, trcrsctlr25\n"
        "mrs x30, trcrsctlr26\n"
        "mrs x17, trcrsctlr27\n"
        "mrs x17, trcrsctlr28\n"
        "mrs x6, trcrsctlr29\n"
        "mrs x23, trcrsctlr3\n"
        "mrs x2, trcrsctlr30\n"
        "mrs x1, trcrsctlr31\n"
        "mrs x29, trcrsctlr4\n"
        "mrs x23, trcrsctlr5\n"
        "mrs x13, trcrsctlr6\n"
        "mrs x14, trcrsctlr7\n"
        "mrs x19, trcrsctlr8\n"
        "mrs x24, trcrsctlr9\n"
        "mrs x4, trcrsr\n"
        "mrs x20, trcseqevr0\n"
        "mrs x27, trcseqevr1\n"
        "mrs x9, trcseqevr2\n"
        "mrs x15, trcseqrstevr\n"
        "mrs x1, trcseqstr\n"
        "mrs x29, trcssccr0\n"
        "mrs x29, trcssccr1\n"
        "mrs x17, trcssccr2\n"
        "mrs x4, trcssccr3\n"
        "mrs x5, trcssccr4\n"
        "mrs x15, trcssccr5\n"
        "mrs x13, trcssccr6\n"
        "mrs x10, trcssccr7\n"
        "mrs x9, trcsscsr0\n"
        "mrs x9, trcsscsr1\n"
        "mrs x8, trcsscsr2\n"
        "mrs x23, trcsscsr3\n"
        "mrs x23, trcsscsr4\n"
        "mrs x20, trcsscsr5\n"
        "mrs x8, trcsscsr6\n"
        "mrs x12, trcsscsr7\n"
        "mrs x20, trcsspcicr0\n"
        "mrs x7, trcsspcicr1\n"
        "mrs x9, trcsspcicr2\n"
        "mrs x15, trcsspcicr3\n"
        "mrs x17, trcsspcicr4\n"
        "mrs x21, trcsspcicr5\n"
        "mrs x12, trcsspcicr6\n"
        "mrs x3, trcsspcicr7\n"
        "mrs x5, trcstallctlr\n"
        "mrs x20, trcstatr\n"
        "mrs x5, trcsyncpr\n"
        "mrs x2, trctraceidr\n"
        "mrs x6, trctsctlr\n"
        "mrs x16, trcvdarcctlr\n"
        "mrs x28, trcvdctlr\n"
        "mrs x25, trcvdsacctlr\n"
        "mrs x15, trcvictlr\n"
        "mrs x17, trcviiectlr\n"
        "mrs x7, trcvipcssctlr\n"
        "mrs x14, trcvissctlr\n"
        "mrs x29, trcvmidcctlr0\n"
        "mrs x10, trcvmidcctlr1\n"
        "mrs x24, trcvmidcvr0\n"
        "mrs x14, trcvmidcvr1\n"
        "mrs x13, trcvmidcvr2\n"
        "mrs x4, trcvmidcvr3\n"
        "mrs x17, trcvmidcvr4\n"
        "mrs x6, trcvmidcvr5\n"
        "mrs x7, trcvmidcvr6\n"
        "mrs x2, trcvmidcvr7\n"
        "mrs x5, trfcr_el1\n"
        "mrs x10, trfcr_el12\n"
        "mrs x17, trfcr_el2\n"
        "mrs x2, ttbr0_el1\n"
        "mrs x10, ttbr0_el12\n"
        "mrs x7, ttbr0_el2\n"
        "mrs x11, ttbr0_el3\n"
        "mrs x8, ttbr1_el1\n"
        "mrs x25, ttbr1_el12\n"
        "mrs x18, ttbr1_el2\n"
        "mrs x6, uao\n"
        "mrs x28, vbar_el1\n"
        "mrs x0, vbar_el12\n"
        "mrs x23, vbar_el2\n"
        "mrs x27, vbar_el3\n"
        "mrs x13, vdisr_el2\n"
        "mrs x12, vdisr_el3\n"
        "mrs x13, vmecid_a_el2\n"
        "mrs x23, vmecid_p_el2\n"
        "mrs x16, vmpidr_el2\n"
        "mrs x6, vncr_el2\n"
        "mrs x12, vpidr_el2\n"
        "mrs x8, vsesr_el2\n"
        "mrs x10, vsesr_el3\n"
        "mrs x24, vstcr_el2\n"
        "mrs x1, vsttbr_el2\n"
        "mrs x15, vtcr_el2\n"
        "mrs x8, vttbr_el2\n"
        "mrs x18, zcr_el1\n"
        "mrs x30, zcr_el12\n"
        "mrs x11, zcr_el2\n"
        "mrs x4, zcr_el3\n"
        "mrrs x2, x3, par_el1\n"
        "msrr par_el1, x4, x5\n"
        "mrrs x2, x3, rcwmask_el1\n"
        "msrr rcwmask_el1, x4, x5\n"
        "mrrs x2, x3, rcwsmask_el1\n"
        "msrr rcwsmask_el1, x4, x5\n"
        "mrrs x2, x3, ttbr0_el1\n"
        "msrr ttbr0_el1, x4, x5\n"
        "mrrs x2, x3, ttbr0_el12\n"
        "msrr ttbr0_el12, x4, x5\n"
        "mrrs x2, x3, ttbr0_el2\n"
        "msrr ttbr0_el2, x4, x5\n"
        "mrrs x2, x3, ttbr1_el1\n"
        "msrr ttbr1_el1, x4, x5\n"
        "mrrs x2, x3, ttbr1_el12\n"
        "msrr ttbr1_el12, x4, x5\n"
        "mrrs x2, x3, ttbr1_el2\n"
        "msrr ttbr1_el2, x4, x5\n"
        "mrrs x2, x3, vttbr_el2\n"
        "msrr vttbr_el2, x4, x5\n"
        ".globl axiom_asm_table_end\n"
        "axiom_asm_table_end:\n"
        ".text\n");

extern const unsigned char axiom_asm_table[], axiom_asm_table_end[];
static const char *const want[] = {
    "aad038d5",  /* mrs x10, accdata_el1 */
    "3e1038d5",  /* mrs x30, actlr_el1 */
    "24103cd5",  /* mrs x4, actlr_el2 */
    "2c103ed5",  /* mrs x12, actlr_el3 */
    "145138d5",  /* mrs x20, afsr0_el1 */
    "01513dd5",  /* mrs x1, afsr0_el12 */
    "02513cd5",  /* mrs x2, afsr0_el2 */
    "1a513ed5",  /* mrs x26, afsr0_el3 */
    "315138d5",  /* mrs x17, afsr1_el1 */
    "23513dd5",  /* mrs x3, afsr1_el12 */
    "2b513cd5",  /* mrs x11, afsr1_el2 */
    "32513ed5",  /* mrs x18, afsr1_el3 */
    "e10039d5",  /* mrs x1, aidr_el1 */
    "1d4338d5",  /* mrs x29, allint */
    "30a338d5",  /* mrs x16, amair2_el1 */
    "26a33dd5",  /* mrs x6, amair2_el12 */
    "21a33cd5",  /* mrs x1, amair2_el2 */
    "22a33ed5",  /* mrs x2, amair2_el3 */
    "0da338d5",  /* mrs x13, amair_el1 */
    "0da33dd5",  /* mrs x13, amair_el12 */
    "02a33cd5",  /* mrs x2, amair_el2 */
    "07a33ed5",  /* mrs x7, amair_el3 */
    "22d23bd5",  /* mrs x2, amcfgr_el0 */
    "d1d23bd5",  /* mrs x17, amcg1idr_el0 */
    "4dd23bd5",  /* mrs x13, amcgcr_el0 */
    "81d23bd5",  /* mrs x1, amcntenclr0_el0 */
    "1ad33bd5",  /* mrs x26, amcntenclr1_el0 */
    "b2d23bd5",  /* mrs x18, amcntenset0_el0 */
    "23d33bd5",  /* mrs x3, amcntenset1_el0 */
    "1ed23bd5",  /* mrs x30, amcr_el0 */
    "07d43bd5",  /* mrs x7, amevcntr00_el0 */
    "34d43bd5",  /* mrs x20, amevcntr01_el0 */
    "54d43bd5",  /* mrs x20, amevcntr02_el0 */
    "72d43bd5",  /* mrs x18, amevcntr03_el0 */
    "1edc3bd5",  /* mrs x30, amevcntr10_el0 */
    "41dd3bd5",  /* mrs x1, amevcntr110_el0 */
    "72dd3bd5",  /* mrs x18, amevcntr111_el0 */
    "92dd3bd5",  /* mrs x18, amevcntr112_el0 */
    "acdd3bd5",  /* mrs x12, amevcntr113_el0 */
    "c1dd3bd5",  /* mrs x1, amevcntr114_el0 */
    "e7dd3bd5",  /* mrs x7, amevcntr115_el0 */
    "21dc3bd5",  /* mrs x1, amevcntr11_el0 */
    "51dc3bd5",  /* mrs x17, amevcntr12_el0 */
    "7bdc3bd5",  /* mrs x27, amevcntr13_el0 */
    "84dc3bd5",  /* mrs x4, amevcntr14_el0 */
    "a9dc3bd5",  /* mrs x9, amevcntr15_el0 */
    "cddc3bd5",  /* mrs x13, amevcntr16_el0 */
    "e4dc3bd5",  /* mrs x4, amevcntr17_el0 */
    "11dd3bd5",  /* mrs x17, amevcntr18_el0 */
    "23dd3bd5",  /* mrs x3, amevcntr19_el0 */
    "12d83cd5",  /* mrs x18, amevcntvoff00_el2 */
    "49d93cd5",  /* mrs x9, amevcntvoff010_el2 */
    "71d93cd5",  /* mrs x17, amevcntvoff011_el2 */
    "9ad93cd5",  /* mrs x26, amevcntvoff012_el2 */
    "b5d93cd5",  /* mrs x21, amevcntvoff013_el2 */
    "c5d93cd5",  /* mrs x5, amevcntvoff014_el2 */
    "e3d93cd5",  /* mrs x3, amevcntvoff015_el2 */
    "32d83cd5",  /* mrs x18, amevcntvoff01_el2 */
    "52d83cd5",  /* mrs x18, amevcntvoff02_el2 */
    "74d83cd5",  /* mrs x20, amevcntvoff03_el2 */
    "86d83cd5",  /* mrs x6, amevcntvoff04_el2 */
    "abd83cd5",  /* mrs x11, amevcntvoff05_el2 */
    "c3d83cd5",  /* mrs x3, amevcntvoff06_el2 */
    "f1d83cd5",  /* mrs x17, amevcntvoff07_el2 */
    "16d93cd5",  /* mrs x22, amevcntvoff08_el2 */
    "22d93cd5",  /* mrs x2, amevcntvoff09_el2 */
    "12da3cd5",  /* mrs x18, amevcntvoff10_el2 */
    "41db3cd5",  /* mrs x1, amevcntvoff110_el2 */
    "73db3cd5",  /* mrs x19, amevcntvoff111_el2 */
    "86db3cd5",  /* mrs x6, amevcntvoff112_el2 */
    "afdb3cd5",  /* mrs x15, amevcntvoff113_el2 */
    "d5db3cd5",  /* mrs x21, amevcntvoff114_el2 */
    "f1db3cd5",  /* mrs x17, amevcntvoff115_el2 */
    "2dda3cd5",  /* mrs x13, amevcntvoff11_el2 */
    "58da3cd5",  /* mrs x24, amevcntvoff12_el2 */
    "6ada3cd5",  /* mrs x10, amevcntvoff13_el2 */
    "8eda3cd5",  /* mrs x14, amevcntvoff14_el2 */
    "b2da3cd5",  /* mrs x18, amevcntvoff15_el2 */
    "ddda3cd5",  /* mrs x29, amevcntvoff16_el2 */
    "eeda3cd5",  /* mrs x14, amevcntvoff17_el2 */
    "0bdb3cd5",  /* mrs x11, amevcntvoff18_el2 */
    "29db3cd5",  /* mrs x9, amevcntvoff19_el2 */
    "07d63bd5",  /* mrs x7, amevtyper00_el0 */
    "39d63bd5",  /* mrs x25, amevtyper01_el0 */
    "45d63bd5",  /* mrs x5, amevtyper02_el0 */
    "76d63bd5",  /* mrs x22, amevtyper03_el0 */
    "18de3bd5",  /* mrs x24, amevtyper10_el0 */
    "47df3bd5",  /* mrs x7, amevtyper110_el0 */
    "62df3bd5",  /* mrs x2, amevtyper111_el0 */
    "92df3bd5",  /* mrs x18, amevtyper112_el0 */
    "a9df3bd5",  /* mrs x9, amevtyper113_el0 */
    "d0df3bd5",  /* mrs x16, amevtyper114_el0 */
    "efdf3bd5",  /* mrs x15, amevtyper115_el0 */
    "3cde3bd5",  /* mrs x28, amevtyper11_el0 */
    "4ade3bd5",  /* mrs x10, amevtyper12_el0 */
    "77de3bd5",  /* mrs x23, amevtyper13_el0 */
    "8ede3bd5",  /* mrs x14, amevtyper14_el0 */
    "a9de3bd5",  /* mrs x9, amevtyper15_el0 */
    "d3de3bd5",  /* mrs x19, amevtyper16_el0 */
    "e2de3bd5",  /* mrs x2, amevtyper17_el0 */
    "03df3bd5",  /* mrs x3, amevtyper18_el0 */
    "30df3bd5",  /* mrs x16, amevtyper19_el0 */
    "6dd23bd5",  /* mrs x13, amuserenr_el0 */
    "252238d5",  /* mrs x5, apdakeyhi_el1 */
    "182238d5",  /* mrs x24, apdakeylo_el1 */
    "6a2238d5",  /* mrs x10, apdbkeyhi_el1 */
    "442238d5",  /* mrs x4, apdbkeylo_el1 */
    "3d2338d5",  /* mrs x29, apgakeyhi_el1 */
    "0f2338d5",  /* mrs x15, apgakeylo_el1 */
    "2d2138d5",  /* mrs x13, apiakeyhi_el1 */
    "012138d5",  /* mrs x1, apiakeylo_el1 */
    "7e2138d5",  /* mrs x30, apibkeyhi_el1 */
    "552138d5",  /* mrs x21, apibkeylo_el1 */
    "029031d5",  /* mrs x2, brbcr_el1 */
    "189035d5",  /* mrs x24, brbcr_el12 */
    "119034d5",  /* mrs x17, brbcr_el2 */
    "329031d5",  /* mrs x18, brbfcr_el1 */
    "199231d5",  /* mrs x25, brbidr0_el1 */
    "1c8031d5",  /* mrs x28, brbinf0_el1 */
    "1a8a31d5",  /* mrs x26, brbinf10_el1 */
    "0a8b31d5",  /* mrs x10, brbinf11_el1 */
    "0a8c31d5",  /* mrs x10, brbinf12_el1 */
    "168d31d5",  /* mrs x22, brbinf13_el1 */
    "0b8e31d5",  /* mrs x11, brbinf14_el1 */
    "138f31d5",  /* mrs x19, brbinf15_el1 */
    "8f8031d5",  /* mrs x15, brbinf16_el1 */
    "928131d5",  /* mrs x18, brbinf17_el1 */
    "998231d5",  /* mrs x25, brbinf18_el1 */
    "8e8331d5",  /* mrs x14, brbinf19_el1 */
    "028131d5",  /* mrs x2, brbinf1_el1 */
    "9a8431d5",  /* mrs x26, brbinf20_el1 */
    "828531d5",  /* mrs x2, brbinf21_el1 */
    "9e8631d5",  /* mrs x30, brbinf22_el1 */
    "888731d5",  /* mrs x8, brbinf23_el1 */
    "8f8831d5",  /* mrs x15, brbinf24_el1 */
    "968931d5",  /* mrs x22, brbinf25_el1 */
    "958a31d5",  /* mrs x21, brbinf26_el1 */
    "828b31d5",  /* mrs x2, brbinf27_el1 */
    "818c31d5",  /* mrs x1, brbinf28_el1 */
    "978d31d5",  /* mrs x23, brbinf29_el1 */
    "168231d5",  /* mrs x22, brbinf2_el1 */
    "898e31d5",  /* mrs x9, brbinf30_el1 */
    "948f31d5",  /* mrs x20, brbinf31_el1 */
    "128331d5",  /* mrs x18, brbinf3_el1 */
    "158431d5",  /* mrs x21, brbinf4_el1 */
    "1a8531d5",  /* mrs x26, brbinf5_el1 */
    "0e8631d5",  /* mrs x14, brbinf6_el1 */
    "098731d5",  /* mrs x9, brbinf7_el1 */
    "168831d5",  /* mrs x22, brbinf8_el1 */
    "0c8931d5",  /* mrs x12, brbinf9_el1 */
    "1c9131d5",  /* mrs x28, brbinfinj_el1 */
    "358031d5",  /* mrs x21, brbsrc0_el1 */
    "2b8a31d5",  /* mrs x11, brbsrc10_el1 */
    "208b31d5",  /* mrs x0, brbsrc11_el1 */
    "3e8c31d5",  /* mrs x30, brbsrc12_el1 */
    "2e8d31d5",  /* mrs x14, brbsrc13_el1 */
    "2b8e31d5",  /* mrs x11, brbsrc14_el1 */
    "258f31d5",  /* mrs x5, brbsrc15_el1 */
    "b38031d5",  /* mrs x19, brbsrc16_el1 */
    "a38131d5",  /* mrs x3, brbsrc17_el1 */
    "af8231d5",  /* mrs x15, brbsrc18_el1 */
    "a18331d5",  /* mrs x1, brbsrc19_el1 */
    "268131d5",  /* mrs x6, brbsrc1_el1 */
    "b88431d5",  /* mrs x24, brbsrc20_el1 */
    "a98531d5",  /* mrs x9, brbsrc21_el1 */
    "a48631d5",  /* mrs x4, brbsrc22_el1 */
    "b78731d5",  /* mrs x23, brbsrc23_el1 */
    "a78831d5",  /* mrs x7, brbsrc24_el1 */
    "ac8931d5",  /* mrs x12, brbsrc25_el1 */
    "ac8a31d5",  /* mrs x12, brbsrc26_el1 */
    "bd8b31d5",  /* mrs x29, brbsrc27_el1 */
    "bb8c31d5",  /* mrs x27, brbsrc28_el1 */
    "af8d31d5",  /* mrs x15, brbsrc29_el1 */
    "228231d5",  /* mrs x2, brbsrc2_el1 */
    "a58e31d5",  /* mrs x5, brbsrc30_el1 */
    "ae8f31d5",  /* mrs x14, brbsrc31_el1 */
    "2c8331d5",  /* mrs x12, brbsrc3_el1 */
    "318431d5",  /* mrs x17, brbsrc4_el1 */
    "288531d5",  /* mrs x8, brbsrc5_el1 */
    "3c8631d5",  /* mrs x28, brbsrc6_el1 */
    "248731d5",  /* mrs x4, brbsrc7_el1 */
    "3a8831d5",  /* mrs x26, brbsrc8_el1 */
    "2d8931d5",  /* mrs x13, brbsrc9_el1 */
    "3b9131d5",  /* mrs x27, brbsrcinj_el1 */
    "518031d5",  /* mrs x17, brbtgt0_el1 */
    "488a31d5",  /* mrs x8, brbtgt10_el1 */
    "568b31d5",  /* mrs x22, brbtgt11_el1 */
    "4d8c31d5",  /* mrs x13, brbtgt12_el1 */
    "4b8d31d5",  /* mrs x11, brbtgt13_el1 */
    "558e31d5",  /* mrs x21, brbtgt14_el1 */
    "5c8f31d5",  /* mrs x28, brbtgt15_el1 */
    "cc8031d5",  /* mrs x12, brbtgt16_el1 */
    "de8131d5",  /* mrs x30, brbtgt17_el1 */
    "c78231d5",  /* mrs x7, brbtgt18_el1 */
    "c48331d5",  /* mrs x4, brbtgt19_el1 */
    "428131d5",  /* mrs x2, brbtgt1_el1 */
    "c58431d5",  /* mrs x5, brbtgt20_el1 */
    "c48531d5",  /* mrs x4, brbtgt21_el1 */
    "c78631d5",  /* mrs x7, brbtgt22_el1 */
    "d58731d5",  /* mrs x21, brbtgt23_el1 */
    "c78831d5",  /* mrs x7, brbtgt24_el1 */
    "c08931d5",  /* mrs x0, brbtgt25_el1 */
    "cf8a31d5",  /* mrs x15, brbtgt26_el1 */
    "da8b31d5",  /* mrs x26, brbtgt27_el1 */
    "d28c31d5",  /* mrs x18, brbtgt28_el1 */
    "c58d31d5",  /* mrs x5, brbtgt29_el1 */
    "488231d5",  /* mrs x8, brbtgt2_el1 */
    "c98e31d5",  /* mrs x9, brbtgt30_el1 */
    "c08f31d5",  /* mrs x0, brbtgt31_el1 */
    "448331d5",  /* mrs x4, brbtgt3_el1 */
    "4d8431d5",  /* mrs x13, brbtgt4_el1 */
    "518531d5",  /* mrs x17, brbtgt5_el1 */
    "4b8631d5",  /* mrs x11, brbtgt6_el1 */
    "538731d5",  /* mrs x19, brbtgt7_el1 */
    "528831d5",  /* mrs x18, brbtgt8_el1 */
    "4a8931d5",  /* mrs x10, brbtgt9_el1 */
    "5e9131d5",  /* mrs x30, brbtgtinj_el1 */
    "449031d5",  /* mrs x4, brbts_el1 */
    "560039d5",  /* mrs x22, ccsidr2_el1 */
    "1b0039d5",  /* mrs x27, ccsidr_el1 */
    "300039d5",  /* mrs x16, clidr_el1 */
    "1ee03bd5",  /* mrs x30, cntfrq_el0 */
    "13e13cd5",  /* mrs x19, cnthctl_el2 */
    "34e23cd5",  /* mrs x20, cnthp_ctl_el2 */
    "55e23cd5",  /* mrs x21, cnthp_cval_el2 */
    "17e23cd5",  /* mrs x23, cnthp_tval_el2 */
    "21e53cd5",  /* mrs x1, cnthps_ctl_el2 */
    "4ee53cd5",  /* mrs x14, cnthps_cval_el2 */
    "1ce53cd5",  /* mrs x28, cnthps_tval_el2 */
    "3be33cd5",  /* mrs x27, cnthv_ctl_el2 */
    "58e33cd5",  /* mrs x24, cnthv_cval_el2 */
    "1ee33cd5",  /* mrs x30, cnthv_tval_el2 */
    "3be43cd5",  /* mrs x27, cnthvs_ctl_el2 */
    "55e43cd5",  /* mrs x21, cnthvs_cval_el2 */
    "19e43cd5",  /* mrs x25, cnthvs_tval_el2 */
    "b1e03cd5",  /* mrs x17, cntiscale_el2 */
    "0ce138d5",  /* mrs x12, cntkctl_el1 */
    "0ce13dd5",  /* mrs x12, cntkctl_el12 */
    "2ce23bd5",  /* mrs x12, cntp_ctl_el0 */
    "2ce23dd5",  /* mrs x12, cntp_ctl_el02 */
    "43e23bd5",  /* mrs x3, cntp_cval_el0 */
    "4fe23dd5",  /* mrs x15, cntp_cval_el02 */
    "14e23bd5",  /* mrs x20, cntp_tval_el0 */
    "0ce23dd5",  /* mrs x12, cntp_tval_el02 */
    "21e03bd5",  /* mrs x1, cntpct_el0 */
    "a6e03bd5",  /* mrs x6, cntpctss_el0 */
    "c2e03cd5",  /* mrs x2, cntpoff_el2 */
    "26e23fd5",  /* mrs x6, cntps_ctl_el1 */
    "4ee23fd5",  /* mrs x14, cntps_cval_el1 */
    "05e23fd5",  /* mrs x5, cntps_tval_el1 */
    "83e03cd5",  /* mrs x3, cntscale_el2 */
    "2ae33bd5",  /* mrs x10, cntv_ctl_el0 */
    "33e33dd5",  /* mrs x19, cntv_ctl_el02 */
    "41e33bd5",  /* mrs x1, cntv_cval_el0 */
    "43e33dd5",  /* mrs x3, cntv_cval_el02 */
    "00e33bd5",  /* mrs x0, cntv_tval_el0 */
    "12e33dd5",  /* mrs x18, cntv_tval_el02 */
    "44e03bd5",  /* mrs x4, cntvct_el0 */
    "d1e03bd5",  /* mrs x17, cntvctss_el0 */
    "e3e03cd5",  /* mrs x3, cntvfrq_el2 */
    "7ee03cd5",  /* mrs x30, cntvoff_el2 */
    "2bd038d5",  /* mrs x11, contextidr_el1 */
    "33d03dd5",  /* mrs x19, contextidr_el12 */
    "20d03cd5",  /* mrs x0, contextidr_el2 */
    "421038d5",  /* mrs x2, cpacr_el1 */
    "5b103dd5",  /* mrs x27, cpacr_el12 */
    "06f23fd5",  /* mrs x6, cpm_ioacc_ctl_el3 */
    "53113cd5",  /* mrs x19, cptr_el2 */
    "4c113ed5",  /* mrs x12, cptr_el3 */
    "04003ad5",  /* mrs x4, csselr_el1 */
    "34003bd5",  /* mrs x20, ctr_el0 */
    "484238d5",  /* mrs x8, currentel */
    "1e303cd5",  /* mrs x30, dacr32_el2 */
    "2b423bd5",  /* mrs x11, daif */
    "d37e30d5",  /* mrs x19, dbgauthstatus_el1 */
    "ab0030d5",  /* mrs x11, dbgbcr0_el1 */
    "af0a30d5",  /* mrs x15, dbgbcr10_el1 */
    "a30b30d5",  /* mrs x3, dbgbcr11_el1 */
    "a30c30d5",  /* mrs x3, dbgbcr12_el1 */
    "bb0d30d5",  /* mrs x27, dbgbcr13_el1 */
    "af0e30d5",  /* mrs x15, dbgbcr14_el1 */
    "ae0f30d5",  /* mrs x14, dbgbcr15_el1 */
    "af0130d5",  /* mrs x15, dbgbcr1_el1 */
    "af0230d5",  /* mrs x15, dbgbcr2_el1 */
    "a90330d5",  /* mrs x9, dbgbcr3_el1 */
    "a20430d5",  /* mrs x2, dbgbcr4_el1 */
    "a40530d5",  /* mrs x4, dbgbcr5_el1 */
    "a30630d5",  /* mrs x3, dbgbcr6_el1 */
    "b70730d5",  /* mrs x23, dbgbcr7_el1 */
    "aa0830d5",  /* mrs x10, dbgbcr8_el1 */
    "b70930d5",  /* mrs x23, dbgbcr9_el1 */
    "880030d5",  /* mrs x8, dbgbvr0_el1 */
    "8f0a30d5",  /* mrs x15, dbgbvr10_el1 */
    "9a0b30d5",  /* mrs x26, dbgbvr11_el1 */
    "960c30d5",  /* mrs x22, dbgbvr12_el1 */
    "850d30d5",  /* mrs x5, dbgbvr13_el1 */
    "900e30d5",  /* mrs x16, dbgbvr14_el1 */
    "800f30d5",  /* mrs x0, dbgbvr15_el1 */
    "860130d5",  /* mrs x6, dbgbvr1_el1 */
    "9e0230d5",  /* mrs x30, dbgbvr2_el1 */
    "9e0330d5",  /* mrs x30, dbgbvr3_el1 */
    "900430d5",  /* mrs x16, dbgbvr4_el1 */
    "8b0530d5",  /* mrs x11, dbgbvr5_el1 */
    "840630d5",  /* mrs x4, dbgbvr6_el1 */
    "960730d5",  /* mrs x22, dbgbvr7_el1 */
    "910830d5",  /* mrs x17, dbgbvr8_el1 */
    "9d0930d5",  /* mrs x29, dbgbvr9_el1 */
    "c07930d5",  /* mrs x0, dbgclaimclr_el1 */
    "d87830d5",  /* mrs x24, dbgclaimset_el1 */
    "100433d5",  /* mrs x16, dbgdtr_el0 */
    "090533d5",  /* mrs x9, dbgdtrrx_el0 */
    "140513d5",  /* msr dbgdtrtx_el0, x20 */
    "9b1430d5",  /* mrs x27, dbgprcr_el1 */
    "020734d5",  /* mrs x2, dbgvcr32_el2 */
    "f60030d5",  /* mrs x22, dbgwcr0_el1 */
    "fb0a30d5",  /* mrs x27, dbgwcr10_el1 */
    "e80b30d5",  /* mrs x8, dbgwcr11_el1 */
    "f00c30d5",  /* mrs x16, dbgwcr12_el1 */
    "eb0d30d5",  /* mrs x11, dbgwcr13_el1 */
    "fd0e30d5",  /* mrs x29, dbgwcr14_el1 */
    "e50f30d5",  /* mrs x5, dbgwcr15_el1 */
    "eb0130d5",  /* mrs x11, dbgwcr1_el1 */
    "f80230d5",  /* mrs x24, dbgwcr2_el1 */
    "e70330d5",  /* mrs x7, dbgwcr3_el1 */
    "f10430d5",  /* mrs x17, dbgwcr4_el1 */
    "f10530d5",  /* mrs x17, dbgwcr5_el1 */
    "f80630d5",  /* mrs x24, dbgwcr6_el1 */
    "f00730d5",  /* mrs x16, dbgwcr7_el1 */
    "ea0830d5",  /* mrs x10, dbgwcr8_el1 */
    "f40930d5",  /* mrs x20, dbgwcr9_el1 */
    "c70030d5",  /* mrs x7, dbgwvr0_el1 */
    "d30a30d5",  /* mrs x19, dbgwvr10_el1 */
    "d90b30d5",  /* mrs x25, dbgwvr11_el1 */
    "d90c30d5",  /* mrs x25, dbgwvr12_el1 */
    "d80d30d5",  /* mrs x24, dbgwvr13_el1 */
    "db0e30d5",  /* mrs x27, dbgwvr14_el1 */
    "c60f30d5",  /* mrs x6, dbgwvr15_el1 */
    "d90130d5",  /* mrs x25, dbgwvr1_el1 */
    "c70230d5",  /* mrs x7, dbgwvr2_el1 */
    "da0330d5",  /* mrs x26, dbgwvr3_el1 */
    "cc0430d5",  /* mrs x12, dbgwvr4_el1 */
    "d70530d5",  /* mrs x23, dbgwvr5_el1 */
    "d90630d5",  /* mrs x25, dbgwvr6_el1 */
    "c70730d5",  /* mrs x7, dbgwvr7_el1 */
    "c60830d5",  /* mrs x6, dbgwvr8_el1 */
    "d00930d5",  /* mrs x16, dbgwvr9_el1 */
    "ef003bd5",  /* mrs x15, dczid_el0 */
    "2bc138d5",  /* mrs x11, disr_el1 */
    "b7423bd5",  /* mrs x23, dit */
    "20453bd5",  /* mrs x0, dlr_el0 */
    "00453bd5",  /* mrs x0, dspsr_el0 */
    "394038d5",  /* mrs x25, elr_el1 */
    "28403dd5",  /* mrs x8, elr_el12 */
    "2f403cd5",  /* mrs x15, elr_el2 */
    "28403ed5",  /* mrs x8, elr_el3 */
    "065338d5",  /* mrs x6, erridr_el1 */
    "365338d5",  /* mrs x22, errselr_el1 */
    "735438d5",  /* mrs x19, erxaddr_el1 */
    "3e5438d5",  /* mrs x30, erxctlr_el1 */
    "0b5438d5",  /* mrs x11, erxfr_el1 */
    "4e5338d5",  /* mrs x14, erxgsr_el1 */
    "195538d5",  /* mrs x25, erxmisc0_el1 */
    "3d5538d5",  /* mrs x29, erxmisc1_el1 */
    "575538d5",  /* mrs x23, erxmisc2_el1 */
    "6b5538d5",  /* mrs x11, erxmisc3_el1 */
    "de5438d5",  /* mrs x30, erxpfgcdn_el1 */
    "ab5438d5",  /* mrs x11, erxpfgctl_el1 */
    "825438d5",  /* mrs x2, erxpfgf_el1 */
    "475438d5",  /* mrs x7, erxstatus_el1 */
    "035238d5",  /* mrs x3, esr_el1 */
    "07523dd5",  /* mrs x7, esr_el12 */
    "0f523cd5",  /* mrs x15, esr_el2 */
    "06523ed5",  /* mrs x6, esr_el3 */
    "0a6038d5",  /* mrs x10, far_el1 */
    "06603dd5",  /* mrs x6, far_el12 */
    "0f603cd5",  /* mrs x15, far_el2 */
    "13603ed5",  /* mrs x19, far_el3 */
    "bc113ed5",  /* mrs x28, fgwte3_el3 */
    "13443bd5",  /* mrs x19, fpcr */
    "1a533cd5",  /* mrs x26, fpexc32_el2 */
    "40443bd5",  /* mrs x0, fpmr */
    "2f443bd5",  /* mrs x15, fpsr */
    "dd1038d5",  /* mrs x29, gcr_el1 */
    "142538d5",  /* mrs x20, gcscr_el1 */
    "0b253dd5",  /* mrs x11, gcscr_el12 */
    "19253cd5",  /* mrs x25, gcscr_el2 */
    "14253ed5",  /* mrs x20, gcscr_el3 */
    "422538d5",  /* mrs x2, gcscre0_el1 */
    "3a253bd5",  /* mrs x26, gcspr_el0 */
    "352538d5",  /* mrs x21, gcspr_el1 */
    "23253dd5",  /* mrs x3, gcspr_el12 */
    "3d253cd5",  /* mrs x29, gcspr_el2 */
    "2c253ed5",  /* mrs x12, gcspr_el3 */
    "990039d5",  /* mrs x25, gmid_el1 */
    "d6213ed5",  /* mrs x22, gpccr_el3 */
    "98213ed5",  /* mrs x24, gptbr_el3 */
    "86233cd5",  /* mrs x6, hacdbsbr_el2 */
    "af233cd5",  /* mrs x15, hacdbscons_el2 */
    "fc113cd5",  /* mrs x28, hacr_el2 */
    "c5313cd5",  /* mrs x5, hafgrtr_el2 */
    "0d113cd5",  /* mrs x13, hcr_el2 */
    "59123cd5",  /* mrs x25, hcrx_el2 */
    "54233cd5",  /* mrs x20, hdbssbr_el2 */
    "6a233cd5",  /* mrs x10, hdbssprod_el2 */
    "02313cd5",  /* mrs x2, hdfgrtr2_el2 */
    "99313cd5",  /* mrs x25, hdfgrtr_el2 */
    "3e313cd5",  /* mrs x30, hdfgwtr2_el2 */
    "b7313cd5",  /* mrs x23, hdfgwtr_el2 */
    "ec313cd5",  /* mrs x12, hfgitr2_el2 */
    "ce113cd5",  /* mrs x14, hfgitr_el2 */
    "4c313cd5",  /* mrs x12, hfgrtr2_el2 */
    "97113cd5",  /* mrs x23, hfgrtr_el2 */
    "7e313cd5",  /* mrs x30, hfgwtr2_el2 */
    "a2113cd5",  /* mrs x2, hfgwtr_el2 */
    "97603cd5",  /* mrs x23, hpfar_el2 */
    "65113cd5",  /* mrs x5, hstr_el2 */
    "85c838d5",  /* mrs x5, icc_ap0r0_el1 */
    "a4c838d5",  /* mrs x4, icc_ap0r1_el1 */
    "c0c838d5",  /* mrs x0, icc_ap0r2_el1 */
    "e4c838d5",  /* mrs x4, icc_ap0r3_el1 */
    "12c938d5",  /* mrs x18, icc_ap1r0_el1 */
    "3cc938d5",  /* mrs x28, icc_ap1r1_el1 */
    "4ec938d5",  /* mrs x14, icc_ap1r2_el1 */
    "79c938d5",  /* mrs x25, icc_ap1r3_el1 */
    "d4cb18d5",  /* msr icc_asgi1r_el1, x20 */
    "64c838d5",  /* mrs x4, icc_bpr0_el1 */
    "73cc38d5",  /* mrs x19, icc_bpr1_el1 */
    "9acc38d5",  /* mrs x26, icc_ctlr_el1 */
    "93cc3ed5",  /* mrs x19, icc_ctlr_el3 */
    "2fcb18d5",  /* msr icc_dir_el1, x15 */
    "35c818d5",  /* msr icc_eoir0_el1, x21 */
    "3dcc18d5",  /* msr icc_eoir1_el1, x29 */
    "4bc838d5",  /* mrs x11, icc_hppir0_el1 */
    "44cc38d5",  /* mrs x4, icc_hppir1_el1 */
    "11c838d5",  /* mrs x17, icc_iar0_el1 */
    "11cc38d5",  /* mrs x17, icc_iar1_el1 */
    "c4cc38d5",  /* mrs x4, icc_igrpen0_el1 */
    "e0cc38d5",  /* mrs x0, icc_igrpen1_el1 */
    "e0cc3ed5",  /* mrs x0, icc_igrpen1_el3 */
    "b9c938d5",  /* mrs x25, icc_nmiar1_el1 */
    "174638d5",  /* mrs x23, icc_pmr_el1 */
    "74cb38d5",  /* mrs x20, icc_rpr_el1 */
    "e3cb18d5",  /* msr icc_sgi0r_el1, x3 */
    "b0cb18d5",  /* msr icc_sgi1r_el1, x16 */
    "b7cc38d5",  /* mrs x23, icc_sre_el1 */
    "bdc93cd5",  /* mrs x29, icc_sre_el2 */
    "a4cc3ed5",  /* mrs x4, icc_sre_el3 */
    "0dc83cd5",  /* mrs x13, ich_ap0r0_el2 */
    "3bc83cd5",  /* mrs x27, ich_ap0r1_el2 */
    "46c83cd5",  /* mrs x6, ich_ap0r2_el2 */
    "7ac83cd5",  /* mrs x26, ich_ap0r3_el2 */
    "1bc93cd5",  /* mrs x27, ich_ap1r0_el2 */
    "26c93cd5",  /* mrs x6, ich_ap1r1_el2 */
    "40c93cd5",  /* mrs x0, ich_ap1r2_el2 */
    "68c93cd5",  /* mrs x8, ich_ap1r3_el2 */
    "66cb3cd5",  /* mrs x6, ich_eisr_el2 */
    "a9cb3cd5",  /* mrs x9, ich_elrsr_el2 */
    "10cb3cd5",  /* mrs x16, ich_hcr_el2 */
    "07cc3cd5",  /* mrs x7, ich_lr0_el2 */
    "58cd3cd5",  /* mrs x24, ich_lr10_el2 */
    "72cd3cd5",  /* mrs x18, ich_lr11_el2 */
    "8acd3cd5",  /* mrs x10, ich_lr12_el2 */
    "a8cd3cd5",  /* mrs x8, ich_lr13_el2 */
    "d1cd3cd5",  /* mrs x17, ich_lr14_el2 */
    "edcd3cd5",  /* mrs x13, ich_lr15_el2 */
    "3acc3cd5",  /* mrs x26, ich_lr1_el2 */
    "44cc3cd5",  /* mrs x4, ich_lr2_el2 */
    "61cc3cd5",  /* mrs x1, ich_lr3_el2 */
    "9dcc3cd5",  /* mrs x29, ich_lr4_el2 */
    "b7cc3cd5",  /* mrs x23, ich_lr5_el2 */
    "cbcc3cd5",  /* mrs x11, ich_lr6_el2 */
    "fccc3cd5",  /* mrs x28, ich_lr7_el2 */
    "0ecd3cd5",  /* mrs x14, ich_lr8_el2 */
    "35cd3cd5",  /* mrs x21, ich_lr9_el2 */
    "52cb3cd5",  /* mrs x18, ich_misr_el2 */
    "facb3cd5",  /* mrs x26, ich_vmcr_el2 */
    "3ccb3cd5",  /* mrs x28, ich_vtr_el2 */
    "900538d5",  /* mrs x16, id_aa64afr0_el1 */
    "ad0538d5",  /* mrs x13, id_aa64afr1_el1 */
    "1a0538d5",  /* mrs x26, id_aa64dfr0_el1 */
    "3d0538d5",  /* mrs x29, id_aa64dfr1_el1 */
    "5c0538d5",  /* mrs x28, id_aa64dfr2_el1 */
    "f00438d5",  /* mrs x16, id_aa64fpfr0_el1 */
    "040638d5",  /* mrs x4, id_aa64isar0_el1 */
    "310638d5",  /* mrs x17, id_aa64isar1_el1 */
    "440638d5",  /* mrs x4, id_aa64isar2_el1 */
    "700638d5",  /* mrs x16, id_aa64isar3_el1 */
    "100738d5",  /* mrs x16, id_aa64mmfr0_el1 */
    "200738d5",  /* mrs x0, id_aa64mmfr1_el1 */
    "5b0738d5",  /* mrs x27, id_aa64mmfr2_el1 */
    "6e0738d5",  /* mrs x14, id_aa64mmfr3_el1 */
    "980738d5",  /* mrs x24, id_aa64mmfr4_el1 */
    "050438d5",  /* mrs x5, id_aa64pfr0_el1 */
    "330438d5",  /* mrs x19, id_aa64pfr1_el1 */
    "400438d5",  /* mrs x0, id_aa64pfr2_el1 */
    "b80438d5",  /* mrs x24, id_aa64smfr0_el1 */
    "990438d5",  /* mrs x25, id_aa64zfr0_el1 */
    "640138d5",  /* mrs x4, id_afr0_el1 */
    "450138d5",  /* mrs x5, id_dfr0_el1 */
    "a40338d5",  /* mrs x4, id_dfr1_el1 */
    "0f0238d5",  /* mrs x15, id_isar0_el1 */
    "330238d5",  /* mrs x19, id_isar1_el1 */
    "570238d5",  /* mrs x23, id_isar2_el1 */
    "630238d5",  /* mrs x3, id_isar3_el1 */
    "910238d5",  /* mrs x17, id_isar4_el1 */
    "a10238d5",  /* mrs x1, id_isar5_el1 */
    "ea0238d5",  /* mrs x10, id_isar6_el1 */
    "950138d5",  /* mrs x21, id_mmfr0_el1 */
    "b00138d5",  /* mrs x16, id_mmfr1_el1 */
    "d00138d5",  /* mrs x16, id_mmfr2_el1 */
    "f10138d5",  /* mrs x17, id_mmfr3_el1 */
    "cf0238d5",  /* mrs x15, id_mmfr4_el1 */
    "d90338d5",  /* mrs x25, id_mmfr5_el1 */
    "180138d5",  /* mrs x24, id_pfr0_el1 */
    "230138d5",  /* mrs x3, id_pfr1_el1 */
    "9c0338d5",  /* mrs x28, id_pfr2_el1 */
    "31503cd5",  /* mrs x17, ifsr32_el2 */
    "01c138d5",  /* mrs x1, isr_el1 */
    "67a438d5",  /* mrs x7, lorc_el1 */
    "26a438d5",  /* mrs x6, lorea_el1 */
    "e8a438d5",  /* mrs x8, lorid_el1 */
    "41a438d5",  /* mrs x1, lorn_el1 */
    "18a438d5",  /* mrs x24, lorsa_el1 */
    "23a238d5",  /* mrs x3, mair2_el1 */
    "30a23dd5",  /* mrs x16, mair2_el12 */
    "2ea13cd5",  /* mrs x14, mair2_el2 */
    "31a13ed5",  /* mrs x17, mair2_el3 */
    "00a238d5",  /* mrs x0, mair_el1 */
    "18a23dd5",  /* mrs x24, mair_el12 */
    "1ca23cd5",  /* mrs x28, mair_el2 */
    "1da23ed5",  /* mrs x29, mair_el3 */
    "020230d5",  /* mrs x2, mdccint_el1 */
    "0e0133d5",  /* mrs x14, mdccsr_el0 */
    "2a113cd5",  /* mrs x10, mdcr_el2 */
    "33133ed5",  /* mrs x19, mdcr_el3 */
    "101030d5",  /* mrs x16, mdrar_el1 */
    "530230d5",  /* mrs x19, mdscr_el1 */
    "500430d5",  /* mrs x16, mdselr_el1 */
    "460530d5",  /* mrs x6, mdstepop_el1 */
    "36a83cd5",  /* mrs x22, mecid_a0_el2 */
    "68a83cd5",  /* mrs x8, mecid_a1_el2 */
    "0ea83cd5",  /* mrs x14, mecid_p0_el2 */
    "50a83cd5",  /* mrs x16, mecid_p1_el2 */
    "31aa3ed5",  /* mrs x17, mecid_rl_a_el3 */
    "f9a83cd5",  /* mrs x25, mecidr_el2 */
    "af603ed5",  /* mrs x15, mfar_el3 */
    "100038d5",  /* mrs x16, midr_el1 */
    "3ea538d5",  /* mrs x30, mpam0_el1 */
    "07a538d5",  /* mrs x7, mpam1_el1 */
    "16a53dd5",  /* mrs x22, mpam1_el12 */
    "10a53cd5",  /* mrs x16, mpam2_el2 */
    "1ca53ed5",  /* mrs x28, mpam3_el3 */
    "1ca43cd5",  /* mrs x28, mpamhcr_el2 */
    "9ea438d5",  /* mrs x30, mpamidr_el1 */
    "7da538d5",  /* mrs x29, mpamsm_el1 */
    "08a63cd5",  /* mrs x8, mpamvpm0_el2 */
    "3da63cd5",  /* mrs x29, mpamvpm1_el2 */
    "51a63cd5",  /* mrs x17, mpamvpm2_el2 */
    "7ca63cd5",  /* mrs x28, mpamvpm3_el2 */
    "9ea63cd5",  /* mrs x30, mpamvpm4_el2 */
    "a6a63cd5",  /* mrs x6, mpamvpm5_el2 */
    "daa63cd5",  /* mrs x26, mpamvpm6_el2 */
    "eea63cd5",  /* mrs x14, mpamvpm7_el2 */
    "24a43cd5",  /* mrs x4, mpamvpmv_el2 */
    "ad0038d5",  /* mrs x13, mpidr_el1 */
    "830038d5",  /* mrs x3, mpuir_el1 */
    "8c003cd5",  /* mrs x12, mpuir_el2 */
    "0e0338d5",  /* mrs x14, mvfr0_el1 */
    "2a0338d5",  /* mrs x10, mvfr1_el1 */
    "420338d5",  /* mrs x2, mvfr2_el1 */
    "15423bd5",  /* mrs x21, nzcv */
    "871330d5",  /* mrs x7, osdlr_el1 */
    "4d0030d5",  /* mrs x13, osdtrrx_el1 */
    "420330d5",  /* mrs x2, osdtrtx_el1 */
    "460630d5",  /* mrs x6, oseccr_el1 */
    "951010d5",  /* msr oslar_el1, x21 */
    "891130d5",  /* mrs x9, oslsr_el1 */
    "794238d5",  /* mrs x25, pan */
    "037438d5",  /* mrs x3, par_el1 */
    "bc6038d5",  /* mrs x28, pfar_el1 */
    "b8603dd5",  /* mrs x24, pfar_el12 */
    "a4603cd5",  /* mrs x4, pfar_el2 */
    "7ea238d5",  /* mrs x30, pir_el1 */
    "76a23dd5",  /* mrs x22, pir_el12 */
    "74a23cd5",  /* mrs x20, pir_el2 */
    "75a23ed5",  /* mrs x21, pir_el3 */
    "4ba238d5",  /* mrs x11, pire0_el1 */
    "44a23dd5",  /* mrs x4, pire0_el12 */
    "48a23cd5",  /* mrs x8, pire0_el2 */
    "3c4338d5",  /* mrs x28, pm */
    "e49a38d5",  /* mrs x4, pmbidr_el1 */
    "1e9a38d5",  /* mrs x30, pmblimitr_el1 */
    "2e9a38d5",  /* mrs x14, pmbptr_el1 */
    "679a38d5",  /* mrs x7, pmbsr_el1 */
    "f7ef3bd5",  /* mrs x23, pmccfiltr_el0 */
    "1e9d3bd5",  /* mrs x30, pmccntr_el0 */
    "e3eb30d5",  /* mrs x3, pmccntsvr_el1 */
    "cc9c3bd5",  /* mrs x12, pmceid0_el0 */
    "fc9c3bd5",  /* mrs x28, pmceid1_el0 */
    "4f9c3bd5",  /* mrs x15, pmcntenclr_el0 */
    "259c3bd5",  /* mrs x5, pmcntenset_el0 */
    "159c3bd5",  /* mrs x21, pmcr_el0 */
    "ba9e38d5",  /* mrs x26, pmecr_el1 */
    "07e83bd5",  /* mrs x7, pmevcntr0_el0 */
    "45e93bd5",  /* mrs x5, pmevcntr10_el0 */
    "76e93bd5",  /* mrs x22, pmevcntr11_el0 */
    "8de93bd5",  /* mrs x13, pmevcntr12_el0 */
    "b0e93bd5",  /* mrs x16, pmevcntr13_el0 */
    "cce93bd5",  /* mrs x12, pmevcntr14_el0 */
    "eae93bd5",  /* mrs x10, pmevcntr15_el0 */
    "0dea3bd5",  /* mrs x13, pmevcntr16_el0 */
    "26ea3bd5",  /* mrs x6, pmevcntr17_el0 */
    "4bea3bd5",  /* mrs x11, pmevcntr18_el0 */
    "6aea3bd5",  /* mrs x10, pmevcntr19_el0 */
    "22e83bd5",  /* mrs x2, pmevcntr1_el0 */
    "97ea3bd5",  /* mrs x23, pmevcntr20_el0 */
    "abea3bd5",  /* mrs x11, pmevcntr21_el0 */
    "c0ea3bd5",  /* mrs x0, pmevcntr22_el0 */
    "eaea3bd5",  /* mrs x10, pmevcntr23_el0 */
    "11eb3bd5",  /* mrs x17, pmevcntr24_el0 */
    "2eeb3bd5",  /* mrs x14, pmevcntr25_el0 */
    "4eeb3bd5",  /* mrs x14, pmevcntr26_el0 */
    "76eb3bd5",  /* mrs x22, pmevcntr27_el0 */
    "80eb3bd5",  /* mrs x0, pmevcntr28_el0 */
    "aceb3bd5",  /* mrs x12, pmevcntr29_el0 */
    "4ae83bd5",  /* mrs x10, pmevcntr2_el0 */
    "d0eb3bd5",  /* mrs x16, pmevcntr30_el0 */
    "73e83bd5",  /* mrs x19, pmevcntr3_el0 */
    "89e83bd5",  /* mrs x9, pmevcntr4_el0 */
    "b0e83bd5",  /* mrs x16, pmevcntr5_el0 */
    "dee83bd5",  /* mrs x30, pmevcntr6_el0 */
    "e2e83bd5",  /* mrs x2, pmevcntr7_el0 */
    "03e93bd5",  /* mrs x3, pmevcntr8_el0 */
    "3de93bd5",  /* mrs x29, pmevcntr9_el0 */
    "19e830d5",  /* mrs x25, pmevcntsvr0_el1 */
    "47e930d5",  /* mrs x7, pmevcntsvr10_el1 */
    "7ce930d5",  /* mrs x28, pmevcntsvr11_el1 */
    "83e930d5",  /* mrs x3, pmevcntsvr12_el1 */
    "a2e930d5",  /* mrs x2, pmevcntsvr13_el1 */
    "c8e930d5",  /* mrs x8, pmevcntsvr14_el1 */
    "e8e930d5",  /* mrs x8, pmevcntsvr15_el1 */
    "01ea30d5",  /* mrs x1, pmevcntsvr16_el1 */
    "3cea30d5",  /* mrs x28, pmevcntsvr17_el1 */
    "58ea30d5",  /* mrs x24, pmevcntsvr18_el1 */
    "65ea30d5",  /* mrs x5, pmevcntsvr19_el1 */
    "28e830d5",  /* mrs x8, pmevcntsvr1_el1 */
    "98ea30d5",  /* mrs x24, pmevcntsvr20_el1 */
    "a4ea30d5",  /* mrs x4, pmevcntsvr21_el1 */
    "daea30d5",  /* mrs x26, pmevcntsvr22_el1 */
    "edea30d5",  /* mrs x13, pmevcntsvr23_el1 */
    "1beb30d5",  /* mrs x27, pmevcntsvr24_el1 */
    "3deb30d5",  /* mrs x29, pmevcntsvr25_el1 */
    "55eb30d5",  /* mrs x21, pmevcntsvr26_el1 */
    "7aeb30d5",  /* mrs x26, pmevcntsvr27_el1 */
    "9eeb30d5",  /* mrs x30, pmevcntsvr28_el1 */
    "a8eb30d5",  /* mrs x8, pmevcntsvr29_el1 */
    "4ce830d5",  /* mrs x12, pmevcntsvr2_el1 */
    "c4eb30d5",  /* mrs x4, pmevcntsvr30_el1 */
    "71e830d5",  /* mrs x17, pmevcntsvr3_el1 */
    "9de830d5",  /* mrs x29, pmevcntsvr4_el1 */
    "b0e830d5",  /* mrs x16, pmevcntsvr5_el1 */
    "d2e830d5",  /* mrs x18, pmevcntsvr6_el1 */
    "efe830d5",  /* mrs x15, pmevcntsvr7_el1 */
    "16e930d5",  /* mrs x22, pmevcntsvr8_el1 */
    "2ae930d5",  /* mrs x10, pmevcntsvr9_el1 */
    "02ec3bd5",  /* mrs x2, pmevtyper0_el0 */
    "48ed3bd5",  /* mrs x8, pmevtyper10_el0 */
    "61ed3bd5",  /* mrs x1, pmevtyper11_el0 */
    "99ed3bd5",  /* mrs x25, pmevtyper12_el0 */
    "b6ed3bd5",  /* mrs x22, pmevtyper13_el0 */
    "c5ed3bd5",  /* mrs x5, pmevtyper14_el0 */
    "eded3bd5",  /* mrs x13, pmevtyper15_el0 */
    "1cee3bd5",  /* mrs x28, pmevtyper16_el0 */
    "22ee3bd5",  /* mrs x2, pmevtyper17_el0 */
    "48ee3bd5",  /* mrs x8, pmevtyper18_el0 */
    "7eee3bd5",  /* mrs x30, pmevtyper19_el0 */
    "20ec3bd5",  /* mrs x0, pmevtyper1_el0 */
    "94ee3bd5",  /* mrs x20, pmevtyper20_el0 */
    "a2ee3bd5",  /* mrs x2, pmevtyper21_el0 */
    "d9ee3bd5",  /* mrs x25, pmevtyper22_el0 */
    "e8ee3bd5",  /* mrs x8, pmevtyper23_el0 */
    "02ef3bd5",  /* mrs x2, pmevtyper24_el0 */
    "33ef3bd5",  /* mrs x19, pmevtyper25_el0 */
    "5bef3bd5",  /* mrs x27, pmevtyper26_el0 */
    "67ef3bd5",  /* mrs x7, pmevtyper27_el0 */
    "82ef3bd5",  /* mrs x2, pmevtyper28_el0 */
    "a8ef3bd5",  /* mrs x8, pmevtyper29_el0 */
    "5bec3bd5",  /* mrs x27, pmevtyper2_el0 */
    "c3ef3bd5",  /* mrs x3, pmevtyper30_el0 */
    "6eec3bd5",  /* mrs x14, pmevtyper3_el0 */
    "80ec3bd5",  /* mrs x0, pmevtyper4_el0 */
    "aaec3bd5",  /* mrs x10, pmevtyper5_el0 */
    "d1ec3bd5",  /* mrs x17, pmevtyper6_el0 */
    "edec3bd5",  /* mrs x13, pmevtyper7_el0 */
    "1ded3bd5",  /* mrs x29, pmevtyper8_el0 */
    "3ded3bd5",  /* mrs x29, pmevtyper9_el0 */
    "e89e38d5",  /* mrs x8, pmiar_el1 */
    "13963bd5",  /* mrs x19, pmicfiltr_el0 */
    "04943bd5",  /* mrs x4, pmicntr_el0 */
    "01ec30d5",  /* mrs x1, pmicntsvr_el1 */
    "509e38d5",  /* mrs x16, pmintenclr_el1 */
    "369e38d5",  /* mrs x22, pmintenset_el1 */
    "c79e38d5",  /* mrs x7, pmmir_el1 */
    "7e9c3bd5",  /* mrs x30, pmovsclr_el0 */
    "639e3bd5",  /* mrs x3, pmovsset_el0 */
    "059938d5",  /* mrs x5, pmscr_el1 */
    "08993dd5",  /* mrs x8, pmscr_el12 */
    "01993cd5",  /* mrs x1, pmscr_el2 */
    "859a38d5",  /* mrs x5, pmsdsfr_el1 */
    "a69c3bd5",  /* mrs x6, pmselr_el0 */
    "bd9938d5",  /* mrs x29, pmsevfr_el1 */
    "899938d5",  /* mrs x9, pmsfcr_el1 */
    "549938d5",  /* mrs x20, pmsicr_el1 */
    "e99938d5",  /* mrs x9, pmsidr_el1 */
    "709938d5",  /* mrs x16, pmsirr_el1 */
    "d89938d5",  /* mrs x24, pmslatfr_el1 */
    "269938d5",  /* mrs x6, pmsnevfr_el1 */
    "699d38d5",  /* mrs x9, pmsscr_el1 */
    "8e9c1bd5",  /* msr pmswinc_el0, x14 */
    "909e38d5",  /* mrs x16, pmuacr_el1 */
    "159e3bd5",  /* mrs x21, pmuserenr_el0 */
    "459d3bd5",  /* mrs x5, pmxevcntr_el0 */
    "289d3bd5",  /* mrs x8, pmxevtyper_el0 */
    "8b9d1bd5",  /* msr pmzr_el0, x11 */
    "99a23bd5",  /* mrs x25, por_el0 */
    "80a238d5",  /* mrs x0, por_el1 */
    "88a23dd5",  /* mrs x8, por_el12 */
    "81a23cd5",  /* mrs x1, por_el2 */
    "80a23ed5",  /* mrs x0, por_el3 */
    "006d38d5",  /* mrs x0, prbar10_el1 */
    "176d3cd5",  /* mrs x23, prbar10_el2 */
    "906d38d5",  /* mrs x16, prbar11_el1 */
    "916d3cd5",  /* mrs x17, prbar11_el2 */
    "066e38d5",  /* mrs x6, prbar12_el1 */
    "106e3cd5",  /* mrs x16, prbar12_el2 */
    "8f6e38d5",  /* mrs x15, prbar13_el1 */
    "876e3cd5",  /* mrs x7, prbar13_el2 */
    "1d6f38d5",  /* mrs x29, prbar14_el1 */
    "0e6f3cd5",  /* mrs x14, prbar14_el2 */
    "836f38d5",  /* mrs x3, prbar15_el1 */
    "956f3cd5",  /* mrs x21, prbar15_el2 */
    "9a6838d5",  /* mrs x26, prbar1_el1 */
    "94683cd5",  /* mrs x20, prbar1_el2 */
    "0d6938d5",  /* mrs x13, prbar2_el1 */
    "15693cd5",  /* mrs x21, prbar2_el2 */
    "8f6938d5",  /* mrs x15, prbar3_el1 */
    "91693cd5",  /* mrs x17, prbar3_el2 */
    "1a6a38d5",  /* mrs x26, prbar4_el1 */
    "1c6a3cd5",  /* mrs x28, prbar4_el2 */
    "8c6a38d5",  /* mrs x12, prbar5_el1 */
    "906a3cd5",  /* mrs x16, prbar5_el2 */
    "096b38d5",  /* mrs x9, prbar6_el1 */
    "166b3cd5",  /* mrs x22, prbar6_el2 */
    "866b38d5",  /* mrs x6, prbar7_el1 */
    "876b3cd5",  /* mrs x7, prbar7_el2 */
    "0a6c38d5",  /* mrs x10, prbar8_el1 */
    "066c3cd5",  /* mrs x6, prbar8_el2 */
    "9a6c38d5",  /* mrs x26, prbar9_el1 */
    "9c6c3cd5",  /* mrs x28, prbar9_el2 */
    "166838d5",  /* mrs x22, prbar_el1 */
    "17683cd5",  /* mrs x23, prbar_el2 */
    "346138d5",  /* mrs x20, prenr_el1 */
    "24613cd5",  /* mrs x4, prenr_el2 */
    "2c6d38d5",  /* mrs x12, prlar10_el1 */
    "2b6d3cd5",  /* mrs x11, prlar10_el2 */
    "a16d38d5",  /* mrs x1, prlar11_el1 */
    "ba6d3cd5",  /* mrs x26, prlar11_el2 */
    "246e38d5",  /* mrs x4, prlar12_el1 */
    "206e3cd5",  /* mrs x0, prlar12_el2 */
    "a26e38d5",  /* mrs x2, prlar13_el1 */
    "b46e3cd5",  /* mrs x20, prlar13_el2 */
    "376f38d5",  /* mrs x23, prlar14_el1 */
    "3c6f3cd5",  /* mrs x28, prlar14_el2 */
    "a86f38d5",  /* mrs x8, prlar15_el1 */
    "ad6f3cd5",  /* mrs x13, prlar15_el2 */
    "a56838d5",  /* mrs x5, prlar1_el1 */
    "a1683cd5",  /* mrs x1, prlar1_el2 */
    "226938d5",  /* mrs x2, prlar2_el1 */
    "35693cd5",  /* mrs x21, prlar2_el2 */
    "ba6938d5",  /* mrs x26, prlar3_el1 */
    "ac693cd5",  /* mrs x12, prlar3_el2 */
    "3b6a38d5",  /* mrs x27, prlar4_el1 */
    "306a3cd5",  /* mrs x16, prlar4_el2 */
    "b56a38d5",  /* mrs x21, prlar5_el1 */
    "a96a3cd5",  /* mrs x9, prlar5_el2 */
    "336b38d5",  /* mrs x19, prlar6_el1 */
    "276b3cd5",  /* mrs x7, prlar6_el2 */
    "b66b38d5",  /* mrs x22, prlar7_el1 */
    "a96b3cd5",  /* mrs x9, prlar7_el2 */
    "216c38d5",  /* mrs x1, prlar8_el1 */
    "2e6c3cd5",  /* mrs x14, prlar8_el2 */
    "a56c38d5",  /* mrs x5, prlar9_el1 */
    "a56c3cd5",  /* mrs x5, prlar9_el2 */
    "286838d5",  /* mrs x8, prlar_el1 */
    "2e683cd5",  /* mrs x14, prlar_el2 */
    "206238d5",  /* mrs x0, prselr_el1 */
    "28623cd5",  /* mrs x8, prselr_el2 */
    "cbd038d5",  /* mrs x11, rcwmask_el1 */
    "7ed038d5",  /* mrs x30, rcwsmask_el1 */
    "ca0038d5",  /* mrs x10, revidr_el1 */
    "b11038d5",  /* mrs x17, rgsr_el1 */
    "4ac038d5",  /* mrs x10, rmr_el1 */
    "47c03cd5",  /* mrs x7, rmr_el2 */
    "41c03ed5",  /* mrs x1, rmr_el3 */
    "1e243bd5",  /* mrs x30, rndr */
    "3c243bd5",  /* mrs x28, rndrrs */
    "29c038d5",  /* mrs x9, rvbar_el1 */
    "26c03cd5",  /* mrs x6, rvbar_el2 */
    "2bc03ed5",  /* mrs x11, rvbar_el3 */
    "a5a23cd5",  /* mrs x5, s2pir_el2 */
    "a0a238d5",  /* mrs x0, s2por_el1 */
    "0a113ed5",  /* mrs x10, scr_el3 */
    "6c1038d5",  /* mrs x12, sctlr2_el1 */
    "62103dd5",  /* mrs x2, sctlr2_el12 */
    "6f103cd5",  /* mrs x15, sctlr2_el2 */
    "68103ed5",  /* mrs x8, sctlr2_el3 */
    "101038d5",  /* mrs x16, sctlr_el1 */
    "14103dd5",  /* mrs x20, sctlr_el12 */
    "06103cd5",  /* mrs x6, sctlr_el2 */
    "07103ed5",  /* mrs x7, sctlr_el3 */
    "f0d03bd5",  /* mrs x16, scxtnum_el0 */
    "f8d038d5",  /* mrs x24, scxtnum_el1 */
    "e0d03dd5",  /* mrs x0, scxtnum_el12 */
    "e2d03cd5",  /* mrs x2, scxtnum_el2 */
    "e8d03ed5",  /* mrs x8, scxtnum_el3 */
    "3a133cd5",  /* mrs x26, sder32_el2 */
    "22113ed5",  /* mrs x2, sder32_el3 */
    "c41238d5",  /* mrs x4, smcr_el1 */
    "cc123dd5",  /* mrs x12, smcr_el12 */
    "d2123cd5",  /* mrs x18, smcr_el2 */
    "c1123ed5",  /* mrs x1, smcr_el3 */
    "cc0039d5",  /* mrs x12, smidr_el1 */
    "801238d5",  /* mrs x0, smpri_el1 */
    "a9123cd5",  /* mrs x9, smprimap_el2 */
    "094138d5",  /* mrs x9, sp_el0 */
    "14413cd5",  /* mrs x20, sp_el1 */
    "07413ed5",  /* mrs x7, sp_el2 */
    "629d30d5",  /* mrs x2, spmaccessr_el1 */
    "729d35d5",  /* mrs x18, spmaccessr_el12 */
    "7e9d34d5",  /* mrs x30, spmaccessr_el2 */
    "709d36d5",  /* mrs x16, spmaccessr_el3 */
    "fb9d30d5",  /* mrs x27, spmcfgr_el1 */
    "189d30d5",  /* mrs x24, spmcgcr0_el1 */
    "249d30d5",  /* mrs x4, spmcgcr1_el1 */
    "559c33d5",  /* mrs x21, spmcntenclr_el0 */
    "3c9c33d5",  /* mrs x28, spmcntenset_el0 */
    "169c33d5",  /* mrs x22, spmcr_el0 */
    "d99d30d5",  /* mrs x25, spmdevaff_el1 */
    "bc9d30d5",  /* mrs x28, spmdevarch_el1 */
    "13e033d5",  /* mrs x19, spmevcntr0_el0 */
    "4ce133d5",  /* mrs x12, spmevcntr10_el0 */
    "78e133d5",  /* mrs x24, spmevcntr11_el0 */
    "8ae133d5",  /* mrs x10, spmevcntr12_el0 */
    "b7e133d5",  /* mrs x23, spmevcntr13_el0 */
    "cfe133d5",  /* mrs x15, spmevcntr14_el0 */
    "e4e133d5",  /* mrs x4, spmevcntr15_el0 */
    "29e033d5",  /* mrs x9, spmevcntr1_el0 */
    "57e033d5",  /* mrs x23, spmevcntr2_el0 */
    "73e033d5",  /* mrs x19, spmevcntr3_el0 */
    "94e033d5",  /* mrs x20, spmevcntr4_el0 */
    "a4e033d5",  /* mrs x4, spmevcntr5_el0 */
    "c1e033d5",  /* mrs x1, spmevcntr6_el0 */
    "fae033d5",  /* mrs x26, spmevcntr7_el0 */
    "1ae133d5",  /* mrs x26, spmevcntr8_el0 */
    "36e133d5",  /* mrs x22, spmevcntr9_el0 */
    "1ce633d5",  /* mrs x28, spmevfilt2r0_el0 */
    "50e733d5",  /* mrs x16, spmevfilt2r10_el0 */
    "74e733d5",  /* mrs x20, spmevfilt2r11_el0 */
    "8de733d5",  /* mrs x13, spmevfilt2r12_el0 */
    "b7e733d5",  /* mrs x23, spmevfilt2r13_el0 */
    "d6e733d5",  /* mrs x22, spmevfilt2r14_el0 */
    "f9e733d5",  /* mrs x25, spmevfilt2r15_el0 */
    "30e633d5",  /* mrs x16, spmevfilt2r1_el0 */
    "44e633d5",  /* mrs x4, spmevfilt2r2_el0 */
    "7de633d5",  /* mrs x29, spmevfilt2r3_el0 */
    "90e633d5",  /* mrs x16, spmevfilt2r4_el0 */
    "b8e633d5",  /* mrs x24, spmevfilt2r5_el0 */
    "d0e633d5",  /* mrs x16, spmevfilt2r6_el0 */
    "f2e633d5",  /* mrs x18, spmevfilt2r7_el0 */
    "1ae733d5",  /* mrs x26, spmevfilt2r8_el0 */
    "3ae733d5",  /* mrs x26, spmevfilt2r9_el0 */
    "19e433d5",  /* mrs x25, spmevfiltr0_el0 */
    "40e533d5",  /* mrs x0, spmevfiltr10_el0 */
    "7ae533d5",  /* mrs x26, spmevfiltr11_el0 */
    "95e533d5",  /* mrs x21, spmevfiltr12_el0 */
    "b2e533d5",  /* mrs x18, spmevfiltr13_el0 */
    "d9e533d5",  /* mrs x25, spmevfiltr14_el0 */
    "fce533d5",  /* mrs x28, spmevfiltr15_el0 */
    "36e433d5",  /* mrs x22, spmevfiltr1_el0 */
    "55e433d5",  /* mrs x21, spmevfiltr2_el0 */
    "7ee433d5",  /* mrs x30, spmevfiltr3_el0 */
    "96e433d5",  /* mrs x22, spmevfiltr4_el0 */
    "b4e433d5",  /* mrs x20, spmevfiltr5_el0 */
    "c7e433d5",  /* mrs x7, spmevfiltr6_el0 */
    "e2e433d5",  /* mrs x2, spmevfiltr7_el0 */
    "00e533d5",  /* mrs x0, spmevfiltr8_el0 */
    "21e533d5",  /* mrs x1, spmevfiltr9_el0 */
    "04e233d5",  /* mrs x4, spmevtyper0_el0 */
    "54e333d5",  /* mrs x20, spmevtyper10_el0 */
    "6be333d5",  /* mrs x11, spmevtyper11_el0 */
    "9ee333d5",  /* mrs x30, spmevtyper12_el0 */
    "a3e333d5",  /* mrs x3, spmevtyper13_el0 */
    "cce333d5",  /* mrs x12, spmevtyper14_el0 */
    "fae333d5",  /* mrs x26, spmevtyper15_el0 */
    "2ee233d5",  /* mrs x14, spmevtyper1_el0 */
    "51e233d5",  /* mrs x17, spmevtyper2_el0 */
    "61e233d5",  /* mrs x1, spmevtyper3_el0 */
    "94e233d5",  /* mrs x20, spmevtyper4_el0 */
    "a0e233d5",  /* mrs x0, spmevtyper5_el0 */
    "d4e233d5",  /* mrs x20, spmevtyper6_el0 */
    "f1e233d5",  /* mrs x17, spmevtyper7_el0 */
    "15e333d5",  /* mrs x21, spmevtyper8_el0 */
    "27e333d5",  /* mrs x7, spmevtyper9_el0 */
    "8f9d30d5",  /* mrs x15, spmiidr_el1 */
    "489e30d5",  /* mrs x8, spmintenclr_el1 */
    "209e30d5",  /* mrs x0, spmintenset_el1 */
    "6e9c33d5",  /* mrs x14, spmovsclr_el0 */
    "799e33d5",  /* mrs x25, spmovsset_el0 */
    "e29e36d5",  /* mrs x2, spmrootcr_el3 */
    "f79e37d5",  /* mrs x23, spmscr_el1 */
    "bd9c33d5",  /* mrs x29, spmselr_el0 */
    "909c13d5",  /* msr spmzr_el0, x16 */
    "1c4238d5",  /* mrs x28, spsel */
    "31433cd5",  /* mrs x17, spsr_abt */
    "024038d5",  /* mrs x2, spsr_el1 */
    "15403dd5",  /* mrs x21, spsr_el12 */
    "10403cd5",  /* mrs x16, spsr_el2 */
    "02403ed5",  /* mrs x2, spsr_el3 */
    "77433cd5",  /* mrs x23, spsr_fiq */
    "17433cd5",  /* mrs x23, spsr_irq */
    "4f433cd5",  /* mrs x15, spsr_und */
    "c8423bd5",  /* mrs x8, ssbs */
    "59423bd5",  /* mrs x25, svcr */
    "e2423bd5",  /* mrs x2, tco */
    "7b2038d5",  /* mrs x27, tcr2_el1 */
    "68203dd5",  /* mrs x8, tcr2_el12 */
    "67203cd5",  /* mrs x7, tcr2_el2 */
    "572038d5",  /* mrs x23, tcr_el1 */
    "58203dd5",  /* mrs x24, tcr_el12 */
    "46203cd5",  /* mrs x6, tcr_el2 */
    "47203ed5",  /* mrs x7, tcr_el3 */
    "170032d5",  /* mrs x23, teecr32_el1 */
    "141032d5",  /* mrs x20, teehbr32_el1 */
    "0e5638d5",  /* mrs x14, tfsr_el1 */
    "0f563dd5",  /* mrs x15, tfsr_el12 */
    "1b563cd5",  /* mrs x27, tfsr_el2 */
    "0c563ed5",  /* mrs x12, tfsr_el3 */
    "225638d5",  /* mrs x2, tfsre0_el1 */
    "afd03bd5",  /* mrs x15, tpidr2_el0 */
    "5dd03bd5",  /* mrs x29, tpidr_el0 */
    "95d038d5",  /* mrs x21, tpidr_el1 */
    "49d03cd5",  /* mrs x9, tpidr_el2 */
    "58d03ed5",  /* mrs x24, tpidr_el3 */
    "61d03bd5",  /* mrs x1, tpidrro_el0 */
    "539b38d5",  /* mrs x19, trbbaser_el1 */
    "f49b38d5",  /* mrs x20, trbidr_el1 */
    "149b38d5",  /* mrs x20, trblimitr_el1 */
    "869b38d5",  /* mrs x6, trbmar_el1 */
    "229b38d5",  /* mrs x2, trbptr_el1 */
    "739b38d5",  /* mrs x19, trbsr_el1 */
    "c49b38d5",  /* mrs x4, trbtrg_el1 */
    "4a2031d5",  /* mrs x10, trcacatr0 */
    "482231d5",  /* mrs x8, trcacatr1 */
    "742431d5",  /* mrs x20, trcacatr10 */
    "772631d5",  /* mrs x23, trcacatr11 */
    "762831d5",  /* mrs x22, trcacatr12 */
    "692a31d5",  /* mrs x9, trcacatr13 */
    "732c31d5",  /* mrs x19, trcacatr14 */
    "722e31d5",  /* mrs x18, trcacatr15 */
    "442431d5",  /* mrs x4, trcacatr2 */
    "402631d5",  /* mrs x0, trcacatr3 */
    "4f2831d5",  /* mrs x15, trcacatr4 */
    "412a31d5",  /* mrs x1, trcacatr5 */
    "4f2c31d5",  /* mrs x15, trcacatr6 */
    "482e31d5",  /* mrs x8, trcacatr7 */
    "752031d5",  /* mrs x21, trcacatr8 */
    "632231d5",  /* mrs x3, trcacatr9 */
    "162031d5",  /* mrs x22, trcacvr0 */
    "062231d5",  /* mrs x6, trcacvr1 */
    "352431d5",  /* mrs x21, trcacvr10 */
    "2f2631d5",  /* mrs x15, trcacvr11 */
    "292831d5",  /* mrs x9, trcacvr12 */
    "362a31d5",  /* mrs x22, trcacvr13 */
    "302c31d5",  /* mrs x16, trcacvr14 */
    "292e31d5",  /* mrs x9, trcacvr15 */
    "0e2431d5",  /* mrs x14, trcacvr2 */
    "0e2631d5",  /* mrs x14, trcacvr3 */
    "0e2831d5",  /* mrs x14, trcacvr4 */
    "182a31d5",  /* mrs x24, trcacvr5 */
    "032c31d5",  /* mrs x3, trcacvr6 */
    "1c2e31d5",  /* mrs x28, trcacvr7 */
    "312031d5",  /* mrs x17, trcacvr8 */
    "262231d5",  /* mrs x6, trcacvr9 */
    "c97e31d5",  /* mrs x9, trcauthstatus */
    "020631d5",  /* mrs x2, trcauxctlr */
    "1d0f31d5",  /* mrs x29, trcbbctlr */
    "0f0e31d5",  /* mrs x15, trcccctlr */
    "403031d5",  /* mrs x0, trccidcctlr0 */
    "493131d5",  /* mrs x9, trccidcctlr1 */
    "0e3031d5",  /* mrs x14, trccidcvr0 */
    "023231d5",  /* mrs x2, trccidcvr1 */
    "1a3431d5",  /* mrs x26, trccidcvr2 */
    "103631d5",  /* mrs x16, trccidcvr3 */
    "1e3831d5",  /* mrs x30, trccidcvr4 */
    "0e3a31d5",  /* mrs x14, trccidcvr5 */
    "083c31d5",  /* mrs x8, trccidcvr6 */
    "0c3e31d5",  /* mrs x12, trccidcvr7 */
    "e67c31d5",  /* mrs x6, trccidr0 */
    "fd7d31d5",  /* mrs x29, trccidr1 */
    "fe7e31d5",  /* mrs x30, trccidr2 */
    "fd7f31d5",  /* mrs x29, trccidr3 */
    "c67931d5",  /* mrs x6, trcclaimclr */
    "c27831d5",  /* mrs x2, trcclaimset */
    "b20431d5",  /* mrs x18, trccntctlr0 */
    "a20531d5",  /* mrs x2, trccntctlr1 */
    "a40631d5",  /* mrs x4, trccntctlr2 */
    "b70731d5",  /* mrs x23, trccntctlr3 */
    "b00031d5",  /* mrs x16, trccntrldvr0 */
    "a80131d5",  /* mrs x8, trccntrldvr1 */
    "be0231d5",  /* mrs x30, trccntrldvr2 */
    "ab0331d5",  /* mrs x11, trccntrldvr3 */
    "a40831d5",  /* mrs x4, trccntvr0 */
    "b30931d5",  /* mrs x19, trccntvr1 */
    "ba0a31d5",  /* mrs x26, trccntvr2 */
    "b40b31d5",  /* mrs x20, trccntvr3 */
    "100431d5",  /* mrs x16, trcconfigr */
    "c87a31d5",  /* mrs x8, trcdevaff0 */
    "dc7b31d5",  /* mrs x28, trcdevaff1 */
    "c37f31d5",  /* mrs x3, trcdevarch */
    "f67231d5",  /* mrs x22, trcdevid */
    "eb7331d5",  /* mrs x11, trcdevtype */
    "c72031d5",  /* mrs x7, trcdvcmr0 */
    "cf2431d5",  /* mrs x15, trcdvcmr1 */
    "dc2831d5",  /* mrs x28, trcdvcmr2 */
    "dc2c31d5",  /* mrs x28, trcdvcmr3 */
    "ef2031d5",  /* mrs x15, trcdvcmr4 */
    "ec2431d5",  /* mrs x12, trcdvcmr5 */
    "e02831d5",  /* mrs x0, trcdvcmr6 */
    "e52c31d5",  /* mrs x5, trcdvcmr7 */
    "802031d5",  /* mrs x0, trcdvcvr0 */
    "9e2431d5",  /* mrs x30, trcdvcvr1 */
    "8f2831d5",  /* mrs x15, trcdvcvr2 */
    "952c31d5",  /* mrs x21, trcdvcvr3 */
    "ae2031d5",  /* mrs x14, trcdvcvr4 */
    "ac2431d5",  /* mrs x12, trcdvcvr5 */
    "a92831d5",  /* mrs x9, trcdvcvr6 */
    "b72c31d5",  /* mrs x23, trcdvcvr7 */
    "040831d5",  /* mrs x4, trceventctl0r */
    "0d0931d5",  /* mrs x13, trceventctl1r */
    "8b0931d5",  /* mrs x11, trcextinselr1 */
    "8c0a31d5",  /* mrs x12, trcextinselr2 */
    "8a0b31d5",  /* mrs x10, trcextinselr3 */
    "e30831d5",  /* mrs x3, trcidr0 */
    "fa0931d5",  /* mrs x26, trcidr1 */
    "ca0231d5",  /* mrs x10, trcidr10 */
    "c00331d5",  /* mrs x0, trcidr11 */
    "ca0431d5",  /* mrs x10, trcidr12 */
    "d80531d5",  /* mrs x24, trcidr13 */
    "ea0a31d5",  /* mrs x10, trcidr2 */
    "fa0b31d5",  /* mrs x26, trcidr3 */
    "ec0c31d5",  /* mrs x12, trcidr4 */
    "e30d31d5",  /* mrs x3, trcidr5 */
    "fe0e31d5",  /* mrs x30, trcidr6 */
    "fd0f31d5",  /* mrs x29, trcidr7 */
    "c60031d5",  /* mrs x6, trcidr8 */
    "d60131d5",  /* mrs x22, trcidr9 */
    "e00031d5",  /* mrs x0, trcimspec0 */
    "fc0131d5",  /* mrs x28, trcimspec1 */
    "f70231d5",  /* mrs x23, trcimspec2 */
    "e90331d5",  /* mrs x9, trcimspec3 */
    "e80431d5",  /* mrs x8, trcimspec4 */
    "eb0531d5",  /* mrs x11, trcimspec5 */
    "e20631d5",  /* mrs x2, trcimspec6 */
    "ec0731d5",  /* mrs x12, trcimspec7 */
    "8c7031d5",  /* mrs x12, trcitctrl */
    "7b1238d5",  /* mrs x27, trcitecr_el1 */
    "72123dd5",  /* mrs x18, trcitecr_el12 */
    "62123cd5",  /* mrs x2, trcitecr_el2 */
    "2b0231d5",  /* mrs x11, trciteedcr */
    "dd7c11d5",  /* msr trclar, x29 */
    "cd7d31d5",  /* mrs x13, trclsr */
    "981011d5",  /* msr trcoslar, x24 */
    "881131d5",  /* mrs x8, trcoslsr */
    "9b1431d5",  /* mrs x27, trcpdcr */
    "811531d5",  /* mrs x1, trcpdsr */
    "e87831d5",  /* mrs x8, trcpidr0 */
    "e37931d5",  /* mrs x3, trcpidr1 */
    "e17a31d5",  /* mrs x1, trcpidr2 */
    "fa7b31d5",  /* mrs x26, trcpidr3 */
    "f57431d5",  /* mrs x21, trcpidr4 */
    "e97531d5",  /* mrs x9, trcpidr5 */
    "f47631d5",  /* mrs x20, trcpidr6 */
    "fd7731d5",  /* mrs x29, trcpidr7 */
    "040131d5",  /* mrs x4, trcprgctlr */
    "070231d5",  /* mrs x7, trcprocselr */
    "280131d5",  /* mrs x8, trcqctlr */
    "0d1a31d5",  /* mrs x13, trcrsctlr10 */
    "101b31d5",  /* mrs x16, trcrsctlr11 */
    "0a1c31d5",  /* mrs x10, trcrsctlr12 */
    "061d31d5",  /* mrs x6, trcrsctlr13 */
    "181e31d5",  /* mrs x24, trcrsctlr14 */
    "0b1f31d5",  /* mrs x11, trcrsctlr15 */
    "391031d5",  /* mrs x25, trcrsctlr16 */
    "3e1131d5",  /* mrs x30, trcrsctlr17 */
    "2d1231d5",  /* mrs x13, trcrsctlr18 */
    "3c1331d5",  /* mrs x28, trcrsctlr19 */
    "001231d5",  /* mrs x0, trcrsctlr2 */
    "391431d5",  /* mrs x25, trcrsctlr20 */
    "381531d5",  /* mrs x24, trcrsctlr21 */
    "341631d5",  /* mrs x20, trcrsctlr22 */
    "2c1731d5",  /* mrs x12, trcrsctlr23 */
    "3d1831d5",  /* mrs x29, trcrsctlr24 */
    "3c1931d5",  /* mrs x28, trcrsctlr25 */
    "3e1a31d5",  /* mrs x30, trcrsctlr26 */
    "311b31d5",  /* mrs x17, trcrsctlr27 */
    "311c31d5",  /* mrs x17, trcrsctlr28 */
    "261d31d5",  /* mrs x6, trcrsctlr29 */
    "171331d5",  /* mrs x23, trcrsctlr3 */
    "221e31d5",  /* mrs x2, trcrsctlr30 */
    "211f31d5",  /* mrs x1, trcrsctlr31 */
    "1d1431d5",  /* mrs x29, trcrsctlr4 */
    "171531d5",  /* mrs x23, trcrsctlr5 */
    "0d1631d5",  /* mrs x13, trcrsctlr6 */
    "0e1731d5",  /* mrs x14, trcrsctlr7 */
    "131831d5",  /* mrs x19, trcrsctlr8 */
    "181931d5",  /* mrs x24, trcrsctlr9 */
    "040a31d5",  /* mrs x4, trcrsr */
    "940031d5",  /* mrs x20, trcseqevr0 */
    "9b0131d5",  /* mrs x27, trcseqevr1 */
    "890231d5",  /* mrs x9, trcseqevr2 */
    "8f0631d5",  /* mrs x15, trcseqrstevr */
    "810731d5",  /* mrs x1, trcseqstr */
    "5d1031d5",  /* mrs x29, trcssccr0 */
    "5d1131d5",  /* mrs x29, trcssccr1 */
    "511231d5",  /* mrs x17, trcssccr2 */
    "441331d5",  /* mrs x4, trcssccr3 */
    "451431d5",  /* mrs x5, trcssccr4 */
    "4f1531d5",  /* mrs x15, trcssccr5 */
    "4d1631d5",  /* mrs x13, trcssccr6 */
    "4a1731d5",  /* mrs x10, trcssccr7 */
    "491831d5",  /* mrs x9, trcsscsr0 */
    "491931d5",  /* mrs x9, trcsscsr1 */
    "481a31d5",  /* mrs x8, trcsscsr2 */
    "571b31d5",  /* mrs x23, trcsscsr3 */
    "571c31d5",  /* mrs x23, trcsscsr4 */
    "541d31d5",  /* mrs x20, trcsscsr5 */
    "481e31d5",  /* mrs x8, trcsscsr6 */
    "4c1f31d5",  /* mrs x12, trcsscsr7 */
    "741031d5",  /* mrs x20, trcsspcicr0 */
    "671131d5",  /* mrs x7, trcsspcicr1 */
    "691231d5",  /* mrs x9, trcsspcicr2 */
    "6f1331d5",  /* mrs x15, trcsspcicr3 */
    "711431d5",  /* mrs x17, trcsspcicr4 */
    "751531d5",  /* mrs x21, trcsspcicr5 */
    "6c1631d5",  /* mrs x12, trcsspcicr6 */
    "631731d5",  /* mrs x3, trcsspcicr7 */
    "050b31d5",  /* mrs x5, trcstallctlr */
    "140331d5",  /* mrs x20, trcstatr */
    "050d31d5",  /* mrs x5, trcsyncpr */
    "220031d5",  /* mrs x2, trctraceidr */
    "060c31d5",  /* mrs x6, trctsctlr */
    "500a31d5",  /* mrs x16, trcvdarcctlr */
    "5c0831d5",  /* mrs x28, trcvdctlr */
    "590931d5",  /* mrs x25, trcvdsacctlr */
    "4f0031d5",  /* mrs x15, trcvictlr */
    "510131d5",  /* mrs x17, trcviiectlr */
    "470331d5",  /* mrs x7, trcvipcssctlr */
    "4e0231d5",  /* mrs x14, trcvissctlr */
    "5d3231d5",  /* mrs x29, trcvmidcctlr0 */
    "4a3331d5",  /* mrs x10, trcvmidcctlr1 */
    "383031d5",  /* mrs x24, trcvmidcvr0 */
    "2e3231d5",  /* mrs x14, trcvmidcvr1 */
    "2d3431d5",  /* mrs x13, trcvmidcvr2 */
    "243631d5",  /* mrs x4, trcvmidcvr3 */
    "313831d5",  /* mrs x17, trcvmidcvr4 */
    "263a31d5",  /* mrs x6, trcvmidcvr5 */
    "273c31d5",  /* mrs x7, trcvmidcvr6 */
    "223e31d5",  /* mrs x2, trcvmidcvr7 */
    "251238d5",  /* mrs x5, trfcr_el1 */
    "2a123dd5",  /* mrs x10, trfcr_el12 */
    "31123cd5",  /* mrs x17, trfcr_el2 */
    "022038d5",  /* mrs x2, ttbr0_el1 */
    "0a203dd5",  /* mrs x10, ttbr0_el12 */
    "07203cd5",  /* mrs x7, ttbr0_el2 */
    "0b203ed5",  /* mrs x11, ttbr0_el3 */
    "282038d5",  /* mrs x8, ttbr1_el1 */
    "39203dd5",  /* mrs x25, ttbr1_el12 */
    "32203cd5",  /* mrs x18, ttbr1_el2 */
    "864238d5",  /* mrs x6, uao */
    "1cc038d5",  /* mrs x28, vbar_el1 */
    "00c03dd5",  /* mrs x0, vbar_el12 */
    "17c03cd5",  /* mrs x23, vbar_el2 */
    "1bc03ed5",  /* mrs x27, vbar_el3 */
    "2dc13cd5",  /* mrs x13, vdisr_el2 */
    "2cc13ed5",  /* mrs x12, vdisr_el3 */
    "2da93cd5",  /* mrs x13, vmecid_a_el2 */
    "17a93cd5",  /* mrs x23, vmecid_p_el2 */
    "b0003cd5",  /* mrs x16, vmpidr_el2 */
    "06223cd5",  /* mrs x6, vncr_el2 */
    "0c003cd5",  /* mrs x12, vpidr_el2 */
    "68523cd5",  /* mrs x8, vsesr_el2 */
    "6a523ed5",  /* mrs x10, vsesr_el3 */
    "58263cd5",  /* mrs x24, vstcr_el2 */
    "01263cd5",  /* mrs x1, vsttbr_el2 */
    "4f213cd5",  /* mrs x15, vtcr_el2 */
    "08213cd5",  /* mrs x8, vttbr_el2 */
    "121238d5",  /* mrs x18, zcr_el1 */
    "1e123dd5",  /* mrs x30, zcr_el12 */
    "0b123cd5",  /* mrs x11, zcr_el2 */
    "04123ed5",  /* mrs x4, zcr_el3 */
    "027478d5",  /* mrrs x2, x3, par_el1 */
    "047458d5",  /* msrr par_el1, x4, x5 */
    "c2d078d5",  /* mrrs x2, x3, rcwmask_el1 */
    "c4d058d5",  /* msrr rcwmask_el1, x4, x5 */
    "62d078d5",  /* mrrs x2, x3, rcwsmask_el1 */
    "64d058d5",  /* msrr rcwsmask_el1, x4, x5 */
    "022078d5",  /* mrrs x2, x3, ttbr0_el1 */
    "042058d5",  /* msrr ttbr0_el1, x4, x5 */
    "02207dd5",  /* mrrs x2, x3, ttbr0_el12 */
    "04205dd5",  /* msrr ttbr0_el12, x4, x5 */
    "02207cd5",  /* mrrs x2, x3, ttbr0_el2 */
    "04205cd5",  /* msrr ttbr0_el2, x4, x5 */
    "222078d5",  /* mrrs x2, x3, ttbr1_el1 */
    "242058d5",  /* msrr ttbr1_el1, x4, x5 */
    "22207dd5",  /* mrrs x2, x3, ttbr1_el12 */
    "24205dd5",  /* msrr ttbr1_el12, x4, x5 */
    "22207cd5",  /* mrrs x2, x3, ttbr1_el2 */
    "24205cd5",  /* msrr ttbr1_el2, x4, x5 */
    "02217cd5",  /* mrrs x2, x3, vttbr_el2 */
    "04215cd5",  /* msrr vttbr_el2, x4, x5 */
};

static int hexval(char c) { return c <= '9' ? c - '0' : c - 'a' + 10; }

int main(void) {
    const unsigned char *p = axiom_asm_table;
    int bad = 0, n = (int)(sizeof want / sizeof want[0]);
    for (int i = 0; i < n; i++) {
        const char *w = want[i];
        for (int k = 0; w[k] != 0; k += 2) {
            if (p >= axiom_asm_table_end || *p != hexval(w[k]) * 16 + hexval(w[k + 1])) {
                if (bad++ < 5) { print_str("mismatch at line ", 17); print_i64(i); }
            }
            p++;
        }
    }
    print_i64(bad);
    print_i64(p == axiom_asm_table_end);
    return 0;
}
#else
int main(void) { print_i64(0); print_i64(1); return 0; }
#endif
/* expect: 0 */
/* expect: 1 */
