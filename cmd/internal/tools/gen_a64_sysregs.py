#!/usr/bin/env python3
"""Generates cmd/axiom/src/asm/a64sysreg.ax, the AArch64 assembler's table of
named system registers for MRS and MSR, from the register list below (each
with its op0, op1, CRn, CRm, op2 as the Arm Architecture Reference Manual's
"System Register Encoding" tables give them):

    python3 cmd/internal/tools/gen_a64_sysregs.py > cmd/axiom/src/asm/a64sysreg.ax && axiom fmt cmd/axiom/src/asm/a64sysreg.ax

The assembler also accepts the generic S<op0>_<op1>_C<n>_C<m>_<op2> form, so
this table is only the names; a register it does not list is still reachable
by that form.  The 15-bit value stored is op0<14:14> op1<13:11> CRn<10:7>
CRm<6:3> op2<2:0> with op0 reduced by 2 (as MRS/MSR encode it from bit 5).
"""

# name: (op0, op1, CRn, CRm, op2)
REGS = {
    # thread and timer registers used from EL0
    'tpidr_el0': (3, 3, 13, 0, 2), 'tpidrro_el0': (3, 3, 13, 0, 3), 'tpidr_el1': (3, 0, 13, 0, 4),
    'tpidr_el2': (3, 4, 13, 0, 2), 'tpidr2_el0': (3, 3, 13, 0, 5),
    'cntfrq_el0': (3, 3, 14, 0, 0), 'cntpct_el0': (3, 3, 14, 0, 1), 'cntvct_el0': (3, 3, 14, 0, 2),
    'cntpctss_el0': (3, 3, 14, 0, 5), 'cntvctss_el0': (3, 3, 14, 0, 6),
    'cntp_tval_el0': (3, 3, 14, 2, 0), 'cntp_ctl_el0': (3, 3, 14, 2, 1), 'cntp_cval_el0': (3, 3, 14, 2, 2),
    'cntv_tval_el0': (3, 3, 14, 3, 0), 'cntv_ctl_el0': (3, 3, 14, 3, 1), 'cntv_cval_el0': (3, 3, 14, 3, 2),
    'cntkctl_el1': (3, 0, 14, 1, 0), 'cntvoff_el2': (3, 4, 14, 0, 3),
    # process state
    'nzcv': (3, 3, 4, 2, 0), 'daif': (3, 3, 4, 2, 1), 'dit': (3, 3, 4, 2, 5), 'ssbs': (3, 3, 4, 2, 6),
    'tco': (3, 3, 4, 2, 7), 'spsel': (3, 0, 4, 2, 0), 'pan': (3, 0, 4, 2, 3), 'uao': (3, 0, 4, 2, 4),
    'fpcr': (3, 3, 4, 4, 0), 'fpsr': (3, 3, 4, 4, 1), 'allint': (3, 0, 4, 3, 0), 'svcr': (3, 3, 4, 2, 2),
    'currentel': (3, 0, 4, 2, 2),
    # random number
    'rndr': (3, 3, 2, 4, 0), 'rndrrs': (3, 3, 2, 4, 1),
    # identification
    'midr_el1': (3, 0, 0, 0, 0), 'mpidr_el1': (3, 0, 0, 0, 5), 'revidr_el1': (3, 0, 0, 0, 6),
    'ctr_el0': (3, 3, 0, 0, 1), 'dczid_el0': (3, 3, 0, 0, 7), 'aidr_el1': (3, 1, 0, 0, 7),
    'id_aa64pfr0_el1': (3, 0, 0, 4, 0), 'id_aa64pfr1_el1': (3, 0, 0, 4, 1),
    'id_aa64zfr0_el1': (3, 0, 0, 4, 4), 'id_aa64smfr0_el1': (3, 0, 0, 4, 5),
    'id_aa64dfr0_el1': (3, 0, 0, 5, 0), 'id_aa64dfr1_el1': (3, 0, 0, 5, 1),
    'id_aa64isar0_el1': (3, 0, 0, 6, 0), 'id_aa64isar1_el1': (3, 0, 0, 6, 1), 'id_aa64isar2_el1': (3, 0, 0, 6, 2),
    'id_aa64mmfr0_el1': (3, 0, 0, 7, 0), 'id_aa64mmfr1_el1': (3, 0, 0, 7, 1), 'id_aa64mmfr2_el1': (3, 0, 0, 7, 2),
    # exception handling
    'elr_el1': (3, 0, 4, 0, 1), 'elr_el2': (3, 4, 4, 0, 1), 'elr_el3': (3, 6, 4, 0, 1),
    'spsr_el1': (3, 0, 4, 0, 0), 'spsr_el2': (3, 4, 4, 0, 0), 'spsr_el3': (3, 6, 4, 0, 0),
    'sp_el0': (3, 0, 4, 1, 0), 'sp_el1': (3, 4, 4, 1, 0), 'sp_el2': (3, 6, 4, 1, 0),
    'esr_el1': (3, 0, 5, 2, 0), 'esr_el2': (3, 4, 5, 2, 0), 'esr_el3': (3, 6, 5, 2, 0),
    'far_el1': (3, 0, 6, 0, 0), 'far_el2': (3, 4, 6, 0, 0), 'far_el3': (3, 6, 6, 0, 0),
    'vbar_el1': (3, 0, 12, 0, 0), 'vbar_el2': (3, 4, 12, 0, 0), 'vbar_el3': (3, 6, 12, 0, 0),
    'isr_el1': (3, 0, 12, 1, 0),
    # system control and translation
    'sctlr_el1': (3, 0, 1, 0, 0), 'sctlr_el2': (3, 4, 1, 0, 0), 'sctlr_el3': (3, 6, 1, 0, 0),
    'actlr_el1': (3, 0, 1, 0, 1), 'cpacr_el1': (3, 0, 1, 0, 2), 'cptr_el2': (3, 4, 1, 1, 2),
    'cptr_el3': (3, 6, 1, 1, 2), 'scr_el3': (3, 6, 1, 1, 0), 'hcr_el2': (3, 4, 1, 1, 0),
    'ttbr0_el1': (3, 0, 2, 0, 0), 'ttbr1_el1': (3, 0, 2, 0, 1), 'tcr_el1': (3, 0, 2, 0, 2),
    'ttbr0_el2': (3, 4, 2, 0, 0), 'tcr_el2': (3, 4, 2, 0, 2), 'ttbr0_el3': (3, 6, 2, 0, 0),
    'mair_el1': (3, 0, 10, 2, 0), 'mair_el2': (3, 4, 10, 2, 0), 'mair_el3': (3, 6, 10, 2, 0),
    'amair_el1': (3, 0, 10, 3, 0), 'contextidr_el1': (3, 0, 13, 0, 1), 'vttbr_el2': (3, 4, 2, 1, 0),
    'par_el1': (3, 0, 7, 4, 0), 'mdscr_el1': (2, 0, 0, 2, 2), 'oslar_el1': (2, 0, 1, 0, 4),
    'osdlr_el1': (2, 0, 1, 3, 4), 'mdccint_el1': (2, 0, 0, 2, 0),
    # pointer authentication keys
    'apiakeylo_el1': (3, 0, 2, 1, 0), 'apiakeyhi_el1': (3, 0, 2, 1, 1),
    'apibkeylo_el1': (3, 0, 2, 1, 2), 'apibkeyhi_el1': (3, 0, 2, 1, 3),
    'apdakeylo_el1': (3, 0, 2, 2, 0), 'apdakeyhi_el1': (3, 0, 2, 2, 1),
    'apdbkeylo_el1': (3, 0, 2, 2, 2), 'apdbkeyhi_el1': (3, 0, 2, 2, 3),
    'apgakeylo_el1': (3, 0, 2, 3, 0), 'apgakeyhi_el1': (3, 0, 2, 3, 1),
    # SVE/SME control
    'zcr_el1': (3, 0, 1, 2, 0), 'zcr_el2': (3, 4, 1, 2, 0), 'zcr_el3': (3, 6, 1, 2, 0),
    'smcr_el1': (3, 0, 1, 2, 6), 'smcr_el2': (3, 4, 1, 2, 6), 'smcr_el3': (3, 6, 1, 2, 6),
    'smpri_el1': (3, 0, 1, 2, 4), 'tpidr_el3': (3, 6, 13, 0, 2),
    # performance monitors (common ones)
    'pmcr_el0': (3, 3, 9, 12, 0), 'pmccntr_el0': (3, 3, 9, 13, 0), 'pmccfiltr_el0': (3, 3, 14, 15, 7),
    'pmcntenset_el0': (3, 3, 9, 12, 1), 'pmcntenclr_el0': (3, 3, 9, 12, 2), 'pmuserenr_el0': (3, 3, 9, 14, 0),
    'pmselr_el0': (3, 3, 9, 12, 5), 'pmxevcntr_el0': (3, 3, 9, 13, 2), 'pmxevtyper_el0': (3, 3, 9, 13, 1),
    'pmovsclr_el0': (3, 3, 9, 12, 3), 'pmovsset_el0': (3, 3, 9, 14, 3), 'pmswinc_el0': (3, 3, 9, 12, 4),
}


def enc(op0, op1, crn, crm, op2):
    return (((op0 - 2) << 14) | (op1 << 11) | (crn << 7) | (crm << 3) | op2)


names = sorted(REGS)
print('''// =============================================================================
// asm/a64sysreg.ax - the AArch64 assembler's named system registers for MRS
// and MSR, with the 15-bit encoding (op0 op1 CRn CRm op2) that MRS/MSR hold
// from bit 5.  Generated from the Arm Architecture Reference Manual's System
// Register Encoding tables by cmd/internal/tools/gen_a64_sysregs.py: do not
// edit.  A register not listed here is still reachable by the generic
// S<op0>_<op1>_C<n>_C<m>_<op2> form.
// =============================================================================''')
print('pub let NAMES: [%d][]u8 = [' % len(names))
for n in names:
    print('    "%s",' % n)
print('];')
print('pub let ENC: [%d]u64 = [' % len(names))
vals = ['0x%04x' % enc(*REGS[n]) for n in names]
for i in range(0, len(vals), 10):
    print('    ' + ', '.join(vals[i:i + 10]) + ',')
print('];')
