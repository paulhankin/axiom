/* flags: --c-std=gnu17 */
/* native-only */
/* AArch64 assembler: the rest of the base and system instructions (pointer authentication, MTE, RCpc, LSE128, read-check-write, memory copy and set, CSSC, the SYS aliases, flag manipulation, D128, GCS, TME, LS64), against the system assembler's bytes */
#include "host.h"

#ifdef __AXIOM_ASM_AARCH64__
__asm__(".section .rodata.axiom_asm_table, \"a\"\n"
        ".globl axiom_asm_table\n"
        "axiom_asm_table:\n"
        "casp x0, x1, x2, x3, [x4]\n"
        "caspal w0, w1, w2, w3, [sp]\n"
        "bfc x0, #3, #4\n"
        "bfi w0, w1, #3, #4\n"
        "sxtb x0, w1\n"
        "uxth w0, w1\n"
        "sxtw x0, w1\n"
        "mneg x0, x1, x2\n"
        "smnegl x0, w1, w2\n"
        "umulh x0, x1, x2\n"
        "cneg x0, x1, eq\n"
        "csetm w0, ne\n"
        "ngcs x0, x1\n"
        "ldlarb w0, [x1]\n"
        "stllrh w0, [x1]\n"
        "ldaddalb w0, w1, [x2]\n"
        "stumaxl x0, [x1]\n"
        "ldaprh w0, [x1]\n"
        "mov x0, sp\n"
        "mov sp, x1\n"
        "mov w0, #0x10000\n"
        "movz x0, #1, lsl #48\n"
        "mov x0, #-1\n"
        "bti jc\n"
        "hint #34\n"
        "yield\n"
        "dmb ishld\n"
        "isb sy\n"
        "msr daifset, #2\n"
        "mrs x0, nzcv\n"
        "msr s3_3_c4_c2_0, x1\n"
        "ret x1\n"
        "br x16\n"
        "blr x8\n"
        "ldp q0, q1, [sp, #-32]!\n"
        "ldnp d0, d1, [x0]\n"
        "stnp x0, x1, [x2, #8]\n"
        "ldpsw x0, x1, [x2], #8\n"
        "ldxp x0, x1, [x2]\n"
        "stlxp w3, x0, x1, [x2]\n"
        "prfum pldl2strm, [x1, #-1]\n"
        "prfm pstl3keep, [x1, x2, lsl #3]\n"
        "ldrsw x0, [x1, w2, sxtw #2]\n"
        "ldr x0, [x1, x2, sxtx]\n"
        "ldr b0, [x1, x2, lsl #0]\n"
        "str q0, [x1, w2, uxtw #4]\n"
        "fmov d0, #-1.25\n"
        "fmov v0.2d, #2.0\n"
        "fmov v0.d[1], x1\n"
        "fmov x1, v0.d[1]\n"
        "fcmp h0, #0.0\n"
        "fccmpe s0, s1, #4, lt\n"
        "frint32z d0, d1\n"
        "frint64x s0, s1\n"
        "fjcvtzs w0, d1\n"
        "fcvt h0, d1\n"
        "fmadd h0, h1, h2, h3\n"
        "ldadd x0, x1, [x2]\n"
        "swpal x0, x1, [sp]\n"
        "cas w0, w1, [x2]\n"
        "ldsmaxa w0, w1, [x2]\n"
        "stadd w0, [x1]\n"
        "sev\n"
        "wfe\n"
        "mrs x0, rndrrs\n"
        "ld1 {v0.16b-v3.16b}, [x0], #64\n"
        "st4 {v0.s, v1.s, v2.s, v3.s}[3], [x0], x2\n"
        "ld1r {v0.8h}, [x0]\n"
        "tbl v0.16b, {v1.16b, v2.16b}, v3.16b\n"
        "dup v0.4s, w1\n"
        "umov x0, v1.d[1]\n"
        "smov x0, v1.b[3]\n"
        "ins v0.d[1], v1.d[0]\n"
        "movi v0.2d, #0xff00ff00ff00ff00\n"
        "mvni v0.4s, #1, msl #8\n"
        "bic v0.4s, #1, lsl #8\n"
        "orr v0.8h, #0x10\n"
        "uzp1 v0.4s, v1.4s, v2.4s\n"
        "sdot v0.4s, v1.16b, v2.4b[3]\n"
        "bfmmla v0.4s, v1.8h, v2.8h\n"
        "fcmla v0.4s, v1.4s, v2.s[1], #90\n"
        "addp d0, v1.2d\n"
        "fmaxnmp h0, v1.2h\n"
        "ldar w0, [x1, #0]\n"
        "hvc #1\n"
        "smc #2\n"
        "dcps1\n"
        "dcps2 #3\n"
        "dcps3\n"
        "bfm x0, x1, #3, #4\n"
        "sbfm w0, w1, #3, #4\n"
        "ubfm x0, x1, #63, #0\n"
        "asrv x0, x1, x2\n"
        "lslv w0, w1, w2\n"
        "umaddl x0, w1, w2, x3\n"
        "smsubl x0, w1, w2, x3\n"
        "adds x0, sp, w1, uxtw #2\n"
        "cmp sp, #4\n"
        "cmn w0, w1, sxtb\n"
        "bfxil x0, x1, #2, #3\n"
        "ubfiz w0, w1, #2, #3\n"
        "sbfx x0, x1, #2, #3\n"
        "tst w0, #0xff\n"
        "eon x0, x1, x2, ror #3\n"
        "orn w0, w1, w2\n"
        "ldxrb w0, [x1]\n"
        "stxrh w2, w0, [x1]\n"
        "mvn x0, x1, lsl #3\n"
        "neg w0, w1, asr #2\n"
        "negs x0, x1\n"
        "ngc w0, w1\n"
        "sbcs x0, x1, x2\n"
        "cinc w0, w1, eq\n"
        "cinv x0, x1, ne\n"
        "csinc w0, w1, w2, ge\n"
        "csneg x0, x1, x2, lt\n"
        "ccmn x0, #3, #4, eq\n"
        "rev16 x0, x1\n"
        "rev32 x0, x1\n"
        "rev64 x0, x1\n"
        "cls w0, w1\n"
        "smulh x0, x1, x2\n"
        "umull x0, w1, w2\n"
        "smull x0, w1, w2\n"
        "mul w0, w1, w2\n"
        "udiv w0, w1, w2\n"
        "crc32cx w0, w1, x2\n"
        "msr spsel, #1\n"
        "msr pan, #1\n"
        "mrs x0, tpidr_el0\n"
        "mrs x0, s3_0_c15_c2_0\n"
        "bti c\n"
        "stlrb w0, [x1]\n"
        "ldaprb w0, [x1, #0]\n"
        "ldnp q0, q1, [x0, #-1024]\n"
        "stp w0, w1, [sp, #-8]!\n"
        "movk w0, #3, lsl #16\n"
        "sevl\n"
        "wfi\n"
        "esb\n"
        "csdb\n"
        "caspa w22, w23, w30, wzr, [x3]\n"
        "casp w10, w11, w2, w3, [x3]\n"
        "caspa x6, x7, x2, x3, [sp]\n"
        "caspl x14, x15, x20, x21, [x3]\n"
        "caspl w20, w21, w6, w7, [sp]\n"
        "casp x4, x5, x26, x27, [x3]\n"
        "caspl x26, x27, x6, x7, [x3]\n"
        "caspl x20, x21, x0, x1, [x1, #0]\n"
        "casp x8, x9, x2, x3, [x1, #0]\n"
        "caspal x22, x23, x16, x17, [x3]\n"
        "caspa x14, x15, x16, x17, [sp]\n"
        "caspal w4, w5, w20, w21, [sp]\n"
        "caspal x14, x15, x6, x7, [sp]\n"
        "caspa w28, w29, w18, w19, [sp]\n"
        "fcvtzs x25, d1, #58\n"
        "fcvtzu w12, d6, #30\n"
        "ucvtf d28, w4, #15\n"
        "ucvtf h13, w27, #26\n"
        "ucvtf s12, w9, #25\n"
        "ucvtf s19, w30, #1\n"
        "fcvtzs x5, h16, #7\n"
        "fcvtzs x26, d0, #42\n"
        "scvtf s30, x11, #39\n"
        "scvtf s0, x11, #56\n"
        "scvtf h7, w19, #30\n"
        "fcvtzu x16, s16, #58\n"
        "scvtf s2, x27, #54\n"
        "fcvtzs x29, d5, #38\n"
        "fcvtzu x4, h2, #62\n"
        "fcvtzu x22, d20, #60\n"
        "cpyfp [x0]!, [x1]!, x2!\n"
        "cpyfe [x0]!, [x1]!, x2!\n"
        "cpyp [x0]!, [x1]!, x2!\n"
        "setm [x0]!, x1!, x2\n"
        "sete [x0]!, x1!, x2\n"
        "setpt [x0]!, x1!, x2\n"
        "ldtrsw x0, [sp]\n"
        "stg x0, [x1]\n"
        "stg sp, [x1], #-4096\n"
        "stzg x0, [x1, #4080]\n"
        "st2g x0, [x1, #16]!\n"
        "stz2g x0, [x1]\n"
        "ldg x0, [x1, #32]\n"
        "stgp x0, x1, [x2, #-1024]!\n"
        "stgp x0, x1, [x2], #1008\n"
        "stgp x0, x1, [x2]\n"
        "ldgm x0, [x1]\n"
        "stgm x0, [x1]\n"
        "stzgm x0, [x1]\n"
        "addg x0, sp, #1008, #15\n"
        "subg sp, x1, #16, #1\n"
        "gmi x0, x1, x2\n"
        "subp x0, sp, x2\n"
        "subps x0, x1, sp\n"
        "cmpp x1, x2\n"
        "irg x0, x1\n"
        "ldapurb w0, [x1]\n"
        "ldapursb x0, [x1, #255]\n"
        "ldapursh w0, [x1]\n"
        "ldapursw x0, [x1]\n"
        "stlurh w0, [x1, #-256]\n"
        "stlur x0, [x1]\n"
        "stlur w0, [x1]\n"
        "ldapur w0, [x1]\n"
        "ldapur b0, [x1, #3]\n"
        "ldapur q0, [x1, #3]\n"
        "stlur d0, [x1]\n"
        "ldap1 {v0.d}[1], [x1]\n"
        "stl1 {v0.d}[0], [x1]\n"
        "ldiapp w0, w1, [x2]\n"
        "ldiapp x0, x1, [x2]\n"
        "stilp x0, x1, [x2, #-16]!\n"
        "stilp x0, x1, [x2]\n"
        "stlr x0, [x1, #-8]!\n"
        "stlr w0, [x1, #-4]!\n"
        "ldapr w0, [x1], #4\n"
        "ldraa x0, [x1]\n"
        "ldrab x0, [x1, #4088]\n"
        "ldrab x0, [sp, #8]!\n"
        "rcwcas x0, x1, [x2]\n"
        "rcwcasa x0, x1, [x2]\n"
        "rcwcasal x0, x1, [x2]\n"
        "rcwcasl x0, x1, [x2]\n"
        "rcwscas x0, x1, [sp]\n"
        "rcwcasp x0, x1, x2, x3, [x4]\n"
        "rcwscaspal x0, x1, x2, x3, [x4]\n"
        "rcwclr x0, x1, [x2]\n"
        "rcwclra x0, x1, [x2]\n"
        "rcwsclr x0, x1, [x2]\n"
        "rcwset x0, x1, [x2]\n"
        "rcwsset x0, x1, [x2]\n"
        "rcwswp x0, x1, [x2]\n"
        "rcwsswp x0, x1, [x2]\n"
        "rcwclrp x0, x1, [x2]\n"
        "rcwsclrp x0, x1, [x2]\n"
        "rcwsetp x0, x1, [x2]\n"
        "rcwssetp x0, x1, [x2]\n"
        "rcwswpp x0, x1, [x2]\n"
        "rcwsswppal x0, x1, [x2]\n"
        "ldclrp x0, x1, [x2]\n"
        "ldclrpa x0, x1, [x2]\n"
        "ldclrpal x0, x1, [x2]\n"
        "ldclrpl x0, x1, [x2]\n"
        "ldsetp x0, x1, [x2]\n"
        "swpp x0, x1, [x2]\n"
        "swppal x0, x1, [sp]\n"
        "abs w0, w1\n"
        "abs x0, x1\n"
        "cnt x0, x1\n"
        "ctz w0, w1\n"
        "smax w0, w1, w2\n"
        "smax x0, x1, #-128\n"
        "umax x0, x1, #255\n"
        "smin w0, w1, #127\n"
        "umin x0, x1, x2\n"
        "gcspushm x0\n"
        "gcspopm x0\n"
        "gcspopm\n"
        "gcsss1 x0\n"
        "gcsss2 x0\n"
        "gcspushx\n"
        "gcspopcx\n"
        "gcspopx\n"
        "gcsstr x0, [x1]\n"
        "gcssttr x0, [sp]\n"
        "gcsb dsync\n"
        "tstart x0\n"
        "tcommit\n"
        "tcancel #0xffff\n"
        "ttest x0\n"
        "ld64b x0, [x1]\n"
        "st64b x0, [x1]\n"
        "st64bv x2, x0, [x1]\n"
        "st64bv0 x2, x0, [x1]\n"
        "chkfeat x16\n"
        "trcit x3\n"
        "clrbhb\n"
        "sb\n"
        "ssbb\n"
        "pssbb\n"
        "dgh\n"
        "psb csync\n"
        "tsb csync\n"
        "pacia1716\n"
        "pacib1716\n"
        "autia1716\n"
        "autib1716\n"
        "paciaz\n"
        "pacibz\n"
        "autiaz\n"
        "autibz\n"
        "xpaclri\n"
        "bti\n"
        "setf8 w1\n"
        "setf16 w2\n"
        "cfinv\n"
        "xaflag\n"
        "axflag\n"
        "rmif x1, #63, #15\n"
        "wfet x3\n"
        "wfit x4\n"
        "xpaci x1\n"
        "xpacd x2\n"
        "pacdza x3\n"
        "autdzb x4\n"
        "pacda x1, sp\n"
        "retab\n"
        "braaz x1\n"
        "brabz x1\n"
        "blraaz x1\n"
        "blrabz x1\n"
        "blraa x1, sp\n"
        "blrab x1, x2\n"
        "brab x1, x2\n"
        "eretaa\n"
        "eretab\n"
        "eret\n"
        "drps\n"
        "ldaprb w0, [x1]\n"
        "stllr x0, [x1]\n"
        "pacia x1, x2\n"
        "paciza x3\n"
        "pacga x1, x2, sp\n"
        "retaa\n"
        "braa x1, x2\n"
        "ldraa x0, [x1, #-4096]!\n"
        "irg x0, sp, x3\n"
        "stg x0, [x1, #16]!\n"
        "tlbi rvae1, x3\n"
        "dc cvadp, x1\n"
        "wfet x1\n"
        "rmif x1, #3, #2\n"
        "ldiapp x0, x1, [x2], #16\n"
        "stilp w0, w1, [x2, #-8]!\n"
        "ldapr x0, [x1], #8\n"
        "ldapur x0, [x1, #-3]\n"
        "sysp #0, c8, c0, #0, x0, x1\n"
        "tlbip vae1, x0, x1\n"
        "trcit x0\n"
        "at s12e0r, x3\n"
        "at s12e0w, x3\n"
        "at s12e1r, x3\n"
        "at s12e1w, x3\n"
        "cfp rctx, x3\n"
        "cosp rctx, x3\n"
        "cpp rctx, x3\n"
        "dc cgdsw, x3\n"
        "dc cgdvac, x3\n"
        "dc cgdvadp, x3\n"
        "dvp rctx, x3\n"
        "ic iallu\n"
        "ic ialluis\n"
        "ic ivau, x3\n"
        "tlbi alle1\n"
        "tlbi alle1is\n"
        "tlbi alle1isnxs\n"
        "tlbip ipas2e1, x4, x5\n"
        "tlbip ipas2e1is, x4, x5\n"
        "tlbip ipas2e1isnxs, x4, x5\n"
        "rcwsclrpa x24, x19, [sp]\n"
        "cpyfpwtwn [x21]!, [x1]!, x15!\n"
        "sysl xzr, #2, c11, c10, #5\n"
        "cpypwtn [x3]!, [x5]!, x2!\n"
        "pacib x4, x24\n"
        "rcwsswpal xzr, x18, [x24]\n"
        "cpypwtwn [x27]!, [x30]!, x18!\n"
        "rcwclra x24, x27, [sp]\n"
        "cpypwtn [x12]!, [x4]!, x9!\n"
        "swppal x26, x2, [x21]\n"
        "tcancel #58726\n"
        "wfet x14\n"
        "rcwclrl x2, x21, [x7]\n"
        "rcwsclr x14, x12, [sp]\n"
        "blrabz x5\n"
        "subg sp, x7, #1008, #12\n"
        "rcwcaspal x2, x3, x16, x17, [sp]\n"
        "rcwssetal x3, x22, [x10]\n"
        "gcsss1 x22\n"
        "rcwsclrpa xzr, x8, [sp]\n"
        "braa x19, x22\n"
        "rcwsclrl x16, x6, [sp]\n"
        "stl1 {v29.d}[1], [x28]\n"
        "rcwsclrpa xzr, x24, [x1]\n"
        "sys #7, c15, c10, #2, x4\n"
        "cpyp [x6]!, [x16]!, x1!\n"
        "pacib x29, x19\n"
        "wfit x23\n"
        "rcwscasa xzr, x9, [x24]\n"
        "rcwswpl xzr, x6, [x26]\n"
        "braa x27, x1\n"
        "addg sp, x18, #400, #15\n"
        "rcwsswppa x21, x11, [x10]\n"
        "rcwsetl xzr, x27, [x3]\n"
        "rcwsset x25, x27, [x28]\n"
        "swppal x9, x16, [sp]\n"
        "rcwseta xzr, x17, [x13]\n"
        "stlr x28, [x18, #-8]!\n"
        "cpyp [x19]!, [x18]!, x3!\n"
        "pacga x25, x18, x28\n"
        "rcwscaspl x2, x3, x20, x21, [sp]\n"
        "brabz x20\n"
        "pacdza x27\n"
        "setpt [x12]!, x26!, xzr\n"
        "ldclrpal x22, x27, [sp]\n"
        "rcwsclrl x24, xzr, [x28]\n"
        "cpyfpwtn [x5]!, [x23]!, x19!\n"
        "rcwclrpal x19, x14, [x23]\n"
        "cpyprt [x9]!, [x2]!, x29!\n"
        "ld64b x16, [sp]\n"
        "rcwsswppa x17, x28, [x6]\n"
        "cpyfprtn [x3]!, [x30]!, x23!\n"
        "tstart x20\n"
        "rcwclra xzr, x5, [x22]\n"
        "rcwclra xzr, x23, [x21]\n"
        "cpyfpwtwn [x3]!, [x10]!, x20!\n"
        "cpyfprtn [x4]!, [x5]!, x29!\n"
        "ldgm x3, [sp]\n"
        "gcsstr x28, [sp]\n"
        "st64bv0 x7, x8, [sp]\n"
        "rcwsclrp x28, x28, [x11]\n"
        "rcwsetal xzr, x2, [x3]\n"
        "rcwscaspal x6, x7, x16, x17, [x8]\n"
        "xpaci x8\n"
        "rcwsswppal xzr, xzr, [x14]\n"
        "ldg xzr, [sp, #-3520]\n"
        "swppal x16, x8, [x22]\n"
        "rcwsswp x25, xzr, [x27]\n"
        "ttest xzr\n"
        "rcwsswpl x8, x5, [x26]\n"
        "cpyprn [x15]!, [x29]!, x20!\n"
        "stz2g x23, [x1], #-1552\n"
        "rcwset x22, x22, [sp]\n"
        "cpypwt [x23]!, [x19]!, x15!\n"
        "rcwsswppa x22, x2, [sp]\n"
        "dvp rctx, xzr\n"
        "pacia x2, x25\n"
        "ldrab x15, [x17, #-8]\n"
        "xpaci xzr\n"
        "cpypt [x2]!, [x30]!, x25!\n"
        "braaz x29\n"
        "rcwsclrpal xzr, x20, [x29]\n"
        "cpyfpwtrn [x18]!, [x12]!, x27!\n"
        "gcspopm xzr\n"
        "rcwcaspa x0, x1, x10, x11, [sp]\n"
        "rcwssetpa xzr, x16, [x10]\n"
        "rcwsetpl x18, x12, [x26]\n"
        "rcwswpp x11, x22, [x2]\n"
        "ldapursb xzr, [x25, #28]\n"
        "rcwsset xzr, xzr, [sp]\n"
        "rcwswpl xzr, xzr, [x0]\n"
        "rcwcaspa x20, x21, x14, x15, [x19]\n"
        "rcwscasal x5, x3, [x16]\n"
        "cpypn [x11]!, [x30]!, x10!\n"
        "cpyfprtn [x17]!, [x2]!, x9!\n"
        "rcwsetpa x29, x3, [x2]\n"
        "rcwsswpa x26, xzr, [sp]\n"
        "trcit xzr\n"
        "rcwcas x30, x4, [x30]\n"
        "ldiapp x0, x28, [x18]\n"
        "sttrb w20, [sp]\n"
        "cpyfpwt [x10]!, [x13]!, x25!\n"
        "sys #2, c11, c3, #5, xzr\n"
        "cpypn [x17]!, [x23]!, x26!\n"
        "cpyprtn [x17]!, [x20]!, x3!\n"
        "rcwseta x3, xzr, [x29]\n"
        "ctz xzr, xzr\n"
        "rcwsetl xzr, x25, [sp]\n"
        "autdza x7\n"
        "ic ivau, xzr\n"
        "cpyfprtwn [x4]!, [x26]!, x10!\n"
        "pacizb x23\n"
        "setgpn [x6]!, x23!, x18\n"
        "rcwsclrpl x0, x21, [x1]\n"
        "rcwsset x19, x20, [x8]\n"
        "rcwssetpl x3, xzr, [x23]\n"
        "gmi x8, x30, x10\n"
        "pacib x28, x16\n"
        "cpyfprtrn [x16]!, [x3]!, x9!\n"
        "stilp xzr, x11, [x30, #-16]!\n"
        "cpypt [x20]!, [x22]!, x30!\n"
        "autib xzr, x18\n"
        "rcwset x4, x3, [sp]\n"
        "blrab x26, x3\n"
        "rcwcasl x26, x27, [sp]\n"
        "stzgm xzr, [x16]\n"
        "cpyfpt [x18]!, [x29]!, x16!\n"
        "braaz x22\n"
        "setgp [x19]!, x30!, x17\n"
        "setgptn [x9]!, x4!, x1\n"
        "stlur s21, [sp, #-154]\n"
        "rcwssetpal xzr, x17, [sp]\n"
        "ldgm x1, [sp]\n"
        "rcwssetp x7, x30, [x9]\n"
        "gcsss1 xzr\n"
        "cpypwtwn [x12]!, [x29]!, x1!\n"
        "paciza x20\n"
        "rcwswpp x10, x4, [sp]\n"
        "rcwscasal xzr, x9, [x26]\n"
        "rcwsclrpal x17, x19, [x22]\n"
        "autda xzr, x1\n"
        "rcwssetal x15, x13, [sp]\n"
        "pacdza xzr\n"
        "rcwcasp x22, x23, x4, x5, [x18]\n"
        "cfp rctx, x30\n"
        "sttr x8, [sp, #187]\n"
        "rcwsswp x14, xzr, [sp]\n"
        "pacga x26, x1, sp\n"
        "ctz w3, w21\n"
        "ctz wzr, w25\n"
        "rcwsetpa xzr, x19, [x7]\n"
        "gmi x10, x16, x7\n"
        "rcwseta xzr, x30, [x11]\n"
        "rcwscasa x20, xzr, [sp]\n"
        "ldtrh wzr, [x24]\n"
        "ldtrsw x7, [sp, #175]\n"
        "cpyfp [x5]!, [x18]!, x14!\n"
        "rcwswppa x20, x12, [x29]\n"
        "umax x13, x0, x6\n"
        "cpyfpwtn [x4]!, [x0]!, x21!\n"
        "rcwsswppa x10, xzr, [x29]\n"
        "cpyprtwn [x25]!, [x16]!, x27!\n"
        "rcwseta x29, x4, [x7]\n"
        "cpypwn [x6]!, [x4]!, x1!\n"
        "addg x13, x20, #976, #12\n"
        "cpyfpwtn [x0]!, [x21]!, x24!\n"
        "blrabz x17\n"
        "umin xzr, x4, x17\n"
        "autdb x2, x6\n"
        "pacib x14, x11\n"
        "rcwswpp x7, x23, [x21]\n"
        "rcwsclr x21, x19, [sp]\n"
        "rcwsclrpa xzr, xzr, [x18]\n"
        "autdzb xzr\n"
        "dsb #28\n"
        "rcwscasl xzr, x12, [sp]\n"
        "cpyptwn [x8]!, [x18]!, x9!\n"
        "ldtrsh x1, [x25, #139]\n"
        "cpypwtrn [x17]!, [x13]!, x8!\n"
        "ldapursb xzr, [x19]\n"
        "rcwsetp x3, x23, [sp]\n"
        "ldtrsh w9, [sp]\n"
        "pacga x15, x29, x20\n"
        "cpyp [x14]!, [x28]!, x26!\n"
        "pacizb xzr\n"
        "gmi xzr, sp, x1\n"
        "rcwswppa x20, xzr, [sp]\n"
        "cpyprt [x2]!, [x17]!, x29!\n"
        "stzg sp, [x10, #1248]!\n"
        "braaz x14\n"
        "smin x8, xzr, x10\n"
        "gcspushm xzr\n"
        "cpyfprt [x15]!, [x23]!, x28!\n"
        "tstart x26\n"
        "cpypwtwn [x14]!, [x20]!, x25!\n"
        "rcwcaspal x12, x13, x26, x27, [x7]\n"
        "rcwswppa x30, x7, [x29]\n"
        "ldsetp x4, x22, [sp]\n"
        "rcwsclr x4, x20, [sp]\n"
        "autib x13, x3\n"
        "rcwssetp x26, xzr, [x26]\n"
        "rcwclr x13, xzr, [sp]\n"
        "swpp x30, x29, [sp]\n"
        "rcwset x9, xzr, [x22]\n"
        "setpt [x16]!, x8!, x14\n"
        "blraa x11, sp\n"
        "pacdzb x1\n"
        "rcwsetp x11, x23, [x29]\n"
        "cpypwt [x30]!, [x6]!, x20!\n"
        "rcwsswpal x24, xzr, [sp]\n"
        "rcwclrpl xzr, xzr, [sp]\n"
        "cpyfpwn [x1]!, [x8]!, x24!\n"
        "rcwssetp x3, x25, [x16]\n"
        "ldgm x14, [x11]\n"
        "ttest x22\n"
        "xpaci x0\n"
        "setpn [x13]!, x18!, xzr\n"
        "cosp rctx, xzr\n"
        "ldsetpa x16, x24, [x24]\n"
        "cpyfpwn [x25]!, [x15]!, x2!\n"
        "rcwcaspl x28, x29, x24, x25, [sp]\n"
        "subg x28, x7, #864, #13\n"
        "rcwscasa x10, x15, [x24]\n"
        "autiza xzr\n"
        "stzgm x28, [x14]\n"
        "cpyfprtwn [x28]!, [x15]!, x14!\n"
        "sys #3, c1, c10, #0\n"
        "rcwsswppl xzr, xzr, [sp]\n"
        "cfp rctx, x26\n"
        "rcwsswppl x12, x13, [x0]\n"
        "rcwsswp x0, x14, [x23]\n"
        "rcwssetal x10, xzr, [x28]\n"
        "rcwscas xzr, x1, [x23]\n"
        "rcwcaspal x14, x15, x10, x11, [x3]\n"
        "autiza x29\n"
        "cpypwtwn [x5]!, [x14]!, x3!\n"
        "rcwsswpp xzr, x21, [x13]\n"
        "ldsetpl x8, x27, [sp]\n"
        "gcsss1 x21\n"
        "ldapursw xzr, [sp]\n"
        "cpyfprtwn [x18]!, [x0]!, x15!\n"
        "cpypwtn [x3]!, [x8]!, x28!\n"
        "rcwsetpl x2, xzr, [x23]\n"
        "paciza x28\n"
        "autda x30, x28\n"
        "rcwsswpal x26, xzr, [x21]\n"
        "rcwscaspal x6, x7, x8, x9, [sp]\n"
        "swppl x23, x3, [x10]\n"
        "rcwsetl xzr, xzr, [x12]\n"
        "ldraa x6, [x25, #-408]\n"
        "rcwswppal x21, x23, [sp]\n"
        "stg sp, [sp], #2816\n"
        "umax xzr, xzr, xzr\n"
        "pacda xzr, x2\n"
        "rcwsetal x11, x19, [x21]\n"
        "brab x14, sp\n"
        "ldtrb wzr, [x3]\n"
        "umin xzr, xzr, #253\n"
        "wfet xzr\n"
        "cpyptrn [x9]!, [x17]!, x7!\n"
        "rcwsswpp xzr, x6, [x12]\n"
        "umin w4, w29, wzr\n"
        "rcwsclral x16, xzr, [sp]\n"
        "tcancel #38643\n"
        "st2g x14, [sp], #2128\n"
        "cpyprn [x17]!, [x15]!, x0!\n"
        "rcwssetpl xzr, x21, [x13]\n"
        "ldapurh w16, [x28]\n"
        "ldraa x25, [sp, #3544]\n"
        "cpyfprtn [x11]!, [x5]!, x18!\n"
        "smax w17, wzr, wzr\n"
        "cpyptwn [x16]!, [x20]!, x3!\n"
        "setf8 wzr\n"
        "cpyprtn [x13]!, [x29]!, x28!\n"
        "autdzb x10\n"
        "setf16 wzr\n"
        "pacdb xzr, x25\n"
        "rcwsclra x30, x13, [x29]\n"
        "stl1 {v25.d}[0], [x27]\n"
        "rcwsclra x8, x11, [sp]\n"
        "rcwswppa x30, x9, [sp]\n"
        "stgp x18, x15, [x7, #736]!\n"
        "rcwclr x21, x21, [x14]\n"
        "rcwsclra xzr, xzr, [x18]\n"
        "ldclrpal x27, x6, [x25]\n"
        "rcwclrpal x17, x12, [x8]\n"
        "subps x24, x28, x6\n"
        "ldapr w22, [x4], #4\n"
        "cpyfprtwn [x25]!, [x19]!, x23!\n"
        "xpacd x6\n"
        "subg x13, x17, #16, #6\n"
        "autdza x15\n"
        "setf16 w27\n"
        "cpyprt [x30]!, [x10]!, x19!\n"
        "setpt [x15]!, x16!, x29\n"
        "stzg sp, [x21, #3376]\n"
        "ldclrp x23, x16, [x30]\n"
        "pacda xzr, x12\n"
        "pacdzb x9\n"
        "cpyfpt [x9]!, [x30]!, x21!\n"
        "autdzb x13\n"
        "rcwscas xzr, x8, [sp]\n"
        "setgp [x30]!, x12!, x25\n"
        "pacdb x16, x29\n"
        "xpacd xzr\n"
        "ldtr x20, [x19]\n"
        "setf8 w11\n"
        "cpyptwn [x17]!, [x10]!, x1!\n"
        "rcwsetl x8, x20, [sp]\n"
        "cpyfpwtrn [x6]!, [x12]!, x8!\n"
        "rcwsswpa x20, xzr, [x23]\n"
        "pacia x5, sp\n"
        "autia xzr, sp\n"
        "ldtrsb xzr, [x28]\n"
        "setpn [x13]!, x1!, x17\n"
        "ldsetpa x4, x8, [x26]\n"
        "cpyprtn [x8]!, [x24]!, x27!\n"
        "stlr x26, [sp, #-8]!\n"
        "ldrab x2, [x23]\n"
        "ldsetp x30, x15, [sp]\n"
        "setf8 w21\n"
        "pacdza x13\n"
        "abs xzr, x22\n"
        "gcssttr xzr, [sp]\n"
        "rcwclrpl x28, x5, [sp]\n"
        "cnt wzr, wzr\n"
        "cpyfprt [x30]!, [x9]!, x4!\n"
        "smax xzr, x24, #-108\n"
        "sysp #1, c8, c0, #0\n"
        "rcwssetpl xzr, xzr, [sp]\n"
        "brab xzr, x0\n"
        "cpyptrn [x1]!, [x28]!, x7!\n"
        "trcit x28\n"
        "ldapursh x16, [sp]\n"
        "pacdzb x8\n"
        "blrab x10, sp\n"
        "ldapurh w23, [x6, #193]\n"
        "rcwscaspl x0, x1, x26, x27, [x19]\n"
        "ldtrb w23, [x24, #80]\n"
        "rcwscasal xzr, x17, [x2]\n"
        "pacdb xzr, x23\n"
        "rcwssetpl x15, x9, [x4]\n"
        "xpacd x16\n"
        "cpyfpwtn [x4]!, [x14]!, x6!\n"
        "gcspushm x5\n"
        "rcwcaspa x28, x29, x8, x9, [x12]\n"
        "cmpp x15, x1\n"
        "umax wzr, w28, w8\n"
        "ldsetpal x19, x2, [x29]\n"
        "rcwswppl xzr, xzr, [sp]\n"
        "gcspushm x30\n"
        "setgptn [x3]!, x30!, x13\n"
        "paciza xzr\n"
        "cpyprtn [x0]!, [x28]!, x15!\n"
        "rcwscaspl x20, x21, x24, x25, [x20]\n"
        "rcwclrpa x7, xzr, [x11]\n"
        "rcwscasl x27, x21, [x25]\n"
        "ldtr x18, [sp]\n"
        "ldapurh wzr, [x28, #-174]\n"
        "autizb xzr\n"
        "cpypwtrn [x8]!, [x9]!, x1!\n"
        "ldsetpl x1, x11, [sp]\n"
        "smin xzr, x7, #-2\n"
        "rcwsswpl x0, x9, [x9]\n"
        "ldtrsw x13, [x2, #-174]\n"
        "rcwscaspal x6, x7, x12, x13, [sp]\n"
        "rcwssetl xzr, x10, [x21]\n"
        "cpyptrn [x23]!, [x12]!, x3!\n"
        "rcwssetl xzr, xzr, [x12]\n"
        "autib x11, sp\n"
        "setpn [x25]!, x22!, x8\n"
        "rcwswpal xzr, xzr, [x10]\n"
        "rcwswpa x12, x27, [x6]\n"
        "ldtrsh x20, [sp]\n"
        "rcwcaspal x12, x13, x12, x13, [x27]\n"
        "rcwclral x25, x22, [sp]\n"
        "rcwcasa x20, x9, [x7]\n"
        "brab xzr, x4\n"
        "cpyfptn [x0]!, [x5]!, x20!\n"
        "ldtrsh wzr, [sp]\n"
        "rcwsclrpal x15, x25, [sp]\n"
        "rcwswpa xzr, x27, [x21]\n"
        "rcwsclral xzr, x9, [x19]\n"
        "rcwsetpl x29, x26, [x15]\n"
        "gcssttr xzr, [x13]\n"
        "cpyfptwn [x28]!, [x13]!, x10!\n"
        "gcssttr xzr, [x16]\n"
        "st64bv0 x7, x10, [sp]\n"
        "setp [x2]!, x29!, x17\n"
        "rcwsclral xzr, xzr, [x30]\n"
        "ld64b x10, [x24]\n"
        "st64bv x18, x6, [sp]\n"
        "subps xzr, x24, x0\n"
        "ldclrpal x2, x10, [x4]\n"
        "ldapursh x17, [x6]\n"
        "cpyfpwt [x16]!, [x10]!, x26!\n"
        "cpyfptn [x17]!, [x9]!, x27!\n"
        "rcwsclrpl x24, x18, [x26]\n"
        "cpyfpwt [x24]!, [x6]!, x18!\n"
        "stlurh wzr, [sp, #58]\n"
        "cpyfpwtrn [x13]!, [x10]!, x27!\n"
        "abs w26, wzr\n"
        "rcwcasp x24, x25, x4, x5, [x19]\n"
        "rcwscaspa x4, x5, x8, x9, [x21]\n"
        "gcsss2 x8\n"
        "cpyfptwn [x25]!, [x0]!, x30!\n"
        "rcwswpa x30, x29, [x3]\n"
        "rcwssetl x28, x16, [sp]\n"
        "stzg x24, [sp, #-352]!\n"
        "cpp rctx, x30\n"
        "ldapr w5, [sp], #4\n"
        "rcwssetpal xzr, xzr, [sp]\n"
        "rcwsseta x27, xzr, [sp]\n"
        "cnt xzr, x10\n"
        "rcwssetpa x26, xzr, [x22]\n"
        "tcancel #17653\n"
        "gcspopm x26\n"
        "rcwsetpal x27, xzr, [x11]\n"
        "rcwsclra x8, xzr, [x26]\n"
        "subps x14, x27, x23\n"
        "setf16 w0\n"
        "gcsss2 xzr\n"
        "blrabz xzr\n"
        "setpn [x26]!, x22!, x13\n"
        "setgptn [x21]!, x14!, x25\n"
        "stlurb wzr, [x20, #20]\n"
        "pacdzb x25\n"
        "swppa x12, x7, [x19]\n"
        "cpyfpwt [x28]!, [x13]!, x17!\n"
        "ldtrsb xzr, [x25, #-116]\n"
        "rcwsseta xzr, x29, [x5]\n"
        "rcwcasp x4, x5, x26, x27, [sp]\n"
        "cpyfpwtrn [x8]!, [x26]!, x5!\n"
        "rcwsswpal x0, x8, [sp]\n"
        "pacdb x26, x14\n"
        "cpypwtn [x26]!, [x24]!, x25!\n"
        "cpyptrn [x10]!, [x29]!, x12!\n"
        "setptn [x29]!, x0!, x24\n"
        "ldsetpl x13, x12, [x8]\n"
        "rcwscasp x6, x7, x12, x13, [x25]\n"
        "rcwsclral x8, x26, [x16]\n"
        "stgm xzr, [x4]\n"
        "rcwscaspa x0, x1, x24, x25, [x8]\n"
        "rmif xzr, #46, #3\n"
        "cpp rctx, x17\n"
        "rcwssetpa x27, xzr, [sp]\n"
        "cpypwt [x28]!, [x2]!, x6!\n"
        "cpyfpwtwn [x29]!, [x12]!, x25!\n"
        "autia xzr, x3\n"
        "rcwclrpa xzr, xzr, [x29]\n"
        "rcwsclrp xzr, x9, [sp]\n"
        "cpyptn [x18]!, [x5]!, x14!\n"
        "rcwsetpal x6, xzr, [sp]\n"
        "setp [x22]!, x24!, x15\n"
        "dsb #3\n"
        "stz2g x11, [x0], #96\n"
        "rcwscasa x9, xzr, [sp]\n"
        "cpyfp [x25]!, [x14]!, x30!\n"
        "cpyfptn [x10]!, [x29]!, x16!\n"
        "cpyprt [x14]!, [x30]!, x1!\n"
        "rcwcasa x30, x18, [x23]\n"
        "blraaz x10\n"
        "cosp rctx, x27\n"
        "addg sp, x7, #944, #12\n"
        "sys #0, c11, c1, #7\n"
        "sttr w14, [x1, #-251]\n"
        "rcwscasal xzr, xzr, [sp]\n"
        "gcsstr xzr, [x13]\n"
        "cpyprtrn [x5]!, [x20]!, x21!\n"
        "rmif xzr, #1, #15\n"
        "cpyprtrn [x9]!, [x1]!, x6!\n"
        "ldsetp x9, x11, [x28]\n"
        "blraa x21, x27\n"
        "pacizb x0\n"
        "rcwclral x30, x29, [sp]\n"
        "brabz x2\n"
        "pacizb x7\n"
        "st2g x4, [x14, #1808]\n"
        "cpyfptn [x4]!, [x6]!, x10!\n"
        "autda xzr, x5\n"
        "rcwscas xzr, xzr, [x10]\n"
        "st64bv0 x26, x20, [x27]\n"
        "dvp rctx, x11\n"
        "rcwswpa x21, x21, [x15]\n"
        "stgm x6, [sp]\n"
        "cpypwn [x15]!, [x26]!, x13!\n"
        "cpyprtwn [x27]!, [x19]!, x26!\n"
        "tstart xzr\n"
        "setp [x11]!, x5!, xzr\n"
        "cpyfp [x20]!, [x28]!, x11!\n"
        "rcwclr xzr, x21, [x14]\n"
        "rcwssetpa x11, x12, [x25]\n"
        "sttr w4, [x0, #-48]\n"
        "autda xzr, x9\n"
        "rcwscaspa x14, x15, x12, x13, [x29]\n"
        "cpyfprt [x19]!, [x9]!, x15!\n"
        "cpyptn [x1]!, [x28]!, x20!\n"
        "st2g x11, [x8], #2304\n"
        "smin x23, x29, xzr\n"
        "cpypwtrn [x7]!, [x22]!, x1!\n"
        "cpypwt [x4]!, [x13]!, x27!\n"
        "rcwsswppl xzr, x16, [x5]\n"
        "rcwscasl x18, xzr, [sp]\n"
        "rcwsswppal xzr, x5, [x23]\n"
        "setgpt [x7]!, x10!, xzr\n"
        "rcwsclrl xzr, xzr, [x18]\n"
        "rcwsswpa xzr, xzr, [sp]\n"
        "sttrb wzr, [x10]\n"
        "cpyfprt [x0]!, [x29]!, x27!\n"
        "ldclrp x4, x16, [x16]\n"
        "setgpt [x12]!, x18!, xzr\n"
        "rcwswppal x10, x26, [x1]\n"
        "cpyfptwn [x4]!, [x26]!, x25!\n"
        "ldsetpa x22, x20, [x4]\n"
        "setgpt [x20]!, x0!, xzr\n"
        "pacda xzr, sp\n"
        "sttrb w19, [x29, #237]\n"
        "cpypwn [x4]!, [x17]!, x0!\n"
        "cpyptn [x30]!, [x28]!, x18!\n"
        "rcwsclrp x1, xzr, [x23]\n"
        "ldclrpa x10, x30, [x7]\n"
        "rcwsswpl x20, x1, [sp]\n"
        "blrab x10, x2\n"
        "stl1 {v11.d}[1], [sp]\n"
        "rcwclrpal xzr, xzr, [sp]\n"
        "rcwclral xzr, xzr, [sp]\n"
        "autia x22, x8\n"
        "pacia xzr, x17\n"
        "autdb xzr, x8\n"
        "ldtr wzr, [x30]\n"
        "cpyfpt [x14]!, [x28]!, x3!\n"
        "setptn [x1]!, x23!, x9\n"
        "cosp rctx, x10\n"
        "rcwsswpl x11, xzr, [sp]\n"
        "rcwcasal x29, x15, [sp]\n"
        "rcwssetl x19, x5, [x10]\n"
        "rcwsetpl x8, xzr, [x25]\n"
        "cnt x24, xzr\n"
        "cpyfprtrn [x15]!, [x5]!, x16!\n"
        "ldtrsb w12, [x29]\n"
        "sttr wzr, [x22]\n"
        "subp x5, x1, x15\n"
        "ldtrsb xzr, [x16]\n"
        "ldclrpa x7, x21, [x17]\n"
        "rcwcaspl x22, x23, x16, x17, [sp]\n"
        "rcwsswpa x18, xzr, [x24]\n"
        "ldtr x8, [x23, #28]\n"
        "rcwsetpa xzr, xzr, [x16]\n"
        "autia x9, sp\n"
        "cpyptwn [x24]!, [x14]!, x20!\n"
        "rcwswpl xzr, x25, [x18]\n"
        "cpyfpwtwn [x24]!, [x10]!, x26!\n"
        "irg sp, sp, x3\n"
        "dsb ishnxs\n"
        "rcwclrpl x23, x13, [x7]\n"
        "rcwclral xzr, x24, [sp]\n"
        "ldapurb wzr, [x8]\n"
        "ldap1 {v30.d}[1], [x22]\n"
        "setp [x14]!, x19!, x9\n"
        "rcwsswppal x8, xzr, [sp]\n"
        "autiza x21\n"
        "rcwscaspl x4, x5, x20, x21, [x30]\n"
        "cfp rctx, x1\n"
        "cpyfpn [x15]!, [x27]!, x19!\n"
        "cpypn [x26]!, [x17]!, x20!\n"
        "rcwscasp x14, x15, x22, x23, [x16]\n"
        "cpypn [x23]!, [x5]!, x27!\n"
        "setptn [x28]!, x17!, x2\n"
        "rcwswpal x17, x21, [sp]\n"
        "rcwsetpal x15, xzr, [x4]\n"
        "braa x24, sp\n"
        "cpypwn [x1]!, [x19]!, x16!\n"
        "setgptn [x7]!, x10!, x22\n"
        "rcwclrpa x29, x27, [x13]\n"
        "cpyprtrn [x2]!, [x16]!, x14!\n"
        "rcwswpl x8, x20, [sp]\n"
        "cpyfpt [x9]!, [x5]!, x4!\n"
        "rcwswp xzr, x0, [sp]\n"
        "setgpn [x21]!, x13!, xzr\n"
        "cpyfpn [x16]!, [x6]!, x13!\n"
        "stzgm x0, [sp]\n"
        "cpyprn [x8]!, [x11]!, x0!\n"
        "rcwsswppl xzr, xzr, [x24]\n"
        "rcwsetpa xzr, x29, [sp]\n"
        "rcwsetal x25, xzr, [sp]\n"
        "rcwsclrl xzr, x25, [x6]\n"
        "cpyprn [x18]!, [x13]!, x15!\n"
        "blraaz x26\n"
        "rcwswppal x19, x27, [x30]\n"
        "rcwscaspa x4, x5, x28, x29, [x0]\n"
        "cpyfptwn [x1]!, [x27]!, x29!\n"
        "ldclrpl x4, x0, [sp]\n"
        "rcwclrp xzr, x15, [x23]\n"
        "rcwsswpp x2, xzr, [x12]\n"
        "cpyprtrn [x4]!, [x30]!, x28!\n"
        "rcwclrp x21, x7, [sp]\n"
        "subp x30, x8, x8\n"
        "subp x9, x26, sp\n"
        "ld64b x2, [x24]\n"
        "cpyfptrn [x23]!, [x15]!, x2!\n"
        "autizb x10\n"
        "rcwswppal x20, x12, [x7]\n"
        "rcwsclrpl x11, x0, [sp]\n"
        "ldclrp x13, x16, [x26]\n"
        "cpyfptrn [x20]!, [x22]!, x23!\n"
        "cpyfpn [x23]!, [x28]!, x12!\n"
        "dvp rctx, x12\n"
        "rcwcasa xzr, x29, [sp]\n"
        "setgp [x18]!, x10!, xzr\n"
        "ldapursh wzr, [x30, #-16]\n"
        "ttest x21\n"
        "rcwsetp x30, x28, [sp]\n"
        "rcwclrpa x18, xzr, [x14]\n"
        "rcwssetpal x12, x25, [x7]\n"
        "rcwclrpl x10, xzr, [x28]\n"
        "rcwclrpal x15, xzr, [x24]\n"
        "rcwclrl x14, x20, [x28]\n"
        "setgpt [x5]!, x22!, xzr\n"
        "rcwcasl x16, x1, [x8]\n"
        "autizb x11\n"
        "swppa x9, x3, [x26]\n"
        "ldtrb wzr, [sp, #-167]\n"
        "autib x17, x23\n"
        "stgm x6, [x10]\n"
        "gcsstr xzr, [x9]\n"
        "stlurb w17, [x26, #247]\n"
        "ldsetpal x0, x8, [x29]\n"
        "wfit x30\n"
        "rcwswppl x12, x5, [x4]\n"
        "ldclrpa x15, x11, [sp]\n"
        "dsb #24\n"
        "ldtrsw x25, [x29]\n"
        "ldg x20, [x20]\n"
        "blraaz x25\n"
        "rcwssetpal x25, x6, [x27]\n"
        "rcwcas x10, x10, [sp]\n"
        "ldap1 {v24.d}[0], [x17]\n"
        "ldap1 {v25.d}[0], [x23]\n"
        "rcwcasal x17, x8, [sp]\n"
        "rcwsclrpal xzr, x1, [sp]\n"
        "cpyfprtrn [x7]!, [x28]!, x17!\n"
        "autizb x29\n"
        "ldtrh wzr, [x4]\n"
        "sysl x8, #1, c4, c0, #2\n"
        "ldapursw x25, [sp, #29]\n"
        "rcwcaspl x12, x13, x16, x17, [x26]\n"
        "cpypwtrn [x4]!, [x10]!, x13!\n"
        "rcwcas x30, x23, [x22]\n"
        "rcwclrp x18, x15, [x28]\n"
        "cpyfprtrn [x3]!, [x24]!, x0!\n"
        "rcwsetpal xzr, xzr, [x8]\n"
        "rcwcasal x2, x29, [sp]\n"
        "setgp [x10]!, x2!, x6\n"
        "irg sp, x29\n"
        "rcwsseta x28, x15, [sp]\n"
        "brabz x0\n"
        "ldapursb x2, [x14, #-168]\n"
        "rcwcaspa x28, x29, x16, x17, [sp]\n"
        "rcwswppl x3, x4, [x10]\n"
        "rcwswp x9, x22, [x26]\n"
        "ldtrh wzr, [x14, #42]\n"
        "setptn [x13]!, x7!, x12\n"
        "rcwscasp x22, x23, x16, x17, [x24]\n"
        "rcwclrl x26, x19, [sp]\n"
        "autdb x17, x28\n"
        "wfit x17\n"
        "sysl xzr, #3, c14, c1, #7\n"
        "swppl x4, x22, [x1]\n"
        "cpyfprn [x1]!, [x28]!, x9!\n"
        "cpyfprn [x17]!, [x18]!, x3!\n"
        "setgpn [x12]!, x30!, x23\n"
        "cpyfpwn [x15]!, [x17]!, x21!\n"
        "rcwssetal x1, xzr, [x4]\n"
        "rcwcasl x11, x3, [x18]\n"
        "autdb x16, sp\n"
        "cpyfptrn [x15]!, [x18]!, x17!\n"
        "cpyprtwn [x25]!, [x19]!, x16!\n"
        "st64b x16, [x27]\n"
        "rcwscasp x14, x15, x26, x27, [x8]\n"
        "cpyptn [x11]!, [x23]!, x1!\n"
        "autdza xzr\n"
        "cpp rctx, x14\n"
        "stlurb wzr, [sp]\n"
        "cmpp x7, x21\n"
        "cpypt [x7]!, [x12]!, x30!\n"
        "rcwsetal x17, x14, [sp]\n"
        "ldsetpa x9, x26, [x30]\n"
        "swppl x11, x22, [sp]\n"
        "blraa x28, x26\n"
        "st64b x18, [x2]\n"
        "rcwclrl x27, x9, [x16]\n"
        "cpypt [x25]!, [x0]!, x8!\n"
        "ldsetpal x28, x20, [sp]\n"
        "cpyprtwn [x29]!, [x27]!, x6!\n"
        "rcwcaspl x4, x5, x6, x7, [sp]\n"
        "rcwswpal x20, x8, [x15]\n"
        "swppl x30, x13, [sp]\n"
        "sttrb w13, [x6, #141]\n"
        "swppa x8, x26, [sp]\n"
        "ldclrpl x4, x24, [sp]\n"
        "cmpp x26, sp\n"
        "setgpn [x27]!, x13!, x17\n"
        "rcwswppl xzr, x2, [sp]\n"
        "autdza x18\n"
        "stz2g x6, [x13]\n"
        "cpyfpn [x11]!, [x25]!, x17!\n"
        "rcwsclrpl x2, x20, [sp]\n"
        "cpyfprn [x2]!, [x16]!, x24!\n"
        "rcwsseta xzr, x10, [sp]\n"
        "rcwswp x26, x24, [sp]\n"
        "ldsetpl x13, x29, [sp]\n"
        "autiza x24\n"
        "cpyfprn [x30]!, [x26]!, x22!\n"
        "ldapurb w27, [x20, #-197]\n"
        "cpyfptrn [x15]!, [x24]!, x17!\n"
        "cpyfpwn [x1]!, [x25]!, x22!\n"
        "swpp x20, x15, [x22]\n"
        "stlurb w6, [sp]\n"
        "ldg x5, [x26]\n"
        "rcwscasl xzr, x16, [x11]\n"
        "rcwsswpp x2, x19, [x12]\n"
        "sysl x27, #0, c13, c4, #3\n"
        "ldapurh w12, [x10]\n"
        "ldapursw x1, [x23, #-12]\n"
        "swpp x25, x17, [x21]\n"
        "gcsss2 x10\n"
        "st64bv x0, x20, [x1]\n"
        "st64bv xzr, x6, [x2]\n"
        "ldsetpal x29, x29, [sp]\n"
        "ldapurb wzr, [x14]\n"
        "rcwswpal x3, x14, [x29]\n"
        "ldtrb wzr, [x20, #223]\n"
        "stlurh w9, [sp]\n"
        "stlurh wzr, [sp]\n"
        "ldclrpl x0, x25, [x24]\n"
        "st64b x22, [x16]\n"
        "ldtrh wzr, [sp, #-254]\n"
        "sttrh wzr, [x3]\n"
        "swppa x8, x26, [x9]\n"
        "sttrh wzr, [sp]\n"
        "sysp #6, c9, c5, #1, x12\n"
        "sttrh wzr, [x25, #-131]\n"
        "sttrh w18, [x1, #137]\n"
        "sysp #5, c9, c5, #5, x12\n"
        ".globl axiom_asm_table_end\n"
        "axiom_asm_table_end:\n"
        ".text\n");

