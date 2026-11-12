/* flags: --c-std=gnu17 */
/* native-only */
/* The AArch64 assembler against a byte table: base, SIMD, FP, LSE, CRC32, RCPC, pointer authentication, named system registers (the generated table) and pstate forms; the bytes were checked against the system assembler in development. */
#include "host.h"

#ifdef __AXIOM_ASM_AARCH64__
__asm__(".section .rodata.axiom_asm_table, \"a\"\n"
        ".globl axiom_asm_table\n"
        "axiom_asm_table:\n"
        "add x4, x18, x27\n"
        "add x25, x24, #516\n"
        "sub x8, x3, x15\n"
        "sub x24, x14, #3868\n"
        "and x20, x12, x25\n"
        "orr x0, x28, x26\n"
        "eor x22, x14, x8\n"
        "adds x18, x3, x28\n"
        "adds x10, x0, #182\n"
        "subs x0, x20, x17\n"
        "subs x0, x28, #3122\n"
        "mul x21, x6, x13\n"
        "madd x23, x0, x16, x7\n"
        "msub x24, x14, x15, x17\n"
        "sdiv x7, x11, x7\n"
        "udiv x21, x7, x24\n"
        "smull x14, w9, w0\n"
        "umulh x13, x26, x17\n"
        "smulh x20, x3, x5\n"
        "lsl x20, x23, #37\n"
        "lsr x3, x23, #42\n"
        "asr x28, x23, #54\n"
        "ror x16, x26, #24\n"
        "ubfx x9, x9, #18, #16\n"
        "sbfx x27, x16, #12, #19\n"
        "bfi x27, x1, #15, #8\n"
        "ubfiz x23, x25, #12, #14\n"
        "cmp x21, x5\n"
        "cmn x11, x17\n"
        "tst x28, x22\n"
        "csel x24, x21, x23, lt\n"
        "csinc x2, x14, x21, hi\n"
        "csinv x3, x24, x5, hi\n"
        "csneg x26, x12, x11, ge\n"
        "cset x23, eq\n"
        "csetm x15, eq\n"
        "mov x9, #51589\n"
        "movk x20, #22328, lsl #16\n"
        "movz x5, #29745\n"
        "ldr x0, [x24, #8]\n"
        "ldr x17, [sp, x27, lsl #3]\n"
        "ldr x17, [x7], #-8\n"
        "str x16, [x11, #4088]\n"
        "str x11, [x14, x8, lsl #3]\n"
        "str x21, [x17], #16\n"
        "ldp x23, x0, [x12, #-16]\n"
        "stp x16, x25, [x4, #-16]!\n"
        "ldrb w16, [x24]\n"
        "ldrh w17, [x6]\n"
        "ldrsw x13, [x1]\n"
        "strb w15, [x27]\n"
        "ldadd x11, x18, [x17]\n"
        "ldset x6, x16, [x13]\n"
        "ldclr x15, x26, [x11]\n"
        "ldeor x13, x11, [x0]\n"
        "swp x17, x17, [x19]\n"
        "ldaddal x25, x19, [x10]\n"
        "swpal x14, x19, [x0]\n"
        "cas x25, x7, [x20]\n"
        "casal x5, x17, [x18]\n"
        "casa x5, x27, [x2]\n"
        "ldxr x25, [x17]\n"
        "stxr w25, x27, [x26]\n"
        "ldar x8, [x1]\n"
        "stlr x26, [x21]\n"
        "ldapr x2, [x2]\n"
        "fadd d1, d28, d0\n"
        "fadd s17, s15, s17\n"
        "fsub d7, d11, d22\n"
        "fsub s18, s4, s10\n"
        "fmul d10, d16, d10\n"
        "fmul s17, s18, s29\n"
        "fdiv d20, d31, d30\n"
        "fdiv s7, s1, s19\n"
        "fmax d24, d21, d26\n"
        "fmax s12, s16, s6\n"
        "fmin d16, d13, d27\n"
        "fmin s1, s14, s1\n"
        "fcvt d0, s1\n"
        "scvtf d0, x1\n"
        "fcmp d0, d1\n"
        "fmov d0, #1.5\n"
        "add v0.8h, v1.8h, v2.8h\n"
        "sub v0.16b, v1.16b, v2.16b\n"
        "mul v0.8b, v1.8b, v2.8b\n"
        "orr v0.16b, v1.16b, v2.16b\n"
        "cmeq v0.4s, v1.4s, v2.4s\n"
        "umax v0.2s, v1.2s, v2.2s\n"
        "smin v0.4s, v1.4s, v2.4s\n"
        "fadd v0.2d, v1.2d, v2.2d\n"
        "fmul v0.2s, v1.2s, v2.2s\n"
        "fsub v0.2d, v1.2d, v2.2d\n"
        "dup v0.4s, w1\n"
        "ins v0.s[1], w2\n"
        "umov w0, v1.s[2]\n"
        "ld1 {v0.16b}, [x1]\n"
        "st1 {v0.4s, v1.4s}, [x2]\n"
        "b.eq .+8\n"
        "cbz x0, .+16\n"
        "tbz x1, #3, .-8\n"
        "ret\n"
        "nop\n"
        "mrs x0, tpidr_el0\n"
        "msr fpcr, x1\n"
        "mrs x2, nzcv\n"
        "dmb ish\n"
        "isb\n"
        "dsb sy\n"
        "svc #0\n"
        "brk #1\n"
        "add x27, x27, x1\n"
        "add x2, x2, #2957\n"
        "sub x26, x5, x23\n"
        "sub x25, x21, #2524\n"
        "and x8, x19, x6\n"
        "orr x13, x20, x12\n"
        "adds x27, x0, x11\n"
        "adds x14, sp, #2608\n"
        "subs x28, x28, #1347\n"
        "mul x17, x5, x7\n"
        "madd x7, x0, x5, x10\n"
        "msub x5, x4, x16, x16\n"
        "sdiv x11, x16, x21\n"
        "udiv x17, x5, x28\n"
        "smull x14, w25, w13\n"
        "umulh x23, x16, x24\n"
        "smulh x11, x25, x18\n"
        "lsl x11, x11, #57\n"
        "lsr x5, x24, #51\n"
        "asr x22, x23, #59\n"
        "ror x20, x16, #31\n"
        "ubfx x15, x8, #15, #17\n"
        "sbfx x16, x26, #11, #15\n"
        "bfi x28, x28, #14, #12\n"
        "ubfiz x18, x23, #17, #15\n"
        "cmp x15, x21\n"
        "cmn x7, x10\n"
        "tst x26, x22\n"
        "csel x26, x5, x28, hi\n"
        "csinc x8, x24, x15, lt\n"
        "csinv x9, x25, x22, hi\n"
        "csneg x17, x16, x16, ls\n"
        "cset x19, cs\n"
        "csetm x9, ne\n"
        "mov x15, #48050\n"
        "movk x21, #9879, lsl #16\n"
        "movz x25, #44755\n"
        "ldr x23, [x0, #8]\n"
        "ldr x23, [x3, x1, lsl #3]\n"
        "ldr x18, [x20], #8\n"
        "str x8, [x18, #8]\n"
        "str x21, [x28, x3, lsl #3]\n"
        "str x24, [x16], #8\n"
        "ldp x27, x8, [x7, #0]\n"
        "stp x28, x1, [x13, #-16]!\n"
        "ldrb w28, [x22]\n"
        "ldrh w24, [x1]\n"
        "ldrsw x1, [x11]\n"
        "strb w11, [x5]\n"
        "ldadd x7, x21, [x0]\n"
        "ldset x2, x3, [x2]\n"
        "ldclr x0, x1, [x23]\n"
        "ldeor x0, x11, [x8]\n"
        "swp x4, x26, [sp]\n"
        "ldaddal x5, x23, [x5]\n"
        "swpal x16, x22, [x0]\n"
        "cas x12, x18, [x1]\n"
        "casal x25, x7, [x4]\n"
        "casa x1, x0, [x11]\n"
        "ldxr x19, [x20]\n"
        "stxr w23, x23, [x3]\n"
        "ldar x9, [x10]\n"
        "stlr x15, [x0]\n"
        "ldapr x9, [x14]\n"
        "fadd d2, d16, d25\n"
        "fadd s9, s30, s14\n"
        "fsub d5, d20, d6\n"
        "fsub s1, s28, s8\n"
        "fmul d25, d31, d20\n"
        "fmul s9, s21, s16\n"
        "fdiv d16, d26, d1\n"
        "fdiv s8, s3, s16\n"
        "fmax d2, d8, d10\n"
        "fmax s10, s6, s29\n"
        "fmin d14, d2, d15\n"
        "fmin s14, s28, s4\n"
        "add v0.4h, v1.4h, v2.4h\n"
        "sub v0.8b, v1.8b, v2.8b\n"
        "mul v0.2s, v1.2s, v2.2s\n"
        "and v0.16b, v1.16b, v2.16b\n"
        "cmeq v0.2d, v1.2d, v2.2d\n"
        "fadd v0.4s, v1.4s, v2.4s\n"
        "fmul v0.2d, v1.2d, v2.2d\n"
        "fsub v0.4s, v1.4s, v2.4s\n"
        "add x7, x18, x17\n"
        "add x4, x11, #3883\n"
        "sub x20, x18, x2\n"
        "sub x19, x0, #3843\n"
        "and x8, x17, x7\n"
        "orr x17, x26, x17\n"
        "eor x7, x20, x4\n"
        "adds x23, x0, x21\n"
        "adds x24, x2, #1305\n"
        "subs x24, x18, x1\n"
        "subs x9, x24, #254\n"
        "mul x26, x27, x8\n"
        "madd x15, x19, x23, x28\n"
        "msub x12, x22, x25, x13\n"
        "sdiv x12, x23, x25\n"
        "udiv x18, x14, x4\n"
        "smull x28, w11, w3\n"
        "umulh x1, x4, x15\n"
        "smulh x6, x8, x21\n"
        "lsl x13, x24, #38\n"
        "lsr x13, x16, #49\n"
        "asr x18, x11, #52\n"
        "ror x18, x7, #43\n"
        "ubfx x21, x0, #8, #20\n"
        "sbfx x21, x22, #5, #11\n"
        "bfi x17, x28, #18, #19\n"
        "ubfiz x3, x22, #20, #7\n"
        "cmp x20, x26\n"
        "cmn x18, x8\n"
        "tst x9, x3\n"
        "csel x2, x15, x27, ls\n"
        "csinc x15, x2, x11, eq\n"
        "csinv x13, x28, x4, eq\n"
        "csneg x9, x13, x24, ge\n"
        "cset x27, eq\n"
        "csetm x1, eq\n"
        "mov x12, #43378\n"
        "movk x17, #36578, lsl #16\n"
        "movz x16, #30926\n"
        "ldr x1, [x9, #0]\n"
        "ldr x2, [x3, x19, lsl #3]\n"
        "ldr x17, [x1], #8\n"
        "str x13, [x9, #4088]\n"
        "str x8, [x4, x22, lsl #3]\n"
        "str x1, [x27], #-8\n"
        "ldp x10, x11, [x4, #16]\n"
        "stp x12, x14, [x27, #-16]!\n"
        "ldrb w16, [x12]\n"
        "ldrh w20, [x27]\n"
        "ldrsw x19, [x21]\n"
        "strb w17, [x3]\n"
        "ldadd x19, x25, [x16]\n"
        "ldset x8, x13, [x20]\n"
        "ldclr x23, x22, [x7]\n"
        "ldeor x9, x13, [x8]\n"
        "swp x16, x9, [x17]\n"
        "ldaddal x10, x0, [x25]\n"
        "swpal x13, x18, [x10]\n"
        "cas x0, x12, [x19]\n"
        "casal x18, x20, [x4]\n"
        "casa x1, x20, [x20]\n"
        "ldxr x10, [x14]\n"
        "stxr w11, x21, [sp]\n"
        "ldar x11, [x19]\n"
        "stlr x22, [x8]\n"
        "ldapr x23, [x15]\n"
        "fadd d1, d3, d1\n"
        "fadd s23, s16, s29\n"
        "fsub d19, d20, d11\n"
        "fsub s23, s11, s20\n"
        "fmul d23, d16, d19\n"
        "fmul s24, s6, s1\n"
        "fdiv d8, d19, d14\n"
        "fdiv s17, s15, s20\n"
        "fmax d11, d27, d6\n"
        "fmax s6, s20, s21\n"
        "fmin d14, d28, d10\n"
        "fmin s5, s21, s13\n"
        "add v0.2s, v1.2s, v2.2s\n"
        "sub v0.8h, v1.8h, v2.8h\n"
        "mul v0.4h, v1.4h, v2.4h\n"
        "eor v0.8b, v1.8b, v2.8b\n"
        "cmeq v0.8b, v1.8b, v2.8b\n"
        "smin v0.16b, v1.16b, v2.16b\n"
        "add x7, x9, x3\n"
        "add x23, x12, #3922\n"
        "sub x4, x2, x2\n"
        "sub x0, x12, #2370\n"
        "and x25, x24, x1\n"
        "orr x8, x24, x5\n"
        "adds x25, x8, x6\n"
        "adds x5, x9, #2372\n"
        "subs x20, x27, x23\n"
        "subs x27, x27, #3050\n"
        "mul x2, x27, x19\n"
        "madd x10, x21, x12, x16\n"
        "msub x7, x5, x7, x15\n"
        "sdiv x8, x2, x26\n"
        "udiv x27, x17, x26\n"
        "smull x9, w0, w9\n"
        "umulh x18, x22, x28\n"
        "smulh x9, x27, x24\n"
        "lsl x16, x6, #52\n"
        "lsr x13, x19, #36\n"
        "asr x13, x14, #20\n"
        "ror x7, x9, #33\n"
        "ubfx x26, x25, #1, #3\n"
        "sbfx x1, x14, #20, #9\n"
        "bfi x16, x17, #20, #16\n"
        "ubfiz x22, x10, #4, #7\n"
        "cmp x2, x13\n"
        "cmn x6, x20\n"
        "tst x20, x14\n"
        "csel x8, x5, x11, ge\n"
        "csinc x23, x18, x10, ls\n"
        "csinv x17, x6, x28, lt\n"
        "csneg x3, x26, x1, ls\n"
        "cset x7, cc\n"
        "csetm x24, ne\n"
        "mov x3, #43393\n"
        "movk x5, #38132, lsl #16\n"
        "movz x14, #3365\n"
        "ldr x1, [x11, #0]\n"
        "ldr x28, [x9, x23, lsl #3]\n"
        "ldr x21, [x10], #8\n"
        "str x10, [x9, #16]\n"
        "str x4, [x24, x20, lsl #3]\n"
        "str x13, [x27], #16\n"
        "ldp x21, x26, [x2, #16]\n"
        "stp x19, x6, [x28, #-16]!\n"
        "ldrb w14, [x9]\n"
        "ldrh w4, [x8]\n"
        "ldrsw x12, [x19]\n"
        "strb w5, [x10]\n"
        "ldadd x18, x0, [x11]\n"
        "ldset x1, x14, [x5]\n"
        "ldclr x11, x25, [x25]\n"
        "ldeor x11, x9, [x18]\n"
        "swp x3, x14, [x6]\n"
        "ldaddal x13, x6, [x3]\n"
        "swpal x1, x1, [x1]\n"
        "cas x23, x5, [x19]\n"
        "casal x21, x4, [x19]\n"
        "casa x1, x17, [x15]\n"
        "ldxr x18, [x7]\n"
        "stxr w10, x1, [x3]\n"
        "ldar x26, [x16]\n"
        "stlr x9, [x24]\n"
        "ldapr x13, [x20]\n"
        "fadd d12, d30, d12\n"
        "fadd s15, s28, s26\n"
        "fsub d31, d2, d14\n"
        "fsub s26, s28, s15\n"
        "fmul d27, d13, d31\n"
        "fmul s12, s2, s2\n"
        "fdiv d16, d16, d15\n"
        "fdiv s13, s14, s26\n"
        "fmax d16, d9, d20\n"
        "fmax s3, s20, s7\n"
        "fmin d25, d2, d31\n"
        "fmin s24, s5, s27\n"
        "add v0.16b, v1.16b, v2.16b\n"
        "sub v0.2d, v1.2d, v2.2d\n"
        "umax v0.8h, v1.8h, v2.8h\n"
        "fmul v0.4s, v1.4s, v2.4s\n"
        "add x19, x8, x23\n"
        "add x11, x25, #237\n"
        "sub x26, x14, x24\n"
        "sub x7, x20, #424\n"
        "and x28, x5, x3\n"
        "orr x12, x17, x3\n"
        "eor x23, x6, x13\n"
        "adds x5, x24, x25\n"
        "adds x2, x4, #3644\n"
        "subs x4, x4, x0\n"
        "subs x27, x0, #1715\n"
        "mul x24, x6, x5\n"
        "madd x27, x5, x9, x10\n"
        "msub x6, x17, x28, x21\n"
        "sdiv x20, x6, x5\n"
        "udiv x22, x6, x28\n"
        "smull x12, w9, w0\n"
        "umulh x11, x13, x5\n"
        "smulh x4, x8, x2\n"
        "lsl x10, x9, #0\n"
        "lsr x19, x21, #43\n"
        "asr x2, x9, #45\n"
        "ror x26, x9, #61\n"
        "ubfx x22, x10, #5, #16\n"
        "sbfx x15, x22, #5, #2\n"
        "bfi x8, x0, #11, #13\n"
        "ubfiz x0, x17, #13, #12\n"
        "cmp x12, x18\n"
        "cmn x26, x0\n"
        "tst x14, x1\n"
        "csel x22, x5, x19, ne\n"
        "csinc x3, x24, x7, ge\n"
        "csinv x11, x16, x11, hi\n"
        "csneg x8, x24, x14, eq\n"
        "cset x18, cc\n"
        "csetm x27, cc\n"
        "mov x1, #56751\n"
        "movk x2, #27326, lsl #16\n"
        "movz x10, #47530\n"
        "ldr x4, [x10, #16]\n"
        "ldr x22, [x17, x2, lsl #3]\n"
        "ldr x9, [x21], #-8\n"
        "str x9, [x5, #0]\n"
        "str x20, [x4, x23, lsl #3]\n"
        "str x22, [x9], #-8\n"
        "ldp x5, x23, [x1, #0]\n"
        "stp x19, x17, [sp, #-16]!\n"
        "ldrb w12, [x1]\n"
        "ldrh w7, [x23]\n"
        "ldrsw x19, [x11]\n"
        "strb w26, [x8]\n"
        "ldadd x14, x20, [x13]\n"
        "ldset x4, x1, [sp]\n"
        "ldclr x20, x1, [x25]\n"
        "ldeor x15, x10, [x26]\n"
        "swp x6, x4, [x23]\n"
        "ldaddal x18, x4, [x20]\n"
        "swpal x28, x25, [x13]\n"
        "cas x3, x5, [x13]\n"
        "casal x11, x4, [x1]\n"
        "casa x27, x13, [x9]\n"
        "ldxr x4, [x14]\n"
        "stxr w19, x27, [x5]\n"
        "ldar x16, [x14]\n"
        "stlr x15, [x22]\n"
        "ldapr x23, [x10]\n"
        "fadd d30, d17, d18\n"
        "fadd s30, s25, s9\n"
        "fsub d7, d24, d11\n"
        "fsub s31, s21, s11\n"
        "fmul d5, d31, d17\n"
        "fmul s23, s4, s22\n"
        "fdiv d2, d19, d23\n"
        "fdiv s17, s31, s16\n"
        "fmax d18, d21, d11\n"
        "fmax s0, s30, s16\n"
        "fmin d20, d17, d29\n"
        "fmin s18, s22, s22\n"
        "sub v0.4s, v1.4s, v2.4s\n"
        "cmeq v0.4h, v1.4h, v2.4h\n"
        ".globl axiom_asm_table_end\n"
        "axiom_asm_table_end:\n"
        ".text\n");

