/* flags: --c-std=gnu17 */
/* native-only */
/* The AArch64 assembler's remaining SVE and SVE2 forms against a table: one line of every shape not in asm_table_a64_sve.c that the architecture's SVE encoding space disassembles to (the gathers, scatters, first-fault, non-fault, non-temporal, replicating and structure loads and stores and the prefetches in every addressing mode; adr; the dot products, matrix multiplies and BF16 forms; the SVE2 widening, narrowing, saturating, pairwise, complex, bit-permute, polynomial, crypto and ternary forms; cpy and the mov aliases), each with the bytes its encoding has; assembled as file-scope asm and compared. */
#include "host.h"

#ifdef __AXIOM_ASM_AARCH64__
__asm__(".section .rodata.axiom_asm_table, \"a\"\n"
        ".globl axiom_asm_table\n"
        "axiom_asm_table:\n"
        "ldff1sh {z4.d}, p2/z, [x16, x9, lsl #1]\n"
        "ld1w {z0.d}, p3/z, [sp, z3.d, lsl #2]\n"
        "ldff1b {z10.d}, p4/z, [z26.d, #25]\n"
        "stnt1d {z20.d}, p3, [x16, #4, mul vl]\n"
        "sqdmlslbt z11.s, z9.h, z4.h\n"
        "ldnt1h {z23.h}, p1/z, [x30, #-5, mul vl]\n"
        "ld1w {z31.d}, p6/z, [x22, z14.d, sxtw #2]\n"
        "st1d {z22.d}, p1, [x11, z10.d, uxtw #3]\n"
        "ldff1w {z18.d}, p3/z, [x29, z3.d]\n"
        "uabdlt z11.d, z0.s, z18.s\n"
        "ldff1sh {z2.s}, p7/z, [x10, z12.s, uxtw #1]\n"
        "sqdmulh z25.h, z21.h, z14.h\n"
        "st1w {z23.d}, p0, [x26, z21.d, sxtw #2]\n"
        "ldff1h {z0.s}, p7/z, [x24, z6.s, uxtw #1]\n"
        "match p15.b, p7/z, z20.b, z0.b\n"
        "ldff1b {z13.b}, p6/z, [x26, x24]\n"
        "ld4b {z9.b-z12.b}, p1/z, [x5, x10]\n"
        "histcnt z30.d, p5/z, z19.d, z7.d\n"
        "ld2h {z22.h-z23.h}, p0/z, [x26, #-2, mul vl]\n"
        "ld1b {z18.d}, p4/z, [z21.d, #26]\n"
        "ldff1w {z19.d}, p5/z, [x12, z21.d, lsl #2]\n"
        "ldff1h {z28.s}, p0/z, [z24.s, #58]\n"
        "ld1rqd {z8.d}, p2/z, [x22, #80]\n"
        "addp z27.s, p1/m, z27.s, z16.s\n"
        "ld1roh {z19.h}, p7/z, [x14, #-64]\n"
        "ld1d {z2.d}, p3/z, [x18, z14.d, lsl #3]\n"
        "ldff1sw {z7.d}, p1/z, [x0, z22.d]\n"
        "ldff1sh {z7.s}, p1/z, [x16, z13.s, sxtw #1]\n"
        "ldnt1h {z6.d}, p1/z, [z9.d, x15]\n"
        "umlslb z7.d, z15.s, z13.s[2]\n"
        "st1b {z10.d}, p4, [x4, z16.d]\n"
        "st2b {z3.b-z4.b}, p0, [x26, x4]\n"
        "ld1rb {z15.h}, p1/z, [x28, #1]\n"
        "ldff1sw {z31.d}, p5/z, [z28.d, #72]\n"
        "ldff1sh {z22.d}, p7/z, [x23, z24.d, lsl #1]\n"
        "ldnt1sh {z22.s}, p4/z, [z16.s, x18]\n"
        "smlslt z24.h, z13.b, z3.b\n"
        "ld1h {z26.d}, p5/z, [x0, z8.d, sxtw #1]\n"
        "ld1h {z4.d}, p4/z, [x1, z16.d, uxtw #1]\n"
        "st1b {z11.s}, p4, [x29, z27.s, sxtw]\n"
        "ldff1w {z31.d}, p5/z, [x27, z29.d, sxtw]\n"
        "ldff1h {z2.s}, p7/z, [x12, x6, lsl #1]\n"
        "cdot z24.s, z4.b, z1.b[3], #0\n"
        "ldnt1b {z8.b}, p0/z, [x7, x25]\n"
        "st3d {z1.d-z3.d}, p2, [x0, x25, lsl #3]\n"
        "ld1rd {z17.d}, p2/z, [sp, #32]\n"
        "ld1d {z12.d}, p6/z, [x12, z10.d, uxtw #3]\n"
        "ld1sw {z5.d}, p7/z, [x10, z14.d]\n"
        "ldnf1sb {z20.s}, p0/z, [x24, #-8, mul vl]\n"
        "clasta d21, p0, d21, z4.d\n"
        "ld1sh {z13.s}, p5/z, [z14.s, #18]\n"
        "sqshrunt z10.s, z13.d, #30\n"
        "rshrnt z4.h, z9.s, #15\n"
        "ldnt1h {z2.h}, p0/z, [x18, x7, lsl #1]\n"
        "stnt1h {z29.s}, p1, [z22.s, x9]\n"
        "ld1rqd {z19.d}, p6/z, [x20, x11, lsl #3]\n"
        "ursra z7.b, z1.b, #2\n"
        "ld1rsh {z5.d}, p0/z, [x3, #56]\n"
        "st4h {z26.h-z29.h}, p6, [x30, x16, lsl #1]\n"
        "ld1sh {z15.s}, p1/z, [sp, z29.s, sxtw #1]\n"
        "sqrdmlah z9.h, z19.h, z1.h[5]\n"
        "eor z10.h, z10.h, #0x1e00\n"
        "ldff1d {z19.d}, p2/z, [x13, z18.d, sxtw #3]\n"
        "prfb #14, p3, [z10.s, #21]\n"
        "st1h {z24.s}, p5, [x18, z28.s, uxtw]\n"
        "ld1b {z2.d}, p0/z, [x8, z31.d, sxtw]\n"
        "ld1sh {z11.d}, p7/z, [x12, z26.d]\n"
        "ldnt1b {z24.d}, p0/z, [z19.d, xzr]\n"
        "ssubwb z4.h, z14.h, z15.b\n"
        "ld1rw {z30.s}, p4/z, [x1, #216]\n"
        "sqrdcmlah z23.h, z20.h, z24.h, #0\n"
        "eor z2.d, z2.d, #0xfffffff803ffffff\n"
        "smlslt z18.s, z23.h, z6.h[4]\n"
        "fmov z18.d, p1/m, #-2.125000000000000000e+00\n"
        "st1h {z15.s}, p0, [x29, z6.s, uxtw #1]\n"
        "ssubwt z6.s, z30.s, z12.h\n"
        "st1h {z11.s}, p0, [x15, z1.s, sxtw #1]\n"
        "ld1sb {z25.d}, p0/z, [x19, z0.d]\n"
        "ld1w {z20.d}, p5/z, [x10, z8.d, lsl #2]\n"
        "ld1rsb {z0.h}, p5/z, [x19, #32]\n"
        "st1h {z6.s}, p3, [x26, z17.s, sxtw]\n"
        "ldff1w {z19.d}, p5/z, [x16, x14, lsl #2]\n"
        "ld1b {z22.d}, p3/z, [x24, z10.d]\n"
        "prfb pldl2keep, p4, [z14.s, #12]\n"
        "st1w {z21.s}, p1, [z23.s, #100]\n"
        "usubwt z19.h, z27.h, z4.b\n"
        "ld1sh {z22.d}, p4/z, [x10, z18.d, uxtw]\n"
        "st4w {z12.s-z15.s}, p1, [x14, x4, lsl #2]\n"
        "mov z3.s, p2/z, #15104\n"
        "st1w {z21.s}, p3, [x27, z6.s, uxtw]\n"
        "st4h {z1.h-z4.h}, p5, [x21, #16, mul vl]\n"
        "ld1sh {z25.d}, p7/z, [x12, z23.d, lsl #1]\n"
        "prfd pldl1keep, p3, [x28, z2.d, uxtw #3]\n"
        "ld2d {z30.d-z31.d}, p6/z, [x13, x16, lsl #3]\n"
        "ld1rd {z14.d}, p7/z, [x18, #496]\n"
        "uqincd w10, #24, mul #15\n"
        "ldff1w {z0.d}, p6/z, [x9, z7.d, sxtw #2]\n"
        "mov z25.h, p11/m, z28.h\n"
        "ld1sh {z8.s}, p1/z, [x10, z27.s, sxtw #1]\n"
        "ldff1sw {z16.d}, p1/z, [x22, z14.d, uxtw]\n"
        "ldff1b {z24.d}, p1/z, [x5, z2.d, sxtw]\n"
        "stnt1d {z31.d}, p3, [z18.d, x0]\n"
        "st1w {z24.d}, p4, [x5, z26.d, lsl #2]\n"
        "ext z26.b, {z24.b-z25.b}, #74\n"
        "sqdecb x9, #18, mul #2\n"
        "ld1sh {z16.s}, p3/z, [x26, z22.s, uxtw #1]\n"
        "saddwb z19.d, z20.d, z1.s\n"
        "prfb #14, p5, [x19, #5, mul vl]\n"
        "ld1rh {z8.d}, p4/z, [x19, #6]\n"
        "st1b {z26.s}, p2, [z2.s, #17]\n"
        "ld1w {z0.s}, p1/z, [x24, z27.s, uxtw #2]\n"
        "ld1w {z0.d}, p7/z, [x28, z4.d]\n"
        "prfd pldl3keep, p5, [x13, #-20, mul vl]\n"
        "ld2b {z31.b-z0.b}, p3/z, [x16, #8, mul vl]\n"
        "ldnf1w {z25.d}, p6/z, [x10, #-5, mul vl]\n"
        "ld3h {z23.h-z25.h}, p2/z, [x15]\n"
        "ldff1sb {z15.d}, p3/z, [x1, z18.d, uxtw]\n"
        "urecpe z21.s, p4/m, z30.s\n"
        "ldff1b {z29.d}, p3/z, [x28, z16.d]\n"
        "st1d {z28.d}, p5, [x7, z29.d, uxtw]\n"
        "ld4h {z27.h-z30.h}, p2/z, [x10, x7, lsl #1]\n"
        "ldnf1d {z24.d}, p7/z, [x10, #-1, mul vl]\n"
        "ldnf1h {z4.h}, p0/z, [x26, #7, mul vl]\n"
        "prfh pstl2keep, p4, [x1, #-11, mul vl]\n"
        "cmla z16.h, z31.h, z5.h[0], #90\n"
        "sminp z24.d, p4/m, z24.d, z11.d\n"
        "prfd pstl3keep, p5, [x30, #-24, mul vl]\n"
        "prfb pstl1keep, p0, [x11, #-10, mul vl]\n"
        "ldff1b {z28.d}, p2/z, [x3, z13.d, uxtw]\n"
        "stnt1w {z0.d}, p5, [z27.d, x5]\n"
        "ldff1d {z6.d}, p1/z, [z24.d, #56]\n"
        "smullb z24.d, z31.s, z6.s[3]\n"
        "smlslb z30.h, z6.b, z13.b\n"
        "usublt z28.h, z20.b, z31.b\n"
        "ldff1h {z3.d}, p1/z, [x5, z15.d]\n"
        "st4h {z29.h-z0.h}, p0, [sp, #-4, mul vl]\n"
        "prfh pldl3strm, p0, [x16, z19.d, uxtw #1]\n"
        "usra z9.d, z20.d, #62\n"
        "ld1sh {z28.s}, p2/z, [x27, z6.s, sxtw]\n"
        "st1d {z23.d}, p7, [x9, z21.d, sxtw]\n"
        "stnt1b {z27.d}, p6, [z7.d, x29]\n"
        "ldnt1sw {z21.d}, p0/z, [z22.d, x4]\n"
        "ld1sw {z25.d}, p6/z, [x9, z10.d, sxtw #2]\n"
        "ld1sh {z25.d}, p4/z, [sp, z3.d, lsl #1]\n"
        "ldff1sh {z10.d}, p1/z, [x23, z9.d, sxtw]\n"
        "prfd #7, p1, [x17, z5.d, sxtw #3]\n"
        "ld1rqh {z9.h}, p5/z, [x8, x17, lsl #1]\n"
        "stnt1b {z24.b}, p7, [x26, #7, mul vl]\n"
        "ld1d {z10.d}, p3/z, [x18, z10.d, sxtw #3]\n"
        "ldnt1d {z27.d}, p2/z, [x27, #-8, mul vl]\n"
        "ldff1d {z30.d}, p3/z, [x20, z29.d, lsl #3]\n"
        "sshllb z0.d, z21.s, #1\n"
        "prfb pstl3strm, p2, [x8, z19.d, sxtw]\n"
        "umaxp z17.d, p2/m, z17.d, z13.d\n"
        "sri z0.d, z1.d, #15\n"
        "ldnt1w {z3.s}, p7/z, [z5.s, x9]\n"
        "stnt1h {z30.h}, p0, [x2, x29, lsl #1]\n"
        "prfw pstl2keep, p2, [z23.d, #120]\n"
        "ld2w {z2.s-z3.s}, p0/z, [x7, #8, mul vl]\n"
        "ld1h {z28.s}, p2/z, [z19.s, #10]\n"
        "sqrdmulh z20.d, z20.d, z28.d\n"
        "ld1sb {z19.s}, p3/z, [x4, z12.s, sxtw]\n"
        "ldff1sh {z30.d}, p2/z, [z18.d, #62]\n"
        "ldff1sw {z21.d}, p1/z, [x18, z29.d, sxtw]\n"
        "ld1b {z3.s}, p5/z, [x2, z4.s, uxtw]\n"
        "st3w {z15.s-z17.s}, p2, [x8, #18, mul vl]\n"
        "ldnt1d {z6.d}, p1/z, [x25, x11, lsl #3]\n"
        "ldnt1w {z5.s}, p1/z, [x0, x14, lsl #2]\n"
        "ldff1h {z16.s}, p3/z, [x27, z27.s, sxtw #1]\n"
        "ldff1sh {z25.d}, p4/z, [x14, z24.d, uxtw]\n"
        "ldnt1h {z2.s}, p3/z, [z27.s, xzr]\n"
        "prfh pldl2strm, p4, [x12, #17, mul vl]\n"
        "saddlbt z25.h, z7.b, z10.b\n"
        "ld1h {z21.d}, p5/z, [x27, z18.d, uxtw]\n"
        "sabdlt z19.h, z5.b, z10.b\n"
        "st1w {z23.d}, p7, [x14, z1.d, uxtw #2]\n"
        "sudot z20.s, z8.b, z2.b[1]\n"
        "st3d {z12.d-z14.d}, p7, [x9, #-9, mul vl]\n"
        "st1h {z31.d}, p0, [x7, z11.d]\n"
        "ldff1d {z27.d}, p4/z, [x18, z2.d, uxtw]\n"
        "sqdecd z13.d, #18, mul #7\n"
        "ldnt1b {z26.s}, p7/z, [z1.s, x11]\n"
        "st1w {z16.d}, p2, [x15, z27.d]\n"
        "ld1w {z27.d}, p4/z, [z0.d, #20]\n"
        "st1w {z30.s}, p6, [x29, z3.s, sxtw]\n"
        "ld2w {z13.s-z14.s}, p5/z, [x2, x20, lsl #2]\n"
        "ld1sw {z11.d}, p7/z, [x12, z11.d, uxtw]\n"
        "ldff1d {z8.d}, p6/z, [z14.d]\n"
        "ldff1w {z29.s}, p1/z, [z1.s, #64]\n"
        "sqdmullb z30.d, z6.s, z3.s[3]\n"
        "fcmla z31.s, z30.s, z1.s[0], #90\n"
        "umullt z0.s, z1.h, z5.h[6]\n"
        "ld1sb {z8.s}, p4/z, [z11.s, #15]\n"
        "ld1d {z18.d}, p3/z, [x24, z17.d]\n"
        "st2b {z13.b-z14.b}, p0, [x2, #4, mul vl]\n"
        "st1d {z1.d}, p0, [x1, z27.d]\n"
        "ldff1w {z2.d}, p0/z, [x21, z19.d, uxtw]\n"
        "prfb pldl1strm, p1, [x15, z5.d]\n"
        "saba z23.h, z24.h, z4.h\n"
        "stnt1d {z30.d}, p1, [x29, x23, lsl #3]\n"
        "ldff1w {z6.d}, p3/z, [x25, z23.d, uxtw #2]\n"
        "umlslb z10.s, z15.h, z22.h\n"
        "ld1w {z6.d}, p3/z, [x30, z0.d, uxtw]\n"
        "usubwb z29.d, z28.d, z15.s\n"
        "uqdech x30, #28, mul #8\n"
        "prfd pstl1strm, p0, [x17, z15.s, sxtw #3]\n"
        "st1h {z14.d}, p2, [x12, z23.d, lsl #1]\n"
        "umullb z24.s, z20.h, z1.h[3]\n"
        "sqinch x5, w5, #14, mul #3\n"
        "ssublt z15.s, z28.h, z18.h\n"
        "ld1h {z6.d}, p1/z, [x13, z13.d, sxtw]\n"
        "ldff1sb {z9.d}, p5/z, [x21, z19.d]\n"
        "eorbt z1.s, z11.s, z10.s\n"
        "prfd pldl3keep, p1, [x8, z8.s, sxtw #3]\n"
        "ldff1sw {z6.d}, p0/z, [x8, z27.d, uxtw #2]\n"
        "smullt z11.d, z26.s, z8.s[0]\n"
        "ldnf1sw {z2.d}, p0/z, [x30, #3, mul vl]\n"
        "ld1roh {z14.h}, p3/z, [x27, x29, lsl #1]\n"
        "ldff1h {z20.d}, p5/z, [sp, z9.d, sxtw #1]\n"
        "ldff1sw {z17.d}, p4/z, [x27, x6, lsl #2]\n"
        "sqdmlalt z24.s, z17.h, z25.h\n"
        "adr z4.s, [z27.s, z15.s, lsl #3]\n"
        "ldff1sb {z7.h}, p3/z, [x7, x17]\n"
        "ldff1sw {z9.d}, p7/z, [x22, z17.d, lsl #2]\n"
        "ld1row {z18.s}, p0/z, [x10, #128]\n"
        "ldnt1d {z29.d}, p6/z, [z18.d, x21]\n"
        "prfw pstl3strm, p5, [x2, x0, lsl #2]\n"
        "sqrshrnb z28.s, z15.d, #25\n"
        "ld1w {z10.s}, p3/z, [x21, z12.s, sxtw]\n"
        "ld1h {z14.d}, p6/z, [x9, z10.d]\n"
        "ldff1h {z29.d}, p5/z, [x17, z9.d, uxtw]\n"
        "ld1sb {z18.d}, p1/z, [x9, z16.d, uxtw]\n"
        "ld1h {z29.d}, p2/z, [x2, z16.d, lsl #1]\n"
        "smlslb z3.d, z23.s, z10.s[1]\n"
        "ld1rqw {z17.s}, p0/z, [sp, x26, lsl #2]\n"
        "saddlb z16.h, z7.b, z24.b\n"
        "adr z31.d, [z0.d, z17.d, sxtw #1]\n"
        "prfh pstl3keep, p7, [x29, x1, lsl #1]\n"
        "sqdmullt z10.d, z10.s, z19.s\n"
        "pmullb z10.d, z18.s, z19.s\n"
        "st1b {z28.d}, p5, [x19, z29.d, uxtw]\n"
        "ld1rqw {z26.s}, p5/z, [x25, x23, lsl #2]\n"
        "ldff1sb {z4.s}, p5/z, [z30.s, #27]\n"
        "srhadd z9.d, p4/m, z9.d, z25.d\n"
        "sqdmlslb z2.d, z9.s, z23.s\n"
        "uqincw z29.s, #20\n"
        "orr z22.d, z22.d, #0x3ffc0000\n"
        "ld1rqb {z18.b}, p0/z, [x29, #80]\n"
        "eor z26.d, z26.d, #0xfffffffffffff003\n"
        "ld1sw {z5.d}, p4/z, [x28, z15.d, sxtw]\n"
        "prfw pstl1strm, p6, [x13, z7.d, lsl #2]\n"
        "ldff1d {z29.d}, p3/z, [x17, z30.d]\n"
        "prfd pstl3strm, p2, [x11, z5.d, uxtw #3]\n"
        "usublb z10.d, z30.s, z19.s\n"
        "smlalt z20.d, z10.s, z0.s\n"
        "ld3d {z29.d-z31.d}, p0/z, [x11, #-6, mul vl]\n"
        "ldff1d {z29.d}, p1/z, [sp, z4.d, sxtw #3]\n"
        "ld1sw {z24.d}, p5/z, [x28, z4.d, lsl #2]\n"
        "prfd pldl3strm, p2, [x16, z17.s, sxtw #3]\n"
        "mov z18.h, p12/m, #-3328\n"
        "sabalb z0.h, z18.b, z24.b\n"
        "ldff1sb {z31.s}, p2/z, [x17, z18.s, sxtw]\n"
        "sqshrnb z11.s, z7.d, #19\n"
        "xar z4.d, z4.d, z9.d, #44\n"
        "smlalt z21.s, z24.h, z1.h[5]\n"
        "st3w {z6.s-z8.s}, p3, [sp, #3, mul vl]\n"
        "ldff1sw {z15.d}, p3/z, [x17, z3.d, sxtw #2]\n"
        "ld1rqw {z29.s}, p1/z, [x19, #-32]\n"
        "umullb z29.d, z20.s, z25.s\n"
        "stnt1b {z17.b}, p5, [x2, x25]\n"
        "ldff1sh {z6.d}, p7/z, [sp, z24.d, uxtw #1]\n"
        "sabdlb z12.d, z4.s, z25.s\n"
        "nmatch p2.h, p6/z, z15.h, z13.h\n"
        "cdot z3.s, z28.b, z14.b, #0\n"
        "ldff1h {z20.d}, p6/z, [x17, z4.d, lsl #1]\n"
        "prfd #6, p2, [x22, #28, mul vl]\n"
        "fminnmp z19.h, p1/m, z19.h, z19.h\n"
        "prfh pldl1strm, p1, [sp, x0, lsl #1]\n"
        "sqrshl z29.h, p5/m, z29.h, z16.h\n"
        "sqrdmlsh z10.h, z6.h, z14.h\n"
        "fmlalb z21.s, z2.h, z20.h\n"
        "uhsubr z6.b, p3/m, z6.b, z15.b\n"
        "ld1b {z30.h}, p3/z, [sp, x9]\n"
        "orr z7.d, z7.d, #0x7fffffffff000000\n"
        "sqrshlr z28.s, p5/m, z28.s, z7.s\n"
        "adclt z24.s, z8.s, z8.s\n"
        "ldnt1w {z19.s}, p1/z, [x3]\n"
        "stnt1w {z1.s}, p7, [x5, #5, mul vl]\n"
        "sqrdcmlah z23.h, z23.h, z6.h[1], #180\n"
        "ld1row {z16.s}, p7/z, [x12, x4, lsl #2]\n"
        "ld1rob {z0.b}, p0/z, [x4, x20]\n"
        "ld1rqb {z19.b}, p6/z, [sp, x30]\n"
        "sqsub z19.s, p1/m, z19.s, z27.s\n"
        "ld1d {z10.d}, p3/z, [sp, z18.d, uxtw #3]\n"
        "umlslt z24.d, z9.s, z0.s[3]\n"
        "srsra z30.h, z23.h, #8\n"
        "fmlalt z29.s, z19.h, z4.h[3]\n"
        "st1d {z17.d}, p5, [z25.d, #192]\n"
        "subhnb z26.s, z29.d, z20.d\n"
        "sbclt z19.d, z21.d, z23.d\n"
        "fmlslb z31.s, z17.h, z6.h[4]\n"
        "smlalb z3.s, z8.h, z0.h[7]\n"
        "srshlr z27.s, p6/m, z27.s, z6.s\n"
        "ld1rsw {z31.d}, p1/z, [x11, #88]\n"
        "prfd pstl2keep, p2, [x7, x17, lsl #3]\n"
        "prfd pstl3keep, p0, [x16, z8.d, lsl #3]\n"
        "prfb pldl2keep, p1, [x4, #29, mul vl]\n"
        "ldff1h {z1.d}, p4/z, [x14, z27.d, sxtw]\n"
        "ld1w {z27.s}, p4/z, [sp, z2.s, uxtw #2]\n"
        "mov z26.d, #0xfe0000000000003f\n"
        "ld1rqd {z24.d}, p2/z, [x11]\n"
        "st1h {z21.d}, p5, [z14.d, #2]\n"
        "sqdmlslb z11.d, z27.s, z3.s[3]\n"
        "ldnf1b {z8.h}, p4/z, [x26, #-6, mul vl]\n"
        "saddlt z22.s, z1.h, z24.h\n"
        "ld1rqh {z18.h}, p7/z, [x22, #112]\n"
        "prfw pldl3strm, p5, [z24.s, #112]\n"
        "tbl z30.d, {z26.d-z27.d}, z4.d\n"
        "prfw pldl2keep, p7, [x18, z29.d, uxtw #2]\n"
        "uqrshl z31.h, p7/m, z31.h, z26.h\n"
        "ushllt z23.d, z12.s, #27\n"
        "ldff1b {z1.d}, p0/z, [sp, x0]\n"
        "prfw pstl1keep, p2, [x27, x8, lsl #2]\n"
        "uadalp z26.d, p0/m, z19.s\n"
        "prfb #6, p2, [x12, x23]\n"
        "prfw pstl1strm, p3, [x12, z19.d, uxtw #2]\n"
        "cmla z13.b, z3.b, z7.b, #0\n"
        "sqrdmlah z18.h, z27.h, z1.h\n"
        "ldff1d {z16.d}, p4/z, [x1, z26.d, uxtw #3]\n"
        "sqdmlalb z15.d, z24.s, z31.s\n"
        "ldff1h {z31.d}, p3/z, [sp, z23.d, lsl #1]\n"
        "prfd pstl1keep, p0, [x8, z13.s, uxtw #3]\n"
        "addhnb z9.s, z22.d, z3.d\n"
        "ldnt1sb {z24.s}, p5/z, [z25.s, x28]\n"
        "sqdmullb z29.d, z23.s, z5.s\n"
        "ld1b {z2.d}, p2/z, [sp, z2.d, uxtw]\n"
        "umullt z21.h, z2.b, z1.b\n"
        "ldnf1b {z24.d}, p2/z, [x28]\n"
        "st4w {z23.s-z26.s}, p3, [x9]\n"
        "fadda d20, p2, d20, z26.d\n"
        "ld1rsb {z22.d}, p1/z, [x8]\n"
        "sqrshrunb z19.h, z22.s, #1\n"
        "ldff1sb {z5.s}, p0/z, [sp, z20.s, uxtw]\n"
        "sqsubr z13.b, p5/m, z13.b, z12.b\n"
        "prfw #6, p2, [x20, z17.s, sxtw #2]\n"
        "ld1rqb {z20.b}, p2/z, [x23, x5]\n"
        "ld1d {z29.d}, p0/z, [z6.d, #160]\n"
        "stnt1w {z4.s}, p0, [x27, x9, lsl #2]\n"
        "st1h {z6.d}, p6, [sp, z9.d]\n"
        "uqinch z9.h, #17, mul #3\n"
        "fmlslt z15.s, z17.h, z7.h[3]\n"
        "mov z17.b, z23.b[54]\n"
        "stnt1h {z25.h}, p5, [x27, #5, mul vl]\n"
        "prfh #15, p6, [x25, z6.d, sxtw #1]\n"
        "prfd #6, p2, [x16, x3, lsl #3]\n"
        "st1w {z8.d}, p2, [sp, z19.d, lsl #2]\n"
        "ld1d {z24.d}, p0/z, [x28, z27.d, sxtw]\n"
        "ldff1sh {z15.s}, p7/z, [x8, xzr, lsl #1]\n"
        "ummla z6.s, z6.b, z26.b\n"
        "umlalb z28.d, z10.s, z2.s[2]\n"
        "prfh pstl1keep, p7, [x20, z23.d, uxtw #1]\n"
        "ldff1sh {z28.d}, p0/z, [x7, z6.d]\n"
        "prfh pstl3strm, p7, [x27, #-15, mul vl]\n"
        "ld1sh {z6.d}, p0/z, [sp, x16, lsl #1]\n"
        "eor z8.b, z8.b, #0xcc\n"
        "ld1sw {z15.d}, p1/z, [z18.d, #48]\n"
        "prfw pstl2strm, p2, [z20.s, #88]\n"
        "ssra z7.d, z2.d, #61\n"
        "prfh pstl3strm, p0, [x13, z21.d, lsl #1]\n"
        "uqrshrnb z8.s, z5.d, #13\n"
        "prfw #14, p0, [x29, x29, lsl #2]\n"
        "umlslt z26.h, z15.b, z6.b\n"
        "ldff1sb {z19.d}, p6/z, [x14, xzr]\n"
        "usdot z14.s, z23.b, z22.b\n"
        "sqdmlalt z17.d, z11.s, z13.s[2]\n"
        "eortb z24.d, z6.d, z20.d\n"
        "prfh pstl2strm, p6, [x26, z12.d, sxtw #1]\n"
        "ld1d {z18.d}, p2/z, [x5, z16.d, uxtw]\n"
        "sqshlr z8.s, p1/m, z8.s, z26.s\n"
        "prfw pstl3keep, p7, [x12, z5.s, sxtw #2]\n"
        "bfmlalb z1.s, z21.h, z29.h\n"
        "prfd pstl3keep, p5, [z23.d, #24]\n"
        "and z11.d, z11.d, #0x3fffffc000000\n"
        "prfb pldl3keep, p4, [x25, x21]\n"
        "prfb pldl1keep, p2, [x28, z15.d]\n"
        "bfdot z27.s, z23.h, z7.h[2]\n"
        "prfh #15, p1, [x29, x25, lsl #1]\n"
        "prfh pstl3strm, p4, [z6.d, #6]\n"
        "sdot z9.d, z24.h, z19.h\n"
        "stnt1b {z12.b}, p6, [sp, x12]\n"
        "bdep z7.h, z26.h, z30.h\n"
        "ldff1d {z25.d}, p2/z, [x24, z12.d, sxtw]\n"
        "uqdecd z10.d, #23, mul #12\n"
        "pmul z15.b, z14.b, z16.b\n"
        "smullt z29.s, z30.h, z17.h\n"
        "ld1w {z19.d}, p7/z, [sp, z0.d]\n"
        "prfb pstl3strm, p4, [x5, z3.d, uxtw]\n"
        "ldnf1w {z30.d}, p4/z, [sp, #-8, mul vl]\n"
        "raddhnt z21.s, z25.d, z24.d\n"
        "dupm z14.s, #0x20\n"
        "sqdmlslt z22.s, z17.h, z14.h\n"
        "decd z28.d, #17, mul #3\n"
        "mov z31.d, p7/m, x2\n"
        "adr z8.d, [z5.d, z17.d]\n"
        "sqrdmlsh z15.h, z17.h, z5.h[6]\n"
        "sli z18.d, z12.d, #32\n"
        "st3d {z24.d-z26.d}, p4, [x16]\n"
        "ldff1d {z6.d}, p4/z, [x20, x25, lsl #3]\n"
        "rsubhnb z17.s, z23.d, z13.d\n"
        "sm4ekey z2.s, z12.s, z4.s\n"
        "pmullt z23.d, z16.s, z2.s\n"
        "prfh pstl3keep, p7, [x3, z7.s, sxtw #1]\n"
        "bgrp z26.h, z15.h, z6.h\n"
        "dupm z10.s, #0xffffe000\n"
        "str z4, [sp, #231, mul vl]\n"
        "ldnt1b {z28.b}, p5/z, [x18, #4, mul vl]\n"
        "fmlalt z14.s, z5.h, z3.h\n"
        "prfh #14, p2, [sp, #-22, mul vl]\n"
        "sqdmlalb z6.s, z28.h, z5.h[7]\n"
        "orr z4.h, z4.h, #0xfff\n"
        "ld3d {z4.d-z6.d}, p7/z, [sp, x14, lsl #3]\n"
        "fcvtx z9.s, p3/m, z29.d\n"
        "prfh pstl3strm, p7, [x3, z8.s, uxtw #1]\n"
        "ld1rqh {z27.h}, p6/z, [x17]\n"
        "ldff1h {z14.d}, p5/z, [sp, x7, lsl #1]\n"
        "ldff1d {z1.d}, p1/z, [sp, xzr, lsl #3]\n"
        "prfw #6, p5, [x23, #-6, mul vl]\n"
        "prfb pldl3keep, p1, [x0, z31.s, uxtw]\n"
        "usmmla z21.s, z2.b, z4.b\n"
        "rsubhnt z20.b, z17.h, z0.h\n"
        "uaddwb z4.d, z6.d, z10.s\n"
        "uqsubr z14.h, p6/m, z14.h, z16.h\n"
        "subhnt z26.s, z29.d, z30.d\n"
        "mov z2.h, #0x7c0\n"
        "smlalb z27.d, z31.s, z24.s\n"
        "shrnt z6.h, z1.s, #14\n"
        "st1d {z26.d}, p2, [x18, z10.d, sxtw #3]\n"
        "prfb pldl3keep, p7, [x28, z29.d, sxtw]\n"
        "prfw pldl1strm, p1, [x25, z17.s, uxtw #2]\n"
        "prfh pldl3keep, p1, [x13, #-17, mul vl]\n"
        "ldnt1sb {z31.s}, p6/z, [z26.s, xzr]\n"
        "ldff1sb {z24.d}, p1/z, [z17.d]\n"
        "sqrshrnt z31.b, z27.h, #6\n"
        "sqdmullt z13.d, z11.s, z0.s[2]\n"
        "mov z4.s, p5/m, s19\n"
        "ld3b {z8.b-z10.b}, p7/z, [sp, x20]\n"
        "uaba z23.b, z5.b, z27.b\n"
        "sqdecb x25, w25, #22, mul #7\n"
        "fmlalb z22.s, z3.h, z0.h[5]\n"
        "fsubr z17.s, p3/m, z17.s, #1.0\n"
        "ld4d {z4.d-z7.d}, p5/z, [sp, #16, mul vl]\n"
        "bfmlalb z6.s, z16.h, z4.h[3]\n"
        "ld1sw {z18.d}, p0/z, [x23, z7.d, uxtw #2]\n"
        "mov z29.h, p5/m, w21\n"
        "stnt1b {z30.s}, p2, [z13.s, xzr]\n"
        "uaddwt z24.h, z10.h, z15.b\n"
        "ld1rod {z27.d}, p3/z, [x29, #-256]\n"
        "addhnt z0.h, z26.s, z30.s\n"
        "sqshrunb z10.h, z15.s, #7\n"
        "urshlr z0.d, p0/m, z0.d, z18.d\n"
        "mov z29.s, #0xffc000\n"
        "eor z17.s, z17.s, #0x180000\n"
        "ldnf1sh {z17.s}, p5/z, [x0, #6, mul vl]\n"
        "orr z23.h, z23.h, #0xff81\n"
        "uabalt z28.h, z2.b, z22.b\n"
        "prfd #14, p5, [x1, z25.d, lsl #3]\n"
        "orr z29.d, z29.d, #0x7ffffffff8000000\n"
        "shrnb z30.s, z1.d, #14\n"
        "prfd pldl2strm, p6, [z25.d, #240]\n"
        "ld1w {z19.d}, p5/z, [sp, z16.d, uxtw]\n"
        "prfw pstl2strm, p1, [x26, z2.s, sxtw #2]\n"
        "smullb z0.s, z9.h, z22.h\n"
        "uqshl z15.d, p2/m, z15.d, z21.d\n"
        "umlalt z19.d, z11.s, z11.s\n"
        "prfb pldl1strm, p5, [x23, z28.s, uxtw]\n"
        "rshrnb z19.b, z4.h, #3\n"
        "ssublbt z30.h, z5.b, z4.b\n"
        "st1d {z28.d}, p6, [sp, z31.d, lsl #3]\n"
        "uqsub z26.s, p1/m, z26.s, z7.s\n"
        "uqinch w29, #27, mul #4\n"
        "adr z1.d, [z19.d, z11.d, uxtw #2]\n"
        "ld1rsb {z11.s}, p5/z, [sp, #33]\n"
        "ldnf1sh {z21.d}, p6/z, [x22]\n"
        "umlalt z1.d, z16.s, z15.s[2]\n"
        "sshllt z19.h, z17.b, #1\n"
        "st2h {z18.h-z19.h}, p1, [sp, x24, lsl #1]\n"
        "prfb pstl1keep, p7, [x5, z11.d, sxtw]\n"
        "ld4w {z8.s-z11.s}, p6/z, [sp, #-16, mul vl]\n"
        "ld1rod {z5.d}, p4/z, [x29, x1, lsl #3]\n"
        "prfw pldl2keep, p1, [x12, #17, mul vl]\n"
        "ld1sw {z13.d}, p0/z, [sp, z26.d, uxtw]\n"
        "ld1rw {z12.d}, p5/z, [sp, #172]\n"
        "and z21.s, z21.s, #0xfffffffd\n"
        "prfh pldl1keep, p3, [x20, z12.d, lsl #1]\n"
        "bfmlalt z14.s, z2.h, z6.h[1]\n"
        "prfb #14, p0, [x17, z28.s, uxtw]\n"
        "fmaxnmp z5.d, p3/m, z5.d, z21.d\n"
        "ld1sh {z24.s}, p6/z, [sp, z4.s, uxtw #1]\n"
        "sabalt z31.s, z31.h, z0.h\n"
        "fmlslt z18.s, z7.h, z13.h\n"
        "sqxtunt z28.b, z28.h\n"
        "stnt1w {z10.s}, p5, [sp, x19, lsl #2]\n"
        "eor z11.d, z11.d, #0xfffffff1ffffffff\n"
        "stnt1w {z24.s}, p7, [z24.s, xzr]\n"
        "st1b {z17.s}, p3, [sp, #-6, mul vl]\n"
        "prfd pldl3keep, p5, [z10.s, #64]\n"
        "fmaxp z22.s, p0/m, z22.s, z5.s\n"
        "st1d {z31.d}, p2, [x9, z2.d, lsl #3]\n"
        "eor z6.d, z6.d, #0xfffffffffc00001f\n"
        "ldff1sh {z4.d}, p1/z, [sp, z24.d]\n"
        "bcax z28.d, z28.d, z22.d, z11.d\n"
        "uaddlb z30.s, z29.h, z31.h\n"
        "ldff1b {z17.d}, p0/z, [sp, z14.d, uxtw]\n"
        "ld1rob {z5.b}, p6/z, [x17, #-64]\n"
        "prfw pstl1strm, p2, [x26, #27, mul vl]\n"
        "prfw pldl1keep, p1, [x25, z1.d, lsl #2]\n"
        "ld1d {z24.d}, p1/z, [sp, x24, lsl #3]\n"
        "mov z26.h, p2/m, h20\n"
        "eor z0.s, z0.s, #0xc0000fff\n"
        "orr z25.d, z25.d, #0xffffe0000000000f\n"
        "ldnf1h {z18.d}, p4/z, [sp, #3, mul vl]\n"
        "ldff1sw {z4.d}, p0/z, [sp, z17.d, lsl #2]\n"
        "adclb z2.d, z4.d, z27.d\n"
        "st1w {z6.d}, p5, [sp, x8, lsl #2]\n"
        "ssublb z30.d, z9.s, z23.s\n"
        "uabalb z8.d, z3.s, z14.s\n"
        "ld1rqd {z4.d}, p1/z, [sp]\n"
        "sqdecb xzr, mul4, mul #11\n"
        "ldff1sh {z23.d}, p2/z, [z21.d]\n"
        "prfh #15, p1, [x0, #30, mul vl]\n"
        "st1d {z11.d}, p0, [sp, z23.d]\n"
        "bext z3.s, z18.s, z6.s\n"
        "ld1rqd {z29.d}, p2/z, [sp, #-128]\n"
        "prfb pstl3keep, p4, [sp, #-11, mul vl]\n"
        "ld3b {z15.b-z17.b}, p5/z, [x19]\n"
        "ldnt1w {z26.s}, p6/z, [z31.s, xzr]\n"
        "prfb pstl2strm, p2, [z25.d, #2]\n"
        "ldff1sb {z9.s}, p7/z, [sp, x6]\n"
        "ld1sw {z15.d}, p2/z, [sp, z16.d, sxtw #2]\n"
        "ld1w {z18.d}, p0/z, [sp, #3, mul vl]\n"
        "ssubltb z29.s, z17.h, z3.h\n"
        "uqrshlr z3.s, p7/m, z3.s, z20.s\n"
        "prfw pldl2keep, p3, [z8.s, #84]\n"
        "sqabs z6.s, p5/m, z20.s\n"
        "faddv h12, p4, z19.h\n"
        "ushllb z31.d, z18.s, #30\n"
        "eor3 z25.d, z25.d, z25.d, z14.d\n"
        "ld1rb {z21.s}, p4/z, [sp, #62]\n"
        "sdot z9.s, z22.b, z3.b[2]\n"
        "prfh pldl3keep, p7, [sp, z12.d, lsl #1]\n"
        "st1h {z24.d}, p7, [sp, #-1, mul vl]\n"
        "sqxtunb z2.h, z28.s\n"
        "sqshl z17.b, p3/m, z17.b, z24.b\n"
        "umlalb z26.d, z1.s, z30.s\n"
        "ld1rh {z29.d}, p3/z, [sp, #14]\n"
        "ld1rd {z26.d}, p2/z, [x5]\n"
        "mov z5.d, #0xfffffffe0000000f\n"
        "fmlslb z25.s, z2.h, z25.h\n"
        "histseg z8.b, z29.b, z29.b\n"
        "ld2h {z26.h-z27.h}, p3/z, [sp, x6, lsl #1]\n"
        "ld1rsh {z6.s}, p6/z, [sp, #12]\n"
        "uabdlb z28.d, z21.s, z26.s\n"
        "prfh pldl1strm, p4, [x5, z26.s, sxtw #1]\n"
        "prfb pstl3keep, p6, [x30, z21.d, uxtw]\n"
        "shsub z15.s, p4/m, z15.s, z22.s\n"
        "fminnmv h14, p3, z19.h\n"
        "st1b {z5.s}, p0, [z15.s]\n"
        "prfh #15, p2, [x27, z3.s, uxtw #1]\n"
        "ldnt1d {z4.d}, p3/z, [sp, x3, lsl #3]\n"
        "ld1rb {z14.h}, p6/z, [x25]\n"
        "ldff1sw {z18.d}, p6/z, [sp, z28.d, uxtw #2]\n"
        "ld1sb {z17.s}, p2/z, [sp, x30]\n"
        "srshl z0.b, p3/m, z0.b, z19.b\n"
        "fmmla z23.d, z6.d, z6.d\n"
        "prfd #7, p1, [x22, z23.s, uxtw #3]\n"
        "prfh #7, p0, [z14.s, #28]\n"
        "uqshrnt z5.s, z25.d, #8\n"
        "prfw pstl3strm, p6, [sp, z21.s, uxtw #2]\n"
        "st3h {z5.h-z7.h}, p7, [x30]\n"
        "ld1h {z7.s}, p3/z, [sp, #7, mul vl]\n"
        "mov z20.s, #0xc000000\n"
        "bfcvt z26.h, p3/m, z24.s\n"
        "mov z18.h, #27904\n"
        "uaddlt z8.d, z6.s, z3.s\n"
        "sqdmlalbt z17.s, z6.h, z20.h\n"
        "st1b {z7.s}, p3, [sp, x27]\n"
        "st1b {z30.d}, p0, [sp, z19.d, uxtw]\n"
        "ldff1sw {z7.d}, p0/z, [sp, z14.d, sxtw]\n"
        "prfh pldl2keep, p0, [z5.s, #32]\n"
        "and z18.s, z18.s, #0x38000000\n"
        "ldr p6, [sp, #-234, mul vl]\n"
        "ldff1w {z1.d}, p6/z, [z15.d]\n"
        "ldff1b {z30.d}, p2/z, [x5, xzr]\n"
        "st3w {z21.s-z23.s}, p5, [sp, x28, lsl #2]\n"
        "bsl z5.d, z5.d, z3.d, z28.d\n"
        "st1h {z25.s}, p2, [z17.s]\n"
        "ldff1sh {z4.d}, p4/z, [sp, z12.d, lsl #1]\n"
        "ldnf1d {z28.d}, p7/z, [x1]\n"
        "whilehi p4.d, xzr, xzr\n"
        "rax1 z29.d, z0.d, z28.d\n"
        "prfw pstl2keep, p5, [x26, z3.d, uxtw #2]\n"
        "prfd pldl2strm, p5, [x24, z1.d, uxtw #3]\n"
        "sqxtnb z24.b, z30.h\n"
        "stnt1w {z31.s}, p1, [sp, #3, mul vl]\n"
        "decb x21, #20, mul #5\n"
        "prfw pstl2keep, p6, [x30, #-19, mul vl]\n"
        "orr z24.d, z24.d, #0x3ffff800\n"
        "st1d {z16.d}, p0, [sp, z31.d, uxtw #3]\n"
        "splice z22.s, p5, {z24.s-z25.s}\n"
        "ldff1d {z28.d}, p3/z, [x8, xzr, lsl #3]\n"
        "dupm z4.b, #0xaa\n"
        "prfw pldl3strm, p7, [x0, z30.d, lsl #2]\n"
        "prfw pldl3strm, p4, [x28, z16.s, sxtw #2]\n"
        "st1w {z1.d}, p6, [z31.d]\n"
        "ldff1b {z29.d}, p4/z, [z1.d]\n"
        "prfw pldl1keep, p2, [x12, x29, lsl #2]\n"
        "prfb pldl2strm, p5, [z12.d, #17]\n"
        "ldnf1sw {z27.d}, p3/z, [x17]\n"
        "ldnt1w {z30.s}, p5/z, [x19, #5, mul vl]\n"
        "sqdech x6, w6, #21, mul #9\n"
        "suqadd z5.b, p4/m, z5.b, z28.b\n"
        "bfdot z1.s, z14.h, z16.h\n"
        "eor z25.d, z25.d, #0x3f80000000000\n"
        "ldff1h {z29.s}, p7/z, [x0, xzr, lsl #1]\n"
        "adr z22.d, [z26.d, z5.d, sxtw]\n"
        "prfh pldl1strm, p5, [z26.d, #40]\n"
        "and z8.d, z8.d, #0x1ffc000\n"
        "prfd #15, p0, [z21.d, #184]\n"
        "prfw #14, p7, [x25, z14.d, lsl #2]\n"
        "uqdecd x30, #17, mul #9\n"
        "prfw #6, p1, [z20.d, #64]\n"
        "prfb #14, p6, [x16, z17.d]\n"
        "ldnt1sw {z21.d}, p6/z, [z3.d, xzr]\n"
        "prfh pldl2keep, p1, [x25, z14.s, uxtw #1]\n"
        "st4b {z14.b-z17.b}, p0, [sp, x12]\n"
        "ldff1h {z23.d}, p5/z, [sp, z18.d, uxtw]\n"
        "ld2d {z25.d-z26.d}, p6/z, [x25]\n"
        "orr z4.d, z4.d, #0xfc000fffffffffff\n"
        "prfd pldl3keep, p0, [x9]\n"
        "cntp xzr, p10, p11.d\n"
        "prfw #15, p1, [x28, z19.s, uxtw #2]\n"
        "raddhnb z13.b, z18.h, z2.h\n"
        "ldnt1w {z13.s}, p1/z, [sp, #-7, mul vl]\n"
        "prfh pstl3keep, p4, [x21, z6.d, lsl #1]\n"
        "udot z4.s, z9.b, z1.b[0]\n"
        "st1w {z23.d}, p1, [sp, z6.d, uxtw]\n"
        "mov z16.d, #0xfe\n"
        "uqdecd wzr, #23, mul #10\n"
        "uqincd x17, #20, mul #12\n"
        "uhadd z0.b, p3/m, z0.b, z23.b\n"
        "sqrshrunt z29.s, z1.d, #16\n"
        "sqdmlslt z22.d, z9.s, z0.s[2]\n"
        "ldnt1h {z31.h}, p4/z, [sp, x21, lsl #1]\n"
        "lastb w11, p6, z2.b\n"
        "inch x3, #24, mul #14\n"
        "ldff1sh {z8.d}, p6/z, [sp, z18.d, sxtw #1]\n"
        "incw x26, #28, mul #4\n"
        "stnt1d {z31.d}, p3, [x11]\n"
        "incd x26, #17, mul #15\n"
        "st3d {z22.d-z24.d}, p5, [sp, x11, lsl #3]\n"
        "udot z13.d, z14.h, z15.h\n"
        "st1h {z24.d}, p7, [sp, z29.d, sxtw]\n"
        "mov z31.h, #0x803f\n"
        "whilehs p4.d, w7, wzr\n"
        "ldnt1d {z21.d}, p5/z, [x18]\n"
        "saddwt z24.d, z31.d, z30.s\n"
        "prfb pldl2strm, p4, [x9, #-5, mul vl]\n"
        "mov z8.s, s21\n"
        "ld1rqw {z21.s}, p6/z, [x6]\n"
        "fadda h2, p0, h2, z25.h\n"
        "ldff1sh {z9.d}, p2/z, [sp, z0.d, uxtw]\n"
        "ld1roh {z10.h}, p4/z, [x25]\n"
        "ld1h {z3.d}, p6/z, [sp, z4.d, uxtw]\n"
        "sqincb x11, #18, mul #3\n"
        "shadd z20.d, p0/m, z20.d, z10.d\n"
        "st1d {z13.d}, p6, [sp, z12.d, uxtw]\n"
        "prfd pldl1strm, p6, [x0, x16, lsl #3]\n"
        "prfw pldl3keep, p1, [x26, z25.s, sxtw #2]\n"
        "orr z27.d, z27.d, #0xfffffff801ffffff\n"
        "uqincw w18, #15, mul #14\n"
        "st1h {z17.s}, p6, [sp, z14.s, uxtw]\n"
        "whilehi p4.b, wzr, w1\n"
        "prfd pldl3keep, p6, [x23, z19.d, lsl #3]\n"
        "ld1row {z24.s}, p1/z, [sp, x5, lsl #2]\n"
        "orr z15.d, z15.d, #0xfffff800000fffff\n"
        "ldnf1sb {z29.h}, p5/z, [x28]\n"
        "ldnf1h {z20.s}, p3/z, [x23]\n"
        "ld1h {z1.s}, p2/z, [sp, z13.s, sxtw]\n"
        "nbsl z11.d, z11.d, z18.d, z25.d\n"
        "orr z2.d, z2.d, #0x80000000000007ff\n"
        "fcvtlt z29.s, p5/m, z26.h\n"
        "clasta s12, p6, s12, z2.s\n"
        "ld1sh {z3.d}, p6/z, [z11.d]\n"
        "prfh pldl3keep, p7, [x7, x25, lsl #1]\n"
        "ldff1w {z9.s}, p3/z, [sp, z5.s, uxtw #2]\n"
        "uqshlr z7.s, p4/m, z7.s, z31.s\n"
        "smmla z4.s, z20.b, z6.b\n"
        "eor z15.h, z15.h, #0x1fe0\n"
        "ldff1w {z8.d}, p3/z, [sp, z16.d, sxtw]\n"
        "ld1h {z12.d}, p7/z, [sp, z13.d, uxtw #1]\n"
        "sqshrnt z5.s, z8.d, #21\n"
        "eor z9.d, z9.d, #0x3ffffff80000000\n"
        "bfmlalt z29.s, z17.h, z3.h\n"
        "ld1b {z11.d}, p0/z, [sp]\n"
        "sqadd z22.d, p4/m, z22.d, z2.d\n"
        "mov z20.s, #0xfff8001f\n"
        "brkns p13.b, p10/z, p2.b, p13.b\n"
        "prfb pstl3keep, p2, [z9.d, #19]\n"
        "prfd pstl1strm, p1, [z28.s, #80]\n"
        "prfw pldl2strm, p7, [x2, x16, lsl #2]\n"
        "prfh pstl1keep, p1, [z1.s, #50]\n"
        "ldnt1sh {z11.d}, p5/z, [z21.d, xzr]\n"
        "st1h {z3.s}, p1, [sp, z22.s, uxtw #1]\n"
        "fmaxv s26, p6, z25.s\n"
        "fmaxnmv d9, p7, z25.d\n"
        "bsl2n z30.d, z30.d, z13.d, z15.d\n"
        "sqdecw x14, #24\n"
        "sqdecw x17, #28, mul #4\n"
        "ld1h {z13.d}, p3/z, [sp, x25, lsl #1]\n"
        "and z11.s, z11.s, #0xffff80ff\n"
        "st1w {z8.d}, p1, [sp, z24.d]\n"
        "mov z12.d, #0xfffffff800007fff\n"
        "ld1sw {z2.d}, p3/z, [sp, #-6, mul vl]\n"
        "orr z27.d, z27.d, #0xfffffc000fffffff\n"
        "prfd pldl3strm, p0, [x14, #-18, mul vl]\n"
        "eor z27.h, z27.h, #0xc07f\n"
        "uqdecw w17, #20, mul #16\n"
        "ldnt1b {z15.b}, p4/z, [x29]\n"
        "adr z21.d, [z22.d, z20.d, uxtw]\n"
        "decw x20, #21, mul #10\n"
        "ld1rod {z13.d}, p4/z, [sp, x14, lsl #3]\n"
        "mov z27.s, #0x8001ffff\n"
        "uqdech w4, #18, mul #8\n"
        "ld3w {z31.s-z1.s}, p1/z, [x1]\n"
        "orr z17.d, z17.d, #0xffc000000000000f\n"
        "uqshrnb z5.s, z30.d, #27\n"
        "ld1sw {z10.d}, p7/z, [sp, z15.d, sxtw]\n"
        "ld1rsw {z4.d}, p5/z, [x22]\n"
        "ld1sw {z26.d}, p0/z, [sp, z29.d]\n"
        "st2b {z12.b-z13.b}, p7, [x6]\n"
        "prfh #6, p4, [x6, z0.d, lsl #1]\n"
        "prfh pldl2keep, p4, [x28, z16.s, sxtw #1]\n"
        "uqadd z26.d, p0/m, z26.d, z26.d\n"
        "ld1sh {z6.d}, p3/z, [sp, z25.d]\n"
        "mov z22.d, #0x7fffffffffffc000\n"
        "whilelo p1.d, w12, wzr\n"
        "eor z16.d, z16.d, #0xfffffffe\n"
        "fminv h16, p2, z15.h\n"
        "eor z20.s, z20.s, #0xf800003f\n"
        "usdot z12.s, z15.b, z6.b[3]\n"
        "ldr z26, [sp, #-30, mul vl]\n"
        "prfd pldl3keep, p4, [x1, x1, lsl #3]\n"
        "ld1sb {z5.s}, p7/z, [sp, #-6, mul vl]\n"
        "mov z0.d, #0xff80000000001fff\n"
        "and z19.d, z19.d, #0xffffc00000000003\n"
        "ld1w {z8.d}, p1/z, [sp, x3, lsl #2]\n"
        "orr z22.d, z22.d, #0xfe01ffffffffffff\n"
        "ld1rsh {z15.d}, p3/z, [x14]\n"
        "prfd pstl3strm, p5, [x21, #27, mul vl]\n"
        "eor z8.s, z8.s, #0x1fc0000\n"
        "ld1sb {z8.d}, p3/z, [sp, z15.d]\n"
        "fmaxnmv h24, p1, z10.h\n"
        "dech xzr, vl1, mul #9\n"
        "fminp z29.s, p1/m, z29.s, z8.s\n"
        "urhadd z20.s, p1/m, z20.s, z24.s\n"
        "fcvtnt z21.s, p5/m, z14.d\n"
        "sbclb z12.s, z18.s, z4.s\n"
        "ld1sb {z5.d}, p6/z, [sp, z25.d, sxtw]\n"
        "faddp z18.h, p0/m, z18.h, z31.h\n"
        "whilege p14.s, x12, xzr\n"
        "ld1sw {z20.d}, p2/z, [z30.d]\n"
        "ldnt1d {z17.d}, p2/z, [z30.d, xzr]\n"
        "stnt1h {z5.d}, p2, [z15.d, xzr]\n"
        "sqincd x30, #28, mul #7\n"
        "sqdecw xzr, vl2, mul #3\n"
        "dupm z23.s, #0x1e\n"
        "ldnt1h {z10.h}, p0/z, [x21]\n"
        "ld1b {z0.d}, p1/z, [sp, z6.d]\n"
        "cntd x8, #28, mul #14\n"
        "ldnf1sh {z20.d}, p1/z, [sp, #-4, mul vl]\n"
        "ldff1w {z7.d}, p0/z, [sp, z28.d]\n"
        "prfh pldl1keep, p4, [z30.d]\n"
        "ldnf1sb {z0.h}, p5/z, [sp, #-7, mul vl]\n"
        "and z30.d, z30.d, #0xfffffffc0000ffff\n"
        "cnth x17, #25, mul #10\n"
        "mov z19.h, #0x8fff\n"
        "uhsub z1.h, p7/m, z1.h, z11.h\n"
        "fminnmv s27, p0, z17.s\n"
        "prfd pldl1strm, p3, [sp, z3.d, lsl #3]\n"
        "orr z20.d, z20.d, #0x1f800000000\n"
        "ldff1h {z30.d}, p2/z, [sp, z10.d]\n"
        "ld1d {z7.d}, p7/z, [sp, z30.d, lsl #3]\n"
        "uminp z8.s, p0/m, z8.s, z2.s\n"
        "eor z1.s, z1.s, #0xfc000001\n"
        "ld1h {z27.d}, p6/z, [sp, z19.d]\n"
        "ld1h {z26.d}, p1/z, [sp, z21.d, lsl #1]\n"
        "usqadd z24.h, p6/m, z24.h, z2.h\n"
        "ld1sh {z8.d}, p0/z, [sp, z9.d, uxtw]\n"
        "and z30.d, z30.d, #0xffffe000000000\n"
        "eor z3.d, z3.d, #0x1ffffffffff8\n"
        "ldnf1b {z27.s}, p1/z, [sp, #-6, mul vl]\n"
        "uqrshrnt z10.b, z12.h, #1\n"
        "whilehi p8.s, xzr, x26\n"
        "shsubr z15.b, p2/m, z15.b, z17.b\n"
        "sqdecd x23, #23, mul #16\n"
        "eor z21.d, z21.d, #0xfffffffff800003f\n"
        "and z15.d, z15.d, #0xffffffe00000001f\n"
        "orr z29.s, z29.s, #0x3fffff80\n"
        "orr z22.b, z22.b, #0xaa\n"
        "eor z30.d, z30.d, #0xfffc00003fffffff\n"
        "eor z21.d, z21.d, #0xffffffff0000003f\n"
        "ldff1w {z4.d}, p6/z, [sp, x4, lsl #2]\n"
        "mov z11.h, #0xf0\n"
        "stnt1h {z20.h}, p7, [sp, x14, lsl #1]\n"
        "sqdecw z14.s, #21, mul #4\n"
        "prfw pldl1strm, p3, [sp, z1.d, sxtw #2]\n"
        "eor z1.d, z1.d, #0x3fffffffe0000000\n"
        "mov z5.d, #0xfffffffffff8000f\n"
        "prfd pstl2strm, p3, [sp, z22.d, sxtw #3]\n"
        "prfh pldl1strm, p4, [x30, z15.d, lsl #1]\n"
        "eor z0.s, z0.s, #0xfff000\n"
        "st1w {z25.s}, p0, [sp, z14.s, sxtw]\n"
        "st1h {z11.d}, p0, [sp, x9, lsl #1]\n"
        "ldff1sh {z17.d}, p3/z, [sp, z25.d, sxtw]\n"
        "prfd pstl2strm, p1, [x14, x6, lsl #3]\n"
        "clastb x13, p0, x13, z9.d\n"
        "ldff1b {z21.d}, p7/z, [sp, z4.d, sxtw]\n"
        "prfh pldl1strm, p6, [x16, x18, lsl #1]\n"
        "mov z8.s, #0xffe0001f\n"
        "prfh pldl2keep, p4, [sp, z28.d, sxtw #1]\n"
        "ld1rqh {z3.h}, p5/z, [sp, x24, lsl #1]\n"
        "ldff1d {z3.d}, p2/z, [sp, z22.d, uxtw]\n"
        "prfh pstl2strm, p4, [x6, x0, lsl #1]\n"
        "sqincb xzr, #23, mul #6\n"
        "index z26.s, wzr, w10\n"
        "sqneg z1.h, p3/m, z2.h\n"
        "flogb z23.s, p0/m, z8.s\n"
        "uqdecd w30, #24, mul #15\n"
        "mov z18.s, #0xfff00\n"
        "ldff1sw {z14.d}, p2/z, [z31.d]\n"
        "ldff1d {z22.d}, p7/z, [sp, z16.d]\n"
        "ldff1w {z7.s}, p4/z, [sp, z8.s, sxtw #2]\n"
        "mov z23.d, #0xfffe000000003fff\n"
        "prfb #14, p4, [x21, z15.s, sxtw]\n"
        "fminv s25, p1, z30.s\n"
        "orr z18.d, z18.d, #0xf000000001ffffff\n"
        "whilels p11.d, wzr, w5\n"
        "sqdecb xzr, wzr, #16, mul #15\n"
        "mov z25.s, #0xff00ffff\n"
        "orr z31.d, z31.d, #0x3fffe000000000\n"
        "aesd z22.b, z22.b, z5.b\n"
        "prfh pldl2strm, p1, [sp, z20.d, lsl #1]\n"
        "and z0.d, z0.d, #0xffff07ffffffffff\n"
        "ld1sw {z9.d}, p0/z, [sp, z31.d, lsl #2]\n"
        "ldff1sw {z30.d}, p1/z, [x19, xzr, lsl #2]\n"
        "fminnmv d2, p3, z21.d\n"
        "sqincw z17.s, #19, mul #8\n"
        "ld1rh {z21.h}, p4/z, [x25]\n"
        "bfmmla z2.s, z21.h, z25.h\n"
        "uqdecw x8, #20, mul #9\n"
        "and z8.d, z8.d, #0xfffffffff0ffffff\n"
        "prfw pldl1strm, p3, [x1, #18, mul vl]\n"
        "st1h {z5.d}, p4, [sp, z3.d, lsl #1]\n"
        "ld1rqh {z18.h}, p1/z, [sp, #-32]\n"
        "mov z17.h, #0xfcff\n"
        "and z1.s, z1.s, #0xf0000000\n"
        "mov z7.s, #0xffff0001\n"
        "uqincb x1, #26\n"
        "mov z19.d, #0x3ffe00000000\n"
        "ldff1sw {z29.d}, p0/z, [sp, x4, lsl #2]\n"
        "ld1row {z4.s}, p4/z, [x21]\n"
        "prfb pldl2strm, p0, [sp, z6.d]\n"
        "ldnt1b {z0.b}, p7/z, [sp, x15]\n"
        "ld1d {z8.d}, p1/z, [sp, z0.d, sxtw #3]\n"
        "urshl z21.h, p3/m, z21.h, z25.h\n"
        "ld1w {z14.d}, p0/z, [sp, z18.d, sxtw #2]\n"
        "and z16.d, z16.d, #0xffffffffffffffbf\n"
        "whilerw p8.d, x17, xzr\n"
        "mov z9.s, #0xff0\n"
        "and z4.d, z4.d, #0x1ffffff800000000\n"
        "orr z3.b, z3.b, #0x40\n"
        "st1b {z22.s}, p2, [sp, z0.s, sxtw]\n"
        "uqincd z5.d, #25, mul #4\n"
        "prfw pldl1keep, p4, [z17.s]\n"
        "prfd #14, p3, [sp, z13.d, lsl #3]\n"
        "fmaxnmv s1, p1, z9.s\n"
        "sqincw x17, #20\n"
        "ld1w {z16.s}, p0/z, [z0.s]\n"
        "ldnt1w {z25.s}, p2/z, [sp, x22, lsl #2]\n"
        "prfb pldl3strm, p4, [x5, z20.d, sxtw]\n"
        "eor z27.d, z27.d, #0xffffffffffffc\n"
        "ld1d {z20.d}, p3/z, [z0.d]\n"
        "ldff1sh {z25.s}, p5/z, [sp, x18, lsl #1]\n"
        "sqincw z0.s, #25\n"
        "ldff1h {z30.s}, p0/z, [sp, z18.s, uxtw #1]\n"
        "st1w {z26.s}, p2, [sp, z0.s, sxtw #2]\n"
        "prfd pstl3strm, p2, [x0, z28.d, lsl #3]\n"
        "cadd z22.b, z22.b, z9.b, #270\n"
        "prfb pstl2strm, p1, [x18, z25.d]\n"
        "st3b {z7.b-z9.b}, p1, [sp, #-6, mul vl]\n"
        "eor z10.d, z10.d, #0xffffffffffe0000f\n"
        "orr z21.h, z21.h, #0xff1f\n"
        "mov z8.d, #0xfffff00000\n"
        "prfd pstl1keep, p3, [x7, z19.s, sxtw #3]\n"
        "incp z8.d, p4.d\n"
        "dupm z30.s, #0xffffff80\n"
        "prfb pstl2strm, p4, [x14, x17]\n"
        "incd z4.d, #15, mul #16\n"
        "prfb pstl3keep, p0, [x24, x24]\n"
        "mov z21.d, #0x1f8000000000000\n"
        "smaxp z5.b, p7/m, z5.b, z28.b\n"
        "clastb w17, p3, w17, z23.b\n"
        "prfw pstl2keep, p3, [x6, z24.d, lsl #2]\n"
        "uqincw z21.s, #27, mul #15\n"
        "eor z20.h, z20.h, #0xff80\n"
        "mov z1.d, #0x3fc000000000\n"
        "st1h {z31.h}, p3, [sp]\n"
        "sqincw x16, w16, #14, mul #10\n"
        "eor z31.d, z31.d, #0xffe00007ffffffff\n"
        "orr z8.d, z8.d, #0xf00000000000000\n"
        "stnt1w {z25.s}, p1, [x4]\n"
        "ursqrte z15.s, p5/m, z9.s\n"
        "prfb pstl3strm, p2, [x16, #-7, mul vl]\n"
        "and z20.h, z20.h, #0xfe03\n"
        "whilels p8.b, xzr, x6\n"
        "mov p11.b, p6/z, p8.b\n"
        "ld1roh {z18.h}, p3/z, [sp, x16, lsl #1]\n"
        "prfb pstl2strm, p4, [sp, z10.s, sxtw]\n"
        "incb x4, #16, mul #5\n"
        "eor z20.d, z20.d, #0x7fffc000000\n"
        "brkb p6.b, p2/m, p3.b\n"
        "mov z0.d, #0x1fffffffff80\n"
        "ldff1h {z7.d}, p1/z, [z18.d]\n"
        "ld1rsw {z3.d}, p7/z, [sp, #212]\n"
        "mov z26.d, #0x3ffffffffc0\n"
        "sqdecp z6.d, p1.d\n"
        "ldnf1sw {z15.d}, p5/z, [sp, #-7, mul vl]\n"
        "orr z29.d, z29.d, #0x1fff0000000\n"
        "and z19.h, z19.h, #0xfffb\n"
        "prfb pldl3strm, p2, [z23.s]\n"
        "fminv d12, p4, z22.d\n"
        "sqcadd z10.b, z10.b, z0.b, #270\n"
        "ldff1h {z8.d}, p2/z, [sp, z2.d, sxtw]\n"
        "and z18.h, z18.h, #0xe0ff\n"
        "eor z29.d, z29.d, #0xffffe000000fffff\n"
        "ld1rob {z8.b}, p5/z, [x14]\n"
        "decw z15.s, #28, mul #4\n"
        "sadalp z22.d, p0/m, z1.s\n"
        "uqdecb x2, #22, mul #5\n"
        "and z10.d, z10.d, #0xfffffe000000000\n"
        "uqinch x8, #14, mul #12\n"
        "st1d {z28.d}, p5, [sp, z29.d, sxtw #3]\n"
        "inch z2.h, #17, mul #14\n"
        "and z30.h, z30.h, #0xfe1f\n"
        "st3d {z7.d-z9.d}, p5, [sp, #-18, mul vl]\n"
        "mov z30.d, #0x1ffff0000000000\n"
        "eor z26.d, z26.d, #0xffffffffc00000ff\n"
        "ldff1d {z4.d}, p2/z, [sp, z0.d, sxtw]\n"
        "eor z28.d, z28.d, #0x80000000007fffff\n"
        "sqdecd x30, w30, #17, mul #13\n"
        "ldff1d {z3.d}, p7/z, [sp, x26, lsl #3]\n"
        "mov z22.s, #0x1fffffc\n"
        "prfw #6, p2, [z25.d]\n"
        "eor z6.s, z6.s, #0xfff80fff\n"
        "prfb pldl3keep, p5, [sp, z5.d, uxtw]\n"
        "prfh #6, p1, [sp, x0, lsl #1]\n"
        "ld1sh {z22.s}, p1/z, [sp, z14.s, sxtw]\n"
        "and z8.h, z8.h, #0xcfff\n"
        "prfb pstl1keep, p2, [x22, z29.d]\n"
        "mov z25.h, #0xfc7f\n"
        "lastb x5, p0, z13.d\n"
        "eor z21.d, z21.d, #0xfffff0000000000f\n"
        "ldff1b {z21.d}, p7/z, [sp, z31.d]\n"
        "uqdech z24.h, #25, mul #8\n"
        "eor z11.d, z11.d, #0xfffffffffffc0003\n"
        "prfd pstl1keep, p4, [z20.s]\n"
        "ld1b {z7.b}, p4/z, [sp, #-2, mul vl]\n"
        "stnt1b {z21.b}, p0, [x27]\n"
        "st1w {z7.d}, p4, [sp, #-3, mul vl]\n"
        "and z7.d, z7.d, #0x1fffffff000\n"
        "eor z4.d, z4.d, #0x1ffff0000000000\n"
        "mov z11.d, #0xffffffffffffe1ff\n"
        "ld1d {z25.d}, p1/z, [sp, z5.d, sxtw]\n"
        "ld1d {z28.d}, p7/z, [sp, z19.d, uxtw]\n"
        "and z18.s, z18.s, #0x3fffffff\n"
        "eor z31.d, z31.d, #0xfffffc0000000001\n"
        "orr z0.d, z0.d, #0xfff81fffffffffff\n"
        "and z26.d, z26.d, #0x7ffffffff0000\n"
        "fmaxv h21, p4, z28.h\n"
        "sqincb x13, w13, #22, mul #3\n"
        "sunpkhi z17.h, z2.b\n"
        "ld1rqb {z19.b}, p1/z, [x8]\n"
        "eor z27.h, z27.h, #0xf007\n"
        "and z4.d, z4.d, #0xfffffffc001fffff\n"
        "ldff1w {z25.d}, p7/z, [x5, xzr, lsl #2]\n"
        "sqinch x12, #16, mul #9\n"
        "and z17.h, z17.h, #0xfff3\n"
        "orr z13.d, z13.d, #0xfffffffffff0001f\n"
        "mov z22.d, #0x3fffffc000000000\n"
        "prfd pstl2strm, p2, [z30.d]\n"
        "eor z19.d, z19.d, #0xfffffffe7fffffff\n"
        "orr z16.d, z16.d, #0xffff00000000000f\n"
        "ldnf1w {z4.d}, p0/z, [x0]\n"
        "mov z0.d, #0xc1ffffffffffffff\n"
        "eor z15.h, z15.h, #0x7c0\n"
        "dech z10.h, #28, mul #7\n"
        "and z3.d, z3.d, #0x1ffe00000000000\n"
        "ldnf1b {z30.d}, p3/z, [sp]\n"
        "mov z13.d, p3/m, d18\n"
        "prfd pldl1strm, p6, [x18, z28.d, lsl #3]\n"
        "eor z20.s, z20.s, #0xffff8001\n"
        "and z22.s, z22.s, #0xfc00007f\n"
        "stnt1h {z8.h}, p4, [sp, #5, mul vl]\n"
        "orr z22.d, z22.d, #0xfffe000000000007\n"
        "bfcvtnt z15.h, p3/m, z5.s\n"
        "dupm z3.b, #0xfd\n"
        "and z20.h, z20.h, #0xff80\n"
        "eor z24.d, z24.d, #0xfffff8000007ffff\n"
        "ctermeq x11, x12\n"
        "and z12.s, z12.s, #0xfffffff3\n"
        "uqdecd z10.d, #19\n"
        "and z14.d, z14.d, #0xff0000001fffffff\n"
        "st1h {z17.s}, p1, [sp, z17.s, sxtw #1]\n"
        "ld1h {z19.s}, p0/z, [z18.s]\n"
        "ld1w {z26.d}, p1/z, [sp, z6.d, sxtw]\n"
        "orr z1.d, z1.d, #0xffe0000000000fff\n"
        "and z9.d, z9.d, #0xfffffffeffffffff\n"
        "and z20.s, z20.s, #0x1ffff8\n"
        "prfd #15, p0, [sp, #13, mul vl]\n"
        "stnt1h {z9.h}, p7, [x14]\n"
        "mov z24.d, #0xc003ffffffffffff\n"
        "mov z8.d, #0x7ff000000000000\n"
        "cntw x22, #26\n"
        "ld1b {z20.s}, p0/z, [z17.s]\n"
        "sqincw x17, vl128, mul #13\n"
        "eor z21.d, z21.d, #0xffc03fffffffffff\n"
        "ldff1sb {z30.s}, p6/z, [sp, z27.s, sxtw]\n"
        "fmaxv d31, p0, z28.d\n"
        "sqincw x22, #15, mul #7\n"
        "orr z4.d, z4.d, #0xfffff007ffffffff\n"
        "ld4h {z17.h-z20.h}, p4/z, [sp, #-4, mul vl]\n"
        "eor z31.s, z31.s, #0x1fffe\n"
        "prfd pstl3keep, p1, [sp, x3, lsl #3]\n"
        "sqinch z7.h, #24, mul #9\n"
        "ld1rqb {z1.b}, p2/z, [sp, #112]\n"
        "and z22.s, z22.s, #0x7ffffff\n"
        "eor z18.d, z18.d, #0xfffff00000000000\n"
        "eor z3.d, z3.d, #0x7ff0000000\n"
        "orr z5.d, z5.d, #0xffffe00000\n"
        "prfb pldl1strm, p0, [sp, z3.s, uxtw]\n"
        "and z27.h, z27.h, #0x7fc\n"
        "st1b {z25.d}, p3, [sp, z22.d]\n"
        "dupm z8.b, #0xdd\n"
        "prfb #15, p5, [sp, z0.d]\n"
        "and z11.s, z11.s, #0xffff0003\n"
        "mov z30.d, #0xffffffffffffffe\n"
        "sqdech z15.h, #17\n"
        "prfh pstl1strm, p4, [sp, z22.s, sxtw #1]\n"
        "eor z12.s, z12.s, #0xf0000fff\n"
        "ldff1d {z17.d}, p4/z, [sp, z0.d, lsl #3]\n"
        "mov z26.d, #0x3fffc00\n"
        "prfh pldl1strm, p5, [z0.s]\n"
        "mov p10.b, p3/m, p15.b\n"
        "ld1rw {z13.s}, p4/z, [x13]\n"
        "cntw x5, #24, mul #16\n"
        "dech x7, #21, mul #3\n"
        "ld1rob {z9.b}, p5/z, [sp, #-256]\n"
        "ld1rqd {z7.d}, p6/z, [sp, x28, lsl #3]\n"
        "and z11.d, z11.d, #0xffffff800\n"
        "uqdech w27, #23\n"
        "eor z16.d, z16.d, #0xfc0fffffffffffff\n"
        "mov z9.s, #0xffffc007\n"
        "prfb pldl3keep, p7, [sp, #19, mul vl]\n"
        "eor z4.d, z4.d, #0x3ffffe\n"
        "fcvtxnt z31.s, p3/m, z23.d\n"
        "ldff1sw {z19.d}, p3/z, [sp, z2.d, sxtw #2]\n"
        "mov z16.h, #0xc07f\n"
        "mov z24.d, #0xfffffff000000000\n"
        "and z24.d, z24.d, #0xfffc000000\n"
        "orr z31.d, z31.d, #0x807fffffffffffff\n"
        "orr z21.d, z21.d, #0x3ffffffffffffe\n"
        "prfw pstl3keep, p5, [sp, #-2, mul vl]\n"
        "uqxtnb z24.s, z13.d\n"
        "and z11.d, z11.d, #0xfffff80000007fff\n"
        "dupm z11.h, #0xff8f\n"
        "uqincb w4, #25, mul #10\n"
        "orr z19.s, z19.s, #0x3fe00\n"
        "eor z17.d, z17.d, #0xfffffffffc01ffff\n"
        "orr z8.s, z8.s, #0x7e00\n"
        "and z18.d, z18.d, #0xffffff\n"
        "mov z4.d, #0xfe00007fffffffff\n"
        "ld1sb {z11.s}, p1/z, [z0.s]\n"
        "dech x11, #27\n"
        "prfb pldl3strm, p3, [x23, x7]\n"
        "mov z12.s, #0x180000\n"
        "ldnt1h {z11.h}, p6/z, [sp, #-7, mul vl]\n"
        "ld2h {z7.h-z8.h}, p5/z, [sp]\n"
        "and z30.d, z30.d, #0xfffffffffdffffff\n"
        "and z28.d, z28.d, #0xfc007fffffffffff\n"
        "ld1rod {z17.d}, p2/z, [sp, #-256]\n"
        "mov z25.h, #0xe00f\n"
        "st1w {z13.d}, p7, [sp, z9.d, uxtw #2]\n"
        "mov z7.b, p5/m, b5\n"
        "mov z17.d, #0xffffffffff0\n"
        "mov z24.d, #0x7ffffff000000000\n"
        "dupm z7.b, #0xfe\n"
        "prfw pstl3strm, p7, [sp, #15, mul vl]\n"
        "eor z12.d, z12.d, #0xffffffc07fffffff\n"
        "eor z12.s, z12.s, #0xffe0\n"
        "orr z24.d, z24.d, #0xfffff3ffffffffff\n"
        "and z27.d, z27.d, #0x7fff000000\n"
        "st1d {z18.d}, p2, [z10.d]\n"
        "ldff1w {z19.s}, p0/z, [sp, z8.s, uxtw]\n"
        "orr z10.d, z10.d, #0xffffff80\n"
        "sqincd x27, w27, #21\n"
        "eor z18.d, z18.d, #0xffffff8000000\n"
        "ldff1d {z8.d}, p2/z, [sp, z3.d, uxtw #3]\n"
        "and z28.d, z28.d, #0xfff80000000001ff\n"
        "fadda s18, p7, s18, z10.s\n"
        "orr z7.s, z7.s, #0xfffc000\n"
        "orr z4.h, z4.h, #0xfe00\n"
        "orr z24.s, z24.s, #0xfc003fff\n"
        "and z25.s, z25.s, #0xffffe\n"
        "ld1sw {z19.d}, p4/z, [sp, z26.d, uxtw #2]\n"
        "whilels p8.b, x26, xzr\n"
        "and z22.d, z22.d, #0xffff00000000007f\n"
        "dupm z19.s, #0x3e00\n"
        "ld1d {z12.d}, p1/z, [sp, z16.d]\n"
        "index z29.b, w16, wzr\n"
        "prfh pstl1strm, p7, [sp, z28.d, uxtw #1]\n"
        "orr z5.d, z5.d, #0x7ffe000000000000\n"
        "mov z14.s, #0xff00001f\n"
        "orr z3.s, z3.s, #0xffffbfff\n"
        "ld1rob {z5.b}, p6/z, [sp, x14]\n"
        "sqxtnt z22.s, z19.d\n"
        "orr z2.d, z2.d, #0x1fffc00000000\n"
        "incd x10, #26\n"
        "and z30.d, z30.d, #0x3ff00000\n"
        "and z11.d, z11.d, #0x3ffffffe\n"
        "whilegt p15.h, wzr, w17\n"
        "prfw pldl2strm, p6, [z9.s]\n"
        "prfw pldl1keep, p5, [sp, #1, mul vl]\n"
        "ldff1sw {z25.d}, p2/z, [sp, z8.d]\n"
        "and z18.s, z18.s, #0x7e00\n"
        "mov z28.s, #0xfffffe03\n"
        "and z6.h, z6.h, #0x3f8\n"
        "and z10.d, z10.d, #0xf00000000000000f\n"
        "mov z31.d, #0xffffffffc00001ff\n"
        "orr z31.d, z31.d, #0xfffc007fffffffff\n"
        "prfb pldl3keep, p0, [z23.d]\n"
        "prfw #14, p1, [sp, #-19, mul vl]\n"
        "and z3.d, z3.d, #0xff0000ffffffffff\n"
        "and z16.d, z16.d, #0xc00000000000000\n"
        "orr z20.s, z20.s, #0xc0000000\n"
        "and z1.d, z1.d, #0xffc00001ffffffff\n"
        "and z25.d, z25.d, #0xfffffffffe00ffff\n"
        "movs p15.b, p12/z, p7.b\n"
        "prfh pldl1keep, p0, [sp, #-12, mul vl]\n"
        "eor z10.d, z10.d, #0xffffff8001ffffff\n"
        "mov z8.q, q23\n"
        "eor z7.d, z7.d, #0x3ffe\n"
        "cntd x17, #16\n"
        "dupm z25.s, #0x7f00\n"
        "whilele p12.s, x18, xzr\n"
        "and z3.s, z3.s, #0xfe000fff\n"
        "orr z17.s, z17.s, #0xfffffc00\n"
        "eor z15.s, z15.s, #0xc\n"
        "mov z1.d, #0x1fffffffffffe000\n"
        "prfw pldl2strm, p2, [x10]\n"
        "uqincw x25, #24, mul #14\n"
        "uunpkhi z23.s, z1.h\n"
        "whilehs p1.d, wzr, w11\n"
        "prfd #15, p7, [sp, x4, lsl #3]\n"
        "eor z9.d, z9.d, #0xc000000000000000\n"
        "ld1roh {z30.h}, p1/z, [sp, #32]\n"
        "prfw pstl2keep, p4, [sp, z31.s, uxtw #2]\n"
        "uqincw z3.s, all, mul #4\n"
        "eor z20.d, z20.d, #0xfffffe0000000000\n"
        "and z14.h, z14.h, #0xffc1\n"
        "prfb pstl1strm, p5, [z25.d]\n"
        "ld2w {z24.s-z25.s}, p1/z, [sp, x4, lsl #2]\n"
        "mov z11.d, #0xffffffff0\n"
        "faddv d25, p0, z22.d\n"
        "ld1row {z28.s}, p1/z, [sp]\n"
        "orr z22.s, z22.s, #0x1fffc\n"
        "whilelt p10.d, wzr, w29\n"
        "stnt1d {z4.d}, p6, [z0.d, xzr]\n"
        "orr z28.s, z28.s, #0xffe00003\n"
        "eor z5.d, z5.d, #0xffffc00\n"
        "ldff1w {z28.d}, p1/z, [sp, z12.d, lsl #2]\n"
        "eor z23.s, z23.s, #0x3ffffffe\n"
        "and z5.d, z5.d, #0xfe00000fffffffff\n"
        "prfh pstl1strm, p5, [z4.d]\n"
        "ld1h {z14.s}, p5/z, [sp, z30.s, sxtw #1]\n"
        "eor z30.d, z30.d, #0xffffffffffc00003\n"
        "and z0.d, z0.d, #0xfffffffc00\n"
        "st1d {z2.d}, p4, [sp, x13, lsl #3]\n"
        "prfd pldl3strm, p3, [sp, z24.s, sxtw #3]\n"
        "and z10.d, z10.d, #0xfffc000000001fff\n"
        "and z11.s, z11.s, #0x1fffff80\n"
        "ld1sb {z27.s}, p6/z, [sp, z22.s, uxtw]\n"
        "and z28.d, z28.d, #0x7fffffffe00000\n"
        "prfh pstl3keep, p4, [x30]\n"
        "eor z12.d, z12.d, #0x7fffffffffffe000\n"
        "whilelo p8.s, wzr, wzr\n"
        "mov z15.d, #0xfe000000000003ff\n"
        "orr z27.d, z27.d, #0xf800001fffffffff\n"
        "and z17.d, z17.d, #0xffffffff800000\n"
        "eor z18.s, z18.s, #0x7fff000\n"
        "uqdecw z24.s, #18, mul #5\n"
        "ldnf1sb {z29.d}, p2/z, [sp]\n"
        "prfb #7, p2, [x15]\n"
        "st1d {z25.d}, p3, [sp, z5.d, sxtw]\n"
        "sqincd z19.d, pow2, mul #4\n"
        "mov z24.d, #0xfff80fffffffffff\n"
        "faddv s8, p1, z3.s\n"
        "prfw pstl3keep, p5, [sp, x26, lsl #2]\n"
        "mov z25.d, #0x7fffff800000000\n"
        "ld1rsb {z20.h}, p6/z, [sp]\n"
        "eor z22.b, z22.b, #0xfe\n"
        "prfd pldl3keep, p1, [sp, #-23, mul vl]\n"
        "sqincd z30.d, #15, mul #10\n"
        "incw x3, #26\n"
        "whilege p11.b, w28, wzr\n"
        "and z27.d, z27.d, #0xfffffffc0000001f\n"
        "orr z25.d, z25.d, #0xfffff87fffffffff\n"
        "ld1row {z3.s}, p1/z, [sp, #-192]\n"
        "fmaxnm z10.s, p1/m, z10.s, #0.0\n"
        "uqdech z16.h, pow2, mul #9\n"
        "sqdech xzr, wzr, vl5, mul #7\n"
        "and z11.d, z11.d, #0xfffffc00000001ff\n"
        "uqinch x3, all, mul #15\n"
        "eor z30.s, z30.s, #0x1ffffc\n"
        "fminnm z3.d, p6/m, z3.d, #1.0\n"
        "prfd pldl2strm, p4, [z13.d]\n"
        "ldff1sw {z21.d}, p1/z, [sp, z22.d, uxtw]\n"
        "uqdecb w12, #24\n"
        "orr z14.s, z14.s, #0xffe000ff\n"
        "eor z18.h, z18.h, #0xe000\n"
        "prfw #15, p4, [sp, z27.s, uxtw #2]\n"
        "cntb x3, #17, mul #9\n"
        "and z27.d, z27.d, #0xfffffffffffffc00\n"
        "decd x25, #16, mul #4\n"
        "ld1sh {z10.d}, p3/z, [sp, #-2, mul vl]\n"
        "index z2.h, wzr, #-14\n"
        "ld1b {z13.s}, p5/z, [sp, z21.s, sxtw]\n"
        "stnt1b {z8.b}, p2, [sp, #-7, mul vl]\n"
        "uqdecp z10.d, p1.d\n"
        "whilele p6.h, w14, wzr\n"
        "orr z6.d, z6.d, #0xfe000fffffffffff\n"
        "orr z23.d, z23.d, #0xfffffffe03ffffff\n"
        "inch z10.h, #27\n"
        "orr z23.b, z23.b, #0xe0\n"
        "orr z14.s, z14.s, #0x1ffffe00\n"
        "ld1rod {z5.d}, p1/z, [x12]\n"
        "prfh pstl3keep, p4, [sp, x6, lsl #1]\n"
        "and z12.d, z12.d, #0xffffffffffff00ff\n"
        "ldnf1d {z2.d}, p1/z, [sp, #-2, mul vl]\n"
        "incw z31.s, #15, mul #2\n"
        "orr z30.s, z30.s, #0xfc0007ff\n"
        "mov z23.d, #0x7ffffffffff800\n"
        "st3b {z9.b-z11.b}, p6, [sp]\n"
        "uqdecp x24, p7.b\n"
        "prfw #14, p0, [sp, z15.d, lsl #2]\n"
        "nots p14.b, p14/z, p11.b\n"
        "prfw pldl3strm, p6, [sp, x2, lsl #2]\n"
        "incp x28, p11.d\n"
        "orr z27.s, z27.s, #0xe001ffff\n"
        "orr z29.h, z29.h, #0xf87f\n"
        "ld1sw {z14.d}, p0/z, [sp, x20, lsl #2]\n"
        "dupm z8.h, #0xffc7\n"
        "ldnf1w {z6.d}, p2/z, [sp]\n"
        "st1d {z31.d}, p4, [sp, #7, mul vl]\n"
        "ldff1sb {z30.d}, p3/z, [sp, z30.d]\n"
        "orr z19.h, z19.h, #0xfff1\n"
        "dupm z12.b, #0x3f\n"
        "eor z20.d, z20.d, #0xfffe0000000fffff\n"
        "and z6.d, z6.d, #0xffff0000ffffffff\n"
        "mov z24.b, p1/m, wsp\n"
        "mov z2.d, #0xf800000000003fff\n"
        "eor z6.d, z6.d, #0x7fffffffffff\n"
        "sqincd x5, w5, #19, mul #4\n"
        "eor z29.h, z29.h, #0xfe0f\n"
        "whilelt p2.h, x27, xzr\n"
        "sm4e z0.s, z0.s, z23.s\n"
        "prfd #15, p4, [sp, z16.s, uxtw #3]\n"
        "and z25.s, z25.s, #0x1c00000\n"
        "eor z3.d, z3.d, #0x1fffffffffff8\n"
        "mov z7.d, #0xffffffffffe0007f\n"
        "prfb pstl2keep, p6, [sp, z13.s, uxtw]\n"
        "orr z3.d, z3.d, #0xfffffffc000\n"
        "orr z21.h, z21.h, #0xe0ff\n"
        "mov z21.s, #0xfffe0007\n"
        "sqdech z4.h, #21, mul #2\n"
        "orr z16.d, z16.d, #0xfe00000000007fff\n"
        "lastb wzr, p1, z23.s\n"
        "sqdech x19, #19, mul #5\n"
        "dupm z11.h, #0xe00\n"
        "and z27.s, z27.s, #0x3fffffc\n"
        "orr z27.d, z27.d, #0xfffffffff1ffffff\n"
        "sqdecw xzr, wzr, #20, mul #8\n"
        "and z13.d, z13.d, #0xfc00000000007fff\n"
        "mov z20.d, #0xfffe0000000000ff\n"
        "mov z12.s, #0x1ffffff\n"
        "prfh pstl3strm, p0, [sp, x15, lsl #1]\n"
        "uqincb x22, #15, mul #2\n"
        "orr z3.s, z3.s, #0xffff7fff\n"
        "mov z18.s, #0xc07fffff\n"
        "prfw #7, p0, [x17]\n"
        "eor z11.d, z11.d, #0xfffc00000000000f\n"
        "orr z1.h, z1.h, #0xffcf\n"
        "eor z12.d, z12.d, #0xffffffc0000fffff\n"
        "uunpklo z22.d, z3.s\n"
        "dupm z7.b, #0xf8\n"
        "prfb pldl1strm, p2, [sp, x23]\n"
        "dupm z19.h, #0xffe0\n"
        "and z29.d, z29.d, #0xfc00000000000\n"
        "eor z28.h, z28.h, #0x7f\n"
        "orr z6.d, z6.d, #0xffffffffff001fff\n"
        "whilelo p3.h, wzr, w27\n"
        "index z1.d, xzr, x11\n"
        "eor z0.d, z0.d, #0xfc0000000fffffff\n"
        "eor z7.d, z7.d, #0xfffffff80007ffff\n"
        "fmin z30.h, p7/m, z30.h, #1.0\n"
        "orr z5.d, z5.d, #0x3ffffff00000000\n"
        "prfd #14, p2, [sp, z7.d, sxtw #3]\n"
        "eor z13.d, z13.d, #0xffffffffff03ffff\n"
        "orr z10.d, z10.d, #0xffffffe000000\n"
        "mov z20.d, #0xffffffe00007ffff\n"
        "eor z26.d, z26.d, #0xffffffe00001ffff\n"
        "eor z14.d, z14.d, #0x1fffff8000\n"
        "str p5, [sp, #-241, mul vl]\n"
        "eor z29.d, z29.d, #0xf000000001ffffff\n"
        "and z11.s, z11.s, #0xffc0007f\n"
        "eor z8.d, z8.d, #0x800fffffffffffff\n"
        "and z11.d, z11.d, #0xffffffffffe003ff\n"
        "mov z8.d, z7.d\n"
        "mov z9.d, #0xffffffff800001ff\n"
        "eor z17.h, z17.h, #0xcfff\n"
        "and z27.d, z27.d, #0xf0000000007fffff\n"
        "orr z10.d, z10.d, #0xfff800000003ffff\n"
        "orr z12.d, z12.d, #0xffffe0000007ffff\n"
        "whilewr p6.b, x14, xzr\n"
        "mov z11.s, #0x800fffff\n"
        "and z28.d, z28.d, #0xfffffff07fffffff\n"
        "eor z11.d, z11.d, #0x3ffffffc000000\n"
        "orr z12.d, z12.d, #0xfffffffffffc000\n"
        "orr z14.h, z14.h, #0x1c\n"
        "eor z3.h, z3.h, #0xffc1\n"
        "prfh pstl3keep, p0, [z17.s]\n"
        "mov z22.h, #0xffe\n"
        "clasta b26, p1, b26, z11.b\n"
        "orr z16.h, z16.h, #0x3c00\n"
        "decp z6.h, p5.h\n"
        "mov z9.d, #0x3fffffffc0000\n"
        "mov z3.d, #0xfffffff0001fffff\n"
        "cnth x4, #19\n"
        "whilels p7.h, w10, wzr\n"
        "eor z1.d, z1.d, #0xffffe003ffffffff\n"
        "orr z5.d, z5.d, #0x3ffffffffffff00\n"
        "eor z20.d, z20.d, #0xffe0000fffffffff\n"
        "prfb pstl1strm, p2, [sp, z18.d, uxtw]\n"
        "orr z17.d, z17.d, #0xffff8003ffffffff\n"
        "eor z18.s, z18.s, #0xfffc00ff\n"
        "mov z11.d, #0xffff8000ffffffff\n"
        "ldff1b {z17.d}, p7/z, [sp, xzr]\n"
        "orr z14.s, z14.s, #0xe00fffff\n"
        "prfh pstl1keep, p5, [sp, #25, mul vl]\n"
        "ldnt1d {z12.d}, p5/z, [sp, #-4, mul vl]\n"
        "orr z8.d, z8.d, #0xfffffffff0000001\n"
        "dupm z0.b, #0x3c\n"
        "ld1sh {z24.d}, p1/z, [sp]\n"
        "uqinch w20, #20\n"
        "uqincd z2.d, #23\n"
        "dupm z26.s, #0xffffffef\n"
        "and z12.d, z12.d, #0xfffe0000\n"
        "mov z28.d, #0x3ffffffe000\n"
        "prfw pstl2keep, p7, [sp, z3.d, lsl #2]\n"
        "ld3b {z14.b-z16.b}, p6/z, [sp, #-21, mul vl]\n"
        "sqdecw x6, w6, #20, mul #11\n"
        "whilehi p7.s, w11, wzr\n"
        "and z2.s, z2.s, #0xff80ffff\n"
        "orr z2.d, z2.d, #0xfffffe00003fffff\n"
        "prfh pstl2keep, p7, [sp, z16.s, uxtw #1]\n"
        "mov z0.s, #0xffff07ff\n"
        "prfh pstl1keep, p3, [sp, z22.s, sxtw #1]\n"
        "aese z15.b, z15.b, z1.b\n"
        "mov z9.s, #0xf83fffff\n"
        "eor z25.s, z25.s, #0xff0003ff\n"
        "mov z11.s, #0xffff0fff\n"
        "mov z10.s, #0xfe00ffff\n"
        "dupm z3.b, #0xbb\n"
        "orr z6.d, z6.d, #0xffffffffffffffe3\n"
        "uqdech z18.h, #17\n"
        "sqincp x5, p14.h, w5\n"
        "and z24.d, z24.d, #0xfffc003fffffffff\n"
        "mov z5.s, #0xfffff7ff\n"
        "and z8.b, z8.b, #0x9f\n"
        "and z12.b, z12.b, #0xf\n"
        "prfd pldl1strm, p4, [sp, #-4, mul vl]\n"
        "orr z0.d, z0.d, #0xffffffffffc001ff\n"
        "eor z6.d, z6.d, #0xfe000000000\n"
        "orr z6.d, z6.d, #0xffffffff00000fff\n"
        "orr z7.s, z7.s, #0xfffffbff\n"
        "eor z21.h, z21.h, #0x3c\n"
        "mov z3.d, #0x8000000fffffffff\n"
        "and z14.d, z14.d, #0xffffffffff8fffff\n"
        "orr z15.d, z15.d, #0xfffffffffffffe0f\n"
        "and z8.s, z8.s, #0xfffff0ff\n"
        "and z18.d, z18.d, #0xfffffffff8000000\n"
        "sqincw xzr, wzr, vl32, mul #6\n"
        "and z1.s, z1.s, #0x1fe00\n"
        "mov z30.d, d26\n"
        "mov z23.d, #0xf9ffffffffffffff\n"
        "prfh pldl1strm, p1, [sp, z2.s, sxtw #1]\n"
        "prfd pldl1keep, p1, [sp, z24.d, lsl #3]\n"
        "orr z19.d, z19.d, #0xfffff80001ffffff\n"
        "mov z13.d, #0x1e0000\n"
        "prfh pstl1strm, p1, [x2]\n"
        "orr z18.s, z18.s, #0x1ffc\n"
        "eor z31.d, z31.d, #0xffffffff800007ff\n"
        "index z10.d, xzr, #4\n"
        "prfh #7, p2, [x10]\n"
        "uqdech x28, pow2\n"
        "and z19.d, z19.d, #0xffffc00000ffffff\n"
        "eor z10.d, z10.d, #0x7fffffff80\n"
        "uqdecb x13, all, mul #6\n"
        "prfw pstl1keep, p2, [sp, z26.s, sxtw #2]\n"
        "eor z24.d, z24.d, #0x1ffc\n"
        "mov z7.d, #0xf03fffffffffffff\n"
        "orr z9.d, z9.d, #0x1ffffffe000\n"
        "mov z10.h, #0xc1ff\n"
        "uqincd z14.d, all, mul #8\n"
        "ld4d {z28.d-z31.d}, p2/z, [sp]\n"
        "ldff1w {z23.s}, p5/z, [sp, xzr, lsl #2]\n"
        "mov z17.h, #0xfe01\n"
        "st1b {z27.h}, p3, [sp]\n"
        "and z12.h, z12.h, #0x3fff\n"
        "eor z30.b, z30.b, #0xee\n"
        "prfw pstl1strm, p4, [z23.s]\n"
        "uqdecp w12, p2.d\n"
        "orr z14.s, z14.s, #0xfffff81f\n"
        "eor z16.s, z16.s, #0x1ffffc0\n"
        "and z17.b, z17.b, #0x7c\n"
        "orr z5.s, z5.s, #0xfffffe1f\n"
        "eor z3.d, z3.d, #0x1ffe0000000\n"
        "and z3.h, z3.h, #0xff7f\n"
        "dech z28.h, #21\n"
        "ldnt1b {z16.b}, p3/z, [sp, #1, mul vl]\n"
        "whilehs p0.d, x6, xzr\n"
        "orr z19.d, z19.d, #0xffffffffffffe3ff\n"
        "orr z27.d, z27.d, #0x3ffffffffff\n"
        "and z3.b, z3.b, #0xe3\n"
        "mov z1.d, #0xfffff8000000007f\n"
        "eor z26.d, z26.d, #0xfe000000007fffff\n"
        "eor z12.s, z12.s, #0xfff8001f\n"
        "clasta h17, p5, h17, z17.h\n"
        "mov z28.d, #0xfffffffc000\n"
        "sqincd xzr, wzr, #24, mul #13\n"
        "mov z20.d, #0xffffffffffc3ffff\n"
        "stnt1d {z23.d}, p7, [sp]\n"
        "orr z23.b, z23.b, #0xbb\n"
        "orr z6.d, z6.d, #0xffffff800fffffff\n"
        "prfw #6, p7, [sp, z6.d, sxtw #2]\n"
        "eor z26.s, z26.s, #0x1ff\n"
        "eor z15.b, z15.b, #0xbb\n"
        "and z13.d, z13.d, #0xff001fffffffffff\n"
        "eor z14.s, z14.s, #0xffe00fff\n"
        "eor z10.s, z10.s, #0x1ffffffc\n"
        "mov z5.d, #0x1fffffffffffffe\n"
        "uqincd x28, all, mul #12\n"
        "mov z24.h, #0xfc0\n"
        "mov z7.s, #0xfc0001ff\n"
        "eor z12.s, z12.s, #0xffffe007\n"
        "uqxtnt z15.h, z4.s\n"
        "eor z8.b, z8.b, #0x3e\n"
        "orr z0.d, z0.d, #0x7fffffffffe\n"
        "and z26.d, z26.d, #0x3ffffffffe00000\n"
        "prfb pldl2keep, p3, [sp, z27.s, sxtw]\n"
        "stnt1d {z23.d}, p5, [sp, x7, lsl #3]\n"
        "eor z24.d, z24.d, #0xfffe07ffffffffff\n"
        "eor z11.h, z11.h, #0xfc0f\n"
        "eor z21.d, z21.d, #0xfff0000001ffffff\n"
        "whilegt p3.h, x2, xzr\n"
        "and z31.s, z31.s, #0xfffff87f\n"
        "whilewr p7.d, xzr, x28\n"
        "eor z6.s, z6.s, #0x8003ffff\n"
        "decp x15, p0.d\n"
        "eor z19.h, z19.h, #0xff8f\n"
        "whilege p6.b, xzr, x12\n"
        "sunpklo z10.s, z16.h\n"
        "and z26.d, z26.d, #0x3ffffffc0000\n"
        "whilele p13.h, xzr, x13\n"
        "orr z4.s, z4.s, #0xffc003ff\n"
        "and z17.d, z17.d, #0xffffff80000000ff\n"
        "mov z16.d, #0xffff00000000007f\n"
        "sqdecp x15, p4.d\n"
        "mov z6.d, #0xffffffffff81ffff\n"
        "ld1d {z27.d}, p1/z, [sp, #-7, mul vl]\n"
        "mov z20.d, #0x7fffffffffffffe0\n"
        "prfd pstl1keep, p1, [sp, z28.d, uxtw #3]\n"
        "mov z6.d, #0xffe001ffffffffff\n"
        "orr z10.s, z10.s, #0x7fc00\n"
        "mov z5.h, #0xf83f\n"
        "whilehs p1.b, xzr, x28\n"
        "mov z21.s, #0x7fffffe\n"
        "ld1rqw {z18.s}, p5/z, [sp, #64]\n"
        "eor z29.s, z29.s, #0xe01fffff\n"
        "eor z14.d, z14.d, #0xfe000001ffffffff\n"
        "uqdecb w30, #21, mul #11\n"
        "whilele p2.b, wzr, w20\n"
        "and z6.d, z6.d, #0x1ffffe0000\n"
        "orr z8.d, z8.d, #0x7fffe\n"
        "mov z17.d, #0xfffffe00003fffff\n"
        "eor z0.s, z0.s, #0xf\n"
        "mov z4.d, #0xffff800fffffffff\n"
        "mov z10.s, #0xffffff1f\n"
        "sqincp x0, p1.h\n"
        "stnt1d {z8.d}, p5, [sp, #-3, mul vl]\n"
        "and z1.d, z1.d, #0xfffffffbffffffff\n"
        "whilerw p0.s, xzr, x5\n"
        "orr z4.s, z4.s, #0xfffe03ff\n"
        "mov z8.d, #0xffff800000ffffff\n"
        "eor z28.d, z28.d, #0xffffffffffff9fff\n"
        "and z26.d, z26.d, #0xfffffffffffe0fff\n"
        "whilelo p9.b, x21, xzr\n"
        "orr z16.d, z16.d, #0x1fffffe\n"
        "ptrue p1.b, #26\n"
        "prfw pldl1keep, p6, [sp, x3, lsl #2]\n"
        "mov z16.d, #0xfe0000001fffffff\n"
        "orr z31.d, z31.d, #0xf87fffffffffffff\n"
        "eor z15.s, z15.s, #0xf80007ff\n"
        "dupm z29.b, #0xdf\n"
        "eor z28.s, z28.s, #0xffffff9f\n"
        "and z17.h, z17.h, #0x81ff\n"
        "orr z12.d, z12.d, #0xffffffc00000\n"
        "eor z21.s, z21.s, #0x1ffffe00\n"
        "mov z11.d, #0xff000000007fffff\n"
        "prfd pstl3keep, p5, [sp, #6, mul vl]\n"
        "eor z13.d, z13.d, #0xfffffffffffffe07\n"
        "eor z9.d, z9.d, #0x3fffffe0\n"
        "orr z9.d, z9.d, #0xffffffffffdfffff\n"
        "mov z2.d, #0x3fffffe0000000\n"
        "whilelo p15.h, xzr, x19\n"
        "and z22.d, z22.d, #0xffffffffffff807f\n"
        "mov z19.s, #0xfffe\n"
        "ldnf1sw {z27.d}, p2/z, [sp]\n"
        "mov z0.d, #0xfffffc000000003f\n"
        "orr z17.d, z17.d, #0x7fffffc\n"
        "orr z9.d, z9.d, #0xffc7ffffffffffff\n"
        "mov z31.d, #0x1ffffffff\n"
        "mov z3.s, #0x7fff000\n"
        "prfw pstl3strm, p4, [x2]\n"
        "orr z16.s, z16.s, #0xffffff1f\n"
        "orr z0.d, z0.d, #0xff80003fffffffff\n"
        "eor z2.d, z2.d, #0xfffffffff0ffffff\n"
        "orr z17.s, z17.s, #0xff03ffff\n"
        "orr z6.b, z6.b, #0x1f\n"
        "prfw pldl1keep, p5, [sp, z19.s, uxtw #2]\n"
        "orr z5.d, z5.d, #0xffffffff0000ffff\n"
        "orr z17.h, z17.h, #0xfc03\n"
        "prfh pldl3strm, p1, [sp, #20, mul vl]\n"
        "sqincw xzr, mul3, mul #7\n"
        "and z15.d, z15.d, #0xfffffffffff83fff\n"
        "lasta wzr, p4, z22.h\n"
        "and z8.d, z8.d, #0x7fffffffffffffc\n"
        "ld1roh {z5.h}, p3/z, [sp]\n"
        "whilelt p12.h, xzr, x2\n"
        "uqincp w9, p6.b\n"
        "orr z20.s, z20.s, #0xfffff000\n"
        "dupm z28.h, #0xffbf\n"
        "movs p15.b, p11.b\n"
        "and z0.d, z0.d, #0xffffffffc03fffff\n"
        "prfh #14, p0, [sp, z19.d, sxtw #1]\n"
        "index z17.d, x4, xzr\n"
        "mov z11.h, h0\n"
        "orr z2.d, z2.d, #0x8000000000001fff\n"
        "and z4.h, z4.h, #0xfeff\n"
        "orr z25.d, z25.d, #0x3ffffe\n"
        "st3h {z15.h-z17.h}, p2, [sp]\n"
        "mov z5.d, #0xffffffff83ffffff\n"
        "ld1rb {z11.d}, p5/z, [sp]\n"
        "and z0.d, z0.d, #0x1fffffffffe0\n"
        "mov z3.d, #0xffffffffff1fffff\n"
        "mov z30.d, #0xffffffffff80001f\n"
        "orr z27.d, z27.d, #0x7ffffffc\n"
        "eor z7.d, z7.d, #0xffffff0000000fff\n"
        "prfh #15, p1, [sp, z26.d, uxtw #1]\n"
        "dupm z20.h, #0xffef\n"
        "and z3.d, z3.d, #0xfffe0003ffffffff\n"
        "and z20.d, z20.d, #0x1fffffffff8000\n"
        "and z2.s, z2.s, #0x3fffe000\n"
        "mov z0.d, #0xffffffff0000003f\n"
        "mov z18.d, #0x1ffffffffe\n"
        "and z21.d, z21.d, #0xfffffff8000000ff\n"
        "prfb pldl2strm, p4, [sp, z5.s, sxtw]\n"
        "eor z21.d, z21.d, #0xffffffffffffcfff\n"
        "sqdech x14, all, mul #12\n"
        "whilelt p11.s, w16, wzr\n"
        "prfb pstl2keep, p7, [x23]\n"
        "mov z0.d, #0xffffc00003ffffff\n"
        "and z24.d, z24.d, #0xffffe3ffffffffff\n"
        "sqdecd z7.d, all, mul #14\n"
        "sqincd xzr, #25\n"
        "orr z19.d, z19.d, #0xfffffff80007ffff\n"
        "and z27.d, z27.d, #0xffff800001ffffff\n"
        "mov z19.s, #0x1ffffc\n"
        "orr z5.b, z5.b, #0xee\n"
        "whilegt p9.h, w6, wzr\n"
        "ld1sb {z12.d}, p1/z, [sp]\n"
        "prfw pldl3strm, p7, [sp, #-7, mul vl]\n"
        "prfd pstl3strm, p2, [x19]\n"
        "mov z21.d, #0xffffffffffe00007\n"
        "prfb #15, p1, [sp, #3, mul vl]\n"
        "orr z28.d, z28.d, #0xf007ffffffffffff\n"
        "orr z7.d, z7.d, #0x3ffffffc000000\n"
        "orr z4.b, z4.b, #0xfd\n"
        "prfb pldl1keep, p2, [x8]\n"
        "and z6.d, z6.d, #0xfffffc0000000001\n"
        "prfb pstl3strm, p6, [sp, #-3, mul vl]\n"
        "mov z3.d, #0xffffffffffe007ff\n"
        "mov z5.s, #0xfffffe7f\n"
        "eor z24.d, z24.d, #0xffffffff8000\n"
        "and z20.d, z20.d, #0xfffff8000fffffff\n"
        "st1w {z14.s}, p3, [sp]\n"
        "eor z11.d, z11.d, #0xfffffffffff007ff\n"
        "orr z31.d, z31.d, #0xfffffc0003ffffff\n"
        "mov z8.d, #0xffffffffffc0000f\n"
        "mov z18.d, #0xfffffffc000001ff\n"
        "eor z23.d, z23.d, #0xffffffffffffe0ff\n"
        "prfh pldl1strm, p1, [sp, z27.s, uxtw #1]\n"
        "prfb #7, p7, [sp, x3]\n"
        "mov z26.d, #0xffffffffe00003ff\n"
        "sqincp z16.h, p9.h\n"
        "eor z12.d, z12.d, #0xf80000000fffffff\n"
        "prfh pstl3strm, p2, [sp, z29.d, lsl #1]\n"
        "and z24.d, z24.d, #0x3fffffffc0\n"
        "mov z5.d, #0xffffffc000000001\n"
        "eor z4.s, z4.s, #0x7fffc\n"
        "eor z10.d, z10.d, #0xfffe000000007fff\n"
        "dupm z27.h, #0xfff3\n"
        "mov z5.d, #0xc0000000ffffffff\n"
        "and z22.d, z22.d, #0x7ffffffffffff8\n"
        "eor z5.d, z5.d, #0x1ffffffffffffff8\n"
        "prfb #6, p6, [sp, z14.s, uxtw]\n"
        "orr z30.s, z30.s, #0xc03fffff\n"
        "prfb pstl1keep, p2, [sp, z25.s, sxtw]\n"
        "and z21.d, z21.d, #0xfffffffff0003fff\n"
        "eor z15.s, z15.s, #0xfffffff8\n"
        "fmul z13.d, p6/m, z13.d, #0.5\n"
        "eor z13.d, z13.d, #0xffffe000000007ff\n"
        "orr z23.d, z23.d, #0xfffffffff800ffff\n"
        "eor z21.d, z21.d, #0xffffff\n"
        "orr z19.d, z19.d, #0xffffffffffc0000\n"
        "eor z8.d, z8.d, #0xffffffffc3ffffff\n"
        "and z27.d, z27.d, #0xffffffc001ffffff\n"
        "and z11.d, z11.d, #0xfff8000001ffffff\n"
        "mov z1.d, #0xfffff00000007fff\n"
        "and z19.s, z19.s, #0x7fffc0\n"
        "eor z3.d, z3.d, #0xffffffffffff8003\n"
        "inch xzr, vl32, mul #13\n"
        "eor z3.s, z3.s, #0xf801ffff\n"
        "and z4.d, z4.d, #0xffffffffffc0007f\n"
        "eor z7.s, z7.s, #0xe0007fff\n"
        "eor z21.d, z21.d, #0xfc000000ffffffff\n"
        "uqdecd z19.d, all, mul #4\n"
        "uqincd w6, #26\n"
        "mov z3.d, #0xffffff800000000\n"
        "eor z23.d, z23.d, #0xffff000000000fff\n"
        "eor z2.d, z2.d, #0xfffff000000007ff\n"
        "orr z7.d, z7.d, #0xfff00000ffffffff\n"
        "clastb xzr, p0, xzr, z27.d\n"
        "mov z28.d, #0x1ffffffffffc00\n"
        "orr z31.d, z31.d, #0xffffffffff87ffff\n"
        "orr z6.d, z6.d, #0xfffffffffffff003\n"
        "mov z14.d, #0xfff000ffffffffff\n"
        "eor z13.d, z13.d, #0xfffff8003fffffff\n"
        "ld1rqw {z6.s}, p0/z, [sp]\n"
        "whilege p13.d, wzr, w27\n"
        "prfh #14, p7, [sp, z29.d, lsl #1]\n"
        "eor z31.s, z31.s, #0xfff3ffff\n"
        "prfd pstl3keep, p6, [sp, z29.d, sxtw #3]\n"
        "mov z9.d, #0xf803ffffffffffff\n"
        "prfd pldl2keep, p2, [sp, z10.s, uxtw #3]\n"
        "eor z28.s, z28.s, #0xc000ffff\n"
        "ld1rd {z16.d}, p2/z, [sp]\n"
        "mov z11.h, #0xbfff\n"
        "and z24.d, z24.d, #0xffffc0000000007f\n"
        "sqdech xzr, wzr, #22\n"
        "and z3.d, z3.d, #0x3fffffffffff\n"
        "orr z25.d, z25.d, #0xffffe000003fffff\n"
        "and z28.d, z28.d, #0xffffffffffffbfff\n"
        "sqdecd x22, all, mul #11\n"
        "mov z4.d, #0xfff80000001fffff\n"
        "mov z13.d, #0x3ffffffff0\n"
        "and z21.d, z21.d, #0x3ffffffffff0\n"
        "eor z9.d, z9.d, #0xffffffe000007fff\n"
        "eor z20.d, z20.d, #0xffffffffffc0000f\n"
        "prfd #14, p2, [z12.s]\n"
        "mov z21.s, #0xfe007fff\n"
        "eor z31.d, z31.d, #0x3fffffffff\n"
        "and z6.d, z6.d, #0xffffffffffffc003\n"
        "orr z6.d, z6.d, #0xff00000007ffffff\n"
        "mov z7.h, #0x80ff\n"
        "mov z14.s, #0x1ffffffc\n"
        "sqinch x1, w1, #21\n"
        "eor z8.d, z8.d, #0xf80000ffffffffff\n"
        "mov z7.d, #0xfffffffffe\n"
        "prfb pldl1strm, p1, [sp, #9, mul vl]\n"
        "mov z4.s, #0x3ffc\n"
        "orr z12.d, z12.d, #0xc7ffffffffffffff\n"
        "mov z10.s, #0x1ffe\n"
        "clasta wzr, p7, wzr, z19.b\n"
        "mov z13.d, #0xffffc000ffffffff\n"
        "mov z13.h, #0xdfff\n"
        "clastb wzr, p1, wzr, z5.s\n"
        "and z18.d, z18.d, #0xfffc0000007fffff\n"
        "eor z16.d, z16.d, #0xc000ffffffffffff\n"
        "prfd pstl3strm, p2, [sp, #-21, mul vl]\n"
        "sqincb xzr, vl5, mul #5\n"
        "dupm z4.b, #0xbf\n"
        "eor z2.d, z2.d, #0xffff07ffffffffff\n"
        "orr z9.d, z9.d, #0xfe0003ffffffffff\n"
        "orr z22.d, z22.d, #0xffffe00003ffffff\n"
        "and z24.s, z24.s, #0xfffffcff\n"
        "mov z13.d, #0xfffc0000000000\n"
        "mov z7.d, #0xffffff80003fffff\n"
        "orr z28.h, z28.h, #0xfcff\n"
        "dupm z4.h, #0xff80\n"
        "prfh #7, p5, [z28.s]\n"
        "eor z30.d, z30.d, #0xfffe0003ffffffff\n"
        "orr z7.d, z7.d, #0xfe000000000007ff\n"
        "and z13.d, z13.d, #0xfffff800ffffffff\n"
        "sqdecd x0, #21\n"
        "and z12.s, z12.s, #0xffffff3f\n"
        "and z20.d, z20.d, #0xf8000001ffffffff\n"
        "mov z24.s, #0xfff000ff\n"
        "eor z2.s, z2.s, #0xfe0007ff\n"
        "and z6.d, z6.d, #0xffff80003fffffff\n"
        "uqdecb xzr, #25, mul #2\n"
        "eor z29.d, z29.d, #0xffffffff0000ffff\n"
        "orr z14.d, z14.d, #0x3ffffffffff000\n"
        "eor z15.h, z15.h, #0xe\n"
        "mov z30.s, #0x7ffffe\n"
        "prfb pstl2keep, p4, [z4.d]\n"
        "eor z15.s, z15.s, #0xfffc000\n"
        "orr z1.d, z1.d, #0xfff8001fffffffff\n"
        "prfd pstl2strm, p4, [sp, z13.d, uxtw #3]\n"
        "mov z7.d, #0xff80001fffffffff\n"
        "ld1rh {z21.d}, p1/z, [sp]\n"
        "and z6.d, z6.d, #0xffffffffffffe001\n"
        "eor z29.s, z29.s, #0xffffffcf\n"
        "mov z21.d, #0xffffffff80007fff\n"
        "eor z14.d, z14.d, #0xffc1ffffffffffff\n"
        "orr z27.d, z27.d, #0xfffffffc0000000f\n"
        "uqincp x23, p14.s\n"
        "mov z26.s, #0xffe000ff\n"
        "and z4.d, z4.d, #0xfe7fffffffffffff\n"
        "eor z22.s, z22.s, #0xfc1fffff\n"
        "and z10.h, z10.h, #0xffe0\n"
        "eor z15.d, z15.d, #0xc00000001fffffff\n"
        "sqdecp x13, p4.s, w13\n"
        "orr z28.h, z28.h, #0xfffe\n"
        "and z24.s, z24.s, #0xffc03fff\n"
        "prfw pldl3keep, p7, [sp, z25.d, sxtw #2]\n"
        "and z24.s, z24.s, #0xff8007ff\n"
        "orr z18.d, z18.d, #0x3fffffffffe00000\n"
        "eor z2.h, z2.h, #0xdfff\n"
        "and z23.s, z23.s, #0xc0001fff\n"
        "orr z22.h, z22.h, #0xc0ff\n"
        "orr z24.d, z24.d, #0x7fffffc00000\n"
        "orr z28.d, z28.d, #0xffffffffc00003ff\n"
        "and z28.s, z28.s, #0xff000fff\n"
        "and z17.d, z17.d, #0xfffffffffffffc3f\n"
        "mov z29.d, #0xffffe000\n"
        "orr z2.d, z2.d, #0xfffffe00000003ff\n"
        "ldnt1b {z0.b}, p1/z, [sp]\n"
        "mov z2.s, #0xfff07fff\n"
        "orr z12.b, z12.b, #0xf\n"
        "and z12.d, z12.d, #0xfffffffe00\n"
        "mov z10.s, #0x7fe0000\n"
        "mov z18.d, #0xfffffffe07ffffff\n"
        "sqincb xzr, wzr, vl1, mul #3\n"
        "mov z15.d, #0xfe07ffffffffffff\n"
        "dupm z3.h, #0xc000\n"
        "prfw pstl3keep, p3, [z13.d]\n"
        "and z10.d, z10.d, #0xffffffffff80003f\n"
        "mov z11.d, #0xfff800000003ffff\n"
        "mov z0.d, #0xffffffffffff0000\n"
        "and z24.h, z24.h, #0xffc\n"
        "and z9.d, z9.d, #0x1fc0000\n"
        "eor z13.d, z13.d, #0xfffffffffc000007\n"
        "orr z23.s, z23.s, #0xffff0\n"
        "prfb pstl3strm, p3, [sp, x17]\n"
        "orr z4.s, z4.s, #0xffffc000\n"
        "orr z22.s, z22.s, #0xffe0000f\n"
        "uqincb x17, mul4\n"
        "decd xzr, #15, mul #16\n"
        "and z19.d, z19.d, #0x3ffffffffffc\n"
        "and z15.d, z15.d, #0xffffffc3ffffffff\n"
        "and z25.d, z25.d, #0xffffe000003fffff\n"
        "mov z27.d, #0xfffffffe003fffff\n"
        "mov z25.d, #0xfc01ffffffffffff\n"
        "cntd xzr, vl256, mul #3\n"
        "prfd pldl2keep, p4, [z2.s]\n"
        "whilegt p5.b, xzr, x18\n"
        "and z12.h, z12.h, #0xc\n"
        "prfh pldl1strm, p3, [x9]\n"
        "and z18.d, z18.d, #0x8000001fffffffff\n"
        "eor z12.d, z12.d, #0x3fffffffffe00\n"
        "and z26.h, z26.h, #0xf8ff\n"
        "and z25.d, z25.d, #0xe001ffffffffffff\n"
        "uqincd xzr, vl256, mul #10\n"
        "prfd #6, p2, [x9]\n"
        "prfb #14, p6, [z29.s]\n"
        "orr z7.s, z7.s, #0xfff801ff\n"
        "eor z0.d, z0.d, #0xffffe00000007fff\n"
        "sqdecd xzr, wzr, vl32, mul #14\n"
        "eor z16.d, z16.d, #0xfffffffffffe7fff\n"
        "and z16.d, z16.d, #0xc07fffffffffffff\n"
        "uqdecw x10, #26\n"
        "orr z17.d, z17.d, #0xe00000000fffffff\n"
        "mov z12.d, #0xffffe0003fffffff\n"
        "prfw pstl1strm, p4, [sp, x2, lsl #2]\n"
        "mov z21.d, #0xffffe00007ffffff\n"
        "mov z15.d, #0xfffffffeffffffff\n"
        "prfd pstl3keep, p1, [sp, z16.d, lsl #3]\n"
        "uqdecd x0, all, mul #16\n"
        "sqdecd x17, w17, #14\n"
        "orr z26.s, z26.s, #0xf000ffff\n"
        "mov z21.d, #0x1fffffff\n"
        "orr z26.d, z26.d, #0xff81ffffffffffff\n"
        "prfw pldl1strm, p0, [sp, z31.s, uxtw #2]\n"
        "and z5.d, z5.d, #0xff81ffffffffffff\n"
        "eor z30.s, z30.s, #0xfffe00ff\n"
        "mov z30.s, #0xfc003fff\n"
        "sqdecb x7, pow2, mul #2\n"
        "eor z14.s, z14.s, #0xefffffff\n"
        "prfh pstl3strm, p2, [sp, #-22, mul vl]\n"
        "and z6.d, z6.d, #0xfffffffff\n"
        "and z25.s, z25.s, #0x7ffe\n"
        "mov z24.d, #0xff800000000003ff\n"
        "and z16.d, z16.d, #0xfffffffffff0ffff\n"
        "index z13.d, #13, xzr\n"
        "mov z14.d, #0xfffffffff8ffffff\n"
        "orr z5.d, z5.d, #0x3fffffff00\n"
        "mov z1.d, #0x7ffc000000000000\n"
        "orr z20.h, z20.h, #0xfdff\n"
        "orr z6.d, z6.d, #0x1fffffffffc00000\n"
        "and z6.d, z6.d, #0x1fffffe00\n"
        "eor z12.d, z12.d, #0xfffffc000000001f\n"
        "orr z19.d, z19.d, #0x3ffffffffffc0000\n"
        "eor z0.d, z0.d, #0xfffffffc0000000\n"
        "eor z0.d, z0.d, #0xfffffffffe00001f\n"
        "sqincd x7, #27\n"
        "incb xzr, vl16, mul #16\n"
        "mov z20.s, #0xffc\n"
        "eor z13.d, z13.d, #0xf0007fffffffffff\n"
        "mov z3.d, #0xfffffffc7fffffff\n"
        "orr z18.d, z18.d, #0xfffc00000000003f\n"
        "prfw pstl1keep, p3, [x23]\n"
        "mov z0.d, #0xfff000001fffffff\n"
        "and z24.d, z24.d, #0xffffffffe00000ff\n"
        "eor z15.d, z15.d, #0xffffffffffcfffff\n"
        "and z18.d, z18.d, #0xfffffffffffc001f\n"
        "mov z28.d, #0xff803fffffffffff\n"
        "and z25.d, z25.d, #0xffffffffffff8fff\n"
        "orr z21.d, z21.d, #0xffffffffffffe00\n"
        "eor z16.s, z16.s, #0xffffefff\n"
        "orr z10.s, z10.s, #0x87ffffff\n"
        "eor z23.d, z23.d, #0x3ffffffffffff800\n"
        "ptrues p3.d, #19\n"
        "and z23.d, z23.d, #0xffffffe0007fffff\n"
        "and z11.d, z11.d, #0xc0007fffffffffff\n"
        "and z25.d, z25.d, #0x3fffffffffff8\n"
        "eor z20.d, z20.d, #0x7ffc0000000000\n"
        "and z27.s, z27.s, #0xffffffe\n"
        "eor z19.d, z19.d, #0xff0000000001ffff\n"
        "prfw #7, p1, [sp, x21, lsl #2]\n"
        "sqdecd xzr, mul4, mul #12\n"
        "mov z5.d, #0xffffffff003fffff\n"
        "eor z14.d, z14.d, #0xfffffffe00000007\n"
        "orr z1.d, z1.d, #0xc00007ffffffffff\n"
        "and z24.d, z24.d, #0xfffffe00000003ff\n"
        "mov z17.d, #0xf000000001ffffff\n"
        "mov z1.s, #0x3fffe0\n"
        "uqdecw w24, #15\n"
        "mov z31.d, #0xfffffffff01fffff\n"
        "orr z25.d, z25.d, #0xfffffffffbffffff\n"
        "prfb #14, p3, [sp, z30.s, sxtw]\n"
        "orr z13.d, z13.d, #0xfffffffe001fffff\n"
        "and z26.d, z26.d, #0xfffffe7fffffffff\n"
        "eor z26.d, z26.d, #0x1ffffffffffc\n"
        "orr z2.s, z2.s, #0xfffe7fff\n"
        "orr z0.s, z0.s, #0xfffe000f\n"
        "orr z7.d, z7.d, #0xff000003ffffffff\n"
        "eor z27.d, z27.d, #0xffffffc001ffffff\n"
        "mov z16.d, #0xffffffffe000000\n"
        "eor z8.d, z8.d, #0xfffe00000000001f\n"
        "and z6.d, z6.d, #0xffffffffff01ffff\n"
        "mov z2.d, #0xfffffffffffe3fff\n"
        "eor z23.d, z23.d, #0xffffffffffff87ff\n"
        ".globl axiom_asm_table_end\n"
        "axiom_asm_table_end:\n"
        ".text\n");