extern const unsigned char axiom_asm_table[], axiom_asm_table_end[];
static const char *const want[] = {
    "827c2048",  /* casp x0, x1, x2, x3, [x4] */
    "e2ff6008",  /* caspal w0, w1, w2, w3, [sp] */
    "e00f7db3",  /* bfc x0, #3, #4 */
    "200c1d33",  /* bfi w0, w1, #3, #4 */
    "201c4093",  /* sxtb x0, w1 */
    "203c0053",  /* uxth w0, w1 */
    "207c4093",  /* sxtw x0, w1 */
    "20fc029b",  /* mneg x0, x1, x2 */
    "20fc229b",  /* smnegl x0, w1, w2 */
    "207cc29b",  /* umulh x0, x1, x2 */
    "201481da",  /* cneg x0, x1, eq */
    "e0039f5a",  /* csetm w0, ne */
    "e00301fa",  /* ngcs x0, x1 */
    "207cdf08",  /* ldlarb w0, [x1] */
    "207c9f48",  /* stllrh w0, [x1] */
    "4100e038",  /* ldaddalb w0, w1, [x2] */
    "3f6060f8",  /* stumaxl x0, [x1] */
    "20c0bf78",  /* ldaprh w0, [x1] */
    "e0030091",  /* mov x0, sp */
    "3f000091",  /* mov sp, x1 */
    "2000a052",  /* mov w0, #0x10000 */
    "2000e0d2",  /* movz x0, #1, lsl #48 */
    "00008092",  /* mov x0, #-1 */
    "df2403d5",  /* bti jc */
    "5f2403d5",  /* hint #34 */
    "3f2003d5",  /* yield */
    "bf3903d5",  /* dmb ishld */
    "df3f03d5",  /* isb sy */
    "df4203d5",  /* msr daifset, #2 */
    "00423bd5",  /* mrs x0, nzcv */
    "01421bd5",  /* msr s3_3_c4_c2_0, x1 */
    "20005fd6",  /* ret x1 */
    "00021fd6",  /* br x16 */
    "00013fd6",  /* blr x8 */
    "e007ffad",  /* ldp q0, q1, [sp, #-32]! */
    "0004406c",  /* ldnp d0, d1, [x0] */
    "408400a8",  /* stnp x0, x1, [x2, #8] */
    "4004c168",  /* ldpsw x0, x1, [x2], #8 */
    "40047fc8",  /* ldxp x0, x1, [x2] */
    "408423c8",  /* stlxp w3, x0, x1, [x2] */
    "23f09ff8",  /* prfum pldl2strm, [x1, #-1] */
    "3478a2f8",  /* prfm pstl3keep, [x1, x2, lsl #3] */
    "20d8a2b8",  /* ldrsw x0, [x1, w2, sxtw #2] */
    "20e862f8",  /* ldr x0, [x1, x2, sxtx] */
    "2078623c",  /* ldr b0, [x1, x2, lsl #0] */
    "2058a23c",  /* str q0, [x1, w2, uxtw #4] */
    "00907e1e",  /* fmov d0, #-1.25 */
    "00f4006f",  /* fmov v0.2d, #2.0 */
    "2000af9e",  /* fmov v0.d[1], x1 */
    "0100ae9e",  /* fmov x1, v0.d[1] */
    "0820e01e",  /* fcmp h0, #0.0 */
    "14b4211e",  /* fccmpe s0, s1, #4, lt */
    "2040681e",  /* frint32z d0, d1 */
    "20c0291e",  /* frint64x s0, s1 */
    "20007e1e",  /* fjcvtzs w0, d1 */
    "20c0631e",  /* fcvt h0, d1 */
    "200cc21f",  /* fmadd h0, h1, h2, h3 */
    "410020f8",  /* ldadd x0, x1, [x2] */
    "e183e0f8",  /* swpal x0, x1, [sp] */
    "417ca088",  /* cas w0, w1, [x2] */
    "4140a0b8",  /* ldsmaxa w0, w1, [x2] */
    "3f0020b8",  /* stadd w0, [x1] */
    "9f2003d5",  /* sev */
    "5f2003d5",  /* wfe */
    "20243bd5",  /* mrs x0, rndrrs */
    "0020df4c",  /* ld1 {v0.16b-v3.16b}, [x0], #64 */
    "00b0a24d",  /* st4 {v0.s, v1.s, v2.s, v3.s}[3], [x0], x2 */
    "00c4404d",  /* ld1r {v0.8h}, [x0] */
    "2020034e",  /* tbl v0.16b, {v1.16b, v2.16b}, v3.16b */
    "200c044e",  /* dup v0.4s, w1 */
    "203c184e",  /* umov x0, v1.d[1] */
    "202c074e",  /* smov x0, v1.b[3] */
    "2004186e",  /* ins v0.d[1], v1.d[0] */
    "40e5056f",  /* movi v0.2d, #0xff00ff00ff00ff00 */
    "20c4006f",  /* mvni v0.4s, #1, msl #8 */
    "2034006f",  /* bic v0.4s, #1, lsl #8 */
    "0096004f",  /* orr v0.8h, #0x10 */
    "2018824e",  /* uzp1 v0.4s, v1.4s, v2.4s */
    "20e8a24f",  /* sdot v0.4s, v1.16b, v2.4b[3] */
    "20ec426e",  /* bfmmla v0.4s, v1.8h, v2.8h */
    "2038826f",  /* fcmla v0.4s, v1.4s, v2.s[1], #90 */
    "20b8f15e",  /* addp d0, v1.2d */
    "20c8305e",  /* fmaxnmp h0, v1.2h */
    "20fcdf88",  /* ldar w0, [x1, #0] */
    "220000d4",  /* hvc #1 */
    "430000d4",  /* smc #2 */
    "0100a0d4",  /* dcps1 */
    "6200a0d4",  /* dcps2 #3 */
    "0300a0d4",  /* dcps3 */
    "201043b3",  /* bfm x0, x1, #3, #4 */
    "20100313",  /* sbfm w0, w1, #3, #4 */
    "20007fd3",  /* ubfm x0, x1, #63, #0 */
    "2028c29a",  /* asrv x0, x1, x2 */
    "2020c21a",  /* lslv w0, w1, w2 */
    "200ca29b",  /* umaddl x0, w1, w2, x3 */
    "208c229b",  /* smsubl x0, w1, w2, x3 */
    "e04b21ab",  /* adds x0, sp, w1, uxtw #2 */
    "ff1300f1",  /* cmp sp, #4 */
    "1f80212b",  /* cmn w0, w1, sxtb */
    "201042b3",  /* bfxil x0, x1, #2, #3 */
    "20081e53",  /* ubfiz w0, w1, #2, #3 */
    "20104293",  /* sbfx x0, x1, #2, #3 */
    "1f1c0072",  /* tst w0, #0xff */
    "200ce2ca",  /* eon x0, x1, x2, ror #3 */
    "2000222a",  /* orn w0, w1, w2 */
    "207c5f08",  /* ldxrb w0, [x1] */
    "207c0248",  /* stxrh w2, w0, [x1] */
    "e00f21aa",  /* mvn x0, x1, lsl #3 */
    "e00b814b",  /* neg w0, w1, asr #2 */
    "e00301eb",  /* negs x0, x1 */
    "e003015a",  /* ngc w0, w1 */
    "200002fa",  /* sbcs x0, x1, x2 */
    "2014811a",  /* cinc w0, w1, eq */
    "200081da",  /* cinv x0, x1, ne */
    "20a4821a",  /* csinc w0, w1, w2, ge */
    "20b482da",  /* csneg x0, x1, x2, lt */
    "040843ba",  /* ccmn x0, #3, #4, eq */
    "2004c0da",  /* rev16 x0, x1 */
    "2008c0da",  /* rev32 x0, x1 */
    "200cc0da",  /* rev64 x0, x1 */
    "2014c05a",  /* cls w0, w1 */
    "207c429b",  /* smulh x0, x1, x2 */
    "207ca29b",  /* umull x0, w1, w2 */
    "207c229b",  /* smull x0, w1, w2 */
    "207c021b",  /* mul w0, w1, w2 */
    "2008c21a",  /* udiv w0, w1, w2 */
    "205cc29a",  /* crc32cx w0, w1, x2 */
    "bf4100d5",  /* msr spsel, #1 */
    "9f4100d5",  /* msr pan, #1 */
    "40d03bd5",  /* mrs x0, tpidr_el0 */
    "00f238d5",  /* mrs x0, s3_0_c15_c2_0 */
    "5f2403d5",  /* bti c */
    "20fc9f08",  /* stlrb w0, [x1] */
    "20c0bf38",  /* ldaprb w0, [x1, #0] */
    "000460ac",  /* ldnp q0, q1, [x0, #-1024] */
    "e007bf29",  /* stp w0, w1, [sp, #-8]! */
    "6000a072",  /* movk w0, #3, lsl #16 */
    "bf2003d5",  /* sevl */
    "7f2003d5",  /* wfi */
    "1f2203d5",  /* esb */
    "9f2203d5",  /* csdb */
    "7e7c7608",  /* caspa w22, w23, w30, wzr, [x3] */
    "627c2a08",  /* casp w10, w11, w2, w3, [x3] */
    "e27f6648",  /* caspa x6, x7, x2, x3, [sp] */
    "74fc2e48",  /* caspl x14, x15, x20, x21, [x3] */
    "e6ff3408",  /* caspl w20, w21, w6, w7, [sp] */
    "7a7c2448",  /* casp x4, x5, x26, x27, [x3] */
    "66fc3a48",  /* caspl x26, x27, x6, x7, [x3] */
    "20fc3448",  /* caspl x20, x21, x0, x1, [x1, #0] */
    "227c2848",  /* casp x8, x9, x2, x3, [x1, #0] */
    "70fc7648",  /* caspal x22, x23, x16, x17, [x3] */
    "f07f6e48",  /* caspa x14, x15, x16, x17, [sp] */
    "f4ff6408",  /* caspal w4, w5, w20, w21, [sp] */
    "e6ff6e48",  /* caspal x14, x15, x6, x7, [sp] */
    "f27f7c08",  /* caspa w28, w29, w18, w19, [sp] */
    "3918589e",  /* fcvtzs x25, d1, #58 */
    "cc88591e",  /* fcvtzu w12, d6, #30 */
    "9cc4431e",  /* ucvtf d28, w4, #15 */
    "6d9bc31e",  /* ucvtf h13, w27, #26 */
    "2c9d031e",  /* ucvtf s12, w9, #25 */
    "d3ff031e",  /* ucvtf s19, w30, #1 */
    "05e6d89e",  /* fcvtzs x5, h16, #7 */
    "1a58589e",  /* fcvtzs x26, d0, #42 */
    "7e65029e",  /* scvtf s30, x11, #39 */
    "6021029e",  /* scvtf s0, x11, #56 */
    "678ac21e",  /* scvtf h7, w19, #30 */
    "101a199e",  /* fcvtzu x16, s16, #58 */
    "622b029e",  /* scvtf s2, x27, #54 */
    "bd68589e",  /* fcvtzs x29, d5, #38 */
    "4408d99e",  /* fcvtzu x4, h2, #62 */
    "9612599e",  /* fcvtzu x22, d20, #60 */
    "40040119",  /* cpyfp [x0]!, [x1]!, x2! */
    "40048119",  /* cpyfe [x0]!, [x1]!, x2! */
    "4004011d",  /* cpyp [x0]!, [x1]!, x2! */
    "2044c219",  /* setm [x0]!, x1!, x2 */
    "2084c219",  /* sete [x0]!, x1!, x2 */
    "2014c219",  /* setpt [x0]!, x1!, x2 */
    "e00b80b8",  /* ldtrsw x0, [sp] */
    "200820d9",  /* stg x0, [x1] */
    "3f0430d9",  /* stg sp, [x1], #-4096 */
    "20f86fd9",  /* stzg x0, [x1, #4080] */
    "201ca0d9",  /* st2g x0, [x1, #16]! */
    "2008e0d9",  /* stz2g x0, [x1] */
    "202060d9",  /* ldg x0, [x1, #32] */
    "4004a069",  /* stgp x0, x1, [x2, #-1024]! */
    "40849f68",  /* stgp x0, x1, [x2], #1008 */
    "40040069",  /* stgp x0, x1, [x2] */
    "2000e0d9",  /* ldgm x0, [x1] */
    "2000a0d9",  /* stgm x0, [x1] */
    "200020d9",  /* stzgm x0, [x1] */
    "e03fbf91",  /* addg x0, sp, #1008, #15 */
    "3f0481d1",  /* subg sp, x1, #16, #1 */
    "2014c29a",  /* gmi x0, x1, x2 */
    "e003c29a",  /* subp x0, sp, x2 */
    "2000dfba",  /* subps x0, x1, sp */
    "3f00c2ba",  /* cmpp x1, x2 */
    "2010df9a",  /* irg x0, x1 */
    "20004019",  /* ldapurb w0, [x1] */
    "20f08f19",  /* ldapursb x0, [x1, #255] */
    "2000c059",  /* ldapursh w0, [x1] */
    "20008099",  /* ldapursw x0, [x1] */
    "20001059",  /* stlurh w0, [x1, #-256] */
    "200000d9",  /* stlur x0, [x1] */
    "20000099",  /* stlur w0, [x1] */
    "20004099",  /* ldapur w0, [x1] */
    "2038401d",  /* ldapur b0, [x1, #3] */
    "2038c01d",  /* ldapur q0, [x1, #3] */
    "200800dd",  /* stlur d0, [x1] */
    "2084414d",  /* ldap1 {v0.d}[1], [x1] */
    "2084010d",  /* stl1 {v0.d}[0], [x1] */
    "40184199",  /* ldiapp w0, w1, [x2] */
    "401841d9",  /* ldiapp x0, x1, [x2] */
    "400801d9",  /* stilp x0, x1, [x2, #-16]! */
    "401801d9",  /* stilp x0, x1, [x2] */
    "200880d9",  /* stlr x0, [x1, #-8]! */
    "20088099",  /* stlr w0, [x1, #-4]! */
    "2008c099",  /* ldapr w0, [x1], #4 */
    "200420f8",  /* ldraa x0, [x1] */
    "20f4bff8",  /* ldrab x0, [x1, #4088] */
    "e01fa0f8",  /* ldrab x0, [sp, #8]! */
    "41082019",  /* rcwcas x0, x1, [x2] */
    "4108a019",  /* rcwcasa x0, x1, [x2] */
    "4108e019",  /* rcwcasal x0, x1, [x2] */
    "41086019",  /* rcwcasl x0, x1, [x2] */
    "e10b2059",  /* rcwscas x0, x1, [sp] */
    "820c2019",  /* rcwcasp x0, x1, x2, x3, [x4] */
    "820ce059",  /* rcwscaspal x0, x1, x2, x3, [x4] */
    "41902038",  /* rcwclr x0, x1, [x2] */
    "4190a038",  /* rcwclra x0, x1, [x2] */
    "41902078",  /* rcwsclr x0, x1, [x2] */
    "41b02038",  /* rcwset x0, x1, [x2] */
    "41b02078",  /* rcwsset x0, x1, [x2] */
    "41a02038",  /* rcwswp x0, x1, [x2] */
    "41a02078",  /* rcwsswp x0, x1, [x2] */
    "40902119",  /* rcwclrp x0, x1, [x2] */
    "40902159",  /* rcwsclrp x0, x1, [x2] */
    "40b02119",  /* rcwsetp x0, x1, [x2] */
    "40b02159",  /* rcwssetp x0, x1, [x2] */
    "40a02119",  /* rcwswpp x0, x1, [x2] */
    "40a0e159",  /* rcwsswppal x0, x1, [x2] */
    "40102119",  /* ldclrp x0, x1, [x2] */
    "4010a119",  /* ldclrpa x0, x1, [x2] */
    "4010e119",  /* ldclrpal x0, x1, [x2] */
    "40106119",  /* ldclrpl x0, x1, [x2] */
    "40302119",  /* ldsetp x0, x1, [x2] */
    "40802119",  /* swpp x0, x1, [x2] */
    "e083e119",  /* swppal x0, x1, [sp] */
    "2020c05a",  /* abs w0, w1 */
    "2020c0da",  /* abs x0, x1 */
    "201cc0da",  /* cnt x0, x1 */
    "2018c05a",  /* ctz w0, w1 */
    "2060c21a",  /* smax w0, w1, w2 */
    "2000c291",  /* smax x0, x1, #-128 */
    "20fcc791",  /* umax x0, x1, #255 */
    "20fcc911",  /* smin w0, w1, #127 */
    "206cc29a",  /* umin x0, x1, x2 */
    "00770bd5",  /* gcspushm x0 */
    "20772bd5",  /* gcspopm x0 */
    "3f772bd5",  /* gcspopm */
    "40770bd5",  /* gcsss1 x0 */
    "60772bd5",  /* gcsss2 x0 */
    "9f7708d5",  /* gcspushx */
    "bf7708d5",  /* gcspopcx */
    "df7708d5",  /* gcspopx */
    "200c1fd9",  /* gcsstr x0, [x1] */
    "e01f1fd9",  /* gcssttr x0, [sp] */
    "7f2203d5",  /* gcsb dsync */
    "603023d5",  /* tstart x0 */
    "7f3003d5",  /* tcommit */
    "e0ff7fd4",  /* tcancel #0xffff */
    "603123d5",  /* ttest x0 */
    "20d03ff8",  /* ld64b x0, [x1] */
    "20903ff8",  /* st64b x0, [x1] */
    "20b022f8",  /* st64bv x2, x0, [x1] */
    "20a022f8",  /* st64bv0 x2, x0, [x1] */
    "1f2503d5",  /* chkfeat x16 */
    "e3720bd5",  /* trcit x3 */
    "df2203d5",  /* clrbhb */
    "ff3003d5",  /* sb */
    "9f3003d5",  /* ssbb */
    "9f3403d5",  /* pssbb */
    "df2003d5",  /* dgh */
    "3f2203d5",  /* psb csync */
    "5f2203d5",  /* tsb csync */
    "1f2103d5",  /* pacia1716 */
    "5f2103d5",  /* pacib1716 */
    "9f2103d5",  /* autia1716 */
    "df2103d5",  /* autib1716 */
    "1f2303d5",  /* paciaz */
    "5f2303d5",  /* pacibz */
    "9f2303d5",  /* autiaz */
    "df2303d5",  /* autibz */
    "ff2003d5",  /* xpaclri */
    "1f2403d5",  /* bti */
    "2d08003a",  /* setf8 w1 */
    "4d48003a",  /* setf16 w2 */
    "1f4000d5",  /* cfinv */
    "3f4000d5",  /* xaflag */
    "5f4000d5",  /* axflag */
    "2f841fba",  /* rmif x1, #63, #15 */
    "031003d5",  /* wfet x3 */
    "241003d5",  /* wfit x4 */
    "e143c1da",  /* xpaci x1 */
    "e247c1da",  /* xpacd x2 */
    "e32bc1da",  /* pacdza x3 */
    "e43fc1da",  /* autdzb x4 */
    "e10bc1da",  /* pacda x1, sp */
    "ff0f5fd6",  /* retab */
    "3f081fd6",  /* braaz x1 */
    "3f0c1fd6",  /* brabz x1 */
    "3f083fd6",  /* blraaz x1 */
    "3f0c3fd6",  /* blrabz x1 */
    "3f083fd7",  /* blraa x1, sp */
    "220c3fd7",  /* blrab x1, x2 */
    "220c1fd7",  /* brab x1, x2 */
    "ff0b9fd6",  /* eretaa */
    "ff0f9fd6",  /* eretab */
    "e0039fd6",  /* eret */
    "e003bfd6",  /* drps */
    "20c0bf38",  /* ldaprb w0, [x1] */
    "207c9fc8",  /* stllr x0, [x1] */
    "4100c1da",  /* pacia x1, x2 */
    "e323c1da",  /* paciza x3 */
    "4130df9a",  /* pacga x1, x2, sp */
    "ff0b5fd6",  /* retaa */
    "22081fd7",  /* braa x1, x2 */
    "200c60f8",  /* ldraa x0, [x1, #-4096]! */
    "e013c39a",  /* irg x0, sp, x3 */
    "201c20d9",  /* stg x0, [x1, #16]! */
    "238608d5",  /* tlbi rvae1, x3 */
    "217d0bd5",  /* dc cvadp, x1 */
    "011003d5",  /* wfet x1 */
    "228401ba",  /* rmif x1, #3, #2 */
    "400841d9",  /* ldiapp x0, x1, [x2], #16 */
    "40080199",  /* stilp w0, w1, [x2, #-8]! */
    "2008c0d9",  /* ldapr x0, [x1], #8 */
    "20d05fd9",  /* ldapur x0, [x1, #-3] */
    "008048d5",  /* sysp #0, c8, c0, #0, x0, x1 */
    "208748d5",  /* tlbip vae1, x0, x1 */
    "e0720bd5",  /* trcit x0 */
    "c3780cd5",  /* at s12e0r, x3 */
    "e3780cd5",  /* at s12e0w, x3 */
    "83780cd5",  /* at s12e1r, x3 */
    "a3780cd5",  /* at s12e1w, x3 */
    "83730bd5",  /* cfp rctx, x3 */
    "c3730bd5",  /* cosp rctx, x3 */
    "e3730bd5",  /* cpp rctx, x3 */
    "c37a08d5",  /* dc cgdsw, x3 */
    "a37a0bd5",  /* dc cgdvac, x3 */
    "a37d0bd5",  /* dc cgdvadp, x3 */
    "a3730bd5",  /* dvp rctx, x3 */
    "1f7508d5",  /* ic iallu */
    "1f7108d5",  /* ic ialluis */
    "23750bd5",  /* ic ivau, x3 */
    "9f870cd5",  /* tlbi alle1 */
    "9f830cd5",  /* tlbi alle1is */
    "9f930cd5",  /* tlbi alle1isnxs */
    "24844cd5",  /* tlbip ipas2e1, x4, x5 */
    "24804cd5",  /* tlbip ipas2e1is, x4, x5 */
    "24904cd5",  /* tlbip ipas2e1isnxs, x4, x5 */
    "f893b359",  /* rcwsclrpa x24, x19, [sp] */
    "f5550119",  /* cpyfpwtwn [x21]!, [x1]!, x15! */
    "bfba2ad5",  /* sysl xzr, #2, c11, c10, #5 */
    "43d4051d",  /* cpypwtn [x3]!, [x5]!, x2! */
    "0407c1da",  /* pacib x4, x24 */
    "12a3ff78",  /* rcwsswpal xzr, x18, [x24] */
    "5b561e1d",  /* cpypwtwn [x27]!, [x30]!, x18! */
    "fb93b838",  /* rcwclra x24, x27, [sp] */
    "2cd5041d",  /* cpypwtn [x12]!, [x4]!, x9! */
    "ba82e219",  /* swppal x26, x2, [x21] */
    "c0ac7cd4",  /* tcancel #58726 */
    "0e1003d5",  /* wfet x14 */
    "f5906238",  /* rcwclrl x2, x21, [x7] */
    "ec932e78",  /* rcwsclr x14, x12, [sp] */
    "bf0c3fd6",  /* blrabz x5 */
    "ff30bfd1",  /* subg sp, x7, #1008, #12 */
    "f00fe219",  /* rcwcaspal x2, x3, x16, x17, [sp] */
    "56b1e378",  /* rcwssetal x3, x22, [x10] */
    "56770bd5",  /* gcsss1 x22 */
    "ff93a859",  /* rcwsclrpa xzr, x8, [sp] */
    "760a1fd7",  /* braa x19, x22 */
    "e6937078",  /* rcwsclrl x16, x6, [sp] */
    "9d87014d",  /* stl1 {v29.d}[1], [x28] */
    "3f90b859",  /* rcwsclrpa xzr, x24, [x1] */
    "44fa0fd5",  /* sys #7, c15, c10, #2, x4 */
    "2604101d",  /* cpyp [x6]!, [x16]!, x1! */
    "7d06c1da",  /* pacib x29, x19 */
    "371003d5",  /* wfit x23 */
    "090bbf59",  /* rcwscasa xzr, x9, [x24] */
    "46a37f38",  /* rcwswpl xzr, x6, [x26] */
    "610b1fd7",  /* braa x27, x1 */
    "5f3e9991",  /* addg sp, x18, #400, #15 */
    "55a1ab59",  /* rcwsswppa x21, x11, [x10] */
    "7bb07f38",  /* rcwsetl xzr, x27, [x3] */
    "9bb33978",  /* rcwsset x25, x27, [x28] */
    "e983f019",  /* swppal x9, x16, [sp] */
    "b1b1bf38",  /* rcwseta xzr, x17, [x13] */
    "5c0a80d9",  /* stlr x28, [x18, #-8]! */
    "7304121d",  /* cpyp [x19]!, [x18]!, x3! */
    "5932dc9a",  /* pacga x25, x18, x28 */
    "f40f6259",  /* rcwscaspl x2, x3, x20, x21, [sp] */
    "9f0e1fd6",  /* brabz x20 */
    "fb2bc1da",  /* pacdza x27 */
    "4c17df19",  /* setpt [x12]!, x26!, xzr */
    "f613fb19",  /* ldclrpal x22, x27, [sp] */
    "9f937878",  /* rcwsclrl x24, xzr, [x28] */
    "65d61719",  /* cpyfpwtn [x5]!, [x23]!, x19! */
    "f392ee19",  /* rcwclrpal x19, x14, [x23] */
    "a927021d",  /* cpyprt [x9]!, [x2]!, x29! */
    "f0d33ff8",  /* ld64b x16, [sp] */
    "d1a0bc59",  /* rcwsswppa x17, x28, [x6] */
    "e3e61e19",  /* cpyfprtn [x3]!, [x30]!, x23! */
    "743023d5",  /* tstart x20 */
    "c592bf38",  /* rcwclra xzr, x5, [x22] */
    "b792bf38",  /* rcwclra xzr, x23, [x21] */
    "83560a19",  /* cpyfpwtwn [x3]!, [x10]!, x20! */
    "a4e70519",  /* cpyfprtn [x4]!, [x5]!, x29! */
    "e303e0d9",  /* ldgm x3, [sp] */
    "fc0f1fd9",  /* gcsstr x28, [sp] */
    "e8a327f8",  /* st64bv0 x7, x8, [sp] */
    "7c913c59",  /* rcwsclrp x28, x28, [x11] */
    "62b0ff38",  /* rcwsetal xzr, x2, [x3] */
    "100de659",  /* rcwscaspal x6, x7, x16, x17, [x8] */
    "e843c1da",  /* xpaci x8 */
    "dfa1ff59",  /* rcwsswppal xzr, xzr, [x14] */
    "ff4372d9",  /* ldg xzr, [sp, #-3520] */
    "d082e819",  /* swppal x16, x8, [x22] */
    "7fa33978",  /* rcwsswp x25, xzr, [x27] */
    "7f3123d5",  /* ttest xzr */
    "45a36878",  /* rcwsswpl x8, x5, [x26] */
    "8f861d1d",  /* cpyprn [x15]!, [x29]!, x20! */
    "37f4f9d9",  /* stz2g x23, [x1], #-1552 */
    "f6b33638",  /* rcwset x22, x22, [sp] */
    "f715131d",  /* cpypwt [x23]!, [x19]!, x15! */
    "f6a3a259",  /* rcwsswppa x22, x2, [sp] */
    "bf730bd5",  /* dvp rctx, xzr */
    "2203c1da",  /* pacia x2, x25 */
    "2ff6fff8",  /* ldrab x15, [x17, #-8] */
    "ff43c1da",  /* xpaci xzr */
    "22371e1d",  /* cpypt [x2]!, [x30]!, x25! */
    "bf0b1fd6",  /* braaz x29 */
    "bf93f459",  /* rcwsclrpal xzr, x20, [x29] */
    "72970c19",  /* cpyfpwtrn [x18]!, [x12]!, x27! */
    "3f772bd5",  /* gcspopm xzr */
    "ea0fa019",  /* rcwcaspa x0, x1, x10, x11, [sp] */
    "5fb1b059",  /* rcwssetpa xzr, x16, [x10] */
    "52b36c19",  /* rcwsetpl x18, x12, [x26] */
    "4ba03619",  /* rcwswpp x11, x22, [x2] */
    "3fc38119",  /* ldapursb xzr, [x25, #28] */
    "ffb33f78",  /* rcwsset xzr, xzr, [sp] */
    "1fa07f38",  /* rcwswpl xzr, xzr, [x0] */
    "6e0eb419",  /* rcwcaspa x20, x21, x14, x15, [x19] */
    "030ae559",  /* rcwscasal x5, x3, [x16] */
    "4bc51e1d",  /* cpypn [x11]!, [x30]!, x10! */
    "31e50219",  /* cpyfprtn [x17]!, [x2]!, x9! */
    "5db0a319",  /* rcwsetpa x29, x3, [x2] */
    "ffa3ba78",  /* rcwsswpa x26, xzr, [sp] */
    "ff720bd5",  /* trcit xzr */
    "c40b3e19",  /* rcwcas x30, x4, [x30] */
    "401a5cd9",  /* ldiapp x0, x28, [x18] */
    "f40b0038",  /* sttrb w20, [sp] */
    "2a170d19",  /* cpyfpwt [x10]!, [x13]!, x25! */
    "bfb30ad5",  /* sys #2, c11, c3, #5, xzr */
    "51c7171d",  /* cpypn [x17]!, [x23]!, x26! */
    "71e4141d",  /* cpyprtn [x17]!, [x20]!, x3! */
    "bfb3a338",  /* rcwseta x3, xzr, [x29] */
    "ff1bc0da",  /* ctz xzr, xzr */
    "f9b37f38",  /* rcwsetl xzr, x25, [sp] */
    "e73bc1da",  /* autdza x7 */
    "3f750bd5",  /* ic ivau, xzr */
    "44651a19",  /* cpyfprtwn [x4]!, [x26]!, x10! */
    "f727c1da",  /* pacizb x23 */
    "e626d21d",  /* setgpn [x6]!, x23!, x18 */
    "20907559",  /* rcwsclrpl x0, x21, [x1] */
    "14b13378",  /* rcwsset x19, x20, [x8] */
    "e3b27f59",  /* rcwssetpl x3, xzr, [x23] */
    "c817ca9a",  /* gmi x8, x30, x10 */
    "1c06c1da",  /* pacib x28, x16 */
    "30a50319",  /* cpyfprtrn [x16]!, [x3]!, x9! */
    "df0b0bd9",  /* stilp xzr, x11, [x30, #-16]! */
    "d437161d",  /* cpypt [x20]!, [x22]!, x30! */
    "5f16c1da",  /* autib xzr, x18 */
    "e3b32438",  /* rcwset x4, x3, [sp] */
    "430f3fd7",  /* blrab x26, x3 */
    "fb0b7a19",  /* rcwcasl x26, x27, [sp] */
    "1f0220d9",  /* stzgm xzr, [x16] */
    "12361d19",  /* cpyfpt [x18]!, [x29]!, x16! */
    "df0a1fd6",  /* braaz x22 */
    "d307d11d",  /* setgp [x19]!, x30!, x17 */
    "8934c11d",  /* setgptn [x9]!, x4!, x1 */
    "f56b169d",  /* stlur s21, [sp, #-154] */
    "ffb3f159",  /* rcwssetpal xzr, x17, [sp] */
    "e103e0d9",  /* ldgm x1, [sp] */
    "27b13e59",  /* rcwssetp x7, x30, [x9] */
    "5f770bd5",  /* gcsss1 xzr */
    "2c541d1d",  /* cpypwtwn [x12]!, [x29]!, x1! */
    "f423c1da",  /* paciza x20 */
    "eaa32419",  /* rcwswpp x10, x4, [sp] */
    "490bff59",  /* rcwscasal xzr, x9, [x26] */
    "d192f359",  /* rcwsclrpal x17, x19, [x22] */
    "3f18c1da",  /* autda xzr, x1 */
    "edb3ef78",  /* rcwssetal x15, x13, [sp] */
    "ff2bc1da",  /* pacdza xzr */
    "440e3619",  /* rcwcasp x22, x23, x4, x5, [x18] */
    "9e730bd5",  /* cfp rctx, x30 */
    "e8bb0bf8",  /* sttr x8, [sp, #187] */
    "ffa32e78",  /* rcwsswp x14, xzr, [sp] */
    "3a30df9a",  /* pacga x26, x1, sp */
    "a31ac05a",  /* ctz w3, w21 */
    "3f1bc05a",  /* ctz wzr, w25 */
    "ffb0b319",  /* rcwsetpa xzr, x19, [x7] */
    "0a16c79a",  /* gmi x10, x16, x7 */
    "7eb1bf38",  /* rcwseta xzr, x30, [x11] */
    "ff0bb459",  /* rcwscasa x20, xzr, [sp] */
    "1f0b4078",  /* ldtrh wzr, [x24] */
    "e7fb8ab8",  /* ldtrsw x7, [sp, #175] */
    "c5051219",  /* cpyfp [x5]!, [x18]!, x14! */
    "b4a3ac19",  /* rcwswppa x20, x12, [x29] */
    "0d64c69a",  /* umax x13, x0, x6 */
    "a4d60019",  /* cpyfpwtn [x4]!, [x0]!, x21! */
    "aaa3bf59",  /* rcwsswppa x10, xzr, [x29] */
    "7967101d",  /* cpyprtwn [x25]!, [x16]!, x27! */
    "e4b0bd38",  /* rcwseta x29, x4, [x7] */
    "2644041d",  /* cpypwn [x6]!, [x4]!, x1! */
    "8d32bd91",  /* addg x13, x20, #976, #12 */
    "00d71519",  /* cpyfpwtn [x0]!, [x21]!, x24! */
    "3f0e3fd6",  /* blrabz x17 */
    "9f6cd19a",  /* umin xzr, x4, x17 */
    "c21cc1da",  /* autdb x2, x6 */
    "6e05c1da",  /* pacib x14, x11 */
    "a7a23719",  /* rcwswpp x7, x23, [x21] */
    "f3933578",  /* rcwsclr x21, x19, [sp] */
    "5f92bf59",  /* rcwsclrpa xzr, xzr, [x18] */
    "ff3fc1da",  /* autdzb xzr */
    "3f3e03d5",  /* dsb #28 */
    "ec0b7f59",  /* rcwscasl xzr, x12, [sp] */
    "2875121d",  /* cpyptwn [x8]!, [x18]!, x9! */
    "21bb8878",  /* ldtrsh x1, [x25, #139] */
    "11950d1d",  /* cpypwtrn [x17]!, [x13]!, x8! */
    "7f028019",  /* ldapursb xzr, [x19] */
    "e3b33719",  /* rcwsetp x3, x23, [sp] */
    "e90bc078",  /* ldtrsh w9, [sp] */
    "af33d49a",  /* pacga x15, x29, x20 */
    "4e071c1d",  /* cpyp [x14]!, [x28]!, x26! */
    "ff27c1da",  /* pacizb xzr */
    "ff17c19a",  /* gmi xzr, sp, x1 */
    "f4a3bf19",  /* rcwswppa x20, xzr, [sp] */
    "a227111d",  /* cpyprt [x2]!, [x17]!, x29! */
    "5fed64d9",  /* stzg sp, [x10, #1248]! */
    "df091fd6",  /* braaz x14 */
    "e86bca9a",  /* smin x8, xzr, x10 */
    "1f770bd5",  /* gcspushm xzr */
    "8f271719",  /* cpyfprt [x15]!, [x23]!, x28! */
    "7a3023d5",  /* tstart x26 */
    "2e57141d",  /* cpypwtwn [x14]!, [x20]!, x25! */
    "fa0cec19",  /* rcwcaspal x12, x13, x26, x27, [x7] */
    "bea3a719",  /* rcwswppa x30, x7, [x29] */
    "e4333619",  /* ldsetp x4, x22, [sp] */
    "f4932478",  /* rcwsclr x4, x20, [sp] */
    "6d14c1da",  /* autib x13, x3 */
    "5ab33f59",  /* rcwssetp x26, xzr, [x26] */
    "ff932d38",  /* rcwclr x13, xzr, [sp] */
    "fe833d19",  /* swpp x30, x29, [sp] */
    "dfb22938",  /* rcwset x9, xzr, [x22] */
    "1015ce19",  /* setpt [x16]!, x8!, x14 */
    "7f093fd7",  /* blraa x11, sp */
    "e12fc1da",  /* pacdzb x1 */
    "abb33719",  /* rcwsetp x11, x23, [x29] */
    "9e16061d",  /* cpypwt [x30]!, [x6]!, x20! */
    "ffa3f878",  /* rcwsswpal x24, xzr, [sp] */
    "ff937f19",  /* rcwclrpl xzr, xzr, [sp] */
    "01470819",  /* cpyfpwn [x1]!, [x8]!, x24! */
    "03b23959",  /* rcwssetp x3, x25, [x16] */
    "6e01e0d9",  /* ldgm x14, [x11] */
    "763123d5",  /* ttest x22 */
    "e043c1da",  /* xpaci x0 */
    "4d26df19",  /* setpn [x13]!, x18!, xzr */
    "df730bd5",  /* cosp rctx, xzr */
    "1033b819",  /* ldsetpa x16, x24, [x24] */
    "59440f19",  /* cpyfpwn [x25]!, [x15]!, x2! */
    "f80f7c19",  /* rcwcaspl x28, x29, x24, x25, [sp] */
    "fc34b6d1",  /* subg x28, x7, #864, #13 */
    "0f0baa59",  /* rcwscasa x10, x15, [x24] */
    "ff33c1da",  /* autiza xzr */
    "dc0120d9",  /* stzgm x28, [x14] */
    "dc650f19",  /* cpyfprtwn [x28]!, [x15]!, x14! */
    "1f1a0bd5",  /* sys #3, c1, c10, #0 */
    "ffa37f59",  /* rcwsswppl xzr, xzr, [sp] */
    "9a730bd5",  /* cfp rctx, x26 */
    "0ca06d59",  /* rcwsswppl x12, x13, [x0] */
    "eea22078",  /* rcwsswp x0, x14, [x23] */
    "9fb3ea78",  /* rcwssetal x10, xzr, [x28] */
    "e10a3f59",  /* rcwscas xzr, x1, [x23] */
    "6a0cee19",  /* rcwcaspal x14, x15, x10, x11, [x3] */
    "fd33c1da",  /* autiza x29 */
    "65540e1d",  /* cpypwtwn [x5]!, [x14]!, x3! */
    "bfa13559",  /* rcwsswpp xzr, x21, [x13] */
    "e8337b19",  /* ldsetpl x8, x27, [sp] */
    "55770bd5",  /* gcsss1 x21 */
    "ff038099",  /* ldapursw xzr, [sp] */
    "f2650019",  /* cpyfprtwn [x18]!, [x0]!, x15! */
    "83d7081d",  /* cpypwtn [x3]!, [x8]!, x28! */
    "e2b27f19",  /* rcwsetpl x2, xzr, [x23] */
    "fc23c1da",  /* paciza x28 */
    "9e1bc1da",  /* autda x30, x28 */
    "bfa2fa78",  /* rcwsswpal x26, xzr, [x21] */
    "e80fe659",  /* rcwscaspal x6, x7, x8, x9, [sp] */
    "57816319",  /* swppl x23, x3, [x10] */
    "9fb17f38",  /* rcwsetl xzr, xzr, [x12] */
    "26d77cf8",  /* ldraa x6, [x25, #-408] */
    "f5a3f719",  /* rcwswppal x21, x23, [sp] */
    "ff072bd9",  /* stg sp, [sp], #2816 */
    "ff67df9a",  /* umax xzr, xzr, xzr */
    "5f08c1da",  /* pacda xzr, x2 */
    "b3b2eb38",  /* rcwsetal x11, x19, [x21] */
    "df0d1fd7",  /* brab x14, sp */
    "7f084038",  /* ldtrb wzr, [x3] */
    "fff7cf91",  /* umin xzr, xzr, #253 */
    "1f1003d5",  /* wfet xzr */
    "e9b4111d",  /* cpyptrn [x9]!, [x17]!, x7! */
    "9fa12659",  /* rcwsswpp xzr, x6, [x12] */
    "a46fdf1a",  /* umin w4, w29, wzr */
    "ff93f078",  /* rcwsclral x16, xzr, [sp] */
    "60de72d4",  /* tcancel #38643 */
    "ee57a8d9",  /* st2g x14, [sp], #2128 */
    "11840f1d",  /* cpyprn [x17]!, [x15]!, x0! */
    "bfb17559",  /* rcwssetpl xzr, x21, [x13] */
    "90034059",  /* ldapurh w16, [x28] */
    "f9b73bf8",  /* ldraa x25, [sp, #3544] */
    "4be60519",  /* cpyfprtn [x11]!, [x5]!, x18! */
    "f163df1a",  /* smax w17, wzr, wzr */
    "7074141d",  /* cpyptwn [x16]!, [x20]!, x3! */
    "ed0b003a",  /* setf8 wzr */
    "8de71d1d",  /* cpyprtn [x13]!, [x29]!, x28! */
    "ea3fc1da",  /* autdzb x10 */
    "ed4b003a",  /* setf16 wzr */
    "3f0fc1da",  /* pacdb xzr, x25 */
    "ad93be78",  /* rcwsclra x30, x13, [x29] */
    "7987010d",  /* stl1 {v25.d}[0], [x27] */
    "eb93a878",  /* rcwsclra x8, x11, [sp] */
    "fea3a919",  /* rcwswppa x30, x9, [sp] */
    "f23c9769",  /* stgp x18, x15, [x7, #736]! */
    "d5913538",  /* rcwclr x21, x21, [x14] */
    "5f92bf78",  /* rcwsclra xzr, xzr, [x18] */
    "3b13e619",  /* ldclrpal x27, x6, [x25] */
    "1191ec19",  /* rcwclrpal x17, x12, [x8] */
    "9803c6ba",  /* subps x24, x28, x6 */
    "9608c099",  /* ldapr w22, [x4], #4 */
    "f9661319",  /* cpyfprtwn [x25]!, [x19]!, x23! */
    "e647c1da",  /* xpacd x6 */
    "2d1a81d1",  /* subg x13, x17, #16, #6 */
    "ef3bc1da",  /* autdza x15 */
    "6d4b003a",  /* setf16 w27 */
    "7e260a1d",  /* cpyprt [x30]!, [x10]!, x19! */
    "0f16dd19",  /* setpt [x15]!, x16!, x29 */
    "bf3a6dd9",  /* stzg sp, [x21, #3376] */
    "d7133019",  /* ldclrp x23, x16, [x30] */
    "9f09c1da",  /* pacda xzr, x12 */
    "e92fc1da",  /* pacdzb x9 */
    "a9361e19",  /* cpyfpt [x9]!, [x30]!, x21! */
    "ed3fc1da",  /* autdzb x13 */
    "e80b3f59",  /* rcwscas xzr, x8, [sp] */
    "9e05d91d",  /* setgp [x30]!, x12!, x25 */
    "b00fc1da",  /* pacdb x16, x29 */
    "ff47c1da",  /* xpacd xzr */
    "740a40f8",  /* ldtr x20, [x19] */
    "6d09003a",  /* setf8 w11 */
    "31740a1d",  /* cpyptwn [x17]!, [x10]!, x1! */
    "f4b36838",  /* rcwsetl x8, x20, [sp] */
    "06950c19",  /* cpyfpwtrn [x6]!, [x12]!, x8! */
    "ffa2b478",  /* rcwsswpa x20, xzr, [x23] */
    "e503c1da",  /* pacia x5, sp */
    "ff13c1da",  /* autia xzr, sp */
    "9f0b8038",  /* ldtrsb xzr, [x28] */
    "2d24d119",  /* setpn [x13]!, x1!, x17 */
    "4433a819",  /* ldsetpa x4, x8, [x26] */
    "68e7181d",  /* cpyprtn [x8]!, [x24]!, x27! */
    "fa0b80d9",  /* stlr x26, [sp, #-8]! */
    "e206a0f8",  /* ldrab x2, [x23] */
    "fe332f19",  /* ldsetp x30, x15, [sp] */
    "ad0a003a",  /* setf8 w21 */
    "ed2bc1da",  /* pacdza x13 */
    "df22c0da",  /* abs xzr, x22 */
    "ff1f1fd9",  /* gcssttr xzr, [sp] */
    "fc936519",  /* rcwclrpl x28, x5, [sp] */
    "ff1fc05a",  /* cnt wzr, wzr */
    "9e240919",  /* cpyfprt [x30]!, [x9]!, x4! */
    "1f53c291",  /* smax xzr, x24, #-108 */
    "1f8049d5",  /* sysp #1, c8, c0, #0 */
    "ffb37f59",  /* rcwssetpl xzr, xzr, [sp] */
    "e00f1fd7",  /* brab xzr, x0 */
    "e1b41c1d",  /* cpyptrn [x1]!, [x28]!, x7! */
    "fc720bd5",  /* trcit x28 */
    "f0038059",  /* ldapursh x16, [sp] */
    "e82fc1da",  /* pacdzb x8 */
    "5f0d3fd7",  /* blrab x10, sp */
    "d7104c59",  /* ldapurh w23, [x6, #193] */
    "7a0e6059",  /* rcwscaspl x0, x1, x26, x27, [x19] */
    "170b4538",  /* ldtrb w23, [x24, #80] */
    "5108ff59",  /* rcwscasal xzr, x17, [x2] */
    "ff0ec1da",  /* pacdb xzr, x23 */
    "8fb06959",  /* rcwssetpl x15, x9, [x4] */
    "f047c1da",  /* xpacd x16 */
    "c4d40e19",  /* cpyfpwtn [x4]!, [x14]!, x6! */
    "05770bd5",  /* gcspushm x5 */
    "880dbc19",  /* rcwcaspa x28, x29, x8, x9, [x12] */
    "ff01c1ba",  /* cmpp x15, x1 */
    "9f67c81a",  /* umax wzr, w28, w8 */
    "b333e219",  /* ldsetpal x19, x2, [x29] */
    "ffa37f19",  /* rcwswppl xzr, xzr, [sp] */
    "1e770bd5",  /* gcspushm x30 */
    "c337cd1d",  /* setgptn [x3]!, x30!, x13 */
    "ff23c1da",  /* paciza xzr */
    "e0e51c1d",  /* cpyprtn [x0]!, [x28]!, x15! */
    "980e7459",  /* rcwscaspl x20, x21, x24, x25, [x20] */
    "6791bf19",  /* rcwclrpa x7, xzr, [x11] */
    "350b7b59",  /* rcwscasl x27, x21, [x25] */
    "f20b40f8",  /* ldtr x18, [sp] */
    "9f235559",  /* ldapurh wzr, [x28, #-174] */
    "ff37c1da",  /* autizb xzr */
    "2894091d",  /* cpypwtrn [x8]!, [x9]!, x1! */
    "e1336b19",  /* ldsetpl x1, x11, [sp] */
    "fff8cb91",  /* smin xzr, x7, #-2 */
    "29a16078",  /* rcwsswpl x0, x9, [x9] */
    "4d2895b8",  /* ldtrsw x13, [x2, #-174] */
    "ec0fe659",  /* rcwscaspal x6, x7, x12, x13, [sp] */
    "aab27f78",  /* rcwssetl xzr, x10, [x21] */
    "77b40c1d",  /* cpyptrn [x23]!, [x12]!, x3! */
    "9fb17f78",  /* rcwssetl xzr, xzr, [x12] */
    "eb17c1da",  /* autib x11, sp */
    "d926c819",  /* setpn [x25]!, x22!, x8 */
    "5fa1ff38",  /* rcwswpal xzr, xzr, [x10] */
    "dba0ac38",  /* rcwswpa x12, x27, [x6] */
    "f40b8078",  /* ldtrsh x20, [sp] */
    "6c0fec19",  /* rcwcaspal x12, x13, x12, x13, [x27] */
    "f693f938",  /* rcwclral x25, x22, [sp] */
    "e908b419",  /* rcwcasa x20, x9, [x7] */
    "e40f1fd7",  /* brab xzr, x4 */
    "80f60519",  /* cpyfptn [x0]!, [x5]!, x20! */
    "ff0bc078",  /* ldtrsh wzr, [sp] */
    "ef93f959",  /* rcwsclrpal x15, x25, [sp] */
    "bba2bf38",  /* rcwswpa xzr, x27, [x21] */
    "6992ff78",  /* rcwsclral xzr, x9, [x19] */
    "fdb17a19",  /* rcwsetpl x29, x26, [x15] */
    "bf1d1fd9",  /* gcssttr xzr, [x13] */
    "5c750d19",  /* cpyfptwn [x28]!, [x13]!, x10! */
    "1f1e1fd9",  /* gcssttr xzr, [x16] */
    "eaa327f8",  /* st64bv0 x7, x10, [sp] */
    "a207d119",  /* setp [x2]!, x29!, x17 */
    "df93ff78",  /* rcwsclral xzr, xzr, [x30] */
    "0ad33ff8",  /* ld64b x10, [x24] */
    "e6b332f8",  /* st64bv x18, x6, [sp] */
    "1f03c0ba",  /* subps xzr, x24, x0 */
    "8210ea19",  /* ldclrpal x2, x10, [x4] */
    "d1008059",  /* ldapursh x17, [x6] */
    "50170a19",  /* cpyfpwt [x16]!, [x10]!, x26! */
    "71f70919",  /* cpyfptn [x17]!, [x9]!, x27! */
    "58937259",  /* rcwsclrpl x24, x18, [x26] */
    "58160619",  /* cpyfpwt [x24]!, [x6]!, x18! */
    "ffa30359",  /* stlurh wzr, [sp, #58] */
    "6d970a19",  /* cpyfpwtrn [x13]!, [x10]!, x27! */
    "fa23c05a",  /* abs w26, wzr */
    "640e3819",  /* rcwcasp x24, x25, x4, x5, [x19] */
    "a80ea459",  /* rcwscaspa x4, x5, x8, x9, [x21] */
    "68772bd5",  /* gcsss2 x8 */
    "d9770019",  /* cpyfptwn [x25]!, [x0]!, x30! */
    "7da0be38",  /* rcwswpa x30, x29, [x3] */
    "f0b37c78",  /* rcwssetl x28, x16, [sp] */
    "f8af7ed9",  /* stzg x24, [sp, #-352]! */
    "fe730bd5",  /* cpp rctx, x30 */
    "e50bc099",  /* ldapr w5, [sp], #4 */
    "ffb3ff59",  /* rcwssetpal xzr, xzr, [sp] */
    "ffb3bb78",  /* rcwsseta x27, xzr, [sp] */
    "5f1dc0da",  /* cnt xzr, x10 */
    "dab2bf59",  /* rcwssetpa x26, xzr, [x22] */
    "a09e68d4",  /* tcancel #17653 */
    "3a772bd5",  /* gcspopm x26 */
    "7bb1ff19",  /* rcwsetpal x27, xzr, [x11] */
    "5f93a878",  /* rcwsclra x8, xzr, [x26] */
    "6e03d7ba",  /* subps x14, x27, x23 */
    "0d48003a",  /* setf16 w0 */
    "7f772bd5",  /* gcsss2 xzr */
    "ff0f3fd6",  /* blrabz xzr */
    "da26cd19",  /* setpn [x26]!, x22!, x13 */
    "d535d91d",  /* setgptn [x21]!, x14!, x25 */
    "9f420119",  /* stlurb wzr, [x20, #20] */
    "f92fc1da",  /* pacdzb x25 */
    "6c82a719",  /* swppa x12, x7, [x19] */
    "3c160d19",  /* cpyfpwt [x28]!, [x13]!, x17! */
    "3fcb9838",  /* ldtrsb xzr, [x25, #-116] */
    "bdb0bf78",  /* rcwsseta xzr, x29, [x5] */
    "fa0f2419",  /* rcwcasp x4, x5, x26, x27, [sp] */
    "a8941a19",  /* cpyfpwtrn [x8]!, [x26]!, x5! */
    "e8a3e078",  /* rcwsswpal x0, x8, [sp] */
    "da0dc1da",  /* pacdb x26, x14 */
    "3ad7181d",  /* cpypwtn [x26]!, [x24]!, x25! */
    "8ab51d1d",  /* cpyptrn [x10]!, [x29]!, x12! */
    "1d34d819",  /* setptn [x29]!, x0!, x24 */
    "0d316c19",  /* ldsetpl x13, x12, [x8] */
    "2c0f2659",  /* rcwscasp x6, x7, x12, x13, [x25] */
    "1a92e878",  /* rcwsclral x8, x26, [x16] */
    "9f00a0d9",  /* stgm xzr, [x4] */
    "180da059",  /* rcwscaspa x0, x1, x24, x25, [x8] */
    "e30717ba",  /* rmif xzr, #46, #3 */
    "f1730bd5",  /* cpp rctx, x17 */
    "fbb3bf59",  /* rcwssetpa x27, xzr, [sp] */
    "dc14021d",  /* cpypwt [x28]!, [x2]!, x6! */
    "3d570c19",  /* cpyfpwtwn [x29]!, [x12]!, x25! */
    "7f10c1da",  /* autia xzr, x3 */
    "bf93bf19",  /* rcwclrpa xzr, xzr, [x29] */
    "ff932959",  /* rcwsclrp xzr, x9, [sp] */
    "d2f5051d",  /* cpyptn [x18]!, [x5]!, x14! */
    "e6b3ff19",  /* rcwsetpal x6, xzr, [sp] */
    "1607cf19",  /* setp [x22]!, x24!, x15 */
    "9f3303d5",  /* dsb #3 */
    "0b64e0d9",  /* stz2g x11, [x0], #96 */
    "ff0ba959",  /* rcwscasa x9, xzr, [sp] */
    "d9070e19",  /* cpyfp [x25]!, [x14]!, x30! */
    "0af61d19",  /* cpyfptn [x10]!, [x29]!, x16! */
    "2e241e1d",  /* cpyprt [x14]!, [x30]!, x1! */
    "f20abe19",  /* rcwcasa x30, x18, [x23] */
    "5f093fd6",  /* blraaz x10 */
    "db730bd5",  /* cosp rctx, x27 */
    "ff30bb91",  /* addg sp, x7, #944, #12 */
    "ffb108d5",  /* sys #0, c11, c1, #7 */
    "2e5810b8",  /* sttr w14, [x1, #-251] */
    "ff0bff59",  /* rcwscasal xzr, xzr, [sp] */
    "bf0d1fd9",  /* gcsstr xzr, [x13] */
    "a5a6141d",  /* cpyprtrn [x5]!, [x20]!, x21! */
    "ef8700ba",  /* rmif xzr, #1, #15 */
    "c9a4011d",  /* cpyprtrn [x9]!, [x1]!, x6! */
    "89332b19",  /* ldsetp x9, x11, [x28] */
    "bb0a3fd7",  /* blraa x21, x27 */
    "e027c1da",  /* pacizb x0 */
    "fd93fe38",  /* rcwclral x30, x29, [sp] */
    "5f0c1fd6",  /* brabz x2 */
    "e727c1da",  /* pacizb x7 */
    "c419a7d9",  /* st2g x4, [x14, #1808] */
    "44f50619",  /* cpyfptn [x4]!, [x6]!, x10! */
    "bf18c1da",  /* autda xzr, x5 */
    "5f093f59",  /* rcwscas xzr, xzr, [x10] */
    "74a33af8",  /* st64bv0 x26, x20, [x27] */
    "ab730bd5",  /* dvp rctx, x11 */
    "f5a1b538",  /* rcwswpa x21, x21, [x15] */
    "e603a0d9",  /* stgm x6, [sp] */
    "af451a1d",  /* cpypwn [x15]!, [x26]!, x13! */
    "5b67131d",  /* cpyprtwn [x27]!, [x19]!, x26! */
    "7f3023d5",  /* tstart xzr */
    "ab04df19",  /* setp [x11]!, x5!, xzr */
    "74051c19",  /* cpyfp [x20]!, [x28]!, x11! */
    "d5913f38",  /* rcwclr xzr, x21, [x14] */
    "2bb3ac59",  /* rcwssetpa x11, x12, [x25] */
    "04081db8",  /* sttr w4, [x0, #-48] */
    "3f19c1da",  /* autda xzr, x9 */
    "ac0fae59",  /* rcwscaspa x14, x15, x12, x13, [x29] */
    "f3250919",  /* cpyfprt [x19]!, [x9]!, x15! */
    "81f61c1d",  /* cpyptn [x1]!, [x28]!, x20! */
    "0b05a9d9",  /* st2g x11, [x8], #2304 */
    "b76bdf9a",  /* smin x23, x29, xzr */
    "2794161d",  /* cpypwtrn [x7]!, [x22]!, x1! */
    "64170d1d",  /* cpypwt [x4]!, [x13]!, x27! */
    "bfa07059",  /* rcwsswppl xzr, x16, [x5] */
    "ff0b7259",  /* rcwscasl x18, xzr, [sp] */
    "ffa2e559",  /* rcwsswppal xzr, x5, [x23] */
    "4715df1d",  /* setgpt [x7]!, x10!, xzr */
    "5f927f78",  /* rcwsclrl xzr, xzr, [x18] */
    "ffa3bf78",  /* rcwsswpa xzr, xzr, [sp] */
    "5f090038",  /* sttrb wzr, [x10] */
    "60271d19",  /* cpyfprt [x0]!, [x29]!, x27! */
    "04123019",  /* ldclrp x4, x16, [x16] */
    "4c16df1d",  /* setgpt [x12]!, x18!, xzr */
    "2aa0fa19",  /* rcwswppal x10, x26, [x1] */
    "24771a19",  /* cpyfptwn [x4]!, [x26]!, x25! */
    "9630b419",  /* ldsetpa x22, x20, [x4] */
    "1414df1d",  /* setgpt [x20]!, x0!, xzr */
    "ff0bc1da",  /* pacda xzr, sp */
    "b3db0e38",  /* sttrb w19, [x29, #237] */
    "0444111d",  /* cpypwn [x4]!, [x17]!, x0! */
    "5ef61c1d",  /* cpyptn [x30]!, [x28]!, x18! */
    "e1923f59",  /* rcwsclrp x1, xzr, [x23] */
    "ea10be19",  /* ldclrpa x10, x30, [x7] */
    "e1a37478",  /* rcwsswpl x20, x1, [sp] */
    "420d3fd7",  /* blrab x10, x2 */
    "eb87014d",  /* stl1 {v11.d}[1], [sp] */
    "ff93ff19",  /* rcwclrpal xzr, xzr, [sp] */
    "ff93ff38",  /* rcwclral xzr, xzr, [sp] */
    "1611c1da",  /* autia x22, x8 */
    "3f02c1da",  /* pacia xzr, x17 */
    "1f1dc1da",  /* autdb xzr, x8 */
    "df0b40b8",  /* ldtr wzr, [x30] */
    "6e341c19",  /* cpyfpt [x14]!, [x28]!, x3! */
    "e136c919",  /* setptn [x1]!, x23!, x9 */
    "ca730bd5",  /* cosp rctx, x10 */
    "ffa36b78",  /* rcwsswpl x11, xzr, [sp] */
    "ef0bfd19",  /* rcwcasal x29, x15, [sp] */
    "45b17378",  /* rcwssetl x19, x5, [x10] */
    "28b37f19",  /* rcwsetpl x8, xzr, [x25] */
    "f81fc0da",  /* cnt x24, xzr */
    "0fa60519",  /* cpyfprtrn [x15]!, [x5]!, x16! */
    "ac0bc038",  /* ldtrsb w12, [x29] */
    "df0a00b8",  /* sttr wzr, [x22] */
    "2500cf9a",  /* subp x5, x1, x15 */
    "1f0a8038",  /* ldtrsb xzr, [x16] */
    "2712b519",  /* ldclrpa x7, x21, [x17] */
    "f00f7619",  /* rcwcaspl x22, x23, x16, x17, [sp] */
    "1fa3b278",  /* rcwsswpa x18, xzr, [x24] */
    "e8ca41f8",  /* ldtr x8, [x23, #28] */
    "1fb2bf19",  /* rcwsetpa xzr, xzr, [x16] */
    "e913c1da",  /* autia x9, sp */
    "98760e1d",  /* cpyptwn [x24]!, [x14]!, x20! */
    "59a27f38",  /* rcwswpl xzr, x25, [x18] */
    "58570a19",  /* cpyfpwtwn [x24]!, [x10]!, x26! */
    "ff13c39a",  /* irg sp, sp, x3 */
    "3f3a03d5",  /* dsb ishnxs */
    "f7906d19",  /* rcwclrpl x23, x13, [x7] */
    "f893ff38",  /* rcwclral xzr, x24, [sp] */
    "1f014019",  /* ldapurb wzr, [x8] */
    "de86414d",  /* ldap1 {v30.d}[1], [x22] */
    "6e06c919",  /* setp [x14]!, x19!, x9 */
    "e8a3ff59",  /* rcwsswppal x8, xzr, [sp] */
    "f533c1da",  /* autiza x21 */
    "d40f6459",  /* rcwscaspl x4, x5, x20, x21, [x30] */
    "81730bd5",  /* cfp rctx, x1 */
    "6fc61b19",  /* cpyfpn [x15]!, [x27]!, x19! */
    "9ac6111d",  /* cpypn [x26]!, [x17]!, x20! */
    "160e2e59",  /* rcwscasp x14, x15, x22, x23, [x16] */
    "77c7051d",  /* cpypn [x23]!, [x5]!, x27! */
    "3c36c219",  /* setptn [x28]!, x17!, x2 */
    "f5a3f138",  /* rcwswpal x17, x21, [sp] */
    "8fb0ff19",  /* rcwsetpal x15, xzr, [x4] */
    "1f0b1fd7",  /* braa x24, sp */
    "0146131d",  /* cpypwn [x1]!, [x19]!, x16! */
    "4735d61d",  /* setgptn [x7]!, x10!, x22 */
    "bd91bb19",  /* rcwclrpa x29, x27, [x13] */
    "c2a5101d",  /* cpyprtrn [x2]!, [x16]!, x14! */
    "f4a36838",  /* rcwswpl x8, x20, [sp] */
    "89340519",  /* cpyfpt [x9]!, [x5]!, x4! */
    "e0a33f38",  /* rcwswp xzr, x0, [sp] */
    "b525df1d",  /* setgpn [x21]!, x13!, xzr */
    "b0c50619",  /* cpyfpn [x16]!, [x6]!, x13! */
    "e00320d9",  /* stzgm x0, [sp] */
    "08840b1d",  /* cpyprn [x8]!, [x11]!, x0! */
    "1fa37f59",  /* rcwsswppl xzr, xzr, [x24] */
    "ffb3bd19",  /* rcwsetpa xzr, x29, [sp] */
    "ffb3f938",  /* rcwsetal x25, xzr, [sp] */
    "d9907f78",  /* rcwsclrl xzr, x25, [x6] */
    "f2850d1d",  /* cpyprn [x18]!, [x13]!, x15! */
    "5f0b3fd6",  /* blraaz x26 */
    "d3a3fb19",  /* rcwswppal x19, x27, [x30] */
    "1c0ca459",  /* rcwscaspa x4, x5, x28, x29, [x0] */
    "a1771b19",  /* cpyfptwn [x1]!, [x27]!, x29! */
    "e4136019",  /* ldclrpl x4, x0, [sp] */
    "ff922f19",  /* rcwclrp xzr, x15, [x23] */
    "82a13f59",  /* rcwsswpp x2, xzr, [x12] */
    "84a71e1d",  /* cpyprtrn [x4]!, [x30]!, x28! */
    "f5932719",  /* rcwclrp x21, x7, [sp] */
    "1e01c89a",  /* subp x30, x8, x8 */
    "4903df9a",  /* subp x9, x26, sp */
    "02d33ff8",  /* ld64b x2, [x24] */
    "57b40f19",  /* cpyfptrn [x23]!, [x15]!, x2! */
    "ea37c1da",  /* autizb x10 */
    "f4a0ec19",  /* rcwswppal x20, x12, [x7] */
    "eb936059",  /* rcwsclrpl x11, x0, [sp] */
    "4d133019",  /* ldclrp x13, x16, [x26] */
    "f4b61619",  /* cpyfptrn [x20]!, [x22]!, x23! */
    "97c51c19",  /* cpyfpn [x23]!, [x28]!, x12! */
    "ac730bd5",  /* dvp rctx, x12 */
    "fd0bbf19",  /* rcwcasa xzr, x29, [sp] */
    "5205df1d",  /* setgp [x18]!, x10!, xzr */
    "df03df59",  /* ldapursh wzr, [x30, #-16] */
    "753123d5",  /* ttest x21 */
    "feb33c19",  /* rcwsetp x30, x28, [sp] */
    "d291bf19",  /* rcwclrpa x18, xzr, [x14] */
    "ecb0f959",  /* rcwssetpal x12, x25, [x7] */
    "8a937f19",  /* rcwclrpl x10, xzr, [x28] */
    "0f93ff19",  /* rcwclrpal x15, xzr, [x24] */
    "94936e38",  /* rcwclrl x14, x20, [x28] */
    "c516df1d",  /* setgpt [x5]!, x22!, xzr */
    "01097019",  /* rcwcasl x16, x1, [x8] */
    "eb37c1da",  /* autizb x11 */
    "4983a319",  /* swppa x9, x3, [x26] */
    "ff9b5538",  /* ldtrb wzr, [sp, #-167] */
    "f116c1da",  /* autib x17, x23 */
    "4601a0d9",  /* stgm x6, [x10] */
    "3f0d1fd9",  /* gcsstr xzr, [x9] */
    "51730f19",  /* stlurb w17, [x26, #247] */
    "a033e819",  /* ldsetpal x0, x8, [x29] */
    "3e1003d5",  /* wfit x30 */
    "8ca06519",  /* rcwswppl x12, x5, [x4] */
    "ef13ab19",  /* ldclrpa x15, x11, [sp] */
    "3f3a03d5",  /* dsb #24 */
    "b90b80b8",  /* ldtrsw x25, [x29] */
    "940260d9",  /* ldg x20, [x20] */
    "3f0b3fd6",  /* blraaz x25 */
    "79b3e659",  /* rcwssetpal x25, x6, [x27] */
    "ea0b2a19",  /* rcwcas x10, x10, [sp] */
    "3886410d",  /* ldap1 {v24.d}[0], [x17] */
    "f986410d",  /* ldap1 {v25.d}[0], [x23] */
    "e80bf119",  /* rcwcasal x17, x8, [sp] */
    "ff93e159",  /* rcwsclrpal xzr, x1, [sp] */
    "27a61c19",  /* cpyfprtrn [x7]!, [x28]!, x17! */
    "fd37c1da",  /* autizb x29 */
    "9f084078",  /* ldtrh wzr, [x4] */
    "484029d5",  /* sysl x8, #1, c4, c0, #2 */
    "f9d38199",  /* ldapursw x25, [sp, #29] */
    "500f6c19",  /* rcwcaspl x12, x13, x16, x17, [x26] */
    "a4950a1d",  /* cpypwtrn [x4]!, [x10]!, x13! */
    "d70a3e19",  /* rcwcas x30, x23, [x22] */
    "92932f19",  /* rcwclrp x18, x15, [x28] */
    "03a41819",  /* cpyfprtrn [x3]!, [x24]!, x0! */
    "1fb1ff19",  /* rcwsetpal xzr, xzr, [x8] */
    "fd0be219",  /* rcwcasal x2, x29, [sp] */
    "4a04c61d",  /* setgp [x10]!, x2!, x6 */
    "bf13df9a",  /* irg sp, x29 */
    "efb3bc78",  /* rcwsseta x28, x15, [sp] */
    "1f0c1fd6",  /* brabz x0 */
    "c2819519",  /* ldapursb x2, [x14, #-168] */
    "f00fbc19",  /* rcwcaspa x28, x29, x16, x17, [sp] */
    "43a16419",  /* rcwswppl x3, x4, [x10] */
    "56a32938",  /* rcwswp x9, x22, [x26] */
    "dfa94278",  /* ldtrh wzr, [x14, #42] */
    "ed34cc19",  /* setptn [x13]!, x7!, x12 */
    "100f3659",  /* rcwscasp x22, x23, x16, x17, [x24] */
    "f3937a38",  /* rcwclrl x26, x19, [sp] */
    "911fc1da",  /* autdb x17, x28 */
    "311003d5",  /* wfit x17 */
    "ffe12bd5",  /* sysl xzr, #3, c14, c1, #7 */
    "24807619",  /* swppl x4, x22, [x1] */
    "21851c19",  /* cpyfprn [x1]!, [x28]!, x9! */
    "71841219",  /* cpyfprn [x17]!, [x18]!, x3! */
    "cc27d71d",  /* setgpn [x12]!, x30!, x23 */
    "af461119",  /* cpyfpwn [x15]!, [x17]!, x21! */
    "9fb0e178",  /* rcwssetal x1, xzr, [x4] */
    "430a6b19",  /* rcwcasl x11, x3, [x18] */
    "f01fc1da",  /* autdb x16, sp */
    "2fb61219",  /* cpyfptrn [x15]!, [x18]!, x17! */
    "1966131d",  /* cpyprtwn [x25]!, [x19]!, x16! */
    "70933ff8",  /* st64b x16, [x27] */
    "1a0d2e59",  /* rcwscasp x14, x15, x26, x27, [x8] */
    "2bf4171d",  /* cpyptn [x11]!, [x23]!, x1! */
    "ff3bc1da",  /* autdza xzr */
    "ee730bd5",  /* cpp rctx, x14 */
    "ff030019",  /* stlurb wzr, [sp] */
    "ff00d5ba",  /* cmpp x7, x21 */
    "c7370c1d",  /* cpypt [x7]!, [x12]!, x30! */
    "eeb3f138",  /* rcwsetal x17, x14, [sp] */
    "c933ba19",  /* ldsetpa x9, x26, [x30] */
    "eb837619",  /* swppl x11, x22, [sp] */
    "9a0b3fd7",  /* blraa x28, x26 */
    "52903ff8",  /* st64b x18, [x2] */
    "09927b38",  /* rcwclrl x27, x9, [x16] */
    "1935001d",  /* cpypt [x25]!, [x0]!, x8! */
    "fc33f419",  /* ldsetpal x28, x20, [sp] */
    "dd641b1d",  /* cpyprtwn [x29]!, [x27]!, x6! */
    "e60f6419",  /* rcwcaspl x4, x5, x6, x7, [sp] */
    "e8a1f438",  /* rcwswpal x20, x8, [x15] */
    "fe836d19",  /* swppl x30, x13, [sp] */
    "cdd80838",  /* sttrb w13, [x6, #141] */
    "e883ba19",  /* swppa x8, x26, [sp] */
    "e4137819",  /* ldclrpl x4, x24, [sp] */
    "5f03dfba",  /* cmpp x26, sp */
    "bb25d11d",  /* setgpn [x27]!, x13!, x17 */
    "ffa36219",  /* rcwswppl xzr, x2, [sp] */
    "f23bc1da",  /* autdza x18 */
    "a609e0d9",  /* stz2g x6, [x13] */
    "2bc61919",  /* cpyfpn [x11]!, [x25]!, x17! */
    "e2937459",  /* rcwsclrpl x2, x20, [sp] */
    "02871019",  /* cpyfprn [x2]!, [x16]!, x24! */
    "eab3bf78",  /* rcwsseta xzr, x10, [sp] */
    "f8a33a38",  /* rcwswp x26, x24, [sp] */
    "ed337d19",  /* ldsetpl x13, x29, [sp] */
    "f833c1da",  /* autiza x24 */
    "de861a19",  /* cpyfprn [x30]!, [x26]!, x22! */
    "9bb25319",  /* ldapurb w27, [x20, #-197] */
    "2fb61819",  /* cpyfptrn [x15]!, [x24]!, x17! */
    "c1461919",  /* cpyfpwn [x1]!, [x25]!, x22! */
    "d4822f19",  /* swpp x20, x15, [x22] */
    "e6030019",  /* stlurb w6, [sp] */
    "450360d9",  /* ldg x5, [x26] */
    "70097f59",  /* rcwscasl xzr, x16, [x11] */
    "82a13359",  /* rcwsswpp x2, x19, [x12] */
    "7bd428d5",  /* sysl x27, #0, c13, c4, #3 */
    "4c014059",  /* ldapurh w12, [x10] */
    "e1429f99",  /* ldapursw x1, [x23, #-12] */
    "b9823119",  /* swpp x25, x17, [x21] */
    "6a772bd5",  /* gcsss2 x10 */
    "34b020f8",  /* st64bv x0, x20, [x1] */
    "46b03ff8",  /* st64bv xzr, x6, [x2] */
    "fd33fd19",  /* ldsetpal x29, x29, [sp] */
    "df014019",  /* ldapurb wzr, [x14] */
    "aea3e338",  /* rcwswpal x3, x14, [x29] */
    "9ffa4d38",  /* ldtrb wzr, [x20, #223] */
    "e9030059",  /* stlurh w9, [sp] */
    "ff030059",  /* stlurh wzr, [sp] */
    "00137919",  /* ldclrpl x0, x25, [x24] */
    "16923ff8",  /* st64b x22, [x16] */
    "ff2b5078",  /* ldtrh wzr, [sp, #-254] */
    "7f080078",  /* sttrh wzr, [x3] */
    "2881ba19",  /* swppa x8, x26, [x9] */
    "ff0b0078",  /* sttrh wzr, [sp] */
    "2c954ed5",  /* sysp #6, c9, c5, #1, x12 */
    "3fdb1778",  /* sttrh wzr, [x25, #-131] */
    "32980878",  /* sttrh w18, [x1, #137] */
    "ac954dd5",  /* sysp #5, c9, c5, #5, x12 */
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