extern const unsigned char axiom_asm_table[], axiom_asm_table_end[];
static const char *const want[] = {
    "44021b8b",  /* add x4, x18, x27 */
    "19130891",  /* add x25, x24, #516 */
    "68000fcb",  /* sub x8, x3, x15 */
    "d8713cd1",  /* sub x24, x14, #3868 */
    "9401198a",  /* and x20, x12, x25 */
    "80031aaa",  /* orr x0, x28, x26 */
    "d60108ca",  /* eor x22, x14, x8 */
    "72001cab",  /* adds x18, x3, x28 */
    "0ad802b1",  /* adds x10, x0, #182 */
    "800211eb",  /* subs x0, x20, x17 */
    "80cb30f1",  /* subs x0, x28, #3122 */
    "d57c0d9b",  /* mul x21, x6, x13 */
    "171c109b",  /* madd x23, x0, x16, x7 */
    "d8c50f9b",  /* msub x24, x14, x15, x17 */
    "670dc79a",  /* sdiv x7, x11, x7 */
    "f508d89a",  /* udiv x21, x7, x24 */
    "2e7d209b",  /* smull x14, w9, w0 */
    "4d7fd19b",  /* umulh x13, x26, x17 */
    "747c459b",  /* smulh x20, x3, x5 */
    "f46a5bd3",  /* lsl x20, x23, #37 */
    "e3fe6ad3",  /* lsr x3, x23, #42 */
    "fcfe7693",  /* asr x28, x23, #54 */
    "5063da93",  /* ror x16, x26, #24 */
    "298552d3",  /* ubfx x9, x9, #18, #16 */
    "1b7a4c93",  /* sbfx x27, x16, #12, #19 */
    "3b1c71b3",  /* bfi x27, x1, #15, #8 */
    "373774d3",  /* ubfiz x23, x25, #12, #14 */
    "bf0205eb",  /* cmp x21, x5 */
    "7f0111ab",  /* cmn x11, x17 */
    "9f0316ea",  /* tst x28, x22 */
    "b8b2979a",  /* csel x24, x21, x23, lt */
    "c285959a",  /* csinc x2, x14, x21, hi */
    "038385da",  /* csinv x3, x24, x5, hi */
    "9aa58bda",  /* csneg x26, x12, x11, ge */
    "f7179f9a",  /* cset x23, eq */
    "ef139fda",  /* csetm x15, eq */
    "a93099d2",  /* mov x9, #51589 */
    "14e7aaf2",  /* movk x20, #22328, lsl #16 */
    "25868ed2",  /* movz x5, #29745 */
    "000740f9",  /* ldr x0, [x24, #8] */
    "f17b7bf8",  /* ldr x17, [sp, x27, lsl #3] */
    "f1845ff8",  /* ldr x17, [x7], #-8 */
    "70fd07f9",  /* str x16, [x11, #4088] */
    "cb7928f8",  /* str x11, [x14, x8, lsl #3] */
    "350601f8",  /* str x21, [x17], #16 */
    "97017fa9",  /* ldp x23, x0, [x12, #-16] */
    "9064bfa9",  /* stp x16, x25, [x4, #-16]! */
    "10034039",  /* ldrb w16, [x24] */
    "d1004079",  /* ldrh w17, [x6] */
    "2d0080b9",  /* ldrsw x13, [x1] */
    "6f030039",  /* strb w15, [x27] */
    "32022bf8",  /* ldadd x11, x18, [x17] */
    "b03126f8",  /* ldset x6, x16, [x13] */
    "7a112ff8",  /* ldclr x15, x26, [x11] */
    "0b202df8",  /* ldeor x13, x11, [x0] */
    "718231f8",  /* swp x17, x17, [x19] */
    "5301f9f8",  /* ldaddal x25, x19, [x10] */
    "1380eef8",  /* swpal x14, x19, [x0] */
    "877eb9c8",  /* cas x25, x7, [x20] */
    "51fee5c8",  /* casal x5, x17, [x18] */
    "5b7ce5c8",  /* casa x5, x27, [x2] */
    "397e5fc8",  /* ldxr x25, [x17] */
    "5b7f19c8",  /* stxr w25, x27, [x26] */
    "28fcdfc8",  /* ldar x8, [x1] */
    "bafe9fc8",  /* stlr x26, [x21] */
    "42c0bff8",  /* ldapr x2, [x2] */
    "812b601e",  /* fadd d1, d28, d0 */
    "f129311e",  /* fadd s17, s15, s17 */
    "6739761e",  /* fsub d7, d11, d22 */
    "92382a1e",  /* fsub s18, s4, s10 */
    "0a0a6a1e",  /* fmul d10, d16, d10 */
    "510a3d1e",  /* fmul s17, s18, s29 */
    "f41b7e1e",  /* fdiv d20, d31, d30 */
    "2718331e",  /* fdiv s7, s1, s19 */
    "b84a7a1e",  /* fmax d24, d21, d26 */
    "0c4a261e",  /* fmax s12, s16, s6 */
    "b0597b1e",  /* fmin d16, d13, d27 */
    "c159211e",  /* fmin s1, s14, s1 */
    "20c0221e",  /* fcvt d0, s1 */
    "2000629e",  /* scvtf d0, x1 */
    "0020611e",  /* fcmp d0, d1 */
    "00106f1e",  /* fmov d0, #1.5 */
    "2084624e",  /* add v0.8h, v1.8h, v2.8h */
    "2084226e",  /* sub v0.16b, v1.16b, v2.16b */
    "209c220e",  /* mul v0.8b, v1.8b, v2.8b */
    "201ca24e",  /* orr v0.16b, v1.16b, v2.16b */
    "208ca26e",  /* cmeq v0.4s, v1.4s, v2.4s */
    "2064a22e",  /* umax v0.2s, v1.2s, v2.2s */
    "206ca24e",  /* smin v0.4s, v1.4s, v2.4s */
    "20d4624e",  /* fadd v0.2d, v1.2d, v2.2d */
    "20dc222e",  /* fmul v0.2s, v1.2s, v2.2s */
    "20d4e24e",  /* fsub v0.2d, v1.2d, v2.2d */
    "200c044e",  /* dup v0.4s, w1 */
    "401c0c4e",  /* ins v0.s[1], w2 */
    "203c140e",  /* umov w0, v1.s[2] */
    "2070404c",  /* ld1 {v0.16b}, [x1] */
    "40a8004c",  /* st1 {v0.4s, v1.4s}, [x2] */
    "40000054",  /* b.eq .+8 */
    "800000b4",  /* cbz x0, .+16 */
    "c1ff1f36",  /* tbz x1, #3, .-8 */
    "c0035fd6",  /* ret */
    "1f2003d5",  /* nop */
    "40d03bd5",  /* mrs x0, tpidr_el0 */
    "01441bd5",  /* msr fpcr, x1 */
    "02423bd5",  /* mrs x2, nzcv */
    "bf3b03d5",  /* dmb ish */
    "df3f03d5",  /* isb */
    "9f3f03d5",  /* dsb sy */
    "010000d4",  /* svc #0 */
    "200020d4",  /* brk #1 */
    "7b03018b",  /* add x27, x27, x1 */
    "42342e91",  /* add x2, x2, #2957 */
    "ba0017cb",  /* sub x26, x5, x23 */
    "b97227d1",  /* sub x25, x21, #2524 */
    "6802068a",  /* and x8, x19, x6 */
    "8d020caa",  /* orr x13, x20, x12 */
    "1b000bab",  /* adds x27, x0, x11 */
    "eec328b1",  /* adds x14, sp, #2608 */
    "9c0f15f1",  /* subs x28, x28, #1347 */
    "b17c079b",  /* mul x17, x5, x7 */
    "0728059b",  /* madd x7, x0, x5, x10 */
    "85c0109b",  /* msub x5, x4, x16, x16 */
    "0b0ed59a",  /* sdiv x11, x16, x21 */
    "b108dc9a",  /* udiv x17, x5, x28 */
    "2e7f2d9b",  /* smull x14, w25, w13 */
    "177ed89b",  /* umulh x23, x16, x24 */
    "2b7f529b",  /* smulh x11, x25, x18 */
    "6b1947d3",  /* lsl x11, x11, #57 */
    "05ff73d3",  /* lsr x5, x24, #51 */
    "f6fe7b93",  /* asr x22, x23, #59 */
    "147ed093",  /* ror x20, x16, #31 */
    "0f7d4fd3",  /* ubfx x15, x8, #15, #17 */
    "50674b93",  /* sbfx x16, x26, #11, #15 */
    "9c2f72b3",  /* bfi x28, x28, #14, #12 */
    "f23a6fd3",  /* ubfiz x18, x23, #17, #15 */
    "ff0115eb",  /* cmp x15, x21 */
    "ff000aab",  /* cmn x7, x10 */
    "5f0316ea",  /* tst x26, x22 */
    "ba809c9a",  /* csel x26, x5, x28, hi */
    "08b78f9a",  /* csinc x8, x24, x15, lt */
    "298396da",  /* csinv x9, x25, x22, hi */
    "119690da",  /* csneg x17, x16, x16, ls */
    "f3379f9a",  /* cset x19, cs */
    "e9039fda",  /* csetm x9, ne */
    "4f7697d2",  /* mov x15, #48050 */
    "f5d2a4f2",  /* movk x21, #9879, lsl #16 */
    "79da95d2",  /* movz x25, #44755 */
    "170440f9",  /* ldr x23, [x0, #8] */
    "777861f8",  /* ldr x23, [x3, x1, lsl #3] */
    "928640f8",  /* ldr x18, [x20], #8 */
    "480600f9",  /* str x8, [x18, #8] */
    "957b23f8",  /* str x21, [x28, x3, lsl #3] */
    "188600f8",  /* str x24, [x16], #8 */
    "fb2040a9",  /* ldp x27, x8, [x7, #0] */
    "bc05bfa9",  /* stp x28, x1, [x13, #-16]! */
    "dc024039",  /* ldrb w28, [x22] */
    "38004079",  /* ldrh w24, [x1] */
    "610180b9",  /* ldrsw x1, [x11] */
    "ab000039",  /* strb w11, [x5] */
    "150027f8",  /* ldadd x7, x21, [x0] */
    "433022f8",  /* ldset x2, x3, [x2] */
    "e11220f8",  /* ldclr x0, x1, [x23] */
    "0b2120f8",  /* ldeor x0, x11, [x8] */
    "fa8324f8",  /* swp x4, x26, [sp] */
    "b700e5f8",  /* ldaddal x5, x23, [x5] */
    "1680f0f8",  /* swpal x16, x22, [x0] */
    "327cacc8",  /* cas x12, x18, [x1] */
    "87fcf9c8",  /* casal x25, x7, [x4] */
    "607de1c8",  /* casa x1, x0, [x11] */
    "937e5fc8",  /* ldxr x19, [x20] */
    "777c17c8",  /* stxr w23, x23, [x3] */
    "49fddfc8",  /* ldar x9, [x10] */
    "0ffc9fc8",  /* stlr x15, [x0] */
    "c9c1bff8",  /* ldapr x9, [x14] */
    "022a791e",  /* fadd d2, d16, d25 */
    "c92b2e1e",  /* fadd s9, s30, s14 */
    "853a661e",  /* fsub d5, d20, d6 */
    "813b281e",  /* fsub s1, s28, s8 */
    "f90b741e",  /* fmul d25, d31, d20 */
    "a90a301e",  /* fmul s9, s21, s16 */
    "501b611e",  /* fdiv d16, d26, d1 */
    "6818301e",  /* fdiv s8, s3, s16 */
    "02496a1e",  /* fmax d2, d8, d10 */
    "ca483d1e",  /* fmax s10, s6, s29 */
    "4e586f1e",  /* fmin d14, d2, d15 */
    "8e5b241e",  /* fmin s14, s28, s4 */
    "2084620e",  /* add v0.4h, v1.4h, v2.4h */
    "2084222e",  /* sub v0.8b, v1.8b, v2.8b */
    "209ca20e",  /* mul v0.2s, v1.2s, v2.2s */
    "201c224e",  /* and v0.16b, v1.16b, v2.16b */
    "208ce26e",  /* cmeq v0.2d, v1.2d, v2.2d */
    "20d4224e",  /* fadd v0.4s, v1.4s, v2.4s */
    "20dc626e",  /* fmul v0.2d, v1.2d, v2.2d */
    "20d4a24e",  /* fsub v0.4s, v1.4s, v2.4s */
    "4702118b",  /* add x7, x18, x17 */
    "64ad3c91",  /* add x4, x11, #3883 */
    "540202cb",  /* sub x20, x18, x2 */
    "130c3cd1",  /* sub x19, x0, #3843 */
    "2802078a",  /* and x8, x17, x7 */
    "510311aa",  /* orr x17, x26, x17 */
    "870204ca",  /* eor x7, x20, x4 */
    "170015ab",  /* adds x23, x0, x21 */
    "586414b1",  /* adds x24, x2, #1305 */
    "580201eb",  /* subs x24, x18, x1 */
    "09fb03f1",  /* subs x9, x24, #254 */
    "7a7f089b",  /* mul x26, x27, x8 */
    "6f72179b",  /* madd x15, x19, x23, x28 */
    "ccb6199b",  /* msub x12, x22, x25, x13 */
    "ec0ed99a",  /* sdiv x12, x23, x25 */
    "d209c49a",  /* udiv x18, x14, x4 */
    "7c7d239b",  /* smull x28, w11, w3 */
    "817ccf9b",  /* umulh x1, x4, x15 */
    "067d559b",  /* smulh x6, x8, x21 */
    "0d675ad3",  /* lsl x13, x24, #38 */
    "0dfe71d3",  /* lsr x13, x16, #49 */
    "72fd7493",  /* asr x18, x11, #52 */
    "f2acc793",  /* ror x18, x7, #43 */
    "156c48d3",  /* ubfx x21, x0, #8, #20 */
    "d53e4593",  /* sbfx x21, x22, #5, #11 */
    "914b6eb3",  /* bfi x17, x28, #18, #19 */
    "c31a6cd3",  /* ubfiz x3, x22, #20, #7 */
    "9f021aeb",  /* cmp x20, x26 */
    "5f0208ab",  /* cmn x18, x8 */
    "3f0103ea",  /* tst x9, x3 */
    "e2919b9a",  /* csel x2, x15, x27, ls */
    "4f048b9a",  /* csinc x15, x2, x11, eq */
    "8d0384da",  /* csinv x13, x28, x4, eq */
    "a9a598da",  /* csneg x9, x13, x24, ge */
    "fb179f9a",  /* cset x27, eq */
    "e1139fda",  /* csetm x1, eq */
    "4c2e95d2",  /* mov x12, #43378 */
    "51dcb1f2",  /* movk x17, #36578, lsl #16 */
    "d0198fd2",  /* movz x16, #30926 */
    "210140f9",  /* ldr x1, [x9, #0] */
    "627873f8",  /* ldr x2, [x3, x19, lsl #3] */
    "318440f8",  /* ldr x17, [x1], #8 */
    "2dfd07f9",  /* str x13, [x9, #4088] */
    "887836f8",  /* str x8, [x4, x22, lsl #3] */
    "61871ff8",  /* str x1, [x27], #-8 */
    "8a2c41a9",  /* ldp x10, x11, [x4, #16] */
    "6c3bbfa9",  /* stp x12, x14, [x27, #-16]! */
    "90014039",  /* ldrb w16, [x12] */
    "74034079",  /* ldrh w20, [x27] */
    "b30280b9",  /* ldrsw x19, [x21] */
    "71000039",  /* strb w17, [x3] */
    "190233f8",  /* ldadd x19, x25, [x16] */
    "8d3228f8",  /* ldset x8, x13, [x20] */
    "f61037f8",  /* ldclr x23, x22, [x7] */
    "0d2129f8",  /* ldeor x9, x13, [x8] */
    "298230f8",  /* swp x16, x9, [x17] */
    "2003eaf8",  /* ldaddal x10, x0, [x25] */
    "5281edf8",  /* swpal x13, x18, [x10] */
    "6c7ea0c8",  /* cas x0, x12, [x19] */
    "94fcf2c8",  /* casal x18, x20, [x4] */
    "947ee1c8",  /* casa x1, x20, [x20] */
    "ca7d5fc8",  /* ldxr x10, [x14] */
    "f57f0bc8",  /* stxr w11, x21, [sp] */
    "6bfedfc8",  /* ldar x11, [x19] */
    "16fd9fc8",  /* stlr x22, [x8] */
    "f7c1bff8",  /* ldapr x23, [x15] */
    "6128611e",  /* fadd d1, d3, d1 */
    "172a3d1e",  /* fadd s23, s16, s29 */
    "933a6b1e",  /* fsub d19, d20, d11 */
    "7739341e",  /* fsub s23, s11, s20 */
    "170a731e",  /* fmul d23, d16, d19 */
    "d808211e",  /* fmul s24, s6, s1 */
    "681a6e1e",  /* fdiv d8, d19, d14 */
    "f119341e",  /* fdiv s17, s15, s20 */
    "6b4b661e",  /* fmax d11, d27, d6 */
    "864a351e",  /* fmax s6, s20, s21 */
    "8e5b6a1e",  /* fmin d14, d28, d10 */
    "a55a2d1e",  /* fmin s5, s21, s13 */
    "2084a20e",  /* add v0.2s, v1.2s, v2.2s */
    "2084626e",  /* sub v0.8h, v1.8h, v2.8h */
    "209c620e",  /* mul v0.4h, v1.4h, v2.4h */
    "201c222e",  /* eor v0.8b, v1.8b, v2.8b */
    "208c222e",  /* cmeq v0.8b, v1.8b, v2.8b */
    "206c224e",  /* smin v0.16b, v1.16b, v2.16b */
    "2701038b",  /* add x7, x9, x3 */
    "97493d91",  /* add x23, x12, #3922 */
    "440002cb",  /* sub x4, x2, x2 */
    "800925d1",  /* sub x0, x12, #2370 */
    "1903018a",  /* and x25, x24, x1 */
    "080305aa",  /* orr x8, x24, x5 */
    "190106ab",  /* adds x25, x8, x6 */
    "251125b1",  /* adds x5, x9, #2372 */
    "740317eb",  /* subs x20, x27, x23 */
    "7bab2ff1",  /* subs x27, x27, #3050 */
    "627f139b",  /* mul x2, x27, x19 */
    "aa420c9b",  /* madd x10, x21, x12, x16 */
    "a7bc079b",  /* msub x7, x5, x7, x15 */
    "480cda9a",  /* sdiv x8, x2, x26 */
    "3b0ada9a",  /* udiv x27, x17, x26 */
    "097c299b",  /* smull x9, w0, w9 */
    "d27edc9b",  /* umulh x18, x22, x28 */
    "697f589b",  /* smulh x9, x27, x24 */
    "d02c4cd3",  /* lsl x16, x6, #52 */
    "6dfe64d3",  /* lsr x13, x19, #36 */
    "cdfd5493",  /* asr x13, x14, #20 */
    "2785c993",  /* ror x7, x9, #33 */
    "3a0f41d3",  /* ubfx x26, x25, #1, #3 */
    "c1715493",  /* sbfx x1, x14, #20, #9 */
    "303e6cb3",  /* bfi x16, x17, #20, #16 */
    "56197cd3",  /* ubfiz x22, x10, #4, #7 */
    "5f000deb",  /* cmp x2, x13 */
    "df0014ab",  /* cmn x6, x20 */
    "9f020eea",  /* tst x20, x14 */
    "a8a08b9a",  /* csel x8, x5, x11, ge */
    "57968a9a",  /* csinc x23, x18, x10, ls */
    "d1b09cda",  /* csinv x17, x6, x28, lt */
    "439781da",  /* csneg x3, x26, x1, ls */
    "e7279f9a",  /* cset x7, cc */
    "f8039fda",  /* csetm x24, ne */
    "233095d2",  /* mov x3, #43393 */
    "859eb2f2",  /* movk x5, #38132, lsl #16 */
    "aea481d2",  /* movz x14, #3365 */
    "610140f9",  /* ldr x1, [x11, #0] */
    "3c7977f8",  /* ldr x28, [x9, x23, lsl #3] */
    "558540f8",  /* ldr x21, [x10], #8 */
    "2a0900f9",  /* str x10, [x9, #16] */
    "047b34f8",  /* str x4, [x24, x20, lsl #3] */
    "6d0701f8",  /* str x13, [x27], #16 */
    "556841a9",  /* ldp x21, x26, [x2, #16] */
    "931bbfa9",  /* stp x19, x6, [x28, #-16]! */
    "2e014039",  /* ldrb w14, [x9] */
    "04014079",  /* ldrh w4, [x8] */
    "6c0280b9",  /* ldrsw x12, [x19] */
    "45010039",  /* strb w5, [x10] */
    "600132f8",  /* ldadd x18, x0, [x11] */
    "ae3021f8",  /* ldset x1, x14, [x5] */
    "39132bf8",  /* ldclr x11, x25, [x25] */
    "49222bf8",  /* ldeor x11, x9, [x18] */
    "ce8023f8",  /* swp x3, x14, [x6] */
    "6600edf8",  /* ldaddal x13, x6, [x3] */
    "2180e1f8",  /* swpal x1, x1, [x1] */
    "657eb7c8",  /* cas x23, x5, [x19] */
    "64fef5c8",  /* casal x21, x4, [x19] */
    "f17de1c8",  /* casa x1, x17, [x15] */
    "f27c5fc8",  /* ldxr x18, [x7] */
    "617c0ac8",  /* stxr w10, x1, [x3] */
    "1afedfc8",  /* ldar x26, [x16] */
    "09ff9fc8",  /* stlr x9, [x24] */
    "8dc2bff8",  /* ldapr x13, [x20] */
    "cc2b6c1e",  /* fadd d12, d30, d12 */
    "8f2b3a1e",  /* fadd s15, s28, s26 */
    "5f386e1e",  /* fsub d31, d2, d14 */
    "9a3b2f1e",  /* fsub s26, s28, s15 */
    "bb097f1e",  /* fmul d27, d13, d31 */
    "4c08221e",  /* fmul s12, s2, s2 */
    "101a6f1e",  /* fdiv d16, d16, d15 */
    "cd193a1e",  /* fdiv s13, s14, s26 */
    "3049741e",  /* fmax d16, d9, d20 */
    "834a271e",  /* fmax s3, s20, s7 */
    "59587f1e",  /* fmin d25, d2, d31 */
    "b8583b1e",  /* fmin s24, s5, s27 */
    "2084224e",  /* add v0.16b, v1.16b, v2.16b */
    "2084e26e",  /* sub v0.2d, v1.2d, v2.2d */
    "2064626e",  /* umax v0.8h, v1.8h, v2.8h */
    "20dc226e",  /* fmul v0.4s, v1.4s, v2.4s */
    "1301178b",  /* add x19, x8, x23 */
    "2bb70391",  /* add x11, x25, #237 */
    "da0118cb",  /* sub x26, x14, x24 */
    "87a206d1",  /* sub x7, x20, #424 */
    "bc00038a",  /* and x28, x5, x3 */
    "2c0203aa",  /* orr x12, x17, x3 */
    "d7000dca",  /* eor x23, x6, x13 */
    "050319ab",  /* adds x5, x24, x25 */
    "82f038b1",  /* adds x2, x4, #3644 */
    "840000eb",  /* subs x4, x4, x0 */
    "1bcc1af1",  /* subs x27, x0, #1715 */
    "d87c059b",  /* mul x24, x6, x5 */
    "bb28099b",  /* madd x27, x5, x9, x10 */
    "26d61c9b",  /* msub x6, x17, x28, x21 */
    "d40cc59a",  /* sdiv x20, x6, x5 */
    "d608dc9a",  /* udiv x22, x6, x28 */
    "2c7d209b",  /* smull x12, w9, w0 */
    "ab7dc59b",  /* umulh x11, x13, x5 */
    "047d429b",  /* smulh x4, x8, x2 */
    "2afd40d3",  /* lsl x10, x9, #0 */
    "b3fe6bd3",  /* lsr x19, x21, #43 */
    "22fd6d93",  /* asr x2, x9, #45 */
    "3af5c993",  /* ror x26, x9, #61 */
    "565145d3",  /* ubfx x22, x10, #5, #16 */
    "cf1a4593",  /* sbfx x15, x22, #5, #2 */
    "083075b3",  /* bfi x8, x0, #11, #13 */
    "202e73d3",  /* ubfiz x0, x17, #13, #12 */
    "9f0112eb",  /* cmp x12, x18 */
    "5f0300ab",  /* cmn x26, x0 */
    "df0101ea",  /* tst x14, x1 */
    "b610939a",  /* csel x22, x5, x19, ne */
    "03a7879a",  /* csinc x3, x24, x7, ge */
    "0b828bda",  /* csinv x11, x16, x11, hi */
    "08078eda",  /* csneg x8, x24, x14, eq */
    "f2279f9a",  /* cset x18, cc */
    "fb239fda",  /* csetm x27, cc */
    "e1b59bd2",  /* mov x1, #56751 */
    "c257adf2",  /* movk x2, #27326, lsl #16 */
    "4a3597d2",  /* movz x10, #47530 */
    "440940f9",  /* ldr x4, [x10, #16] */
    "367a62f8",  /* ldr x22, [x17, x2, lsl #3] */
    "a9865ff8",  /* ldr x9, [x21], #-8 */
    "a90000f9",  /* str x9, [x5, #0] */
    "947837f8",  /* str x20, [x4, x23, lsl #3] */
    "36851ff8",  /* str x22, [x9], #-8 */
    "255c40a9",  /* ldp x5, x23, [x1, #0] */
    "f347bfa9",  /* stp x19, x17, [sp, #-16]! */
    "2c004039",  /* ldrb w12, [x1] */
    "e7024079",  /* ldrh w7, [x23] */
    "730180b9",  /* ldrsw x19, [x11] */
    "1a010039",  /* strb w26, [x8] */
    "b4012ef8",  /* ldadd x14, x20, [x13] */
    "e13324f8",  /* ldset x4, x1, [sp] */
    "211334f8",  /* ldclr x20, x1, [x25] */
    "4a232ff8",  /* ldeor x15, x10, [x26] */
    "e48226f8",  /* swp x6, x4, [x23] */
    "8402f2f8",  /* ldaddal x18, x4, [x20] */
    "b981fcf8",  /* swpal x28, x25, [x13] */
    "a57da3c8",  /* cas x3, x5, [x13] */
    "24fcebc8",  /* casal x11, x4, [x1] */
    "2d7dfbc8",  /* casa x27, x13, [x9] */
    "c47d5fc8",  /* ldxr x4, [x14] */
    "bb7c13c8",  /* stxr w19, x27, [x5] */
    "d0fddfc8",  /* ldar x16, [x14] */
    "cffe9fc8",  /* stlr x15, [x22] */
    "57c1bff8",  /* ldapr x23, [x10] */
    "3e2a721e",  /* fadd d30, d17, d18 */
    "3e2b291e",  /* fadd s30, s25, s9 */
    "073b6b1e",  /* fsub d7, d24, d11 */
    "bf3a2b1e",  /* fsub s31, s21, s11 */
    "e50b711e",  /* fmul d5, d31, d17 */
    "9708361e",  /* fmul s23, s4, s22 */
    "621a771e",  /* fdiv d2, d19, d23 */
    "f11b301e",  /* fdiv s17, s31, s16 */
    "b24a6b1e",  /* fmax d18, d21, d11 */
    "c04b301e",  /* fmax s0, s30, s16 */
    "345a7d1e",  /* fmin d20, d17, d29 */
    "d25a361e",  /* fmin s18, s22, s22 */
    "2084a26e",  /* sub v0.4s, v1.4s, v2.4s */
    "208c622e",  /* cmeq v0.4h, v1.4h, v2.4h */
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