extern const unsigned char axiom_asm_table[], axiom_asm_table_end[];
static const char *const want[] = {
    "046a09a5",  /* ldff1sh {z4.d}, p2/z, [x16, x9, lsl #1] */
    "e0cf63c5",  /* ld1w {z0.d}, p3/z, [sp, z3.d, lsl #2] */
    "4af339c4",  /* ldff1b {z10.d}, p4/z, [z26.d, #25] */
    "14ee94e5",  /* stnt1d {z20.d}, p3, [x16, #4, mul vl] */
    "2b0d8444",  /* sqdmlslbt z11.s, z9.h, z4.h */
    "d7e78ba4",  /* ldnt1h {z23.h}, p1/z, [x30, #-5, mul vl] */
    "df5a6ec5",  /* ld1w {z31.d}, p6/z, [x22, z14.d, sxtw #2] */
    "7685aae5",  /* st1d {z22.d}, p1, [x11, z10.d, uxtw #3] */
    "b2ef43c5",  /* ldff1w {z18.d}, p3/z, [x29, z3.d] */
    "0b3cd245",  /* uabdlt z11.d, z0.s, z18.s */
    "423dac84",  /* ldff1sh {z2.s}, p7/z, [x10, z12.s, uxtw #1] */
    "b9726e04",  /* sqdmulh z25.h, z21.h, z14.h */
    "57c335e5",  /* st1w {z23.d}, p0, [x26, z21.d, sxtw #2] */
    "007fa684",  /* ldff1h {z0.s}, p7/z, [x24, z6.s, uxtw #1] */
    "8f9e2045",  /* match p15.b, p7/z, z20.b, z0.b */
    "4d7b18a4",  /* ldff1b {z13.b}, p6/z, [x26, x24] */
    "a9c46aa4",  /* ld4b {z9.b-z12.b}, p1/z, [x5, x10] */
    "7ed6e745",  /* histcnt z30.d, p5/z, z19.d, z7.d */
    "56e3afa4",  /* ld2h {z22.h-z23.h}, p0/z, [x26, #-2, mul vl] */
    "b2d23ac4",  /* ld1b {z18.d}, p4/z, [z21.d, #26] */
    "93f575c5",  /* ldff1w {z19.d}, p5/z, [x12, z21.d, lsl #2] */
    "1ce3bd84",  /* ldff1h {z28.s}, p0/z, [z24.s, #58] */
    "c82a85a5",  /* ld1rqd {z8.d}, p2/z, [x22, #80] */
    "1ba69144",  /* addp z27.s, p1/m, z27.s, z16.s */
    "d33daea4",  /* ld1roh {z19.h}, p7/z, [x14, #-64] */
    "42ceeec5",  /* ld1d {z2.d}, p3/z, [x18, z14.d, lsl #3] */
    "07a456c5",  /* ldff1sw {z7.d}, p1/z, [x0, z22.d] */
    "0726ed84",  /* ldff1sh {z7.s}, p1/z, [x16, z13.s, sxtw #1] */
    "26c58fc4",  /* ldnt1h {z6.d}, p1/z, [z9.d, x15] */
    "e7b1fd44",  /* umlslb z7.d, z15.s, z13.s[2] */
    "8ab010e4",  /* st1b {z10.d}, p4, [x4, z16.d] */
    "436324e4",  /* st2b {z3.b-z4.b}, p0, [x26, x4] */
    "8fa74184",  /* ld1rb {z15.h}, p1/z, [x28, #1] */
    "9fb732c5",  /* ldff1sw {z31.d}, p5/z, [z28.d, #72] */
    "f6bef8c4",  /* ldff1sh {z22.d}, p7/z, [x23, z24.d, lsl #1] */
    "16929284",  /* ldnt1sh {z22.s}, p4/z, [z16.s, x18] */
    "b8554344",  /* smlslt z24.h, z13.b, z3.b */
    "1a54e8c4",  /* ld1h {z26.d}, p5/z, [x0, z8.d, sxtw #1] */
    "2450b0c4",  /* ld1h {z4.d}, p4/z, [x1, z16.d, uxtw #1] */
    "abd35be4",  /* st1b {z11.s}, p4, [x29, z27.s, sxtw] */
    "7f775dc5",  /* ldff1w {z31.d}, p5/z, [x27, z29.d, sxtw] */
    "827dc6a4",  /* ldff1h {z2.s}, p7/z, [x12, x6, lsl #1] */
    "9840b944",  /* cdot z24.s, z4.b, z1.b[3], #0 */
    "e8c019a4",  /* ldnt1b {z8.b}, p0/z, [x7, x25] */
    "0168d9e5",  /* st3d {z1.d-z3.d}, p2, [x0, x25, lsl #3] */
    "f1ebc485",  /* ld1rd {z17.d}, p2/z, [sp, #32] */
    "8c59aac5",  /* ld1d {z12.d}, p6/z, [x12, z10.d, uxtw #3] */
    "459d4ec5",  /* ld1sw {z5.d}, p7/z, [x10, z14.d] */
    "14a3b8a5",  /* ldnf1sb {z20.s}, p0/z, [x24, #-8, mul vl] */
    "9580ea05",  /* clasta d21, p0, d21, z4.d */
    "cd95a984",  /* ld1sh {z13.s}, p5/z, [z14.s, #18] */
    "aa056245",  /* sqshrunt z10.s, z13.d, #30 */
    "241d3145",  /* rshrnt z4.h, z9.s, #15 */
    "42c287a4",  /* ldnt1h {z2.h}, p0/z, [x18, x7, lsl #1] */
    "dd26c9e4",  /* stnt1h {z29.s}, p1, [z22.s, x9] */
    "931a8ba5",  /* ld1rqd {z19.d}, p6/z, [x20, x11, lsl #3] */
    "27ec0e45",  /* ursra z7.b, z1.b, #2 */
    "65805c85",  /* ld1rsh {z5.d}, p0/z, [x3, #56] */
    "da7bf0e4",  /* st4h {z26.h-z29.h}, p6, [x30, x16, lsl #1] */
    "ef07fd84",  /* ld1sh {z15.s}, p1/z, [sp, z29.s, sxtw #1] */
    "69126944",  /* sqrdmlah z9.h, z19.h, z1.h[5] */
    "6a3c4005",  /* eor z10.h, z10.h, #0x1e00 */
    "b369f2c5",  /* ldff1d {z19.d}, p2/z, [x13, z18.d, sxtw #3] */
    "4eed1584",  /* prfb #14, p3, [z10.s, #21] */
    "5896dce4",  /* st1h {z24.s}, p5, [x18, z28.s, uxtw] */
    "02415fc4",  /* ld1b {z2.d}, p0/z, [x8, z31.d, sxtw] */
    "8b9ddac4",  /* ld1sh {z11.d}, p7/z, [x12, z26.d] */
    "78c21fc4",  /* ldnt1b {z24.d}, p0/z, [z19.d, xzr] */
    "c4514f45",  /* ssubwb z4.h, z14.h, z15.b */
    "3ed07685",  /* ld1rw {z30.s}, p4/z, [x1, #216] */
    "97325844",  /* sqrdcmlah z23.h, z20.h, z24.h, #0 */
    "c2ee4205",  /* eor z2.d, z2.d, #0xfffffff803ffffff */
    "f2a6b644",  /* smlslt z18.s, z23.h, z6.h[4] */
    "32d0d105",  /* fmov z18.d, p1/m, #-2.125000000000000000e+00 */
    "af83e6e4",  /* st1h {z15.s}, p0, [x29, z6.s, uxtw #1] */
    "c6578c45",  /* ssubwt z6.s, z30.s, z12.h */
    "ebc1e1e4",  /* st1h {z11.s}, p0, [x15, z1.s, sxtw #1] */
    "798240c4",  /* ld1sb {z25.d}, p0/z, [x19, z0.d] */
    "54d568c5",  /* ld1w {z20.d}, p5/z, [x10, z8.d, lsl #2] */
    "60d6e085",  /* ld1rsb {z0.h}, p5/z, [x19, #32] */
    "46cfd1e4",  /* st1h {z6.s}, p3, [x26, z17.s, sxtw] */
    "13766ea5",  /* ldff1w {z19.d}, p5/z, [x16, x14, lsl #2] */
    "16cf4ac4",  /* ld1b {z22.d}, p3/z, [x24, z10.d] */
    "c2f10c84",  /* prfb pldl2keep, p4, [z14.s, #12] */
    "f5a679e5",  /* st1w {z21.s}, p1, [z23.s, #100] */
    "735f4445",  /* usubwt z19.h, z27.h, z4.b */
    "561192c4",  /* ld1sh {z22.d}, p4/z, [x10, z18.d, uxtw] */
    "cc6564e5",  /* st4w {z12.s-z15.s}, p1, [x14, x4, lsl #2] */
    "63279205",  /* mov z3.s, p2/z, #15104 */
    "758f46e5",  /* st1w {z21.s}, p3, [x27, z6.s, uxtw] */
    "a1f6f4e4",  /* st4h {z1.h-z4.h}, p5, [x21, #16, mul vl] */
    "999df7c4",  /* ld1sh {z25.d}, p7/z, [x12, z23.d, lsl #1] */
    "806f22c4",  /* prfd pldl1keep, p3, [x28, z2.d, uxtw #3] */
    "bed9b0a5",  /* ld2d {z30.d-z31.d}, p6/z, [x13, x16, lsl #3] */
    "4efefe85",  /* ld1rd {z14.d}, p7/z, [x18, #496] */
    "0af7ee04",  /* uqincd w10, #24, mul #15 */
    "207967c5",  /* ldff1w {z0.d}, p6/z, [x9, z7.d, sxtw #2] */
    "99ef7905",  /* mov z25.h, p11/m, z28.h */
    "4805fb84",  /* ld1sh {z8.s}, p1/z, [x10, z27.s, sxtw #1] */
    "d0260ec5",  /* ldff1sw {z16.d}, p1/z, [x22, z14.d, uxtw] */
    "b86442c4",  /* ldff1b {z24.d}, p1/z, [x5, z2.d, sxtw] */
    "5f2e80e5",  /* stnt1d {z31.d}, p3, [z18.d, x0] */
    "b8b03ae5",  /* st1w {z24.d}, p4, [x5, z26.d, lsl #2] */
    "1a0b6905",  /* ext z26.b, {z24.b-z25.b}, #74 */
    "49fa3104",  /* sqdecb x9, #18, mul #2 */
    "500fb684",  /* ld1sh {z16.s}, p3/z, [x26, z22.s, uxtw #1] */
    "9342c145",  /* saddwb z19.d, z20.d, z1.s */
    "6e16c585",  /* prfb #14, p5, [x19, #5, mul vl] */
    "68f2c384",  /* ld1rh {z8.d}, p4/z, [x19, #6] */
    "5aa871e4",  /* st1b {z26.s}, p2, [z2.s, #17] */
    "00473b85",  /* ld1w {z0.s}, p1/z, [x24, z27.s, uxtw #2] */
    "80df44c5",  /* ld1w {z0.d}, p7/z, [x28, z4.d] */
    "a475ec85",  /* prfd pldl3keep, p5, [x13, #-20, mul vl] */
    "1fee24a4",  /* ld2b {z31.b-z0.b}, p3/z, [x16, #8, mul vl] */
    "59b97ba5",  /* ldnf1w {z25.d}, p6/z, [x10, #-5, mul vl] */
    "f7e9c0a4",  /* ld3h {z23.h-z25.h}, p2/z, [x15] */
    "2f2c12c4",  /* ldff1sb {z15.d}, p3/z, [x1, z18.d, uxtw] */
    "d5b38044",  /* urecpe z21.s, p4/m, z30.s */
    "9def50c4",  /* ldff1b {z29.d}, p3/z, [x28, z16.d] */
    "fc949de5",  /* st1d {z28.d}, p5, [x7, z29.d, uxtw] */
    "5bc9e7a4",  /* ld4h {z27.h-z30.h}, p2/z, [x10, x7, lsl #1] */
    "58bdffa5",  /* ldnf1d {z24.d}, p7/z, [x10, #-1, mul vl] */
    "44a3b7a4",  /* ldnf1h {z4.h}, p0/z, [x26, #7, mul vl] */
    "2a30f585",  /* prfh pstl2keep, p4, [x1, #-11, mul vl] */
    "f067a544",  /* cmla z16.h, z31.h, z5.h[0], #90 */
    "78b1d644",  /* sminp z24.d, p4/m, z24.d, z11.d */
    "cc77e885",  /* prfd pstl3keep, p5, [x30, #-24, mul vl] */
    "6801f685",  /* prfb pstl1keep, p0, [x11, #-10, mul vl] */
    "7c680dc4",  /* ldff1b {z28.d}, p2/z, [x3, z13.d, uxtw] */
    "603705e5",  /* stnt1w {z0.d}, p5, [z27.d, x5] */
    "06e7a7c5",  /* ldff1d {z6.d}, p1/z, [z24.d, #56] */
    "f8cbf644",  /* smullb z24.d, z31.s, z6.s[3] */
    "de504d44",  /* smlslb z30.h, z6.b, z13.b */
    "9c1e5f45",  /* usublt z28.h, z20.b, z31.b */
    "a3e4cfc4",  /* ldff1h {z3.d}, p1/z, [x5, z15.d] */
    "fde3ffe4",  /* st4h {z29.h-z0.h}, p0, [sp, #-4, mul vl] */
    "052233c4",  /* prfh pldl3strm, p0, [x16, z19.d, uxtw #1] */
    "89e68245",  /* usra z9.d, z20.d, #62 */
    "7c0bc684",  /* ld1sh {z28.s}, p2/z, [x27, z6.s, sxtw] */
    "37dd95e5",  /* st1d {z23.d}, p7, [x9, z21.d, sxtw] */
    "fb381de4",  /* stnt1b {z27.d}, p6, [z7.d, x29] */
    "d58204c5",  /* ldnt1sw {z21.d}, p0/z, [z22.d, x4] */
    "39196ac5",  /* ld1sw {z25.d}, p6/z, [x9, z10.d, sxtw #2] */
    "f993e3c4",  /* ld1sh {z25.d}, p4/z, [sp, z3.d, lsl #1] */
    "ea26c9c4",  /* ldff1sh {z10.d}, p1/z, [x23, z9.d, sxtw] */
    "276665c4",  /* prfd #7, p1, [x17, z5.d, sxtw #3] */
    "091591a4",  /* ld1rqh {z9.h}, p5/z, [x8, x17, lsl #1] */
    "58ff17e4",  /* stnt1b {z24.b}, p7, [x26, #7, mul vl] */
    "4a4eeac5",  /* ld1d {z10.d}, p3/z, [x18, z10.d, sxtw #3] */
    "7beb88a5",  /* ldnt1d {z27.d}, p2/z, [x27, #-8, mul vl] */
    "9eeefdc5",  /* ldff1d {z30.d}, p3/z, [x20, z29.d, lsl #3] */
    "a0a24145",  /* sshllb z0.d, z21.s, #1 */
    "0d0973c4",  /* prfb pstl3strm, p2, [x8, z19.d, sxtw] */
    "b1a9d544",  /* umaxp z17.d, p2/m, z17.d, z13.d */
    "20f0d145",  /* sri z0.d, z1.d, #15 */
    "a3bc0985",  /* ldnt1w {z3.s}, p7/z, [z5.s, x9] */
    "5e609de4",  /* stnt1h {z30.h}, p0, [x2, x29, lsl #1] */
    "eaea1ec5",  /* prfw pstl2keep, p2, [z23.d, #120] */
    "e2e024a5",  /* ld2w {z2.s-z3.s}, p0/z, [x7, #8, mul vl] */
    "7ccaa584",  /* ld1h {z28.s}, p2/z, [z19.s, #10] */
    "9476fc04",  /* sqrdmulh z20.d, z20.d, z28.d */
    "930c4c84",  /* ld1sb {z19.s}, p3/z, [x4, z12.s, sxtw] */
    "5eaabfc4",  /* ldff1sh {z30.d}, p2/z, [z18.d, #62] */
    "55265dc5",  /* ldff1sw {z21.d}, p1/z, [x18, z29.d, sxtw] */
    "43540484",  /* ld1b {z3.s}, p5/z, [x2, z4.s, uxtw] */
    "0fe956e5",  /* st3w {z15.s-z17.s}, p2, [x8, #18, mul vl] */
    "26c78ba5",  /* ldnt1d {z6.d}, p1/z, [x25, x11, lsl #3] */
    "05c40ea5",  /* ldnt1w {z5.s}, p1/z, [x0, x14, lsl #2] */
    "706ffb84",  /* ldff1h {z16.s}, p3/z, [x27, z27.s, sxtw #1] */
    "d93198c4",  /* ldff1sh {z25.d}, p4/z, [x14, z24.d, uxtw] */
    "62af9f84",  /* ldnt1h {z2.s}, p3/z, [z27.s, xzr] */
    "8331d185",  /* prfh pldl2strm, p4, [x12, #17, mul vl] */
    "f9804a45",  /* saddlbt z25.h, z7.b, z10.b */
    "755792c4",  /* ld1h {z21.d}, p5/z, [x27, z18.d, uxtw] */
    "b3344a45",  /* sabdlt z19.h, z5.b, z10.b */
    "d79d21e5",  /* st1w {z23.d}, p7, [x14, z1.d, uxtw #2] */
    "141daa44",  /* sudot z20.s, z8.b, z2.b[1] */
    "2cfddde5",  /* st3d {z12.d-z14.d}, p7, [x9, #-9, mul vl] */
    "ffa08be4",  /* st1h {z31.d}, p0, [x7, z11.d] */
    "5b7282c5",  /* ldff1d {z27.d}, p4/z, [x18, z2.d, uxtw] */
    "4dcae604",  /* sqdecd z13.d, #18, mul #7 */
    "3abc0b84",  /* ldnt1b {z26.s}, p7/z, [z1.s, x11] */
    "f0a91be5",  /* st1w {z16.d}, p2, [x15, z27.d] */
    "1bd025c5",  /* ld1w {z27.d}, p4/z, [z0.d, #20] */
    "bedb43e5",  /* st1w {z30.s}, p6, [x29, z3.s, sxtw] */
    "4dd434a5",  /* ld2w {z13.s-z14.s}, p5/z, [x2, x20, lsl #2] */
    "8b1d0bc5",  /* ld1sw {z11.d}, p7/z, [x12, z11.d, uxtw] */
    "c8f9a0c5",  /* ldff1d {z8.d}, p6/z, [z14.d] */
    "3de43085",  /* ldff1w {z29.s}, p1/z, [z1.s, #64] */
    "dee8f344",  /* sqdmullb z30.d, z6.s, z3.s[3] */
    "df17e164",  /* fcmla z31.s, z30.s, z1.s[0], #90 */
    "20d4bd44",  /* umullt z0.s, z1.h, z5.h[6] */
    "68912f84",  /* ld1sb {z8.s}, p4/z, [z11.s, #15] */
    "12cfd1c5",  /* ld1d {z18.d}, p3/z, [x24, z17.d] */
    "4de032e4",  /* st2b {z13.b-z14.b}, p0, [x2, #4, mul vl] */
    "21a09be5",  /* st1d {z1.d}, p0, [x1, z27.d] */
    "a26213c5",  /* ldff1w {z2.d}, p0/z, [x21, z19.d, uxtw] */
    "e18565c4",  /* prfb pldl1strm, p1, [x15, z5.d] */
    "17fb4445",  /* saba z23.h, z24.h, z4.h */
    "be6797e5",  /* stnt1d {z30.d}, p1, [x29, x23, lsl #3] */
    "266f37c5",  /* ldff1w {z6.d}, p3/z, [x25, z23.d, uxtw #2] */
    "ea599644",  /* umlslb z10.s, z15.h, z22.h */
    "c64f00c5",  /* ld1w {z6.d}, p3/z, [x30, z0.d, uxtw] */
    "9d5bcf45",  /* usubwb z29.d, z28.d, z15.s */
    "9eff7704",  /* uqdech x30, #28, mul #8 */
    "29626f84",  /* prfd pstl1strm, p0, [x17, z15.s, sxtw #3] */
    "8ea9b7e4",  /* st1h {z14.d}, p2, [x12, z23.d, lsl #1] */
    "98daa944",  /* umullb z24.s, z20.h, z1.h[3] */
    "c5f16204",  /* sqinch x5, w5, #14, mul #3 */
    "8f179245",  /* ssublt z15.s, z28.h, z18.h */
    "a645cdc4",  /* ld1h {z6.d}, p1/z, [x13, z13.d, sxtw] */
    "a9b653c4",  /* ldff1sb {z9.d}, p5/z, [x21, z19.d] */
    "61918a45",  /* eorbt z1.s, z11.s, z10.s */
    "04656884",  /* prfd pldl3keep, p1, [x8, z8.s, sxtw #3] */
    "06213bc5",  /* ldff1sw {z6.d}, p0/z, [x8, z27.d, uxtw #2] */
    "4bc7e844",  /* smullt z11.d, z26.s, z8.s[0] */
    "c2a393a4",  /* ldnf1sw {z2.d}, p0/z, [x30, #3, mul vl] */
    "6e0fbda4",  /* ld1roh {z14.h}, p3/z, [x27, x29, lsl #1] */
    "f477e9c4",  /* ldff1h {z20.d}, p5/z, [sp, z9.d, sxtw #1] */
    "717386a4",  /* ldff1sw {z17.d}, p4/z, [x27, x6, lsl #2] */
    "38669944",  /* sqdmlalt z24.s, z17.h, z25.h */
    "64afaf04",  /* adr z4.s, [z27.s, z15.s, lsl #3] */
    "e76cd1a5",  /* ldff1sb {z7.h}, p3/z, [x7, x17] */
    "c9be71c5",  /* ldff1sw {z9.d}, p7/z, [x22, z17.d, lsl #2] */
    "522124a5",  /* ld1row {z18.s}, p0/z, [x10, #128] */
    "5dda95c5",  /* ldnt1d {z29.d}, p6/z, [z18.d, x21] */
    "4dd40085",  /* prfw pstl3strm, p5, [x2, x0, lsl #2] */
    "fc296745",  /* sqrshrnb z28.s, z15.d, #25 */
    "aa4e4c85",  /* ld1w {z10.s}, p3/z, [x21, z12.s, sxtw] */
    "2ed9cac4",  /* ld1h {z14.d}, p6/z, [x9, z10.d] */
    "3d7689c4",  /* ldff1h {z29.d}, p5/z, [x17, z9.d, uxtw] */
    "320510c4",  /* ld1sb {z18.d}, p1/z, [x9, z16.d, uxtw] */
    "5dc8f0c4",  /* ld1h {z29.d}, p2/z, [x2, z16.d, lsl #1] */
    "e3aaea44",  /* smlslb z3.d, z23.s, z10.s[1] */
    "f1031aa5",  /* ld1rqw {z17.s}, p0/z, [sp, x26, lsl #2] */
    "f0005845",  /* saddlb z16.h, z7.b, z24.b */
    "1fa43104",  /* adr z31.d, [z0.d, z17.d, sxtw #1] */
    "acdf8184",  /* prfh pstl3keep, p7, [x29, x1, lsl #1] */
    "4a65d345",  /* sqdmullt z10.d, z10.s, z19.s */
    "4a6ad345",  /* pmullb z10.d, z18.s, z19.s */
    "7c961de4",  /* st1b {z28.d}, p5, [x19, z29.d, uxtw] */
    "3a1717a5",  /* ld1rqw {z26.s}, p5/z, [x25, x23, lsl #2] */
    "c4b73b84",  /* ldff1sb {z4.s}, p5/z, [z30.s, #27] */
    "2993d444",  /* srhadd z9.d, p4/m, z9.d, z25.d */
    "2269d744",  /* sqdmlslb z2.d, z9.s, z23.s */
    "9dc6a004",  /* uqincw z29.s, #20 */
    "76710305",  /* orr z22.d, z22.d, #0x3ffc0000 */
    "b22305a4",  /* ld1rqb {z18.b}, p0/z, [x29, #80] */
    "baa64305",  /* eor z26.d, z26.d, #0xfffffffffffff003 */
    "85134fc5",  /* ld1sw {z5.d}, p4/z, [x28, z15.d, sxtw] */
    "a9d967c4",  /* prfw pstl1strm, p6, [x13, z7.d, lsl #2] */
    "3deedec5",  /* ldff1d {z29.d}, p3/z, [x17, z30.d] */
    "6d6925c4",  /* prfd pstl3strm, p2, [x11, z5.d, uxtw #3] */
    "ca1bd345",  /* usublb z10.d, z30.s, z19.s */
    "5445c044",  /* smlalt z20.d, z10.s, z0.s */
    "7de1cea5",  /* ld3d {z29.d-z31.d}, p0/z, [x11, #-6, mul vl] */
    "fd67e4c5",  /* ldff1d {z29.d}, p1/z, [sp, z4.d, sxtw #3] */
    "989764c5",  /* ld1sw {z24.d}, p5/z, [x28, z4.d, lsl #2] */
    "056a7184",  /* prfd pldl3strm, p2, [x16, z17.s, sxtw #3] */
    "727e5c05",  /* mov z18.h, p12/m, #-3328 */
    "40c25845",  /* sabalb z0.h, z18.b, z24.b */
    "3f2a5284",  /* ldff1sb {z31.s}, p2/z, [x17, z18.s, sxtw] */
    "eb206d45",  /* sqshrnb z11.s, z7.d, #19 */
    "2435b404",  /* xar z4.d, z4.d, z9.d, #44 */
    "158fb144",  /* smlalt z21.s, z24.h, z1.h[5] */
    "e6ef51e5",  /* st3w {z6.s-z8.s}, p3, [sp, #3, mul vl] */
    "2f2e63c5",  /* ldff1sw {z15.d}, p3/z, [x17, z3.d, sxtw #2] */
    "7d260ea5",  /* ld1rqw {z29.s}, p1/z, [x19, #-32] */
    "9d7ad945",  /* umullb z29.d, z20.s, z25.s */
    "517419e4",  /* stnt1b {z17.b}, p5, [x2, x25] */
    "e63fb8c4",  /* ldff1sh {z6.d}, p7/z, [sp, z24.d, uxtw #1] */
    "8c30d945",  /* sabdlb z12.d, z4.s, z25.s */
    "f2996d45",  /* nmatch p2.h, p6/z, z15.h, z13.h */
    "83138e44",  /* cdot z3.s, z28.b, z14.b, #0 */
    "34fae4c4",  /* ldff1h {z20.d}, p6/z, [x17, z4.d, lsl #1] */
    "c66adc85",  /* prfd #6, p2, [x22, #28, mul vl] */
    "73865564",  /* fminnmp z19.h, p1/m, z19.h, z19.h */
    "e1c78084",  /* prfh pldl1strm, p1, [sp, x0, lsl #1] */
    "1d964a44",  /* sqrshl z29.h, p5/m, z29.h, z16.h */
    "ca744e44",  /* sqrdmlsh z10.h, z6.h, z14.h */
    "5580b464",  /* fmlalb z21.s, z2.h, z20.h */
    "e68d1744",  /* uhsubr z6.b, p3/m, z6.b, z15.b */
    "fe4f29a4",  /* ld1b {z30.h}, p3/z, [sp, x9] */
    "c7440305",  /* orr z7.d, z7.d, #0x7fffffffff000000 */
    "fc948e44",  /* sqrshlr z28.s, p5/m, z28.s, z7.s */
    "18d50845",  /* adclt z24.s, z8.s, z8.s */
    "73e400a5",  /* ldnt1w {z19.s}, p1/z, [x3] */
    "a1fc15e5",  /* stnt1w {z1.s}, p7, [x5, #5, mul vl] */
    "f77aae44",  /* sqrdcmlah z23.h, z23.h, z6.h[1], #180 */
    "901d24a5",  /* ld1row {z16.s}, p7/z, [x12, x4, lsl #2] */
    "800034a4",  /* ld1rob {z0.b}, p0/z, [x4, x20] */
    "f31b1ea4",  /* ld1rqb {z19.b}, p6/z, [sp, x30] */
    "73879a44",  /* sqsub z19.s, p1/m, z19.s, z27.s */
    "ea4fb2c5",  /* ld1d {z10.d}, p3/z, [sp, z18.d, uxtw #3] */
    "38bdf044",  /* umlslt z24.d, z9.s, z0.s[3] */
    "feea1845",  /* srsra z30.h, z23.h, #8 */
    "7d4eac64",  /* fmlalt z29.s, z19.h, z4.h[3] */
    "31b7d8e5",  /* st1d {z17.d}, p5, [z25.d, #192] */
    "ba73f445",  /* subhnb z26.s, z29.d, z20.d */
    "b3d6d745",  /* sbclt z19.d, z21.d, z23.d */
    "3f62b664",  /* fmlslb z31.s, z17.h, z6.h[4] */
    "0389b844",  /* smlalb z3.s, z8.h, z0.h[7] */
    "db988644",  /* srshlr z27.s, p6/m, z27.s, z6.s */
    "7f85d684",  /* ld1rsw {z31.d}, p1/z, [x11, #88] */
    "eac89185",  /* prfd pstl2keep, p2, [x7, x17, lsl #3] */
    "0ce268c4",  /* prfd pstl3keep, p0, [x16, z8.d, lsl #3] */
    "8204dd85",  /* prfb pldl2keep, p1, [x4, #29, mul vl] */
    "c171dbc4",  /* ldff1h {z1.d}, p4/z, [x14, z27.d, sxtw] */
    "fb532285",  /* ld1w {z27.s}, p4/z, [sp, z2.s, uxtw #2] */
    "9a39c205",  /* mov z26.d, #0xfe0000000000003f */
    "782980a5",  /* ld1rqd {z24.d}, p2/z, [x11] */
    "d5b5c1e4",  /* st1h {z21.d}, p5, [z14.d, #2] */
    "6b3bf344",  /* sqdmlslb z11.d, z27.s, z3.s[3] */
    "48b33aa4",  /* ldnf1b {z8.h}, p4/z, [x26, #-6, mul vl] */
    "36049845",  /* saddlt z22.s, z1.h, z24.h */
    "d23e87a4",  /* ld1rqh {z18.h}, p7/z, [x22, #112] */
    "05f71c85",  /* prfw pldl3strm, p5, [z24.s, #112] */
    "5e2be405",  /* tbl z30.d, {z26.d-z27.d}, z4.d */
    "425e3dc4",  /* prfw pldl2keep, p7, [x18, z29.d, uxtw #2] */
    "5f9f4b44",  /* uqrshl z31.h, p7/m, z31.h, z26.h */
    "97ad5b45",  /* ushllt z23.d, z12.s, #27 */
    "e16360a4",  /* ldff1b {z1.d}, p0/z, [sp, x0] */
    "68cb0885",  /* prfw pstl1keep, p2, [x27, x8, lsl #2] */
    "7aa2c544",  /* uadalp z26.d, p0/m, z19.s */
    "86c91784",  /* prfb #6, p2, [x12, x23] */
    "894d33c4",  /* prfw pstl1strm, p3, [x12, z19.d, uxtw #2] */
    "6d200744",  /* cmla z13.b, z3.b, z7.b, #0 */
    "72734144",  /* sqrdmlah z18.h, z27.h, z1.h */
    "3070bac5",  /* ldff1d {z16.d}, p4/z, [x1, z26.d, uxtw #3] */
    "0f63df44",  /* sqdmlalb z15.d, z24.s, z31.s */
    "ffeff7c4",  /* ldff1h {z31.d}, p3/z, [sp, z23.d, lsl #1] */
    "08612d84",  /* prfd pstl1keep, p0, [x8, z13.s, uxtw #3] */
    "c962e345",  /* addhnb z9.s, z22.d, z3.d */
    "38971c84",  /* ldnt1sb {z24.s}, p5/z, [z25.s, x28] */
    "fd62c545",  /* sqdmullb z29.d, z23.s, z5.s */
    "e24b02c4",  /* ld1b {z2.d}, p2/z, [sp, z2.d, uxtw] */
    "557c4145",  /* umullt z21.h, z2.b, z1.b */
    "98ab70a4",  /* ldnf1b {z24.d}, p2/z, [x28] */
    "37ed70e5",  /* st4w {z23.s-z26.s}, p3, [x9] */
    "542bd865",  /* fadda d20, p2, d20, z26.d */
    "1685c085",  /* ld1rsb {z22.d}, p1/z, [x8] */
    "d30a3f45",  /* sqrshrunb z19.h, z22.s, #1 */
    "e5231484",  /* ldff1sb {z5.s}, p0/z, [sp, z20.s, uxtw] */
    "8d951e44",  /* sqsubr z13.b, p5/m, z13.b, z12.b */
    "864a7184",  /* prfw #6, p2, [x20, z17.s, sxtw #2] */
    "f40a05a4",  /* ld1rqb {z20.b}, p2/z, [x23, x5] */
    "ddc0b4c5",  /* ld1d {z29.d}, p0/z, [z6.d, #160] */
    "646309e5",  /* stnt1w {z4.s}, p0, [x27, x9, lsl #2] */
    "e6bb89e4",  /* st1h {z6.d}, p6, [sp, z9.d] */
    "29c66204",  /* uqinch z9.h, #17, mul #3 */
    "2f6eaf64",  /* fmlslt z15.s, z17.h, z7.h[3] */
    "f122ed05",  /* mov z17.b, z23.b[54] */
    "79f795e4",  /* stnt1h {z25.h}, p5, [x27, #5, mul vl] */
    "2f3b66c4",  /* prfh #15, p6, [x25, z6.d, sxtw #1] */
    "06ca8385",  /* prfd #6, p2, [x16, x3, lsl #3] */
    "e8ab33e5",  /* st1w {z8.d}, p2, [sp, z19.d, lsl #2] */
    "9843dbc5",  /* ld1d {z24.d}, p0/z, [x28, z27.d, sxtw] */
    "0f7d3fa5",  /* ldff1sh {z15.s}, p7/z, [x8, xzr, lsl #1] */
    "c698da45",  /* ummla z6.s, z6.b, z26.b */
    "5c91f244",  /* umlalb z28.d, z10.s, z2.s[2] */
    "883e37c4",  /* prfh pstl1keep, p7, [x20, z23.d, uxtw #1] */
    "fca0c6c4",  /* ldff1sh {z28.d}, p0/z, [x7, z6.d] */
    "6d3ff185",  /* prfh pstl3strm, p7, [x27, #-15, mul vl] */
    "e64310a5",  /* ld1sh {z6.d}, p0/z, [sp, x16, lsl #1] */
    "28174005",  /* eor z8.b, z8.b, #0xcc */
    "4f862cc5",  /* ld1sw {z15.d}, p1/z, [z18.d, #48] */
    "8bea1685",  /* prfw pstl2strm, p2, [z20.s, #88] */
    "47e08345",  /* ssra z7.d, z2.d, #61 */
    "ada175c4",  /* prfh pstl3strm, p0, [x13, z21.d, lsl #1] */
    "a8387345",  /* uqrshrnb z8.s, z5.d, #13 */
    "aec31d85",  /* prfw #14, p0, [x29, x29, lsl #2] */
    "fa5d4644",  /* umlslt z26.h, z15.b, z6.b */
    "d3799fa5",  /* ldff1sb {z19.d}, p6/z, [x14, xzr] */
    "ee7a9644",  /* usdot z14.s, z23.b, z22.b */
    "7125fd44",  /* sqdmlalt z17.d, z11.s, z13.s[2] */
    "d894d445",  /* eortb z24.d, z6.d, z20.d */
    "4b3b6cc4",  /* prfh pstl2strm, p6, [x26, z12.d, sxtw #1] */
    "b24890c5",  /* ld1d {z18.d}, p2/z, [x5, z16.d, uxtw] */
    "48878c44",  /* sqshlr z8.s, p1/m, z8.s, z26.s */
    "8c5d6584",  /* prfw pstl3keep, p7, [x12, z5.s, sxtw #2] */
    "a182fd64",  /* bfmlalb z1.s, z21.h, z29.h */
    "ecf683c5",  /* prfd pstl3keep, p5, [z23.d, #24] */
    "eb328305",  /* and z11.d, z11.d, #0x3fffffc000000 */
    "24d31584",  /* prfb pldl3keep, p4, [x25, x21] */
    "808b6fc4",  /* prfb pldl1keep, p2, [x28, z15.d] */
    "fb427764",  /* bfdot z27.s, z23.h, z7.h[2] */
    "afc79984",  /* prfh #15, p1, [x29, x25, lsl #1] */
    "cdf083c4",  /* prfh pstl3strm, p4, [z6.d, #6] */
    "0903d344",  /* sdot z9.d, z24.h, z19.h */
    "ec7b0ce4",  /* stnt1b {z12.b}, p6, [sp, x12] */
    "47b75e45",  /* bdep z7.h, z26.h, z30.h */
    "196bccc5",  /* ldff1d {z25.d}, p2/z, [x24, z12.d, sxtw] */
    "eaceeb04",  /* uqdecd z10.d, #23, mul #12 */
    "cf653004",  /* pmul z15.b, z14.b, z16.b */
    "dd779145",  /* smullt z29.s, z30.h, z17.h */
    "f3df40c5",  /* ld1w {z19.d}, p7/z, [sp, z0.d] */
    "ad1023c4",  /* prfb pstl3strm, p4, [x5, z3.d, uxtw] */
    "feb378a5",  /* ldnf1w {z30.d}, p4/z, [sp, #-8, mul vl] */
    "356ff845",  /* raddhnt z21.s, z25.d, z24.d */
    "0ed8c005",  /* dupm z14.s, #0x20 */
    "366e8e44",  /* sqdmlslt z22.s, z17.h, z14.h */
    "3cc6f204",  /* decd z28.d, #17, mul #3 */
    "5fbce805",  /* mov z31.d, p7/m, x2 */
    "a8a0f104",  /* adr z8.d, [z5.d, z17.d] */
    "2f167544",  /* sqrdmlsh z15.h, z17.h, z5.h[6] */
    "92f5c045",  /* sli z18.d, z12.d, #32 */
    "18f2d0e5",  /* st3d {z24.d-z26.d}, p4, [x16] */
    "8672f9a5",  /* ldff1d {z6.d}, p4/z, [x20, x25, lsl #3] */
    "f17aed45",  /* rsubhnb z17.s, z23.d, z13.d */
    "82f12445",  /* sm4ekey z2.s, z12.s, z4.s */
    "176ec245",  /* pmullt z23.d, z16.s, z2.s */
    "6c3c6784",  /* prfh pstl3keep, p7, [x3, z7.s, sxtw #1] */
    "fab94645",  /* bgrp z26.h, z15.h, z6.h */
    "4a9ac005",  /* dupm z10.s, #0xffffe000 */
    "e45f9ce5",  /* str z4, [sp, #231, mul vl] */
    "5cf604a4",  /* ldnt1b {z28.b}, p5/z, [x18, #4, mul vl] */
    "ae84a364",  /* fmlalt z14.s, z5.h, z3.h */
    "ee2bea85",  /* prfh #14, p2, [sp, #-22, mul vl] */
    "862bbd44",  /* sqdmlalb z6.s, z28.h, z5.h[7] */
    "64050005",  /* orr z4.h, z4.h, #0xfff */
    "e4dfcea5",  /* ld3d {z4.d-z6.d}, p7/z, [sp, x14, lsl #3] */
    "a9af0a65",  /* fcvtx z9.s, p3/m, z29.d */
    "6d3c2884",  /* prfh pstl3strm, p7, [x3, z8.s, uxtw #1] */
    "3b3a80a4",  /* ld1rqh {z27.h}, p6/z, [x17] */
    "ee77e7a4",  /* ldff1h {z14.d}, p5/z, [sp, x7, lsl #1] */
    "e167ffa5",  /* ldff1d {z1.d}, p1/z, [sp, xzr, lsl #3] */
    "e656fa85",  /* prfw #6, p5, [x23, #-6, mul vl] */
    "04043f84",  /* prfb pldl3keep, p1, [x0, z31.s, uxtw] */
    "55988445",  /* usmmla z21.s, z2.b, z4.b */
    "347e6045",  /* rsubhnt z20.b, z17.h, z0.h */
    "c448ca45",  /* uaddwb z4.d, z6.d, z10.s */
    "0e9a5f44",  /* uqsubr z14.h, p6/m, z14.h, z16.h */
    "ba77fe45",  /* subhnt z26.s, z29.d, z30.d */
    "8254c005",  /* mov z2.h, #0x7c0 */
    "fb43d844",  /* smlalb z27.d, z31.s, z24.s */
    "26143245",  /* shrnt z6.h, z1.s, #14 */
    "5acaaae5",  /* st1d {z26.d}, p2, [x18, z10.d, sxtw #3] */
    "841f7dc4",  /* prfb pldl3keep, p7, [x28, z29.d, sxtw] */
    "21473184",  /* prfw pldl1strm, p1, [x25, z17.s, uxtw #2] */
    "a425ef85",  /* prfh pldl3keep, p1, [x13, #-17, mul vl] */
    "5f9b1f84",  /* ldnt1sb {z31.s}, p6/z, [z26.s, xzr] */
    "38a620c4",  /* ldff1sb {z24.d}, p1/z, [z17.d] */
    "7f2f2a45",  /* sqrshrnt z31.b, z27.h, #6 */
    "6de5f044",  /* sqdmullt z13.d, z11.s, z0.s[2] */
    "6496a005",  /* mov z4.s, p5/m, s19 */
    "e8df54a4",  /* ld3b {z8.b-z10.b}, p7/z, [sp, x20] */
    "b7fc1b45",  /* uaba z23.b, z5.b, z27.b */
    "d9fa2604",  /* sqdecb x25, w25, #22, mul #7 */
    "7648b064",  /* fmlalb z22.s, z3.h, z0.h[5] */
    "318c9b65",  /* fsubr z17.s, p3/m, z17.s, #1.0 */
    "e4f7e4a5",  /* ld4d {z4.d-z7.d}, p5/z, [sp, #16, mul vl] */
    "064aec64",  /* bfmlalb z6.s, z16.h, z4.h[3] */
    "f20227c5",  /* ld1sw {z18.d}, p0/z, [x23, z7.d, uxtw #2] */
    "bdb66805",  /* mov z29.h, p5/m, w21 */
    "be295fe4",  /* stnt1b {z30.s}, p2, [z13.s, xzr] */
    "584d4f45",  /* uaddwt z24.h, z10.h, z15.b */
    "bb2fa8a5",  /* ld1rod {z27.d}, p3/z, [x29, #-256] */
    "4067be45",  /* addhnt z0.h, z26.s, z30.s */
    "ea013945",  /* sqshrunb z10.h, z15.s, #7 */
    "4082c744",  /* urshlr z0.d, p0/m, z0.d, z18.d */
    "3d91c005",  /* mov z29.s, #0xffc000 */
    "31684005",  /* eor z17.s, z17.s, #0x180000 */
    "11b436a5",  /* ldnf1sh {z17.s}, p5/z, [x0, #6, mul vl] */
    "374d0005",  /* orr z23.h, z23.h, #0xff81 */
    "5ccc5645",  /* uabalt z28.h, z2.b, z22.b */
    "2ef479c4",  /* prfd #14, p5, [x1, z25.d, lsl #3] */
    "7d2c0305",  /* orr z29.d, z29.d, #0x7ffffffff8000000 */
    "3e107245",  /* shrnb z30.s, z1.d, #14 */
    "23fb9ec5",  /* prfd pldl2strm, p6, [z25.d, #240] */
    "f35710c5",  /* ld1w {z19.d}, p5/z, [sp, z16.d, uxtw] */
    "4b476284",  /* prfw pstl2strm, p1, [x26, z2.s, sxtw #2] */
    "20719645",  /* smullb z0.s, z9.h, z22.h */
    "af8ac944",  /* uqshl z15.d, p2/m, z15.d, z21.d */
    "734dcb44",  /* umlalt z19.d, z11.s, z11.s */
    "e1163c84",  /* prfb pldl1strm, p5, [x23, z28.s, uxtw] */
    "93182d45",  /* rshrnb z19.b, z4.h, #3 */
    "be884445",  /* ssublbt z30.h, z5.b, z4.b */
    "fcbbbfe5",  /* st1d {z28.d}, p6, [sp, z31.d, lsl #3] */
    "fa849b44",  /* uqsub z26.s, p1/m, z26.s, z7.s */
    "7df76304",  /* uqinch w29, #27, mul #4 */
    "61aa6b04",  /* adr z1.d, [z19.d, z11.d, uxtw #2] */
    "ebb7e185",  /* ld1rsb {z11.s}, p5/z, [sp, #33] */
    "d5ba10a5",  /* ldnf1sh {z21.d}, p6/z, [x22] */
    "0196ff44",  /* umlalt z1.d, z16.s, z15.s[2] */
    "33a60945",  /* sshllt z19.h, z17.b, #1 */
    "f267b8e4",  /* st2h {z18.h-z19.h}, p1, [sp, x24, lsl #1] */
    "a81c6bc4",  /* prfb pstl1keep, p7, [x5, z11.d, sxtw] */
    "e8fb6ca5",  /* ld4w {z8.s-z11.s}, p6/z, [sp, #-16, mul vl] */
    "a513a1a5",  /* ld1rod {z5.d}, p4/z, [x29, x1, lsl #3] */
    "8245d185",  /* prfw pldl2keep, p1, [x12, #17, mul vl] */
    "ed031ac5",  /* ld1sw {z13.d}, p0/z, [sp, z26.d, uxtw] */
    "ecf76b85",  /* ld1rw {z12.d}, p5/z, [sp, #172] */
    "d5f38005",  /* and z21.s, z21.s, #0xfffffffd */
    "80ae6cc4",  /* prfh pldl1keep, p3, [x20, z12.d, lsl #1] */
    "4e4ce664",  /* bfmlalt z14.s, z2.h, z6.h[1] */
    "2e023c84",  /* prfb #14, p0, [x17, z28.s, uxtw] */
    "a58ed464",  /* fmaxnmp z5.d, p3/m, z5.d, z21.d */
    "f81ba484",  /* ld1sh {z24.s}, p6/z, [sp, z4.s, uxtw #1] */
    "ffc78045",  /* sabalt z31.s, z31.h, z0.h */
    "f2a4ad64",  /* fmlslt z18.s, z7.h, z13.h */
    "9c572845",  /* sqxtunt z28.b, z28.h */
    "ea7713e5",  /* stnt1w {z10.s}, p5, [sp, x19, lsl #2] */
    "8be74205",  /* eor z11.d, z11.d, #0xfffffff1ffffffff */
    "183f5fe5",  /* stnt1w {z24.s}, p7, [z24.s, xzr] */
    "f1ef4ae4",  /* st1b {z17.s}, p3, [sp, #-6, mul vl] */
    "44f58885",  /* prfd pldl3keep, p5, [z10.s, #64] */
    "b6809664",  /* fmaxp z22.s, p0/m, z22.s, z5.s */
    "3fa9a2e5",  /* st1d {z31.d}, p2, [x9, z2.d, lsl #3] */
    "46354305",  /* eor z6.d, z6.d, #0xfffffffffc00001f */
    "e4a7d8c4",  /* ldff1sh {z4.d}, p1/z, [sp, z24.d] */
    "7c397604",  /* bcax z28.d, z28.d, z22.d, z11.d */
    "be0b9f45",  /* uaddlb z30.s, z29.h, z31.h */
    "f1630ec4",  /* ldff1b {z17.d}, p0/z, [sp, z14.d, uxtw] */
    "253a2ea4",  /* ld1rob {z5.b}, p6/z, [x17, #-64] */
    "494bdb85",  /* prfw pstl1strm, p2, [x26, #27, mul vl] */
    "20c761c4",  /* prfw pldl1keep, p1, [x25, z1.d, lsl #2] */
    "f847f8a5",  /* ld1d {z24.d}, p1/z, [sp, x24, lsl #3] */
    "9a8a6005",  /* mov z26.h, p2/m, h20 */
    "a0114005",  /* eor z0.s, z0.s, #0xc0000fff */
    "d99a0205",  /* orr z25.d, z25.d, #0xffffe0000000000f */
    "f2b3f3a4",  /* ldnf1h {z18.d}, p4/z, [sp, #3, mul vl] */
    "e4a371c5",  /* ldff1sw {z4.d}, p0/z, [sp, z17.d, lsl #2] */
    "82d05b45",  /* adclb z2.d, z4.d, z27.d */
    "e65768e5",  /* st1w {z6.d}, p5, [sp, x8, lsl #2] */
    "3e11d745",  /* ssublb z30.d, z9.s, z23.s */
    "68c8ce45",  /* uabalb z8.d, z3.s, z14.s */
    "e42780a5",  /* ld1rqd {z4.d}, p1/z, [sp] */
    "bffb3a04",  /* sqdecb xzr, mul4, mul #11 */
    "b7aaa0c4",  /* ldff1sh {z23.d}, p2/z, [z21.d] */
    "0f24de85",  /* prfh #15, p1, [x0, #30, mul vl] */
    "eba397e5",  /* st1d {z11.d}, p0, [sp, z23.d] */
    "43b28645",  /* bext z3.s, z18.s, z6.s */
    "fd2b88a5",  /* ld1rqd {z29.d}, p2/z, [sp, #-128] */
    "ec13f585",  /* prfb pstl3keep, p4, [sp, #-11, mul vl] */
    "6ff640a4",  /* ld3b {z15.b-z17.b}, p5/z, [x19] */
    "fabb1f85",  /* ldnt1w {z26.s}, p6/z, [z31.s, xzr] */
    "2beb02c4",  /* prfb pstl2strm, p2, [z25.d, #2] */
    "e97fa6a5",  /* ldff1sb {z9.s}, p7/z, [sp, x6] */
    "ef0b70c5",  /* ld1sw {z15.d}, p2/z, [sp, z16.d, sxtw #2] */
    "f2a363a5",  /* ld1w {z18.d}, p0/z, [sp, #3, mul vl] */
    "3d8e8345",  /* ssubltb z29.s, z17.h, z3.h */
    "839e8f44",  /* uqrshlr z3.s, p7/m, z3.s, z20.s */
    "02ed1585",  /* prfw pldl2keep, p3, [z8.s, #84] */
    "86b68844",  /* sqabs z6.s, p5/m, z20.s */
    "6c324065",  /* faddv h12, p4, z19.h */
    "5faa5e45",  /* ushllb z31.d, z18.s, #30 */
    "d9393904",  /* eor3 z25.d, z25.d, z25.d, z14.d */
    "f5d37e84",  /* ld1rb {z21.s}, p4/z, [sp, #62] */
    "c902b344",  /* sdot z9.s, z22.b, z3.b[2] */
    "e4bf6cc4",  /* prfh pldl3keep, p7, [sp, z12.d, lsl #1] */
    "f8ffefe4",  /* st1h {z24.d}, p7, [sp, #-1, mul vl] */
    "82533045",  /* sqxtunb z2.h, z28.s */
    "118f0844",  /* sqshl z17.b, p3/m, z17.b, z24.b */
    "3a48de44",  /* umlalb z26.d, z1.s, z30.s */
    "fdefc784",  /* ld1rh {z29.d}, p3/z, [sp, #14] */
    "bae8c085",  /* ld1rd {z26.d}, p2/z, [x5] */
    "45fcc205",  /* mov z5.d, #0xfffffffe0000000f */
    "59a0b964",  /* fmlslb z25.s, z2.h, z25.h */
    "a8a33d45",  /* histseg z8.b, z29.b, z29.b */
    "facfa6a4",  /* ld2h {z26.h-z27.h}, p3/z, [sp, x6, lsl #1] */
    "e6bb4685",  /* ld1rsh {z6.s}, p6/z, [sp, #12] */
    "bc3ada45",  /* uabdlb z28.d, z21.s, z26.s */
    "a1307a84",  /* prfh pldl1strm, p4, [x5, z26.s, sxtw #1] */
    "cc1b35c4",  /* prfb pstl3keep, p6, [x30, z21.d, uxtw] */
    "cf929244",  /* shsub z15.s, p4/m, z15.s, z22.s */
    "6e2e4565",  /* fminnmv h14, p3, z19.h */
    "e5a160e4",  /* st1b {z5.s}, p0, [z15.s] */
    "6f2b2384",  /* prfh #15, p2, [x27, z3.s, uxtw #1] */
    "e4cf83a5",  /* ldnt1d {z4.d}, p3/z, [sp, x3, lsl #3] */
    "2ebb4084",  /* ld1rb {z14.h}, p6/z, [x25] */
    "f23b3cc5",  /* ldff1sw {z18.d}, p6/z, [sp, z28.d, uxtw #2] */
    "f14bbea5",  /* ld1sb {z17.s}, p2/z, [sp, x30] */
    "608e0244",  /* srshl z0.b, p3/m, z0.b, z19.b */
    "d7e4e664",  /* fmmla z23.d, z6.d, z6.d */
    "c7663784",  /* prfd #7, p1, [x22, z23.s, uxtw #3] */
    "c7e18e84",  /* prfh #7, p0, [z14.s, #28] */
    "25377845",  /* uqshrnt z5.s, z25.d, #8 */
    "ed5b3584",  /* prfw pstl3strm, p6, [sp, z21.s, uxtw #2] */
    "c5ffd0e4",  /* st3h {z5.h-z7.h}, p7, [x30] */
    "e7afc7a4",  /* ld1h {z7.s}, p3/z, [sp, #7, mul vl] */
    "3430c005",  /* mov z20.s, #0xc000000 */
    "1aaf8a65",  /* bfcvt z26.h, p3/m, z24.s */
    "b2ed7825",  /* mov z18.h, #27904 */
    "c80cc345",  /* uaddlt z8.d, z6.s, z3.s */
    "d1089444",  /* sqdmlalbt z17.s, z6.h, z20.h */
    "e74f5be4",  /* st1b {z7.s}, p3, [sp, x27] */
    "fe8313e4",  /* st1b {z30.d}, p0, [sp, z19.d, uxtw] */
    "e7234ec5",  /* ldff1sw {z7.d}, p0/z, [sp, z14.d, sxtw] */
    "a2e09084",  /* prfh pldl2keep, p0, [z5.s, #32] */
    "52288005",  /* and z18.s, z18.s, #0x38000000 */
    "e61ba285",  /* ldr p6, [sp, #-234, mul vl] */
    "e1f920c5",  /* ldff1w {z1.d}, p6/z, [z15.d] */
    "be687fa4",  /* ldff1b {z30.d}, p2/z, [x5, xzr] */
    "f5775ce5",  /* st3w {z21.s-z23.s}, p5, [sp, x28, lsl #2] */
    "853f2304",  /* bsl z5.d, z5.d, z3.d, z28.d */
    "39aae0e4",  /* st1h {z25.s}, p2, [z17.s] */
    "e4b3ecc4",  /* ldff1sh {z4.d}, p4/z, [sp, z12.d, lsl #1] */
    "3cbcf0a5",  /* ldnf1d {z28.d}, p7/z, [x1] */
    "f41bff25",  /* whilehi p4.d, xzr, xzr */
    "1df43c45",  /* rax1 z29.d, z0.d, z28.d */
    "4a5723c4",  /* prfw pstl2keep, p5, [x26, z3.d, uxtw #2] */
    "037721c4",  /* prfd pldl2strm, p5, [x24, z1.d, uxtw #3] */
    "d8432845",  /* sqxtnb z24.b, z30.h */
    "ffe713e5",  /* stnt1w {z31.s}, p1, [sp, #3, mul vl] */
    "95e63404",  /* decb x21, #20, mul #5 */
    "ca5bed85",  /* prfw pstl2keep, p6, [x30, #-19, mul vl] */
    "58aa0305",  /* orr z24.d, z24.d, #0x3ffff800 */
    "f083bfe5",  /* st1d {z16.d}, p0, [sp, z31.d, uxtw #3] */
    "1697ad05",  /* splice z22.s, p5, {z24.s-z25.s} */
    "1c6dffa5",  /* ldff1d {z28.d}, p3/z, [x8, xzr, lsl #3] */
    "840fc005",  /* dupm z4.b, #0xaa */
    "05dc7ec4",  /* prfw pldl3strm, p7, [x0, z30.d, lsl #2] */
    "85537084",  /* prfw pldl3strm, p4, [x28, z16.s, sxtw #2] */
    "e1bb40e5",  /* st1w {z1.d}, p6, [z31.d] */
    "3df020c4",  /* ldff1b {z29.d}, p4/z, [z1.d] */
    "80c91d85",  /* prfw pldl1keep, p2, [x12, x29, lsl #2] */
    "83f511c4",  /* prfb pldl2strm, p5, [z12.d, #17] */
    "3bae90a4",  /* ldnf1sw {z27.d}, p3/z, [x17] */
    "7ef605a5",  /* ldnt1w {z30.s}, p5/z, [x19, #5, mul vl] */
    "a6fa6804",  /* sqdech x6, w6, #21, mul #9 */
    "85931c44",  /* suqadd z5.b, p4/m, z5.b, z28.b */
    "c1817064",  /* bfdot z1.s, z14.h, z16.h */
    "d9a84205",  /* eor z25.d, z25.d, #0x3f80000000000 */
    "1d7cdfa4",  /* ldff1h {z29.s}, p7/z, [x0, xzr, lsl #1] */
    "56a32504",  /* adr z22.d, [z26.d, z5.d, sxtw] */
    "41f794c4",  /* prfh pldl1strm, p5, [z26.d, #40] */
    "48918305",  /* and z8.d, z8.d, #0x1ffc000 */
    "afe297c5",  /* prfd #15, p0, [z21.d, #184] */
    "2edf6ec4",  /* prfw #14, p7, [x25, z14.d, lsl #2] */
    "3efef804",  /* uqdecd x30, #17, mul #9 */
    "86e610c5",  /* prfw #6, p1, [z20.d, #64] */
    "0e9a71c4",  /* prfb #14, p6, [x16, z17.d] */
    "75981fc5",  /* ldnt1sw {z21.d}, p6/z, [z3.d, xzr] */
    "22272e84",  /* prfh pldl2keep, p1, [x25, z14.s, uxtw #1] */
    "ee636ce4",  /* st4b {z14.b-z17.b}, p0, [sp, x12] */
    "f77792c4",  /* ldff1h {z23.d}, p5/z, [sp, z18.d, uxtw] */
    "39fba0a5",  /* ld2d {z25.d-z26.d}, p6/z, [x25] */
    "24360205",  /* orr z4.d, z4.d, #0xfc000fffffffffff */
    "2461c085",  /* prfd pldl3keep, p0, [x9] */
    "7fa9e025",  /* cntp xzr, p10, p11.d */
    "8f473384",  /* prfw #15, p1, [x28, z19.s, uxtw #2] */
    "4d6a6245",  /* raddhnb z13.b, z18.h, z2.h */
    "ede709a5",  /* ldnt1w {z13.s}, p1/z, [sp, #-7, mul vl] */
    "acb266c4",  /* prfh pstl3keep, p4, [x21, z6.d, lsl #1] */
    "2405a144",  /* udot z4.s, z9.b, z1.b[0] */
    "f78706e5",  /* st1w {z23.d}, p1, [sp, z6.d, uxtw] */
    "d0f8c305",  /* mov z16.d, #0xfe */
    "fffee904",  /* uqdecd wzr, #23, mul #10 */
    "91f6fb04",  /* uqincd x17, #20, mul #12 */
    "e08e1144",  /* uhadd z0.b, p3/m, z0.b, z23.b */
    "3d0c7045",  /* sqrshrunt z29.s, z1.d, #16 */
    "3635f044",  /* sqdmlslt z22.d, z9.s, z0.s[2] */
    "ffd395a4",  /* ldnt1h {z31.h}, p4/z, [sp, x21, lsl #1] */
    "4bb82105",  /* lastb w11, p6, z2.b */
    "03e37d04",  /* inch x3, #24, mul #14 */
    "e83bf2c4",  /* ldff1sh {z8.d}, p6/z, [sp, z18.d, sxtw #1] */
    "9ae3b304",  /* incw x26, #28, mul #4 */
    "7fed90e5",  /* stnt1d {z31.d}, p3, [x11] */
    "3ae2fe04",  /* incd x26, #17, mul #15 */
    "f677cbe5",  /* st3d {z22.d-z24.d}, p5, [sp, x11, lsl #3] */
    "cd05cf44",  /* udot z13.d, z14.h, z15.h */
    "f8df9de4",  /* st1h {z24.d}, p7, [sp, z29.d, sxtw] */
    "df0cc005",  /* mov z31.h, #0x803f */
    "e408ff25",  /* whilehs p4.d, w7, wzr */
    "55f680a5",  /* ldnt1d {z21.d}, p5/z, [x18] */
    "f847de45",  /* saddwt z24.d, z31.d, z30.s */
    "2311fb85",  /* prfb pldl2strm, p4, [x9, #-5, mul vl] */
    "a8222405",  /* mov z8.s, s21 */
    "d53800a5",  /* ld1rqw {z21.s}, p6/z, [x6] */
    "22235865",  /* fadda h2, p0, h2, z25.h */
    "e92b80c4",  /* ldff1sh {z9.d}, p2/z, [sp, z0.d, uxtw] */
    "2a33a0a4",  /* ld1roh {z10.h}, p4/z, [x25] */
    "e35b84c4",  /* ld1h {z3.d}, p6/z, [sp, z4.d, uxtw] */
    "4bf23204",  /* sqincb x11, #18, mul #3 */
    "5481d044",  /* shadd z20.d, p0/m, z20.d, z10.d */
    "ed9b8ce5",  /* st1d {z13.d}, p6, [sp, z12.d, uxtw] */
    "01d89085",  /* prfd pldl1strm, p6, [x0, x16, lsl #3] */
    "44477984",  /* prfw pldl3keep, p1, [x26, z25.s, sxtw #2] */
    "bbee0205",  /* orr z27.d, z27.d, #0xfffffff801ffffff */
    "f2f5ad04",  /* uqincw w18, #15, mul #14 */
    "f19bcee4",  /* st1h {z17.s}, p6, [sp, z14.s, uxtw] */
    "f40b2125",  /* whilehi p4.b, wzr, w1 */
    "e4fa73c4",  /* prfd pldl3keep, p6, [x23, z19.d, lsl #3] */
    "f80725a5",  /* ld1row {z24.s}, p1/z, [sp, x5, lsl #2] */
    "0fad0205",  /* orr z15.d, z15.d, #0xfffff800000fffff */
    "9db7d0a5",  /* ldnf1sb {z29.h}, p5/z, [x28] */
    "f4aed0a4",  /* ldnf1h {z20.s}, p3/z, [x23] */
    "e14bcd84",  /* ld1h {z1.s}, p2/z, [sp, z13.s, sxtw] */
    "2b3ff204",  /* nbsl z11.d, z11.d, z18.d, z25.d */
    "62090205",  /* orr z2.d, z2.d, #0x80000000000007ff */
    "5db78964",  /* fcvtlt z29.s, p5/m, z26.h */
    "4c98aa05",  /* clasta s12, p6, s12, z2.s */
    "6399a0c4",  /* ld1sh {z3.d}, p6/z, [z11.d] */
    "e4dc9984",  /* prfh pldl3keep, p7, [x7, x25, lsl #1] */
    "e96f2585",  /* ldff1w {z9.s}, p3/z, [sp, z5.s, uxtw #2] */
    "e7938d44",  /* uqshlr z7.s, p4/m, z7.s, z31.s */
    "849a0645",  /* smmla z4.s, z20.b, z6.b */
    "ef5c4005",  /* eor z15.h, z15.h, #0x1fe0 */
    "e86f50c5",  /* ldff1w {z8.d}, p3/z, [sp, z16.d, sxtw] */
    "ec5fadc4",  /* ld1h {z12.d}, p7/z, [sp, z13.d, uxtw #1] */
    "05256b45",  /* sqshrnt z5.s, z8.d, #21 */
    "490b4305",  /* eor z9.d, z9.d, #0x3ffffff80000000 */
    "3d86e364",  /* bfmlalt z29.s, z17.h, z3.h */
    "eba360a4",  /* ld1b {z11.d}, p0/z, [sp] */
    "5690d844",  /* sqadd z22.d, p4/m, z22.d, z2.d */
    "346ac005",  /* mov z20.s, #0xfff8001f */
    "4d685825",  /* brkns p13.b, p10/z, p2.b, p13.b */
    "2ce913c4",  /* prfb pstl3keep, p2, [z9.d, #19] */
    "89e78a85",  /* prfd pstl1strm, p1, [z28.s, #80] */
    "43dc1085",  /* prfw pldl2strm, p7, [x2, x16, lsl #2] */
    "28e49984",  /* prfh pstl1keep, p1, [z1.s, #50] */
    "ab969fc4",  /* ldnt1sh {z11.d}, p5/z, [z21.d, xzr] */
    "e387f6e4",  /* st1h {z3.s}, p1, [sp, z22.s, uxtw #1] */
    "3a3b8665",  /* fmaxv s26, p6, z25.s */
    "293fc465",  /* fmaxnmv d9, p7, z25.d */
    "fe3dad04",  /* bsl2n z30.d, z30.d, z13.d, z15.d */
    "0efbb004",  /* sqdecw x14, #24 */
    "91fbb304",  /* sqdecw x17, #28, mul #4 */
    "ed4ff9a4",  /* ld1h {z13.d}, p3/z, [sp, x25, lsl #1] */
    "0b8b8005",  /* and z11.s, z11.s, #0xffff80ff */
    "e8a718e5",  /* st1w {z8.d}, p1, [sp, z24.d] */
    "6cedc205",  /* mov z12.d, #0xfffffff800007fff */
    "e2af8aa4",  /* ld1sw {z2.d}, p3/z, [sp, #-6, mul vl] */
    "3bb60205",  /* orr z27.d, z27.d, #0xfffffc000fffffff */
    "c561ee85",  /* prfd pldl3strm, p0, [x14, #-18, mul vl] */
    "1b154005",  /* eor z27.h, z27.h, #0xc07f */
    "91feaf04",  /* uqdecw w17, #20, mul #16 */
    "aff300a4",  /* ldnt1b {z15.b}, p4/z, [x29] */
    "d5a27404",  /* adr z21.d, [z22.d, z20.d, uxtw] */
    "b4e6b904",  /* decw x20, #21, mul #10 */
    "ed13aea5",  /* ld1rod {z13.d}, p4/z, [sp, x14, lsl #3] */
    "3b0ac005",  /* mov z27.s, #0x8001ffff */
    "44fe6704",  /* uqdech w4, #18, mul #8 */
    "3fe440a5",  /* ld3w {z31.s-z1.s}, p1/z, [x1] */
    "b1510205",  /* orr z17.d, z17.d, #0xffc000000000000f */
    "c5336545",  /* uqshrnb z5.s, z30.d, #27 */
    "ea1f4fc5",  /* ld1sw {z10.d}, p7/z, [sp, z15.d, sxtw] */
    "c496c084",  /* ld1rsw {z4.d}, p5/z, [x22] */
    "fa835dc5",  /* ld1sw {z26.d}, p0/z, [sp, z29.d] */
    "ccfc30e4",  /* st2b {z12.b-z13.b}, p7, [x6] */
    "c6b060c4",  /* prfh #6, p4, [x6, z0.d, lsl #1] */
    "82337084",  /* prfh pldl2keep, p4, [x28, z16.s, sxtw #1] */
    "5a83d944",  /* uqadd z26.d, p0/m, z26.d, z26.d */
    "e68fd9c4",  /* ld1sh {z6.d}, p3/z, [sp, z25.d] */
    "1696c305",  /* mov z22.d, #0x7fffffffffffc000 */
    "810dff25",  /* whilelo p1.d, w12, wzr */
    "d0fb4305",  /* eor z16.d, z16.d, #0xfffffffe */
    "f0294765",  /* fminv h16, p2, z15.h */
    "54294005",  /* eor z20.s, z20.s, #0xf800003f */
    "ec19be44",  /* usdot z12.s, z15.b, z6.b[3] */
    "fa4bbc85",  /* ldr z26, [sp, #-30, mul vl] */
    "24d08185",  /* prfd pldl3keep, p4, [x1, x1, lsl #3] */
    "e5bfaaa5",  /* ld1sb {z5.s}, p7/z, [sp, #-6, mul vl] */
    "a04ac205",  /* mov z0.d, #0xff80000000001fff */
    "73928205",  /* and z19.d, z19.d, #0xffffc00000000003 */
    "e84763a5",  /* ld1w {z8.d}, p1/z, [sp, x3, lsl #2] */
    "f63e0205",  /* orr z22.d, z22.d, #0xfe01ffffffffffff */
    "cf8d4085",  /* ld1rsh {z15.d}, p3/z, [x14] */
    "ad76db85",  /* prfd pstl3strm, p5, [x21, #27, mul vl] */
    "c8704005",  /* eor z8.s, z8.s, #0x1fc0000 */
    "e88f4fc4",  /* ld1sb {z8.d}, p3/z, [sp, z15.d] */
    "58254465",  /* fmaxnmv h24, p1, z10.h */
    "3fe47804",  /* dech xzr, vl1, mul #9 */
    "1d859764",  /* fminp z29.s, p1/m, z29.s, z8.s */
    "14879544",  /* urhadd z20.s, p1/m, z20.s, z24.s */
    "d5b5ca64",  /* fcvtnt z21.s, p5/m, z14.d */
    "4cd28445",  /* sbclb z12.s, z18.s, z4.s */
    "e51b59c4",  /* ld1sb {z5.d}, p6/z, [sp, z25.d, sxtw] */
    "f2835064",  /* faddp z18.h, p0/m, z18.h, z31.h */
    "8e11bf25",  /* whilege p14.s, x12, xzr */
    "d48b20c5",  /* ld1sw {z20.d}, p2/z, [z30.d] */
    "d1cb9fc5",  /* ldnt1d {z17.d}, p2/z, [z30.d, xzr] */
    "e5299fe4",  /* stnt1h {z5.d}, p2, [z15.d, xzr] */
    "9ef3f604",  /* sqincd x30, #28, mul #7 */
    "5ff8b204",  /* sqdecw xzr, vl2, mul #3 */
    "77f8c005",  /* dupm z23.s, #0x1e */
    "aae280a4",  /* ldnt1h {z10.h}, p0/z, [x21] */
    "e0c746c4",  /* ld1b {z0.d}, p1/z, [sp, z6.d] */
    "88e3ed04",  /* cntd x8, #28, mul #14 */
    "f4a71ca5",  /* ldnf1sh {z20.d}, p1/z, [sp, #-4, mul vl] */
    "e7e35cc5",  /* ldff1w {z7.d}, p0/z, [sp, z28.d] */
    "c0f380c4",  /* prfh pldl1keep, p4, [z30.d] */
    "e0b7d9a5",  /* ldnf1sb {z0.h}, p5/z, [sp, #-7, mul vl] */
    "bef58205",  /* and z30.d, z30.d, #0xfffffffc0000ffff */
    "31e36904",  /* cnth x17, #25, mul #10 */
    "930dc005",  /* mov z19.h, #0x8fff */
    "619d5344",  /* uhsub z1.h, p7/m, z1.h, z11.h */
    "3b228565",  /* fminnmv s27, p0, z17.s */
    "e1ef63c4",  /* prfd pldl1strm, p3, [sp, z3.d, lsl #3] */
    "b4e80205",  /* orr z20.d, z20.d, #0x1f800000000 */
    "feebcac4",  /* ldff1h {z30.d}, p2/z, [sp, z10.d] */
    "e7dffec5",  /* ld1d {z7.d}, p7/z, [sp, z30.d, lsl #3] */
    "48a09744",  /* uminp z8.s, p0/m, z8.s, z2.s */
    "c1304005",  /* eor z1.s, z1.s, #0xfc000001 */
    "fbdbd3c4",  /* ld1h {z27.d}, p6/z, [sp, z19.d] */
    "fac7f5c4",  /* ld1h {z26.d}, p1/z, [sp, z21.d, lsl #1] */
    "58985d44",  /* usqadd z24.h, p6/m, z24.h, z2.h */
    "e80389c4",  /* ld1sh {z8.d}, p0/z, [sp, z9.d, uxtw] */
    "5eda8205",  /* and z30.d, z30.d, #0xffffe000000000 */
    "23ed4305",  /* eor z3.d, z3.d, #0x1ffffffffff8 */
    "fba75aa4",  /* ldnf1b {z27.s}, p1/z, [sp, #-6, mul vl] */
    "8a3d2f45",  /* uqrshrnt z10.b, z12.h, #1 */
    "f81bba25",  /* whilehi p8.s, xzr, x26 */
    "2f8a1644",  /* shsubr z15.b, p2/m, z15.b, z17.b */
    "f7faff04",  /* sqdecd x23, #23, mul #16 */
    "552d4305",  /* eor z21.d, z21.d, #0xfffffffff800003f */
    "efdb8205",  /* and z15.d, z15.d, #0xffffffe00000001f */
    "ddca0005",  /* orr z29.s, z29.s, #0x3fffff80 */
    "960f0005",  /* orr z22.b, z22.b, #0xaa */
    "7e754205",  /* eor z30.d, z30.d, #0xfffc00003fffffff */
    "b5044305",  /* eor z21.d, z21.d, #0xffffffff0000003f */
    "e47b64a5",  /* ldff1w {z4.d}, p6/z, [sp, x4, lsl #2] */
    "6b64c005",  /* mov z11.h, #0xf0 */
    "f47f8ee4",  /* stnt1h {z20.h}, p7, [sp, x14, lsl #1] */
    "aecaa304",  /* sqdecw z14.s, #21, mul #4 */
    "e14f61c4",  /* prfw pldl1strm, p3, [sp, z1.d, sxtw #2] */
    "011c4305",  /* eor z1.d, z1.d, #0x3fffffffe0000000 */
    "056ec305",  /* mov z5.d, #0xfffffffffff8000f */
    "eb6f76c4",  /* prfd pstl2strm, p3, [sp, z22.d, sxtw #3] */
    "c1b36fc4",  /* prfh pldl1strm, p4, [x30, z15.d, lsl #1] */
    "60a14005",  /* eor z0.s, z0.s, #0xfff000 */
    "f9c34ee5",  /* st1w {z25.s}, p0, [sp, z14.s, sxtw] */
    "eb43e9e4",  /* st1h {z11.d}, p0, [sp, x9, lsl #1] */
    "f12fd9c4",  /* ldff1sh {z17.d}, p3/z, [sp, z25.d, sxtw] */
    "cbc58685",  /* prfd pstl2strm, p1, [x14, x6, lsl #3] */
    "2da1f105",  /* clastb x13, p0, x13, z9.d */
    "f57f44c4",  /* ldff1b {z21.d}, p7/z, [sp, z4.d, sxtw] */
    "01da9284",  /* prfh pldl1strm, p6, [x16, x18, lsl #1] */
    "e859c005",  /* mov z8.s, #0xffe0001f */
    "e2337cc4",  /* prfh pldl2keep, p4, [sp, z28.d, sxtw #1] */
    "e31798a4",  /* ld1rqh {z3.h}, p5/z, [sp, x24, lsl #1] */
    "e36b96c5",  /* ldff1d {z3.d}, p2/z, [sp, z22.d, uxtw] */
    "cbd08084",  /* prfh pstl2strm, p4, [x6, x0, lsl #1] */
    "fff23504",  /* sqincb xzr, #23, mul #6 */
    "fa4faa04",  /* index z26.s, wzr, w10 */
    "41ac4944",  /* sqneg z1.h, p3/m, z2.h */
    "17a11c65",  /* flogb z23.s, p0/m, z8.s */
    "1effee04",  /* uqdecd w30, #24, mul #15 */
    "72c1c005",  /* mov z18.s, #0xfff00 */
    "eeab20c5",  /* ldff1sw {z14.d}, p2/z, [z31.d] */
    "f6ffd0c5",  /* ldff1d {z22.d}, p7/z, [sp, z16.d] */
    "e7736885",  /* ldff1w {z7.s}, p4/z, [sp, z8.s, sxtw #2] */
    "977bc205",  /* mov z23.d, #0xfffe000000003fff */
    "ae126f84",  /* prfb #14, p4, [x21, z15.s, sxtw] */
    "d9278765",  /* fminv s25, p1, z30.s */
    "92230205",  /* orr z18.d, z18.d, #0xf000000001ffffff */
    "fb0fe525",  /* whilels p11.d, wzr, w5 */
    "1ffa2e04",  /* sqdecb xzr, wzr, #16, mul #15 */
    "f942c005",  /* mov z25.s, #0xff00ffff */
    "1fda0205",  /* orr z31.d, z31.d, #0x3fffe000000000 */
    "b6e42245",  /* aesd z22.b, z22.b, z5.b */
    "e3a774c4",  /* prfh pldl2strm, p1, [sp, z20.d, lsl #1] */
    "40878205",  /* and z0.d, z0.d, #0xffff07ffffffffff */
    "e9837fc5",  /* ld1sw {z9.d}, p0/z, [sp, z31.d, lsl #2] */
    "7e669fa4",  /* ldff1sw {z30.d}, p1/z, [x19, xzr, lsl #2] */
    "a22ec565",  /* fminnmv d2, p3, z21.d */
    "71c2a704",  /* sqincw z17.s, #19, mul #8 */
    "35b3c084",  /* ld1rh {z21.h}, p4/z, [x25] */
    "a2e67964",  /* bfmmla z2.s, z21.h, z25.h */
    "88feb804",  /* uqdecw x8, #20, mul #9 */
    "68278305",  /* and z8.d, z8.d, #0xfffffffff0ffffff */
    "214cd285",  /* prfw pldl1strm, p3, [x1, #18, mul vl] */
    "e5b3a3e4",  /* st1h {z5.d}, p4, [sp, z3.d, lsl #1] */
    "f2278ea4",  /* ld1rqh {z18.h}, p1/z, [sp, #-32] */
    "b135c005",  /* mov z17.h, #0xfcff */
    "61208005",  /* and z1.s, z1.s, #0xf0000000 */
    "0782c005",  /* mov z7.s, #0xffff0001 */
    "41f73004",  /* uqincb x1, #26 */
    "93f9c205",  /* mov z19.d, #0x3ffe00000000 */
    "fd6384a4",  /* ldff1sw {z29.d}, p0/z, [sp, x4, lsl #2] */
    "a43220a5",  /* ld1row {z4.s}, p4/z, [x21] */
    "e38366c4",  /* prfb pldl2strm, p0, [sp, z6.d] */
    "e0df0fa4",  /* ldnt1b {z0.b}, p7/z, [sp, x15] */
    "e847e0c5",  /* ld1d {z8.d}, p1/z, [sp, z0.d, sxtw #3] */
    "358f4344",  /* urshl z21.h, p3/m, z21.h, z25.h */
    "ee4372c5",  /* ld1w {z14.d}, p0/z, [sp, z18.d, sxtw #2] */
    "d0cf8305",  /* and z16.d, z16.d, #0xffffffffffffffbf */
    "3832ff25",  /* whilerw p8.d, x17, xzr */
    "e9e0c005",  /* mov z9.s, #0xff0 */
    "24eb8205",  /* and z4.d, z4.d, #0x1ffffff800000000 */
    "03160005",  /* orr z3.b, z3.b, #0x40 */
    "f6cb40e4",  /* st1b {z22.s}, p2, [sp, z0.s, sxtw] */
    "25c7e304",  /* uqincd z5.d, #25, mul #4 */
    "20f20085",  /* prfw pldl1keep, p4, [z17.s] */
    "eeef6dc4",  /* prfd #14, p3, [sp, z13.d, lsl #3] */
    "21258465",  /* fmaxnmv s1, p1, z9.s */
    "91f2b004",  /* sqincw x17, #20 */
    "10c02085",  /* ld1w {z16.s}, p0/z, [z0.s] */
    "f9cb16a5",  /* ldnt1w {z25.s}, p2/z, [sp, x22, lsl #2] */
    "a51074c4",  /* prfb pldl3strm, p4, [x5, z20.d, sxtw] */
    "3bf64305",  /* eor z27.d, z27.d, #0xffffffffffffc */
    "14cca0c5",  /* ld1d {z20.d}, p3/z, [z0.d] */
    "f97732a5",  /* ldff1sh {z25.s}, p5/z, [sp, x18, lsl #1] */
    "20c3a004",  /* sqincw z0.s, #25 */
    "fe63b284",  /* ldff1h {z30.s}, p0/z, [sp, z18.s, uxtw #1] */
    "facb60e5",  /* st1w {z26.s}, p2, [sp, z0.s, sxtw #2] */
    "0de87cc4",  /* prfd pstl3strm, p2, [x0, z28.d, lsl #3] */
    "36dd0045",  /* cadd z22.b, z22.b, z9.b, #270 */
    "4b8679c4",  /* prfb pstl2strm, p1, [x18, z25.d] */
    "e7e75ee4",  /* st3b {z7.b-z9.b}, p1, [sp, #-6, mul vl] */
    "ca5d4305",  /* eor z10.d, z10.d, #0xffffffffffe0000f */
    "95450005",  /* orr z21.h, z21.h, #0xff1f */
    "6862c305",  /* mov z8.d, #0xfffff00000 */
    "e86c7384",  /* prfd pstl1keep, p3, [x7, z19.s, sxtw #3] */
    "8880ec25",  /* incp z8.d, p4.d */
    "1ecbc005",  /* dupm z30.s, #0xffffff80 */
    "cbd11184",  /* prfb pstl2strm, p4, [x14, x17] */
    "e4c1ff04",  /* incd z4.d, #15, mul #16 */
    "0cc31884",  /* prfb pstl3keep, p0, [x24, x24] */
    "b568c205",  /* mov z21.d, #0x1f8000000000000 */
    "85bf1444",  /* smaxp z5.b, p7/m, z5.b, z28.b */
    "f1ae3105",  /* clastb w17, p3, w17, z23.b */
    "cacc78c4",  /* prfw pstl2keep, p3, [x6, z24.d, lsl #2] */
    "75c7ae04",  /* uqincw z21.s, #27, mul #15 */
    "144d4005",  /* eor z20.h, z20.h, #0xff80 */
    "e1d0c205",  /* mov z1.d, #0x3fc000000000 */
    "ffefa0e4",  /* st1h {z31.h}, p3, [sp] */
    "d0f1a904",  /* sqincw x16, w16, #14, mul #10 */
    "bf5d4205",  /* eor z31.d, z31.d, #0xffe00007ffffffff */
    "68400205",  /* orr z8.d, z8.d, #0xf00000000000000 */
    "99e410e5",  /* stnt1w {z25.s}, p1, [x4] */
    "2fb58144",  /* ursqrte z15.s, p5/m, z9.s */
    "0d0af985",  /* prfb pstl3strm, p2, [x16, #-7, mul vl] */
    "143d8005",  /* and z20.h, z20.h, #0xfe03 */
    "f81f2625",  /* whilels p8.b, xzr, x6 */
    "0b590825",  /* mov p11.b, p6/z, p8.b */
    "f20fb0a4",  /* ld1roh {z18.h}, p3/z, [sp, x16, lsl #1] */
    "eb136a84",  /* prfb pstl2strm, p4, [sp, z10.s, sxtw] */
    "04e23404",  /* incb x4, #16, mul #5 */
    "14324305",  /* eor z20.d, z20.d, #0x7fffc000000 */
    "76489025",  /* brkb p6.b, p2/m, p3.b */
    "a0ccc305",  /* mov z0.d, #0x1fffffffff80 */
    "47e6a0c4",  /* ldff1h {z7.d}, p1/z, [z18.d] */
    "e39ff584",  /* ld1rsw {z3.d}, p7/z, [sp, #212] */
    "7ad4c305",  /* mov z26.d, #0x3ffffffffc0 */
    "2680ea25",  /* sqdecp z6.d, p1.d */
    "efb799a4",  /* ldnf1sw {z15.d}, p5/z, [sp, #-7, mul vl] */
    "9d210305",  /* orr z29.d, z29.d, #0x1fff0000000 */
    "d36d8005",  /* and z19.h, z19.h, #0xfffb */
    "e5ea0084",  /* prfb pldl3strm, p2, [z23.s] */
    "cc32c765",  /* fminv d12, p4, z22.d */
    "0adc0145",  /* sqcadd z10.b, z10.b, z0.b, #270 */
    "e86bc2c4",  /* ldff1h {z8.d}, p2/z, [sp, z2.d, sxtw] */
    "521d8005",  /* and z18.h, z18.h, #0xe0ff */
    "dd9c4205",  /* eor z29.d, z29.d, #0xffffe000000fffff */
    "c83520a4",  /* ld1rob {z8.b}, p5/z, [x14] */
    "8fc7b304",  /* decw z15.s, #28, mul #4 */
    "36a0c444",  /* sadalp z22.d, p0/m, z1.s */
    "c2fe3404",  /* uqdecb x2, #22, mul #5 */
    "cada8205",  /* and z10.d, z10.d, #0xfffffe000000000 */
    "c8f57b04",  /* uqinch x8, #14, mul #12 */
    "fcd7bde5",  /* st1d {z28.d}, p5, [sp, z29.d, sxtw #3] */
    "22c27d04",  /* inch z2.h, #17, mul #14 */
    "7e3d8005",  /* and z30.h, z30.h, #0xfe1f */
    "e7f7dae5",  /* st3d {z7.d-z9.d}, p5, [sp, #-18, mul vl] */
    "1ec2c205",  /* mov z30.d, #0x1ffff0000000000 */
    "3a154305",  /* eor z26.d, z26.d, #0xffffffffc00000ff */
    "e46bc0c5",  /* ldff1d {z4.d}, p2/z, [sp, z0.d, sxtw] */
    "fc0a4205",  /* eor z28.d, z28.d, #0x80000000007fffff */
    "3efaec04",  /* sqdecd x30, w30, #17, mul #13 */
    "e37ffaa5",  /* ldff1d {z3.d}, p7/z, [sp, x26, lsl #3] */
    "d6f2c005",  /* mov z22.s, #0x1fffffc */
    "26eb00c5",  /* prfw #6, p2, [z25.d] */
    "066b4005",  /* eor z6.s, z6.s, #0xfff80fff */
    "e41725c4",  /* prfb pldl3keep, p5, [sp, z5.d, uxtw] */
    "e6c78084",  /* prfh #6, p1, [sp, x0, lsl #1] */
    "f607ce84",  /* ld1sh {z22.s}, p1/z, [sp, z14.s, sxtw] */
    "a8158005",  /* and z8.h, z8.h, #0xcfff */
    "c88a7dc4",  /* prfb pstl1keep, p2, [x22, z29.d] */
    "9935c005",  /* mov z25.h, #0xfc7f */
    "a5a1e105",  /* lastb x5, p0, z13.d */
    "f5a24205",  /* eor z21.d, z21.d, #0xfffff0000000000f */
    "f5ff5fc4",  /* ldff1b {z21.d}, p7/z, [sp, z31.d] */
    "38cf6704",  /* uqdech z24.h, #25, mul #8 */
    "eb754305",  /* eor z11.d, z11.d, #0xfffffffffffc0003 */
    "88f28085",  /* prfd pstl1keep, p4, [z20.s] */
    "e7b30ea4",  /* ld1b {z7.b}, p4/z, [sp, #-2, mul vl] */
    "75e310e4",  /* stnt1b {z21.b}, p0, [x27] */
    "e7f36de5",  /* st1w {z7.d}, p4, [sp, #-3, mul vl] */
    "87a38305",  /* and z7.d, z7.d, #0x1fffffff000 */
    "04c24205",  /* eor z4.d, z4.d, #0x1ffff0000000000 */
    "6b9fc305",  /* mov z11.d, #0xffffffffffffe1ff */
    "f947c5c5",  /* ld1d {z25.d}, p1/z, [sp, z5.d, sxtw] */
    "fc5f93c5",  /* ld1d {z28.d}, p7/z, [sp, z19.d, uxtw] */
    "b2038005",  /* and z18.s, z18.s, #0x3fffffff */
    "dfb24205",  /* eor z31.d, z31.d, #0xfffffc0000000001 */
    "206f0205",  /* orr z0.d, z0.d, #0xfff81fffffffffff */
    "5a848305",  /* and z26.d, z26.d, #0x7ffffffff0000 */
    "95334665",  /* fmaxv h21, p4, z28.h */
    "cdf22204",  /* sqincb x13, w13, #22, mul #3 */
    "51387105",  /* sunpkhi z17.h, z2.b */
    "132500a4",  /* ld1rqb {z19.b}, p1/z, [x8] */
    "db244005",  /* eor z27.h, z27.h, #0xf007 */
    "44f68205",  /* and z4.d, z4.d, #0xfffffffc001fffff */
    "b97c7fa5",  /* ldff1w {z25.d}, p7/z, [x5, xzr, lsl #2] */
    "0cf27804",  /* sqinch x12, #16, mul #9 */
    "b1658005",  /* and z17.h, z17.h, #0xfff3 */
    "0d660305",  /* orr z13.d, z13.d, #0xfffffffffff0001f */
    "f6d2c205",  /* mov z22.d, #0x3fffffc000000000 */
    "cbeb80c5",  /* prfd pstl2strm, p2, [z30.d] */
    "b3ff4205",  /* eor z19.d, z19.d, #0xfffffffe7fffffff */
    "70820205",  /* orr z16.d, z16.d, #0xffff00000000000f */
    "04a070a5",  /* ldnf1w {z4.d}, p0/z, [x0] */
    "4017c205",  /* mov z0.d, #0xc1ffffffffffffff */
    "8f544005",  /* eor z15.h, z15.h, #0x7c0 */
    "8ac77604",  /* dech z10.h, #28, mul #7 */
    "63998205",  /* and z3.d, z3.d, #0x1ffe00000000000 */
    "feaf70a4",  /* ldnf1b {z30.d}, p3/z, [sp] */
    "4d8ee005",  /* mov z13.d, p3/m, d18 */
    "41fa7cc4",  /* prfd pldl1strm, p6, [x18, z28.d, lsl #3] */
    "348a4005",  /* eor z20.s, z20.s, #0xffff8001 */
    "96318005",  /* and z22.s, z22.s, #0xfc00007f */
    "e8f395e4",  /* stnt1h {z8.h}, p4, [sp, #5, mul vl] */
    "367a0205",  /* orr z22.d, z22.d, #0xfffe000000000007 */
    "afac8a64",  /* bfcvtnt z15.h, p3/m, z5.s */
    "c336c005",  /* dupm z3.b, #0xfd */
    "144d8005",  /* and z20.h, z20.h, #0xff80 */
    "f8ac4205",  /* eor z24.d, z24.d, #0xfffff8000007ffff */
    "6021ec25",  /* ctermeq x11, x12 */
    "ace38005",  /* and z12.s, z12.s, #0xfffffff3 */
    "6acee004",  /* uqdecd z10.d, #19 */
    "8e448205",  /* and z14.d, z14.d, #0xff0000001fffffff */
    "f1c7f1e4",  /* st1h {z17.s}, p1, [sp, z17.s, sxtw #1] */
    "53c2a084",  /* ld1h {z19.s}, p0/z, [z18.s] */
    "fa4746c5",  /* ld1w {z26.d}, p1/z, [sp, z6.d, sxtw] */
    "c15a0205",  /* orr z1.d, z1.d, #0xffe0000000000fff */
    "c9ff8205",  /* and z9.d, z9.d, #0xfffffffeffffffff */
    "34ea8005",  /* and z20.s, z20.s, #0x1ffff8 */
    "ef63cd85",  /* prfd #15, p0, [sp, #13, mul vl] */
    "c9fd90e4",  /* stnt1h {z9.h}, p7, [x14] */
    "7816c205",  /* mov z24.d, #0xc003ffffffffffff */
    "4881c205",  /* mov z8.d, #0x7ff000000000000 */
    "56e3a004",  /* cntw x22, #26 */
    "34c22084",  /* ld1b {z20.s}, p0/z, [z17.s] */
    "91f1bc04",  /* sqincw x17, vl128, mul #13 */
    "f5564205",  /* eor z21.d, z21.d, #0xffc03fffffffffff */
    "fe3b5b84",  /* ldff1sb {z30.s}, p6/z, [sp, z27.s, sxtw] */
    "9f23c665",  /* fmaxv d31, p0, z28.d */
    "f6f1b604",  /* sqincw x22, #15, mul #7 */
    "c4a60205",  /* orr z4.d, z4.d, #0xfffff007ffffffff */
    "f1f3efa4",  /* ld4h {z17.h-z20.h}, p4/z, [sp, #-4, mul vl] */
    "fff94005",  /* eor z31.s, z31.s, #0x1fffe */
    "ecc78385",  /* prfd pstl3keep, p1, [sp, x3, lsl #3] */
    "07c36804",  /* sqinch z7.h, #24, mul #9 */
    "e12b07a4",  /* ld1rqb {z1.b}, p2/z, [sp, #112] */
    "56038005",  /* and z22.s, z22.s, #0x7ffffff */
    "72a24205",  /* eor z18.d, z18.d, #0xfffff00000000000 */
    "43214305",  /* eor z3.d, z3.d, #0x7ff0000000 */
    "455a0305",  /* orr z5.d, z5.d, #0xffffe00000 */
    "e1032384",  /* prfb pldl1strm, p0, [sp, z3.s, uxtw] */
    "1b758005",  /* and z27.h, z27.h, #0x7fc */
    "f9af16e4",  /* st1b {z25.d}, p3, [sp, z22.d] */
    "4817c005",  /* dupm z8.b, #0xdd */
    "ef9760c4",  /* prfb #15, p5, [sp, z0.d] */
    "2b828005",  /* and z11.s, z11.s, #0xffff0003 */
    "5effc305",  /* mov z30.d, #0xffffffffffffffe */
    "2fca6004",  /* sqdech z15.h, #17 */
    "e9337684",  /* prfh pstl1strm, p4, [sp, z22.s, sxtw #1] */
    "ec214005",  /* eor z12.s, z12.s, #0xf0000fff */
    "f1f3e0c5",  /* ldff1d {z17.d}, p4/z, [sp, z0.d, lsl #3] */
    "fab1c305",  /* mov z26.d, #0x3fffc00 */
    "01f48084",  /* prfh pldl1strm, p5, [z0.s] */
    "fa4f0a25",  /* mov p10.b, p3/m, p15.b */
    "add14085",  /* ld1rw {z13.s}, p4/z, [x13] */
    "05e3af04",  /* cntw x5, #24, mul #16 */
    "a7e67204",  /* dech x7, #21, mul #3 */
    "e93728a4",  /* ld1rob {z9.b}, p5/z, [sp, #-256] */
    "e71b9ca5",  /* ld1rqd {z7.d}, p6/z, [sp, x28, lsl #3] */
    "0bab8305",  /* and z11.d, z11.d, #0xffffff800 */
    "fbfe6004",  /* uqdech w27, #23 */
    "30374205",  /* eor z16.d, z16.d, #0xfc0fffffffffffff */
    "8992c005",  /* mov z9.s, #0xffffc007 */
    "e41fd385",  /* prfb pldl3keep, p7, [sp, #19, mul vl] */
    "84fa4305",  /* eor z4.d, z4.d, #0x3ffffe */
    "ffae0a64",  /* fcvtxnt z31.s, p3/m, z23.d */
    "f32f62c5",  /* ldff1sw {z19.d}, p3/z, [sp, z2.d, sxtw #2] */
    "1015c005",  /* mov z16.h, #0xc07f */
    "78e3c205",  /* mov z24.d, #0xfffffff000000000 */
    "b8318305",  /* and z24.d, z24.d, #0xfffc000000 */
    "ff0e0205",  /* orr z31.d, z31.d, #0x807fffffffffffff */
    "95fe0305",  /* orr z21.d, z21.d, #0x3ffffffffffffe */
    "ec57fe85",  /* prfw pstl3keep, p5, [sp, #-2, mul vl] */
    "b8496045",  /* uqxtnb z24.s, z13.d */
    "6bac8205",  /* and z11.d, z11.d, #0xfffff80000007fff */
    "8b4dc005",  /* dupm z11.h, #0xff8f */
    "24f72904",  /* uqincb w4, #25, mul #10 */
    "13b90005",  /* orr z19.s, z19.s, #0x3fe00 */
    "d1364305",  /* eor z17.d, z17.d, #0xfffffffffc01ffff */
    "a8b80005",  /* orr z8.s, z8.s, #0x7e00 */
    "f2028205",  /* and z18.d, z18.d, #0xffffff */
    "a43dc205",  /* mov z4.d, #0xfe00007fffffffff */
    "0b842084",  /* ld1sb {z11.s}, p1/z, [z0.s] */
    "6be77004",  /* dech x11, #27 */
    "e5ce0784",  /* prfb pldl3strm, p3, [x23, x7] */
    "2c68c005",  /* mov z12.s, #0x180000 */
    "ebfb89a4",  /* ldnt1h {z11.h}, p6/z, [sp, #-7, mul vl] */
    "e7f7a0a4",  /* ld2h {z7.h-z8.h}, p5/z, [sp] */
    "de378305",  /* and z30.d, z30.d, #0xfffffffffdffffff */
    "9c368205",  /* and z28.d, z28.d, #0xfc007fffffffffff */
    "f12ba8a5",  /* ld1rod {z17.d}, p2/z, [sp, #-256] */
    "d91cc005",  /* mov z25.h, #0xe00f */
    "ed9f29e5",  /* st1w {z13.d}, p7, [sp, z9.d, uxtw #2] */
    "a7942005",  /* mov z7.b, p5/m, b5 */
    "f1e4c305",  /* mov z17.d, #0xffffffffff0 */
    "58e3c205",  /* mov z24.d, #0x7ffffff000000000 */
    "c73ec005",  /* dupm z7.b, #0xfe */
    "ed5fcf85",  /* prfw pstl3strm, p7, [sp, #15, mul vl] */
    "0cd74205",  /* eor z12.d, z12.d, #0xffffffc07fffffff */
    "4cd94005",  /* eor z12.s, z12.s, #0xffe0 */
    "b8a70205",  /* orr z24.d, z24.d, #0xfffff3ffffffffff */
    "db418305",  /* and z27.d, z27.d, #0x7fff000000 */
    "52a9c0e5",  /* st1d {z18.d}, p2, [z10.d] */
    "f3630885",  /* ldff1w {z19.s}, p0/z, [sp, z8.s, uxtw] */
    "0acb0305",  /* orr z10.d, z10.d, #0xffffff80 */
    "bbf2e004",  /* sqincd x27, w27, #21 */
    "122b4305",  /* eor z18.d, z18.d, #0xffffff8000000 */
    "e86ba3c5",  /* ldff1d {z8.d}, p2/z, [sp, z3.d, uxtw #3] */
    "bc6a8205",  /* and z28.d, z28.d, #0xfff80000000001ff */
    "523d9865",  /* fadda s18, p7, s18, z10.s */
    "a7910005",  /* orr z7.s, z7.s, #0xfffc000 */
    "c43c0005",  /* orr z4.h, z4.h, #0xfe00 */
    "78320005",  /* orr z24.s, z24.s, #0xfc003fff */
    "59fa8005",  /* and z25.s, z25.s, #0xffffe */
    "f3133ac5",  /* ld1sw {z19.d}, p4/z, [sp, z26.d, uxtw #2] */
    "581f3f25",  /* whilels p8.b, x26, xzr */
    "d6828205",  /* and z22.d, z22.d, #0xffff00000000007f */
    "93b8c005",  /* dupm z19.s, #0x3e00 */
    "ecc7d0c5",  /* ld1d {z12.d}, p1/z, [sp, z16.d] */
    "1d4e3f04",  /* index z29.b, w16, wzr */
    "e93f3cc4",  /* prfh pstl1strm, p7, [sp, z28.d, uxtw #1] */
    "a5790205",  /* orr z5.d, z5.d, #0x7ffe000000000000 */
    "8e41c005",  /* mov z14.s, #0xff00001f */
    "c38b0005",  /* orr z3.s, z3.s, #0xffffbfff */
    "e51b2ea4",  /* ld1rob {z5.b}, p6/z, [sp, x14] */
    "76466045",  /* sqxtnt z22.s, z19.d */
    "c2f10205",  /* orr z2.d, z2.d, #0x1fffc00000000 */
    "4ae3f004",  /* incd x10, #26 */
    "3e618305",  /* and z30.d, z30.d, #0x3ff00000 */
    "8bfb8305",  /* and z11.d, z11.d, #0x3ffffffe */
    "ff037125",  /* whilegt p15.h, wzr, w17 */
    "23f90085",  /* prfw pldl2strm, p6, [z9.s] */
    "e057c185",  /* prfw pldl1keep, p5, [sp, #1, mul vl] */
    "f9ab48c5",  /* ldff1sw {z25.d}, p2/z, [sp, z8.d] */
    "b2b88005",  /* and z18.s, z18.s, #0x7e00 */
    "1cbbc005",  /* mov z28.s, #0xfffffe03 */
    "c66c8005",  /* and z6.h, z6.h, #0x3f8 */
    "ea208205",  /* and z10.d, z10.d, #0xf00000000000000f */
    "5f15c305",  /* mov z31.d, #0xffffffffc00001ff */
    "9f760205",  /* orr z31.d, z31.d, #0xfffc007fffffffff */
    "e4e200c4",  /* prfb pldl3keep, p0, [z23.d] */
    "ee47ed85",  /* prfw #14, p1, [sp, #-19, mul vl] */
    "e3458205",  /* and z3.d, z3.d, #0xff0000ffffffffff */
    "30308205",  /* and z16.d, z16.d, #0xc00000000000000 */
    "34100005",  /* orr z20.s, z20.s, #0xc0000000 */
    "41558205",  /* and z1.d, z1.d, #0xffc00001ffffffff */
    "d93e8305",  /* and z25.d, z25.d, #0xfffffffffe00ffff */
    "ef704725",  /* movs p15.b, p12/z, p7.b */
    "e023f485",  /* prfh pldl1keep, p0, [sp, #-12, mul vl] */
    "2ace4205",  /* eor z10.d, z10.d, #0xffffff8001ffffff */
    "e8223005",  /* mov z8.q, q23 */
    "87f94305",  /* eor z7.d, z7.d, #0x3ffe */
    "11e2e004",  /* cntd x17, #16 */
    "d9c0c005",  /* dupm z25.s, #0x7f00 */
    "5c16bf25",  /* whilele p12.s, x18, xzr */
    "433a8005",  /* and z3.s, z3.s, #0xfe000fff */
    "b1b20005",  /* orr z17.s, z17.s, #0xfffffc00 */
    "2ff04005",  /* eor z15.s, z15.s, #0xc */
    "e19dc305",  /* mov z1.d, #0x1fffffffffffe000 */
    "4349c085",  /* prfw pldl2strm, p2, [x10] */
    "19f7bd04",  /* uqincw x25, #24, mul #14 */
    "3738b305",  /* uunpkhi z23.s, z1.h */
    "e10beb25",  /* whilehs p1.d, wzr, w11 */
    "efdf8485",  /* prfd #15, p7, [sp, x4, lsl #3] */
    "29104205",  /* eor z9.d, z9.d, #0xc000000000000000 */
    "fe27a1a4",  /* ld1roh {z30.h}, p1/z, [sp, #32] */
    "ea533f84",  /* prfw pstl2keep, p4, [sp, z31.s, uxtw #2] */
    "e3c7a304",  /* uqincw z3.s, all, mul #4 */
    "d4ba4205",  /* eor z20.d, z20.d, #0xfffffe0000000000 */
    "4e558005",  /* and z14.h, z14.h, #0xffc1 */
    "29f700c4",  /* prfb pstl1strm, p5, [z25.d] */
    "f8c724a5",  /* ld2w {z24.s-z25.s}, p1/z, [sp, x4, lsl #2] */
    "ebe3c305",  /* mov z11.d, #0xffffffff0 */
    "d922c065",  /* faddv d25, p0, z22.d */
    "fc2720a5",  /* ld1row {z28.s}, p1/z, [sp] */
    "d6f10005",  /* orr z22.s, z22.s, #0x1fffc */
    "ea07fd25",  /* whilelt p10.d, wzr, w29 */
    "04389fe5",  /* stnt1d {z4.d}, p6, [z0.d, xzr] */
    "9c590005",  /* orr z28.s, z28.s, #0xffe00003 */
    "25b24305",  /* eor z5.d, z5.d, #0xffffc00 */
    "fce76cc5",  /* ldff1w {z28.d}, p1/z, [sp, z12.d, lsl #2] */
    "97fb4005",  /* eor z23.s, z23.s, #0x3ffffffe */
    "453d8205",  /* and z5.d, z5.d, #0xfe00000fffffffff */
    "89f480c4",  /* prfh pstl1strm, p5, [z4.d] */
    "ee57fe84",  /* ld1h {z14.s}, p5/z, [sp, z30.s, sxtw #1] */
    "7e554305",  /* eor z30.d, z30.d, #0xffffffffffc00003 */
    "a0b38305",  /* and z0.d, z0.d, #0xfffffffc00 */
    "e253ede5",  /* st1d {z2.d}, p4, [sp, x13, lsl #3] */
    "e56f7884",  /* prfd pldl3strm, p3, [sp, z24.s, sxtw #3] */
    "4a738205",  /* and z10.d, z10.d, #0xfffc000000001fff */
    "abca8005",  /* and z11.s, z11.s, #0x1fffff80 */
    "fb1b1684",  /* ld1sb {z27.s}, p6/z, [sp, z22.s, uxtw] */
    "3c5c8305",  /* and z28.d, z28.d, #0x7fffffffe00000 */
    "cc33c085",  /* prfh pstl3keep, p4, [x30] */
    "2c9e4305",  /* eor z12.d, z12.d, #0x7fffffffffffe000 */
    "e80fbf25",  /* whilelo p8.s, wzr, wzr */
    "0f3ac205",  /* mov z15.d, #0xfe000000000003ff */
    "3b2d0205",  /* orr z27.d, z27.d, #0xf800001fffffffff */
    "114c8305",  /* and z17.d, z17.d, #0xffffffff800000 */
    "d2a14005",  /* eor z18.s, z18.s, #0x7fff000 */
    "58cea404",  /* uqdecw z24.s, #18, mul #5 */
    "fdab90a5",  /* ldnf1sb {z29.d}, p2/z, [sp] */
    "e709c085",  /* prfb #7, p2, [x15] */
    "f9cf85e5",  /* st1d {z25.d}, p3, [sp, z5.d, sxtw] */
    "13c0e304",  /* sqincd z19.d, pow2, mul #4 */
    "186fc205",  /* mov z24.d, #0xfff80fffffffffff */
    "68248065",  /* faddv s8, p1, z3.s */
    "ecd71a85",  /* prfw pstl3keep, p5, [sp, x26, lsl #2] */
    "f9eac205",  /* mov z25.d, #0x7fffff800000000 */
    "f4dbc085",  /* ld1rsb {z20.h}, p6/z, [sp] */
    "d63e4005",  /* eor z22.b, z22.b, #0xfe */
    "e467e985",  /* prfd pldl3keep, p1, [sp, #-23, mul vl] */
    "fec1e904",  /* sqincd z30.d, #15, mul #10 */
    "43e3b004",  /* incw x3, #26 */
    "8b033f25",  /* whilege p11.b, w28, wzr */
    "5bf48205",  /* and z27.d, z27.d, #0xfffffffc0000001f */
    "79af0205",  /* orr z25.d, z25.d, #0xfffff87fffffffff */
    "e3272aa5",  /* ld1row {z3.s}, p1/z, [sp, #-192] */
    "0a849c65",  /* fmaxnm z10.s, p1/m, z10.s, #0.0 */
    "10cc6804",  /* uqdech z16.h, pow2, mul #9 */
    "bff86604",  /* sqdech xzr, wzr, vl5, mul #7 */
    "cbb38205",  /* and z11.d, z11.d, #0xfffffc00000001ff */
    "e3f77e04",  /* uqinch x3, all, mul #15 */
    "5ef24005",  /* eor z30.s, z30.s, #0x1ffffc */
    "2398dd65",  /* fminnm z3.d, p6/m, z3.d, #1.0 */
    "a3f180c5",  /* prfd pldl2strm, p4, [z13.d] */
    "f52716c5",  /* ldff1sw {z21.d}, p1/z, [sp, z22.d, uxtw] */
    "0cff2004",  /* uqdecb w12, #24 */
    "4e5a0005",  /* orr z14.s, z14.s, #0xffe000ff */
    "521c4005",  /* eor z18.h, z18.h, #0xe000 */
    "ef533b84",  /* prfw #15, p4, [sp, z27.s, uxtw #2] */
    "23e22804",  /* cntb x3, #17, mul #9 */
    "bbb68305",  /* and z27.d, z27.d, #0xfffffffffffffc00 */
    "19e6f304",  /* decd x25, #16, mul #4 */
    "eaaf0ea5",  /* ld1sh {z10.d}, p3/z, [sp, #-2, mul vl] */
    "e2477204",  /* index z2.h, wzr, #-14 */
    "ed575584",  /* ld1b {z13.s}, p5/z, [sp, z21.s, sxtw] */
    "e8eb19e4",  /* stnt1b {z8.b}, p2, [sp, #-7, mul vl] */
    "2a80eb25",  /* uqdecp z10.d, p1.d */
    "d6057f25",  /* whilele p6.h, w14, wzr */
    "463e0205",  /* orr z6.d, z6.d, #0xfe000fffffffffff */
    "17ff0205",  /* orr z23.d, z23.d, #0xfffffffe03ffffff */
    "6ac37004",  /* inch z10.h, #27 */
    "571e0005",  /* orr z23.b, z23.b, #0xe0 */
    "6eba0005",  /* orr z14.s, z14.s, #0x1ffffe00 */
    "8525a0a5",  /* ld1rod {z5.d}, p1/z, [x12] */
    "ecd38684",  /* prfh pstl3keep, p4, [sp, x6, lsl #1] */
    "ec868305",  /* and z12.d, z12.d, #0xffffffffffff00ff */
    "e2a7fea5",  /* ldnf1d {z2.d}, p1/z, [sp, #-2, mul vl] */
    "ffc1b104",  /* incw z31.s, #15, mul #2 */
    "1e320005",  /* orr z30.s, z30.s, #0xfc0007ff */
    "77adc305",  /* mov z23.d, #0x7ffffffffff800 */
    "e9fb50e4",  /* st3b {z9.b-z11.b}, p6, [sp] */
    "f88c2b25",  /* uqdecp x24, p7.b */
    "eec36fc4",  /* prfw #14, p0, [sp, z15.d, lsl #2] */
    "6e7b4e25",  /* nots p14.b, p14/z, p11.b */
    "e5db0285",  /* prfw pldl3strm, p6, [sp, x2, lsl #2] */
    "7c89ec25",  /* incp x28, p11.d */
    "7b1a0005",  /* orr z27.s, z27.s, #0xe001ffff */
    "7d2d0005",  /* orr z29.h, z29.h, #0xf87f */
    "ee4394a4",  /* ld1sw {z14.d}, p0/z, [sp, x20, lsl #2] */
    "8855c005",  /* dupm z8.h, #0xffc7 */
    "e6ab70a5",  /* ldnf1w {z6.d}, p2/z, [sp] */
    "fff3e7e5",  /* st1d {z31.d}, p4, [sp, #7, mul vl] */
    "feaf5ec4",  /* ldff1sb {z30.d}, p3/z, [sp, z30.d] */
    "93650005",  /* orr z19.h, z19.h, #0xfff1 */
    "ac06c005",  /* dupm z12.b, #0x3f */
    "547c4205",  /* eor z20.d, z20.d, #0xfffe0000000fffff */
    "e6858205",  /* and z6.d, z6.d, #0xffff0000ffffffff */
    "f8a72805",  /* mov z24.b, p1/m, wsp */
    "422ac205",  /* mov z2.d, #0xf800000000003fff */
    "c6054205",  /* eor z6.d, z6.d, #0x7fffffffffff */
    "65f2e304",  /* sqincd x5, w5, #19, mul #4 */
    "5d3d4005",  /* eor z29.h, z29.h, #0xfe0f */
    "62177f25",  /* whilelt p2.h, x27, xzr */
    "e0e22345",  /* sm4e z0.s, z0.s, z23.s */
    "ef733084",  /* prfd #15, p4, [sp, z16.s, uxtw #3] */
    "59508005",  /* and z25.s, z25.s, #0x1c00000 */
    "a3ed4305",  /* eor z3.d, z3.d, #0x1fffffffffff8 */
    "275ec305",  /* mov z7.d, #0xffffffffffe0007f */
    "ea1b2d84",  /* prfb pstl2keep, p6, [sp, z13.s, uxtw] */
    "a3930305",  /* orr z3.d, z3.d, #0xfffffffc000 */
    "551d0005",  /* orr z21.h, z21.h, #0xe0ff */
    "357ac005",  /* mov z21.s, #0xfffe0007 */
    "a4ca6104",  /* sqdech z4.h, #21, mul #2 */
    "b03a0205",  /* orr z16.d, z16.d, #0xfe00000000007fff */
    "ffa6a105",  /* lastb wzr, p1, z23.s */
    "73fa7404",  /* sqdech x19, #19, mul #5 */
    "4b3cc005",  /* dupm z11.h, #0xe00 */
    "fbf28005",  /* and z27.s, z27.s, #0x3fffffc */
    "9b270305",  /* orr z27.d, z27.d, #0xfffffffff1ffffff */
    "9ffaa704",  /* sqdecw xzr, wzr, #20, mul #8 */
    "8d328205",  /* and z13.d, z13.d, #0xfc00000000007fff */
    "d47ac205",  /* mov z20.d, #0xfffe0000000000ff */
    "0c03c005",  /* mov z12.s, #0x1ffffff */
    "edc38f84",  /* prfh pstl3strm, p0, [sp, x15, lsl #1] */
    "f6f53104",  /* uqincb x22, #15, mul #2 */
    "c3830005",  /* orr z3.s, z3.s, #0xffff7fff */
    "1213c005",  /* mov z18.s, #0xc07fffff */
    "2742c085",  /* prfw #7, p0, [x17] */
    "2b724205",  /* eor z11.d, z11.d, #0xfffc00000000000f */
    "a1550005",  /* orr z1.h, z1.h, #0xffcf */
    "acd54205",  /* eor z12.d, z12.d, #0xffffffc0000fffff */
    "7638f205",  /* uunpklo z22.d, z3.s */
    "872ec005",  /* dupm z7.b, #0xf8 */
    "e1cb1784",  /* prfb pldl1strm, p2, [sp, x23] */
    "535dc005",  /* dupm z19.h, #0xffe0 */
    "bd908205",  /* and z29.d, z29.d, #0xfc00000000000 */
    "dc044005",  /* eor z28.h, z28.h, #0x7f */
    "86460305",  /* orr z6.d, z6.d, #0xffffffffff001fff */
    "e30f7b25",  /* whilelo p3.h, wzr, w27 */
    "e14feb04",  /* index z1.d, xzr, x11 */
    "20344205",  /* eor z0.d, z0.d, #0xfc0000000fffffff */
    "e7ed4205",  /* eor z7.d, z7.d, #0xfffffff80007ffff */
    "3e9c5f65",  /* fmin z30.h, p7/m, z30.h, #1.0 */
    "25030305",  /* orr z5.d, z5.d, #0x3ffffff00000000 */
    "ee6b67c4",  /* prfd #14, p2, [sp, z7.d, sxtw #3] */
    "2d474305",  /* eor z13.d, z13.d, #0xffffffffff03ffff */
    "4a3b0305",  /* orr z10.d, z10.d, #0xffffffe000000 */
    "b4ddc205",  /* mov z20.d, #0xffffffe00007ffff */
    "7add4205",  /* eor z26.d, z26.d, #0xffffffe00001ffff */
    "ae8a4305",  /* eor z14.d, z14.d, #0x1fffff8000 */
    "e51fa1e5",  /* str p5, [sp, #-241, mul vl] */
    "9d234205",  /* eor z29.d, z29.d, #0xf000000001ffffff */
    "0b528005",  /* and z11.s, z11.s, #0xffc0007f */
    "880e4205",  /* eor z8.d, z8.d, #0x800fffffffffffff */
    "8b5e8305",  /* and z11.d, z11.d, #0xffffffffffe003ff */
    "e8306704",  /* mov z8.d, z7.d */
    "290dc305",  /* mov z9.d, #0xffffffff800001ff */
    "b1154005",  /* eor z17.h, z17.h, #0xcfff */
    "5b238205",  /* and z27.d, z27.d, #0xf0000000007fffff */
    "ca6b0205",  /* orr z10.d, z10.d, #0xfff800000003ffff */
    "ac9c0205",  /* orr z12.d, z12.d, #0xffffe0000007ffff */
    "c6313f25",  /* whilewr p6.b, x14, xzr */
    "8b0ac005",  /* mov z11.s, #0x800fffff */
    "5ce78205",  /* and z28.d, z28.d, #0xfffffff07fffffff */
    "6b334305",  /* eor z11.d, z11.d, #0x3ffffffc000000 */
    "ac950305",  /* orr z12.d, z12.d, #0xfffffffffffc000 */
    "4e740005",  /* orr z14.h, z14.h, #0x1c */
    "43554005",  /* eor z3.h, z3.h, #0xffc1 */
    "2ce28084",  /* prfh pstl3keep, p0, [z17.s] */
    "567dc005",  /* mov z22.h, #0xffe */
    "7a852a05",  /* clasta b26, p1, b26, z11.b */
    "70340005",  /* orr z16.h, z16.h, #0x3c00 */
    "a6806d25",  /* decp z6.h, p5.h */
    "e973c305",  /* mov z9.d, #0x3fffffffc0000 */
    "03e6c205",  /* mov z3.d, #0xfffffff0001fffff */
    "64e26004",  /* cnth x4, #19 */
    "570d7f25",  /* whilels p7.h, w10, wzr */
    "819e4205",  /* eor z1.d, z1.d, #0xffffe003ffffffff */
    "25c60305",  /* orr z5.d, z5.d, #0x3ffffffffffff00 */
    "d45d4205",  /* eor z20.d, z20.d, #0xffe0000fffffffff */
    "e90b32c4",  /* prfb pstl1strm, p2, [sp, z18.d, uxtw] */
    "518e0205",  /* orr z17.d, z17.d, #0xffff8003ffffffff */
    "b2724005",  /* eor z18.s, z18.s, #0xfffc00ff */
    "0b8ec205",  /* mov z11.d, #0xffff8000ffffffff */
    "f17f7fa4",  /* ldff1b {z17.d}, p7/z, [sp, xzr] */
    "ce1a0005",  /* orr z14.s, z14.s, #0xe00fffff */
    "e837d985",  /* prfh pstl1keep, p5, [sp, #25, mul vl] */
    "ecf78ca5",  /* ldnt1d {z12.d}, p5/z, [sp, #-4, mul vl] */
    "88240305",  /* orr z8.d, z8.d, #0xfffffffff0000001 */
    "6036c005",  /* dupm z0.b, #0x3c */
    "f8a700a5",  /* ld1sh {z24.d}, p1/z, [sp] */
    "94f66004",  /* uqinch w20, #20 */
    "e2c6e004",  /* uqincd z2.d, #23 */
    "dadbc005",  /* dupm z26.s, #0xffffffef */
    "cc798305",  /* and z12.d, z12.d, #0xfffe0000 */
    "9c9bc305",  /* mov z28.d, #0x3ffffffe000 */
    "eadf63c4",  /* prfw pstl2keep, p7, [sp, z3.d, lsl #2] */
    "eefb49a4",  /* ld3b {z14.b-z16.b}, p6/z, [sp, #-21, mul vl] */
    "86faaa04",  /* sqdecw x6, w6, #20, mul #11 */
    "7709bf25",  /* whilehi p7.s, w11, wzr */
    "024b8005",  /* and z2.s, z2.s, #0xff80ffff */
    "82bd0205",  /* orr z2.d, z2.d, #0xfffffe00003fffff */
    "ea3f3084",  /* prfh pstl2keep, p7, [sp, z16.s, uxtw #1] */
    "4083c005",  /* mov z0.s, #0xffff07ff */
    "e82f7684",  /* prfh pstl1keep, p3, [sp, z22.s, sxtw #1] */
    "2fe02245",  /* aese z15.b, z15.b, z1.b */
    "492bc005",  /* mov z9.s, #0xf83fffff */
    "39424005",  /* eor z25.s, z25.s, #0xff0003ff */
    "6b83c005",  /* mov z11.s, #0xffff0fff */
    "ca3ac005",  /* mov z10.s, #0xfe00ffff */
    "430fc005",  /* dupm z3.b, #0xbb */
    "86df0305",  /* orr z6.d, z6.d, #0xffffffffffffffe3 */
    "32ce6004",  /* uqdech z18.h, #17 */
    "c5896825",  /* sqincp x5, p14.h, w5 */
    "78768205",  /* and z24.d, z24.d, #0xfffc003fffffffff */
    "c5a3c005",  /* mov z5.s, #0xfffff7ff */
    "a80e8005",  /* and z8.b, z8.b, #0x9f */
    "6c068005",  /* and z12.b, z12.b, #0xf */
    "e173fc85",  /* prfd pldl1strm, p4, [sp, #-4, mul vl] */
    "40560305",  /* orr z0.d, z0.d, #0xffffffffffc001ff */
    "c6d84205",  /* eor z6.d, z6.d, #0xfe000000000 */
    "66050305",  /* orr z6.d, z6.d, #0xffffffff00000fff */
    "c7ab0005",  /* orr z7.s, z7.s, #0xfffffbff */
    "75744005",  /* eor z21.h, z21.h, #0x3c */
    "830cc205",  /* mov z3.d, #0x8000000fffffffff */
    "8e4f8305",  /* and z14.d, z14.d, #0xffffffffff8fffff */
    "4fbf0305",  /* orr z15.d, z15.d, #0xfffffffffffffe0f */
    "68a38005",  /* and z8.s, z8.s, #0xfffff0ff */
    "922c8305",  /* and z18.d, z18.d, #0xfffffffff8000000 */
    "5ff1a504",  /* sqincw xzr, wzr, vl32, mul #6 */
    "e1b88005",  /* and z1.s, z1.s, #0x1fe00 */
    "5e232805",  /* mov z30.d, d26 */
    "b72fc205",  /* mov z23.d, #0xf9ffffffffffffff */
    "e1276284",  /* prfh pldl1strm, p1, [sp, z2.s, sxtw #1] */
    "e0e778c4",  /* prfd pldl1keep, p1, [sp, z24.d, lsl #3] */
    "b3ad0205",  /* orr z19.d, z19.d, #0xfffff80001ffffff */
    "6d78c305",  /* mov z13.d, #0x1e0000 */
    "4924c085",  /* prfh pstl1strm, p1, [x2] */
    "52f10005",  /* orr z18.s, z18.s, #0x1ffc */
    "7f0d4305",  /* eor z31.d, z31.d, #0xffffffff800007ff */
    "ea47e404",  /* index z10.d, xzr, #4 */
    "4729c085",  /* prfh #7, p2, [x10] */
    "1cfc7004",  /* uqdech x28, pow2 */
    "33958205",  /* and z19.d, z19.d, #0xffffc00000ffffff */
    "eacb4305",  /* eor z10.d, z10.d, #0x7fffffff80 */
    "edff3504",  /* uqdecb x13, all, mul #6 */
    "e84b7a84",  /* prfw pstl1keep, p2, [sp, z26.s, sxtw #2] */
    "58f14305",  /* eor z24.d, z24.d, #0x1ffc */
    "2727c205",  /* mov z7.d, #0xf03fffffffffffff */
    "699b0305",  /* orr z9.d, z9.d, #0x1ffffffe000 */
    "4a15c005",  /* mov z10.h, #0xc1ff */
    "eec7e704",  /* uqincd z14.d, all, mul #8 */
    "fcebe0a5",  /* ld4d {z28.d-z31.d}, p2/z, [sp] */
    "f7775fa5",  /* ldff1w {z23.s}, p5/z, [sp, xzr, lsl #2] */
    "f13cc005",  /* mov z17.h, #0xfe01 */
    "fbef20e4",  /* st1b {z27.h}, p3, [sp] */
    "ac058005",  /* and z12.h, z12.h, #0x3fff */
    "5e1f4005",  /* eor z30.b, z30.b, #0xee */
    "e9f20085",  /* prfw pstl1strm, p4, [z23.s] */
    "4c88eb25",  /* uqdecp w12, p2.d */
    "2eab0005",  /* orr z14.s, z14.s, #0xfffff81f */
    "50d24005",  /* eor z16.s, z16.s, #0x1ffffc0 */
    "91368005",  /* and z17.b, z17.b, #0x7c */
    "65bb0005",  /* orr z5.s, z5.s, #0xfffffe1f */
    "63194305",  /* eor z3.d, z3.d, #0x1ffe0000000 */
    "c3458005",  /* and z3.h, z3.h, #0xff7f */
    "bcc67004",  /* dech z28.h, #21 */
    "f0ef01a4",  /* ldnt1b {z16.b}, p3/z, [sp, #1, mul vl] */
    "c018ff25",  /* whilehs p0.d, x6, xzr */
    "939f0305",  /* orr z19.d, z19.d, #0xffffffffffffe3ff */
    "3b050205",  /* orr z27.d, z27.d, #0x3ffffffffff */
    "831e8005",  /* and z3.b, z3.b, #0xe3 */
    "61abc205",  /* mov z1.d, #0xfffff8000000007f */
    "ba3b4205",  /* eor z26.d, z26.d, #0xfe000000007fffff */
    "2c6a4005",  /* eor z12.s, z12.s, #0xfff8001f */
    "31966a05",  /* clasta h17, p5, h17, z17.h */
    "bc93c305",  /* mov z28.d, #0xfffffffc000 */
    "1ff3ec04",  /* sqincd xzr, wzr, #24, mul #13 */
    "7457c305",  /* mov z20.d, #0xffffffffffc3ffff */
    "f7ff90e5",  /* stnt1d {z23.d}, p7, [sp] */
    "570f0005",  /* orr z23.b, z23.b, #0xbb */
    "86ce0205",  /* orr z6.d, z6.d, #0xffffff800fffffff */
    "e65f66c4",  /* prfw #6, p7, [sp, z6.d, sxtw #2] */
    "1a014005",  /* eor z26.s, z26.s, #0x1ff */
    "4f0f4005",  /* eor z15.b, z15.b, #0xbb */
    "8d468205",  /* and z13.d, z13.d, #0xff001fffffffffff */
    "ce5a4005",  /* eor z14.s, z14.s, #0xffe00fff */
    "4af34005",  /* eor z10.s, z10.s, #0x1ffffffc */
    "e5fec305",  /* mov z5.d, #0x1fffffffffffffe */
    "fcf7fb04",  /* uqincd x28, all, mul #12 */
    "b854c005",  /* mov z24.h, #0xfc0 */
    "c731c005",  /* mov z7.s, #0xfc0001ff */
    "ac9a4005",  /* eor z12.s, z12.s, #0xffffe007 */
    "8f4c3045",  /* uqxtnt z15.h, z4.s */
    "883e4005",  /* eor z8.b, z8.b, #0x3e */
    "20fd0305",  /* orr z0.d, z0.d, #0x7fffffffffe */
    "9a5c8305",  /* and z26.d, z26.d, #0x3ffffffffe00000 */
    "e20f7b84",  /* prfb pldl2keep, p3, [sp, z27.s, sxtw] */
    "f77787e5",  /* stnt1d {z23.d}, p5, [sp, x7, lsl #3] */
    "387f4205",  /* eor z24.d, z24.d, #0xfffe07ffffffffff */
    "2b354005",  /* eor z11.h, z11.h, #0xfc0f */
    "95644205",  /* eor z21.d, z21.d, #0xfff0000001ffffff */
    "53107f25",  /* whilegt p3.h, x2, xzr */
    "7fab8005",  /* and z31.s, z31.s, #0xfffff87f */
    "e733fc25",  /* whilewr p7.d, xzr, x28 */
    "460a4005",  /* eor z6.s, z6.s, #0x8003ffff */
    "0f88ed25",  /* decp x15, p0.d */
    "934d4005",  /* eor z19.h, z19.h, #0xff8f */
    "e6132c25",  /* whilege p6.b, xzr, x12 */
    "0a3ab005",  /* sunpklo z10.s, z16.h */
    "7a738305",  /* and z26.d, z26.d, #0x3ffffffc0000 */
    "fd176d25",  /* whilele p13.h, xzr, x13 */
    "64520005",  /* orr z4.s, z4.s, #0xffc003ff */
    "11cc8205",  /* and z17.d, z17.d, #0xffffff80000000ff */
    "d082c205",  /* mov z16.d, #0xffff00000000007f */
    "8f8cea25",  /* sqdecp x15, p4.d */
    "264fc305",  /* mov z6.d, #0xffffffffff81ffff */
    "fba7e9a5",  /* ld1d {z27.d}, p1/z, [sp, #-7, mul vl] */
    "34dfc305",  /* mov z20.d, #0x7fffffffffffffe0 */
    "e8673cc4",  /* prfd pstl1keep, p1, [sp, z28.d, uxtw #3] */
    "665ec205",  /* mov z6.d, #0xffe001ffffffffff */
    "0ab10005",  /* orr z10.s, z10.s, #0x7fc00 */
    "452dc005",  /* mov z5.h, #0xf83f */
    "e11b3c25",  /* whilehs p1.b, xzr, x28 */
    "35fbc005",  /* mov z21.s, #0x7fffffe */
    "f23704a5",  /* ld1rqw {z18.s}, p5/z, [sp, #64] */
    "fd1a4005",  /* eor z29.s, z29.s, #0xe01fffff */
    "ee3c4205",  /* eor z14.d, z14.d, #0xfe000001ffffffff */
    "befe2a04",  /* uqdecb w30, #21, mul #11 */
    "f2073425",  /* whilele p2.b, wzr, w20 */
    "667a8305",  /* and z6.d, z6.d, #0x1ffffe0000 */
    "28fa0305",  /* orr z8.d, z8.d, #0x7fffe */
    "91bdc205",  /* mov z17.d, #0xfffffe00003fffff */
    "60004005",  /* eor z0.s, z0.s, #0xf */
    "848ec205",  /* mov z4.d, #0xffff800fffffffff */
    "8ac3c005",  /* mov z10.s, #0xffffff1f */
    "208c6825",  /* sqincp x0, p1.h */
    "e8f79de5",  /* stnt1d {z8.d}, p5, [sp, #-3, mul vl] */
    "c1ef8205",  /* and z1.d, z1.d, #0xfffffffbffffffff */
    "f033a525",  /* whilerw p0.s, xzr, x5 */
    "047b0005",  /* orr z4.s, z4.s, #0xfffe03ff */
    "088dc205",  /* mov z8.d, #0xffff800000ffffff */
    "bc8f4305",  /* eor z28.d, z28.d, #0xffffffffffff9fff */
    "5a7f8305",  /* and z26.d, z26.d, #0xfffffffffffe0fff */
    "a91e3f25",  /* whilelo p9.b, x21, xzr */
    "f0fa0305",  /* orr z16.d, z16.d, #0x1fffffe */
    "41e31825",  /* ptrue p1.b, #26 */
    "e0db0385",  /* prfw pldl1keep, p6, [sp, x3, lsl #2] */
    "703cc205",  /* mov z16.d, #0xfe0000001fffffff */
    "7f2f0205",  /* orr z31.d, z31.d, #0xf87fffffffffffff */
    "ef294005",  /* eor z15.s, z15.s, #0xf80007ff */
    "dd16c005",  /* dupm z29.b, #0xdf */
    "bccb4005",  /* eor z28.s, z28.s, #0xffffff9f */
    "310d8005",  /* and z17.h, z17.h, #0x81ff */
    "2c530305",  /* orr z12.d, z12.d, #0xffffffc00000 */
    "75ba4005",  /* eor z21.s, z21.s, #0x1ffffe00 */
    "cb43c205",  /* mov z11.d, #0xff000000007fffff */
    "ec77c685",  /* prfd pstl3keep, p5, [sp, #6, mul vl] */
    "2dbf4305",  /* eor z13.d, z13.d, #0xfffffffffffffe07 */
    "09db4305",  /* eor z9.d, z9.d, #0x3fffffe0 */
    "c9570305",  /* orr z9.d, z9.d, #0xffffffffffdfffff */
    "021bc305",  /* mov z2.d, #0x3fffffe0000000 */
    "ef1f7325",  /* whilelo p15.h, xzr, x19 */
    "f68e8305",  /* and z22.d, z22.d, #0xffffffffffff807f */
    "d3f9c005",  /* mov z19.s, #0xfffe */
    "fbab90a4",  /* ldnf1sw {z27.d}, p2/z, [sp] */
    "60b3c205",  /* mov z0.d, #0xfffffc000000003f */
    "11f30305",  /* orr z17.d, z17.d, #0x7fffffc */
    "89570205",  /* orr z9.d, z9.d, #0xffc7ffffffffffff */
    "1f04c205",  /* mov z31.d, #0x1ffffffff */
    "c3a1c005",  /* mov z3.s, #0x7fff000 */
    "4d50c085",  /* prfw pstl3strm, p4, [x2] */
    "90c30005",  /* orr z16.s, z16.s, #0xffffff1f */
    "c04d0205",  /* orr z0.d, z0.d, #0xff80003fffffffff */
    "62274305",  /* eor z2.d, z2.d, #0xfffffffff0ffffff */
    "31430005",  /* orr z17.s, z17.s, #0xff03ffff */
    "86060005",  /* orr z6.b, z6.b, #0x1f */
    "e0573384",  /* prfw pldl1keep, p5, [sp, z19.s, uxtw #2] */
    "e5050305",  /* orr z5.d, z5.d, #0xffffffff0000ffff */
    "f1340005",  /* orr z17.h, z17.h, #0xfc03 */
    "e527d485",  /* prfh pldl3strm, p1, [sp, #20, mul vl] */
    "dff3b604",  /* sqincw xzr, mul3, mul #7 */
    "4f6f8305",  /* and z15.d, z15.d, #0xfffffffffff83fff */
    "dfb26005",  /* lasta wzr, p4, z22.h */
    "08f78305",  /* and z8.d, z8.d, #0x7fffffffffffffc */
    "e52fa0a4",  /* ld1roh {z5.h}, p3/z, [sp] */
    "ec176225",  /* whilelt p12.h, xzr, x2 */
    "c9882925",  /* uqincp w9, p6.b */
    "74a20005",  /* orr z20.s, z20.s, #0xfffff000 */
    "dc4dc005",  /* dupm z28.h, #0xffbf */
    "6f6dcb25",  /* movs p15.b, p11.b */
    "e0168305",  /* and z0.d, z0.d, #0xffffffffc03fffff */
    "ee2373c4",  /* prfh #14, p0, [sp, z19.d, sxtw #1] */
    "914cff04",  /* index z17.d, x4, xzr */
    "0b202205",  /* mov z11.h, h0 */
    "a2090205",  /* orr z2.d, z2.d, #0x8000000000001fff */
    "c43d8005",  /* and z4.h, z4.h, #0xfeff */
    "99fa0305",  /* orr z25.d, z25.d, #0x3ffffe */
    "efebd0e4",  /* st3h {z15.h-z17.h}, p2, [sp] */
    "450fc305",  /* mov z5.d, #0xffffffff83ffffff */
    "ebf74084",  /* ld1rb {z11.d}, p5/z, [sp] */
    "e0dc8305",  /* and z0.d, z0.d, #0x1fffffffffe0 */
    "8347c305",  /* mov z3.d, #0xffffffffff1fffff */
    "be4dc305",  /* mov z30.d, #0xffffffffff80001f */
    "9bf30305",  /* orr z27.d, z27.d, #0x7ffffffc */
    "67c44205",  /* eor z7.d, z7.d, #0xffffff0000000fff */
    "ef273ac4",  /* prfh #15, p1, [sp, z26.d, uxtw #1] */
    "d45dc005",  /* dupm z20.h, #0xffef */
    "037e8205",  /* and z3.d, z3.d, #0xfffe0003ffffffff */
    "b48c8305",  /* and z20.d, z20.d, #0x1fffffffff8000 */
    "029a8005",  /* and z2.s, z2.s, #0x3fffe000 */
    "a004c305",  /* mov z0.d, #0xffffffff0000003f */
    "72fcc305",  /* mov z18.d, #0x1ffffffffe */
    "95ec8205",  /* and z21.d, z21.d, #0xfffffff8000000ff */
    "e3136584",  /* prfb pldl2strm, p4, [sp, z5.s, sxtw] */
    "b5974305",  /* eor z21.d, z21.d, #0xffffffffffffcfff */
    "eefb7b04",  /* sqdech x14, all, mul #12 */
    "0b06bf25",  /* whilelt p11.s, w16, wzr */
    "ea1ec085",  /* prfb pstl2keep, p7, [x23] */
    "6095c205",  /* mov z0.d, #0xffffc00003ffffff */
    "989f8205",  /* and z24.d, z24.d, #0xffffe3ffffffffff */
    "e7cbed04",  /* sqdecd z7.d, all, mul #14 */
    "3ff3f004",  /* sqincd xzr, #25 */
    "f3ed0205",  /* orr z19.d, z19.d, #0xfffffff80007ffff */
    "3b8d8205",  /* and z27.d, z27.d, #0xffff800001ffffff */
    "53f2c005",  /* mov z19.s, #0x1ffffc */
    "451f0005",  /* orr z5.b, z5.b, #0xee */
    "d9007f25",  /* whilegt p9.h, w6, wzr */
    "eca780a5",  /* ld1sb {z12.d}, p1/z, [sp] */
    "e55ff985",  /* prfw pldl3strm, p7, [sp, #-7, mul vl] */
    "6d6ac085",  /* prfd pstl3strm, p2, [x19] */
    "b55dc305",  /* mov z21.d, #0xffffffffffe00007 */
    "ef07c385",  /* prfb #15, p1, [sp, #3, mul vl] */
    "dc260205",  /* orr z28.d, z28.d, #0xf007ffffffffffff */
    "67330305",  /* orr z7.d, z7.d, #0x3ffffffc000000 */
    "c4360005",  /* orr z4.b, z4.b, #0xfd */
    "0009c085",  /* prfb pldl1keep, p2, [x8] */
    "c6b28205",  /* and z6.d, z6.d, #0xfffffc0000000001 */
    "ed1bfd85",  /* prfb pstl3strm, p6, [sp, #-3, mul vl] */
    "a35ec305",  /* mov z3.d, #0xffffffffffe007ff */
    "a5bbc005",  /* mov z5.s, #0xfffffe7f */
    "188c4305",  /* eor z24.d, z24.d, #0xffffffff8000 */
    "14ae8205",  /* and z20.d, z20.d, #0xfffff8000fffffff */
    "eeef40e5",  /* st1w {z14.s}, p3, [sp] */
    "cb664305",  /* eor z11.d, z11.d, #0xfffffffffff007ff */
    "ffb50205",  /* orr z31.d, z31.d, #0xfffffc0003ffffff */
    "a855c305",  /* mov z8.d, #0xffffffffffc0000f */
    "d2f4c205",  /* mov z18.d, #0xfffffffc000001ff */
    "579f4305",  /* eor z23.d, z23.d, #0xffffffffffffe0ff */
    "e1273b84",  /* prfh pldl1strm, p1, [sp, z27.s, uxtw #1] */
    "e7df0384",  /* prfb #7, p7, [sp, x3] */
    "9a1dc305",  /* mov z26.d, #0xffffffffe00003ff */
    "30816825",  /* sqincp z16.h, p9.h */
    "0c2c4205",  /* eor z12.d, z12.d, #0xf80000000fffffff */
    "edab7dc4",  /* prfh pstl3strm, p2, [sp, z29.d, lsl #1] */
    "f8d38305",  /* and z24.d, z24.d, #0x3fffffffc0 */
    "45d3c205",  /* mov z5.d, #0xffffffc000000001 */
    "04f24005",  /* eor z4.s, z4.s, #0x7fffc */
    "aa7b4205",  /* eor z10.d, z10.d, #0xfffe000000007fff */
    "bb65c005",  /* dupm z27.h, #0xfff3 */
    "2514c205",  /* mov z5.d, #0xc0000000ffffffff */
    "76ee8305",  /* and z22.d, z22.d, #0x7ffffffffffff8 */
    "25ef4305",  /* eor z5.d, z5.d, #0x1ffffffffffffff8 */
    "e61b2e84",  /* prfb #6, p6, [sp, z14.s, uxtw] */
    "fe120005",  /* orr z30.s, z30.s, #0xc03fffff */
    "e80b7984",  /* prfb pstl1keep, p2, [sp, z25.s, sxtw] */
    "35268305",  /* and z21.d, z21.d, #0xfffffffff0003fff */
    "8feb4005",  /* eor z15.s, z15.s, #0xfffffff8 */
    "0d98da65",  /* fmul z13.d, p6/m, z13.d, #0.5 */
    "ad9b4205",  /* eor z13.d, z13.d, #0xffffe000000007ff */
    "972e0305",  /* orr z23.d, z23.d, #0xfffffffff800ffff */
    "f5024205",  /* eor z21.d, z21.d, #0xffffff */
    "33750305",  /* orr z19.d, z19.d, #0xffffffffffc0000 */
    "68174305",  /* eor z8.d, z8.d, #0xffffffffc3ffffff */
    "5bd68205",  /* and z27.d, z27.d, #0xffffffc001ffffff */
    "ab6c8205",  /* and z11.d, z11.d, #0xfff8000001ffffff */
    "41a4c205",  /* mov z1.d, #0xfffff00000007fff */
    "13d28005",  /* and z19.s, z19.s, #0x7fffc0 */
    "438e4305",  /* eor z3.d, z3.d, #0xffffffffffff8003 */
    "5fe17c04",  /* inch xzr, vl32, mul #13 */
    "a32a4005",  /* eor z3.s, z3.s, #0xf801ffff */
    "04568305",  /* and z4.d, z4.d, #0xffffffffffc0007f */
    "271a4005",  /* eor z7.s, z7.s, #0xe0007fff */
    "b5344205",  /* eor z21.d, z21.d, #0xfc000000ffffffff */
    "f3cfe304",  /* uqdecd z19.d, all, mul #4 */
    "46f7e004",  /* uqincd w6, #26 */
    "03ebc205",  /* mov z3.d, #0xffffff800000000 */
    "77834205",  /* eor z23.d, z23.d, #0xffff000000000fff */
    "c2a34205",  /* eor z2.d, z2.d, #0xfffff000000007ff */
    "67650205",  /* orr z7.d, z7.d, #0xfff00000ffffffff */
    "7fa3f105",  /* clastb xzr, p0, xzr, z27.d */
    "5cb5c305",  /* mov z28.d, #0x1ffffffffffc00 */
    "7f4f0305",  /* orr z31.d, z31.d, #0xffffffffff87ffff */
    "a6a60305",  /* orr z6.d, z6.d, #0xfffffffffffff003 */
    "6e66c205",  /* mov z14.d, #0xfff000ffffffffff */
    "4dae4205",  /* eor z13.d, z13.d, #0xfffff8003fffffff */
    "e62300a5",  /* ld1rqw {z6.s}, p0/z, [sp] */
    "ed03fb25",  /* whilege p13.d, wzr, w27 */
    "eebf7dc4",  /* prfh #14, p7, [sp, z29.d, lsl #1] */
    "bf634005",  /* eor z31.s, z31.s, #0xfff3ffff */
    "ec7b7dc4",  /* prfd pstl3keep, p6, [sp, z29.d, sxtw #3] */
    "c92ec205",  /* mov z9.d, #0xf803ffffffffffff */
    "e26b2a84",  /* prfd pldl2keep, p2, [sp, z10.s, uxtw #3] */
    "3c124005",  /* eor z28.s, z28.s, #0xc000ffff */
    "f0ebc085",  /* ld1rd {z16.d}, p2/z, [sp] */
    "cb0dc005",  /* mov z11.h, #0xbfff */
    "18938205",  /* and z24.d, z24.d, #0xffffc0000000007f */
    "dffa6004",  /* sqdech xzr, wzr, #22 */
    "a3058205",  /* and z3.d, z3.d, #0x3fffffffffff */
    "199d0205",  /* orr z25.d, z25.d, #0xffffe000003fffff */
    "dc8f8305",  /* and z28.d, z28.d, #0xffffffffffffbfff */
    "f6fbfa04",  /* sqdecd x22, all, mul #11 */
    "246cc205",  /* mov z4.d, #0xfff80000001fffff */
    "2de4c305",  /* mov z13.d, #0x3ffffffff0 */
    "35e58305",  /* and z21.d, z21.d, #0x3ffffffffff0 */
    "29dd4205",  /* eor z9.d, z9.d, #0xffffffe000007fff */
    "b4554305",  /* eor z20.d, z20.d, #0xffffffffffc0000f */
    "8ee98085",  /* prfd #14, p2, [z12.s] */
    "b53ac005",  /* mov z21.s, #0xfe007fff */
    "bf044205",  /* eor z31.d, z31.d, #0x3fffffffff */
    "66968305",  /* and z6.d, z6.d, #0xffffffffffffc003 */
    "46440205",  /* orr z6.d, z6.d, #0xff00000007ffffff */
    "070dc005",  /* mov z7.h, #0x80ff */
    "4ef3c005",  /* mov z14.s, #0x1ffffffc */
    "a1f26004",  /* sqinch x1, w1, #21 */
    "882d4205",  /* eor z8.d, z8.d, #0xf80000ffffffffff */
    "c7fcc305",  /* mov z7.d, #0xfffffffffe */
    "e107c985",  /* prfb pldl1strm, p1, [sp, #9, mul vl] */
    "64f1c005",  /* mov z4.s, #0x3ffc */
    "8c170205",  /* orr z12.d, z12.d, #0xc7ffffffffffffff */
    "6af9c005",  /* mov z10.s, #0x1ffe */
    "7fbe3005",  /* clasta wzr, p7, wzr, z19.b */
    "2d96c205",  /* mov z13.d, #0xffffc000ffffffff */
    "cd15c005",  /* mov z13.h, #0xdfff */
    "bfa4b105",  /* clastb wzr, p1, wzr, z5.s */
    "92748205",  /* and z18.d, z18.d, #0xfffc0000007fffff */
    "30164205",  /* eor z16.d, z16.d, #0xc000ffffffffffff */
    "ed6beb85",  /* prfd pstl3strm, p2, [sp, #-21, mul vl] */
    "bff03404",  /* sqincb xzr, vl5, mul #5 */
    "c40ec005",  /* dupm z4.b, #0xbf */
    "42874205",  /* eor z2.d, z2.d, #0xffff07ffffffffff */
    "093e0205",  /* orr z9.d, z9.d, #0xfe0003ffffffffff */
    "969d0205",  /* orr z22.d, z22.d, #0xffffe00003ffffff */
    "b8b38005",  /* and z24.s, z24.s, #0xfffffcff */
    "adb1c205",  /* mov z13.d, #0xfffc0000000000 */
    "c7cdc205",  /* mov z7.d, #0xffffff80003fffff */
    "bc350005",  /* orr z28.h, z28.h, #0xfcff */
    "044dc005",  /* dupm z4.h, #0xff80 */
    "87f78084",  /* prfh #7, p5, [z28.s] */
    "1e7e4205",  /* eor z30.d, z30.d, #0xfffe0003ffffffff */
    "273a0205",  /* orr z7.d, z7.d, #0xfe000000000007ff */
    "8dae8205",  /* and z13.d, z13.d, #0xfffff800ffffffff */
    "a0faf004",  /* sqdecd x0, #21 */
    "acc38005",  /* and z12.s, z12.s, #0xffffff3f */
    "b42c8205",  /* and z20.d, z20.d, #0xf8000001ffffffff */
    "7862c005",  /* mov z24.s, #0xfff000ff */
    "223a4005",  /* eor z2.s, z2.s, #0xfe0007ff */
    "c68d8205",  /* and z6.d, z6.d, #0xffff80003fffffff */
    "3fff3104",  /* uqdecb xzr, #25, mul #2 */
    "fd054305",  /* eor z29.d, z29.d, #0xffffffff0000ffff */
    "2ea50305",  /* orr z14.d, z14.d, #0x3ffffffffff000 */
    "4f7c4005",  /* eor z15.h, z15.h, #0xe */
    "befac005",  /* mov z30.s, #0x7ffffe */
    "8af000c4",  /* prfb pstl2keep, p4, [z4.d] */
    "af914005",  /* eor z15.s, z15.s, #0xfffc000 */
    "216e0205",  /* orr z1.d, z1.d, #0xfff8001fffffffff */
    "eb732dc4",  /* prfd pstl2strm, p4, [sp, z13.d, uxtw #3] */
    "a74dc205",  /* mov z7.d, #0xff80001fffffffff */
    "f5e7c084",  /* ld1rh {z21.d}, p1/z, [sp] */
    "669e8305",  /* and z6.d, z6.d, #0xffffffffffffe001 */
    "bdd34005",  /* eor z29.s, z29.s, #0xffffffcf */
    "f50dc305",  /* mov z21.d, #0xffffffff80007fff */
    "4e574205",  /* eor z14.d, z14.d, #0xffc1ffffffffffff */
    "3bf40205",  /* orr z27.d, z27.d, #0xfffffffc0000000f */
    "d78da925",  /* uqincp x23, p14.s */
    "5a5ac005",  /* mov z26.s, #0xffe000ff */
    "a43f8205",  /* and z4.d, z4.d, #0xfe7fffffffffffff */
    "56334005",  /* eor z22.s, z22.s, #0xfc1fffff */
    "4a5d8005",  /* and z10.h, z10.h, #0xffe0 */
    "cf134205",  /* eor z15.d, z15.d, #0xc00000001fffffff */
    "8d88aa25",  /* sqdecp x13, p4.s, w13 */
    "dc7d0005",  /* orr z28.h, z28.h, #0xfffe */
    "f8528005",  /* and z24.s, z24.s, #0xffc03fff */
    "e45f79c4",  /* prfw pldl3keep, p7, [sp, z25.d, sxtw #2] */
    "784a8005",  /* and z24.s, z24.s, #0xff8007ff */
    "125d0305",  /* orr z18.d, z18.d, #0x3fffffffffe00000 */
    "c2154005",  /* eor z2.h, z2.h, #0xdfff */
    "d7118005",  /* and z23.s, z23.s, #0xc0001fff */
    "36150005",  /* orr z22.h, z22.h, #0xc0ff */
    "18530305",  /* orr z24.d, z24.d, #0x7fffffc00000 */
    "7c150305",  /* orr z28.d, z28.d, #0xffffffffc00003ff */
    "7c428005",  /* and z28.s, z28.s, #0xff000fff */
    "71b78305",  /* and z17.d, z17.d, #0xfffffffffffffc3f */
    "5d9ac305",  /* mov z29.d, #0xffffe000 */
    "02bc0205",  /* orr z2.d, z2.d, #0xfffffe00000003ff */
    "e0e700a4",  /* ldnt1b {z0.b}, p1/z, [sp] */
    "4263c005",  /* mov z2.s, #0xfff07fff */
    "6c060005",  /* orr z12.b, z12.b, #0xf */
    "ccbb8305",  /* and z12.d, z12.d, #0xfffffffe00 */
    "2a79c005",  /* mov z10.s, #0x7fe0000 */
    "32ffc205",  /* mov z18.d, #0xfffffffe07ffffff */
    "3ff02204",  /* sqincb xzr, wzr, vl1, mul #3 */
    "2f3fc205",  /* mov z15.d, #0xfe07ffffffffffff */
    "2314c005",  /* dupm z3.h, #0xc000 */
    "aced00c5",  /* prfw pstl3keep, p3, [z13.d] */
    "ca4d8305",  /* and z10.d, z10.d, #0xffffffffff80003f */
    "cb6bc205",  /* mov z11.d, #0xfff800000003ffff */
    "e085c305",  /* mov z0.d, #0xffffffffffff0000 */
    "38758005",  /* and z24.h, z24.h, #0xffc */
    "c9708305",  /* and z9.d, z9.d, #0x1fc0000 */
    "0d354305",  /* eor z13.d, z13.d, #0xfffffffffc000007 */
    "f7e10005",  /* orr z23.s, z23.s, #0xffff0 */
    "edcf1184",  /* prfb pstl3strm, p3, [sp, x17] */
    "24920005",  /* orr z4.s, z4.s, #0xffffc000 */
    "d6590005",  /* orr z22.s, z22.s, #0xffe0000f */
    "b1f73004",  /* uqincb x17, mul4 */
    "ffe5ff04",  /* decd xzr, #15, mul #16 */
    "73f58305",  /* and z19.d, z19.d, #0x3ffffffffffc */
    "6fd78205",  /* and z15.d, z15.d, #0xffffffc3ffffffff */
    "199d8205",  /* and z25.d, z25.d, #0xffffe000003fffff */
    "9bfec205",  /* mov z27.d, #0xfffffffe003fffff */
    "d936c205",  /* mov z25.d, #0xfc01ffffffffffff */
    "bfe1e204",  /* cntd xzr, vl256, mul #3 */
    "42f08085",  /* prfd pldl2keep, p4, [z2.s] */
    "f5133225",  /* whilegt p5.b, xzr, x18 */
    "2c748005",  /* and z12.h, z12.h, #0xc */
    "212dc085",  /* prfh pldl1strm, p3, [x9] */
    "b20c8205",  /* and z18.d, z18.d, #0x8000001fffffffff */
    "0cbd4305",  /* eor z12.d, z12.d, #0x3fffffffffe00 */
    "9a2d8005",  /* and z26.h, z26.h, #0xf8ff */
    "791e8205",  /* and z25.d, z25.d, #0xe001ffffffffffff */
    "bff5f904",  /* uqincd xzr, vl256, mul #10 */
    "2669c085",  /* prfd #6, p2, [x9] */
    "aefb0084",  /* prfb #14, p6, [z29.s] */
    "a76a0005",  /* orr z7.s, z7.s, #0xfff801ff */
    "209c4205",  /* eor z0.d, z0.d, #0xffffe00000007fff */
    "5ff9ed04",  /* sqdecd xzr, wzr, vl32, mul #14 */
    "b07f4305",  /* eor z16.d, z16.d, #0xfffffffffffe7fff */
    "10178205",  /* and z16.d, z16.d, #0xc07fffffffffffff */
    "4affb004",  /* uqdecw x10, #26 */
    "d11b0205",  /* orr z17.d, z17.d, #0xe00000000fffffff */
    "0c9ec205",  /* mov z12.d, #0xffffe0003fffffff */
    "e9d30285",  /* prfw pstl1strm, p4, [sp, x2, lsl #2] */
    "b59dc205",  /* mov z21.d, #0xffffe00007ffffff */
    "cfffc205",  /* mov z15.d, #0xfffffffeffffffff */
    "ece770c4",  /* prfd pstl3keep, p1, [sp, z16.d, lsl #3] */
    "e0ffff04",  /* uqdecd x0, all, mul #16 */
    "d1f9e004",  /* sqdecd x17, w17, #14 */
    "7a220005",  /* orr z26.s, z26.s, #0xf000ffff */
    "9503c205",  /* mov z21.d, #0x1fffffff */
    "3a4f0205",  /* orr z26.d, z26.d, #0xff81ffffffffffff */
    "e1433f84",  /* prfw pldl1strm, p0, [sp, z31.s, uxtw #2] */
    "254f8205",  /* and z5.d, z5.d, #0xff81ffffffffffff */
    "de7a4005",  /* eor z30.s, z30.s, #0xfffe00ff */
    "7e32c005",  /* mov z30.s, #0xfc003fff */
    "07f83104",  /* sqdecb x7, pow2, mul #2 */
    "ce1b4005",  /* eor z14.s, z14.s, #0xefffffff */
    "ed2bea85",  /* prfh pstl3strm, p2, [sp, #-22, mul vl] */
    "66048205",  /* and z6.d, z6.d, #0xfffffffff */
    "b9f98005",  /* and z25.s, z25.s, #0x7ffe */
    "584ac205",  /* mov z24.d, #0xff800000000003ff */
    "70678305",  /* and z16.d, z16.d, #0xfffffffffff0ffff */
    "ad49ff04",  /* index z13.d, #13, xzr */
    "8e2fc305",  /* mov z14.d, #0xfffffffff8ffffff */
    "a5c30305",  /* orr z5.d, z5.d, #0x3fffffff00 */
    "8171c205",  /* mov z1.d, #0x7ffc000000000000 */
    "d4350005",  /* orr z20.h, z20.h, #0xfdff */
    "c6540305",  /* orr z6.d, z6.d, #0x1fffffffffc00000 */
    "e6ba8305",  /* and z6.d, z6.d, #0x1fffffe00 */
    "4cb34205",  /* eor z12.d, z12.d, #0xfffffc000000001f */
    "73750305",  /* orr z19.d, z19.d, #0x3ffffffffffc0000 */
    "a0134305",  /* eor z0.d, z0.d, #0xfffffffc0000000 */
    "603d4305",  /* eor z0.d, z0.d, #0xfffffffffe00001f */
    "67f3f004",  /* sqincd x7, #27 */
    "3fe13f04",  /* incb xzr, vl16, mul #16 */
    "34f1c005",  /* mov z20.s, #0xffc */
    "4d264205",  /* eor z13.d, z13.d, #0xf0007fffffffffff */
    "83f7c205",  /* mov z3.d, #0xfffffffc7fffffff */
    "72720205",  /* orr z18.d, z18.d, #0xfffc00000000003f */
    "e84ec085",  /* prfw pstl1keep, p3, [x23] */
    "0065c205",  /* mov z0.d, #0xfff000001fffffff */
    "581d8305",  /* and z24.d, z24.d, #0xffffffffe00000ff */
    "af574305",  /* eor z15.d, z15.d, #0xffffffffffcfffff */
    "52768305",  /* and z18.d, z18.d, #0xfffffffffffc001f */
    "dc4ec205",  /* mov z28.d, #0xff803fffffffffff */
    "998f8305",  /* and z25.d, z25.d, #0xffffffffffff8fff */
    "55be0305",  /* orr z21.d, z21.d, #0xffffffffffffe00 */
    "d09b4005",  /* eor z16.s, z16.s, #0xffffefff */
    "6a0b0005",  /* orr z10.s, z10.s, #0x87ffffff */
    "57ae4305",  /* eor z23.d, z23.d, #0x3ffffffffffff800 */
    "63e2d925",  /* ptrues p3.d, #19 */
    "37de8205",  /* and z23.d, z23.d, #0xffffffe0007fffff */
    "0b168205",  /* and z11.d, z11.d, #0xc0007fffffffffff */
    "d9ed8305",  /* and z25.d, z25.d, #0x3fffffffffff8 */
    "94b14205",  /* eor z20.d, z20.d, #0x7ffc0000000000 */
    "5bfb8005",  /* and z27.s, z27.s, #0xffffffe */
    "13434205",  /* eor z19.d, z19.d, #0xff0000000001ffff */
    "e7c71585",  /* prfw #7, p1, [sp, x21, lsl #2] */
    "bffbfb04",  /* sqdecd xzr, mul4, mul #12 */
    "a506c305",  /* mov z5.d, #0xffffffff003fffff */
    "2efc4205",  /* eor z14.d, z14.d, #0xfffffffe00000007 */
    "81150205",  /* orr z1.d, z1.d, #0xc00007ffffffffff */
    "18bc8205",  /* and z24.d, z24.d, #0xfffffe00000003ff */
    "9123c205",  /* mov z17.d, #0xf000000001ffffff */
    "01dac005",  /* mov z1.s, #0x3fffe0 */
    "f8fda004",  /* uqdecw w24, #15 */
    "1f27c305",  /* mov z31.d, #0xfffffffff01fffff */
    "d92f0305",  /* orr z25.d, z25.d, #0xfffffffffbffffff */
    "ee0f7e84",  /* prfb #14, p3, [sp, z30.s, sxtw] */
    "6dfe0205",  /* orr z13.d, z13.d, #0xfffffffe001fffff */
    "babf8205",  /* and z26.d, z26.d, #0xfffffe7fffffffff */
    "5af54305",  /* eor z26.d, z26.d, #0x1ffffffffffc */
    "a27b0005",  /* orr z2.s, z2.s, #0xfffe7fff */
    "407a0005",  /* orr z0.s, z0.s, #0xfffe000f */
    "27450205",  /* orr z7.d, z7.d, #0xff000003ffffffff */
    "5bd64205",  /* eor z27.d, z27.d, #0xffffffc001ffffff */
    "503cc305",  /* mov z16.d, #0xffffffffe000000 */
    "687a4205",  /* eor z8.d, z8.d, #0xfffe00000000001f */
    "06478305",  /* and z6.d, z6.d, #0xffffffffff01ffff */
    "827fc305",  /* mov z2.d, #0xfffffffffffe3fff */
    "778f4305",  /* eor z23.d, z23.d, #0xffffffffffff87ff */
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
