/* flags: --c-std=gnu17 */
/* native-only */
/* The AArch64 assembler against a table: 2410 instruction lines (the general-purpose
   instructions in their operand forms, loads and stores, atomics, system, floating point, vectors), each with the bytes its
   encoding has (from the architecture manual's tables), assembled here as
   file-scope asm in a section of their own; the bytes are compared. */
#include "host.h"

#ifdef __AXIOM_ASM_AARCH64__
__asm__(".section .rodata.axiom_asm_table, \"a\"\n"
        ".globl axiom_asm_table\n"
        "axiom_asm_table:\n"
        "0:\n1:\n2:\n3:\n4:\n"
        "add x4, x8, #4096\n"
        "add x4, sp, #7, lsl #12\n"
        "add xzr, x25, xzr\n"
        "add xzr, x13, xzr, lsr #23\n"
        "add x4, x25, w18, uxtb #2\n"
        "add x4, x0, x0, lsl #2\n"
        "adds x17, sp, #1\n"
        "adds x17, x13, #1, lsl #12\n"
        "adds xzr, xzr, x17\n"
        "adds x11, x21, xzr, lsr #29\n"
        "adds x17, sp, w26, sxth #0\n"
        "adds x17, sp, x3, lsl #2\n"
        "sub sp, x16, #4095\n"
        "sub sp, sp, #7, lsl #12\n"
        "sub x18, x15, xzr\n"
        "sub x13, xzr, xzr, asr #16\n"
        "sub sp, x3, w16, sxtb #2\n"
        "sub sp, x15, x15, uxtx\n"
        "subs xzr, x20, #1\n"
        "subs xzr, x16, #1, lsl #12\n"
        "subs x24, x17, xzr\n"
        "subs xzr, xzr, x21, lsr #25\n"
        "subs xzr, x27, w16, sxth #1\n"
        "subs xzr, x13, x15, sxtx\n"
        "cmp x18, #3386\n"
        "cmn xzr, xzr\n"
        "cmp x0, x17, lsl #3\n"
        "neg x17, x19\n"
        "negs x25, x19, lsl #2\n"
        "adc xzr, x19, x25\n"
        "adcs x20, x17, x27\n"
        "sbc xzr, x1, x2\n"
        "sbcs x14, xzr, xzr\n"
        "ngc x3, x25\n"
        "ngcs x19, x5\n"
        "and sp, x2, #0x5555555555555555\n"
        "orr x8, xzr, #0xf0f0f0f0f0f0f0f\n"
        "eor sp, xzr, #0xff00\n"
        "ands xzr, xzr, #0x1\n"
        "and x25, x8, x8\n"
        "and xzr, x26, x7, ror #4\n"
        "orr x1, xzr, x17\n"
        "orr xzr, x7, xzr, ror #1\n"
        "eor xzr, x4, x28\n"
        "eor x9, x27, xzr, lsr #13\n"
        "ands xzr, x4, x17\n"
        "ands x18, xzr, x5, ror #6\n"
        "bic x11, xzr, x18\n"
        "bic x15, xzr, xzr, lsl #10\n"
        "orn xzr, xzr, x0\n"
        "orn xzr, x25, xzr, lsr #8\n"
        "eon x21, xzr, xzr\n"
        "eon xzr, x24, x2, lsl #4\n"
        "bics x5, x29, xzr\n"
        "bics xzr, xzr, x10, asr #7\n"
        "tst x27, #0x8000000000000000\n"
        "tst x4, x18\n"
        "mvn x17, x24\n"
        "mov x3, x10\n"
        "mov sp, x1\n"
        "mov x13, sp\n"
        "mov x2, #0\n"
        "mov x12, #1\n"
        "mov x27, #65535\n"
        "mov x25, #65536\n"
        "mov x4, #4294901760\n"
        "mov x26, #-1\n"
        "mov x4, #-2\n"
        "mov x10, #-65536\n"
        "mov x3, #20014547599360\n"
        "mov x19, #18446462598732840960\n"
        "mov x18, #6148914691236517205\n"
        "movz x25, #49550, lsl #16\n"
        "movk x2, #29322\n"
        "movn x18, #10714\n"
        "movk x30, #34960, lsl #48\n"
        "lsl x11, x28, #19\n"
        "lsl x18, x17, x29\n"
        "lsr x3, x14, #58\n"
        "lsr x8, x3, x25\n"
        "asr x1, x26, #19\n"
        "asr x0, x19, x21\n"
        "ror x0, x2, #27\n"
        "ror x3, x26, x28\n"
        "lslv x25, x1, x6\n"
        "lsrv x7, x25, x18\n"
        "asrv x13, x5, x3\n"
        "rorv x14, x5, x21\n"
        "udiv x7, x5, x23\n"
        "sdiv x27, x3, x13\n"
        "ubfx x30, x12, #58, #5\n"
        "sbfx x26, x9, #58, #5\n"
        "bfxil x22, x15, #16, #21\n"
        "ubfiz x6, x20, #6, #21\n"
        "sbfiz x0, x0, #2, #51\n"
        "bfi x29, x9, #62, #2\n"
        "ubfm x14, x12, #40, #51\n"
        "sbfm x2, x2, #40, #58\n"
        "bfm x3, x8, #27, #60\n"
        "bfc x21, #3, #5\n"
        "extr x11, x8, x5, #26\n"
        "madd x9, x6, x7, x11\n"
        "msub x26, x8, x2, xzr\n"
        "mul x2, x20, x18\n"
        "mneg x20, x10, x30\n"
        "rbit x7, x12\n"
        "rev16 x30, x9\n"
        "rev x1, x10\n"
        "clz x5, x10\n"
        "cls x25, x27\n"
        "csel xzr, xzr, x3, pl\n"
        "csinc x7, x25, x12, vs\n"
        "csinv x17, x23, x0, vc\n"
        "csneg xzr, xzr, x27, hs\n"
        "cset x16, hi\n"
        "csetm x2, lo\n"
        "cinc x5, x24, cc\n"
        "cinv x4, x26, hi\n"
        "cneg x9, x3, vc\n"
        "ccmp x4, x28, #6, cc\n"
        "ccmp x17, #2, #10, mi\n"
        "ccmn x5, x9, #13, lo\n"
        "ccmn x1, #15, #8, cs\n"
        "cbz x21, 1f\n"
        "cbnz x30, 1b\n"
        "tbz x14, #55, 1f\n"
        "tbnz x17, #32, 1b\n"
        "add wsp, wsp, #0\n"
        "add wsp, wsp, #1, lsl #12\n"
        "add wzr, wzr, w18\n"
        "add wzr, w18, w18, lsl #8\n"
        "add wsp, wsp, w12, sxth #3\n"
        "adds w5, wsp, #0\n"
        "adds w5, wsp, #7, lsl #12\n"
        "adds w29, wzr, wzr\n"
        "adds w30, wzr, wzr, asr #7\n"
        "adds w5, w1, w24, sxth #2\n"
        "sub w5, wsp, #4095\n"
        "sub w5, wsp, #1, lsl #12\n"
        "sub wzr, w19, w27\n"
        "sub wzr, w5, wzr, lsl #30\n"
        "sub w5, w18, w15, sxtw #3\n"
        "subs wzr, w12, #16773120\n"
        "subs wzr, w23, #1, lsl #12\n"
        "subs wzr, w20, w8\n"
        "subs w30, w24, w14, lsr #30\n"
        "subs wzr, wsp, w12, uxth #2\n"
        "cmp w14, #3997\n"
        "cmn w30, wzr\n"
        "cmp w19, w17, lsl #3\n"
        "neg w13, w29\n"
        "negs w3, w21, lsl #2\n"
        "adc wzr, wzr, w23\n"
        "adcs w30, wzr, w18\n"
        "sbc w0, wzr, w6\n"
        "sbcs w9, w17, wzr\n"
        "ngc w18, w24\n"
        "ngcs w8, w26\n"
        "and wsp, w25, #0xff00ff\n"
        "orr wsp, w13, #0xaaaaaaaa\n"
        "eor wsp, wzr, #0xff00\n"
        "ands w28, w3, #0xff00ff\n"
        "and w9, wzr, wzr\n"
        "and wzr, wzr, w0, ror #22\n"
        "orr wzr, wzr, wzr\n"
        "orr wzr, w12, w17, asr #20\n"
        "eor w19, wzr, wzr\n"
        "eor wzr, w22, w14, asr #16\n"
        "ands wzr, wzr, wzr\n"
        "ands w27, w15, wzr, lsl #13\n"
        "bic w23, wzr, wzr\n"
        "bic w27, w24, w26, asr #29\n"
        "orn wzr, wzr, wzr\n"
        "orn wzr, w15, w14, asr #16\n"
        "eon wzr, wzr, wzr\n"
        "eon w0, w16, w22, ror #20\n"
        "bics wzr, wzr, w6\n"
        "bics wzr, w8, wzr, ror #16\n"
        "tst w17, #0xff\n"
        "tst w5, w9\n"
        "mvn w20, w23\n"
        "mov w22, w26\n"
        "mov wsp, w17\n"
        "mov w8, wsp\n"
        "mov w11, #0\n"
        "mov w19, #1\n"
        "mov w23, #65535\n"
        "mov w7, #65536\n"
        "mov w12, #4294901760\n"
        "mov w17, #-1\n"
        "mov w12, #-2\n"
        "mov w5, #-65536\n"
        "mov w15, #1431655765\n"
        "movz w25, #34016, lsl #16\n"
        "movk w27, #43207\n"
        "movn w22, #29139\n"
        "lsl w8, w30, #20\n"
        "lsl w22, w7, w27\n"
        "lsr w21, w0, #28\n"
        "lsr w28, w27, w19\n"
        "asr w12, w10, #30\n"
        "asr w13, w29, w24\n"
        "ror w7, w25, #9\n"
        "ror w6, w2, w20\n"
        "lslv w23, w5, w27\n"
        "lsrv w18, w14, w18\n"
        "asrv w29, w29, w23\n"
        "rorv w4, w19, w30\n"
        "udiv w8, w14, w16\n"
        "sdiv w5, w4, w24\n"
        "ubfx w28, w22, #4, #15\n"
        "sbfx w9, w24, #11, #13\n"
        "bfxil w3, w22, #7, #7\n"
        "ubfiz w21, w9, #22, #2\n"
        "sbfiz w7, w12, #3, #11\n"
        "bfi w29, w3, #15, #6\n"
        "ubfm w1, w1, #1, #13\n"
        "sbfm w21, w1, #31, #28\n"
        "bfm w10, w21, #17, #7\n"
        "bfc w19, #3, #5\n"
        "extr w22, w5, w3, #14\n"
        "madd w12, w7, w15, wzr\n"
        "msub w24, w5, w7, wzr\n"
        "mul w14, w17, w18\n"
        "mneg w12, w6, w14\n"
        "rbit w22, w8\n"
        "rev16 w10, w15\n"
        "rev w18, w3\n"
        "clz w29, w6\n"
        "cls w2, w1\n"
        "csel w0, wzr, wzr, vc\n"
        "csinc w29, w12, w28, eq\n"
        "csinv wzr, w4, wzr, vs\n"
        "csneg w4, wzr, w28, ne\n"
        "cset w17, ne\n"
        "csetm w16, cc\n"
        "cinc w1, w29, vs\n"
        "cinv w24, w3, lt\n"
        "cneg w2, w6, eq\n"
        "ccmp w15, w20, #4, vs\n"
        "ccmp w21, #12, #14, ge\n"
        "ccmn w10, w20, #8, vs\n"
        "ccmn w20, #15, #7, ne\n"
        "cbz w18, 1f\n"
        "cbnz w29, 1b\n"
        "tbz w25, #11, 1f\n"
        "tbnz w11, #27, 1b\n"
        "sxtb w17, w20\n"
        "sxth w1, w28\n"
        "sxtw x11, w17\n"
        "uxtb w13, w17\n"
        "uxth w6, w22\n"
        "rev32 x28, x17\n"
        "rev64 x13, x29\n"
        "smaddl x21, w2, w22, x8\n"
        "smsubl x23, w19, w23, x24\n"
        "umaddl x2, w8, w5, x3\n"
        "umsubl x4, w1, w29, x6\n"
        "smull x27, w13, w27\n"
        "umull x1, w1, w20\n"
        "smnegl x2, w29, w26\n"
        "umnegl x16, w15, w16\n"
        "smulh x11, x3, x10\n"
        "umulh x1, x4, x17\n"
        "crc32b w1, w14, w21\n"
        "crc32h w4, w28, w12\n"
        "crc32w w24, w22, w28\n"
        "crc32cb w28, w14, w0\n"
        "crc32ch w23, w16, w8\n"
        "crc32cw w2, w8, w25\n"
        "crc32x w10, w2, x9\n"
        "crc32cx w1, w27, x12\n"
        "b.eq 1b\n"
        "b.eq 1f\n"
        "b.ne 1b\n"
        "b.ne 1f\n"
        "b.cs 1b\n"
        "b.cs 1f\n"
        "b.hs 1b\n"
        "b.hs 1f\n"
        "b.cc 1b\n"
        "b.cc 1f\n"
        "b.lo 1b\n"
        "b.lo 1f\n"
        "b.mi 1b\n"
        "b.mi 1f\n"
        "b.pl 1b\n"
        "b.pl 1f\n"
        "b.vs 1b\n"
        "b.vs 1f\n"
        "b.vc 1b\n"
        "b.vc 1f\n"
        "b.hi 1b\n"
        "b.hi 1f\n"
        "b.ls 1b\n"
        "b.ls 1f\n"
        "b.ge 1b\n"
        "b.ge 1f\n"
        "b.lt 1b\n"
        "b.lt 1f\n"
        "b.gt 1b\n"
        "b.gt 1f\n"
        "b.le 1b\n"
        "b.le 1f\n"
        "b 1f\n"
        "bl 1b\n"
        "br x1\n"
        "blr x23\n"
        "ret\n"
        "ret x8\n"
        "adr x10, 1b\n"
        "adr x23, 1f\n"
        "ldr x4, [sp]\n"
        "ldr x25, [sp, #6160]\n"
        "ldr x13, [x26, #-8]\n"
        "ldr x10, [sp, #255]!\n"
        "ldr x30, [sp], #-256\n"
        "ldr x4, [sp, x16]\n"
        "ldr x17, [x23, x28, lsl #3]\n"
        "ldr x26, [x9, w6, sxtw]\n"
        "ldr x12, [sp, w3, sxtw #3]\n"
        "ldr x13, [x11, x18, sxtx]\n"
        "str x2, [sp]\n"
        "str x26, [sp, #27360]\n"
        "str x9, [sp, #3]\n"
        "str x10, [x23, #-256]!\n"
        "str x4, [sp], #8\n"
        "str x18, [sp, x8]\n"
        "str x15, [sp, x29, lsl #3]\n"
        "str x23, [x12, w29, uxtw]\n"
        "str x4, [sp, w18, sxtw #3]\n"
        "str x27, [x8, x22, sxtx]\n"
        "ldur x18, [sp, #114]\n"
        "stur x30, [sp, #156]\n"
        "ldr w9, [sp]\n"
        "ldr w17, [x16, #952]\n"
        "ldr w4, [x8, #-8]\n"
        "ldr w29, [x3, #255]!\n"
        "ldr w30, [x23], #-256\n"
        "ldr w17, [sp, x22]\n"
        "ldr w3, [sp, x2, lsl #2]\n"
        "ldr w20, [x18, w27, uxtw]\n"
        "ldr w25, [x27, w20, sxtw #2]\n"
        "ldr w26, [sp, x0, sxtx]\n"
        "str w18, [sp]\n"
        "str w22, [sp, #7204]\n"
        "str w28, [sp, #-8]\n"
        "str w13, [sp, #-16]!\n"
        "str w25, [x15], #8\n"
        "str w13, [x6, x23]\n"
        "str w17, [sp, x16, lsl #2]\n"
        "str w28, [x15, w12, sxtw]\n"
        "str w1, [sp, w0, sxtw #2]\n"
        "str w6, [sp, x22, sxtx]\n"
        "ldur w22, [x20, #-134]\n"
        "stur w26, [sp, #33]\n"
        "ldrb w16, [sp, #2521]\n"
        "ldrb w14, [x9, #198]!\n"
        "ldrb w18, [x4], #2\n"
        "ldrb w20, [sp, x23]\n"
        "ldrb w21, [x18, x11, lsl #0]\n"
        "strb w13, [sp, #150]\n"
        "strb w28, [x2, #-252]!\n"
        "strb w12, [sp], #22\n"
        "strb w25, [sp, x20]\n"
        "strb w23, [sp, x24, lsl #0]\n"
        "ldrh w10, [sp, #1908]\n"
        "ldrh w15, [x11, #169]!\n"
        "ldrh w4, [x0], #10\n"
        "ldrh w11, [x27, x18]\n"
        "ldrh w25, [sp, x8, lsl #1]\n"
        "strh w30, [sp, #6892]\n"
        "strh w22, [sp, #87]!\n"
        "strh w24, [sp], #-36\n"
        "strh w22, [sp, x30]\n"
        "strh w12, [sp, x2, lsl #1]\n"
        "ldrsb x6, [x30, #58]\n"
        "ldrsb w3, [x8, x15]\n"
        "ldrsh w3, [x12, #0]\n"
        "ldrsh x19, [x30, x17]\n"
        "ldrsw x6, [sp, #11360]\n"
        "ldrsw x1, [x30], #4\n"
        "ldrsw x23, [sp, x26, lsl #2]\n"
        "ldurb w21, [x23, #15]\n"
        "sturb w21, [x8, #235]\n"
        "ldurh w25, [x25, #-37]\n"
        "sturh w21, [x20, #143]\n"
        "ldursb w3, [sp, #45]\n"
        "ldursh w21, [sp, #146]\n"
        "ldursw x3, [sp, #-4]\n"
        "ldr x1, 1b\n"
        "ldr w2, 1f\n"
        "ldrsw x3, 1b\n"
        "ldr d4, 1f\n"
        "ldr q5, 1b\n"
        "ldr s6, 1f\n"
        "prfm pldl1keep, [x3]\n"
        "prfm pstl2strm, [x4, #64]\n"
        "prfm pldl1keep, 1b\n"
        "prfum pldl3keep, [x12, #-1]\n"
        "ldp x19, x28, [x22, #-176]\n"
        "ldp x16, x8, [sp, #496]!\n"
        "ldp x20, x28, [x25], #176\n"
        "stp x27, x15, [x3, #192]\n"
        "stp x29, x28, [sp, #-400]!\n"
        "stp x17, x20, [sp], #-312\n"
        "ldnp x7, x16, [sp, #24]\n"
        "stnp x22, x7, [x13]\n"
        "ldp w4, w8, [sp, #-200]\n"
        "ldp w17, w26, [x19, #164]!\n"
        "ldp w8, w8, [sp], #16\n"
        "stp w15, w6, [sp, #224]\n"
        "stp w7, w10, [x5, #204]!\n"
        "stp w17, w4, [sp], #-120\n"
        "ldnp w20, w24, [x25, #12]\n"
        "stnp w10, w19, [sp]\n"
        "ldpsw x10, x3, [x4, #8]\n"
        "ldp s16, s14, [x2, #-80]\n"
        "stp s7, s14, [x18, #56]\n"
        "ldp d27, d20, [x0, #112]\n"
        "stp d14, d5, [x23, #56]\n"
        "ldp q21, q17, [sp, #-944]\n"
        "stp q7, q21, [x11, #-560]\n"
        "ldr b16, [x28, #336]\n"
        "ldr b22, [x2, #-151]!\n"
        "ldr b19, [x10, x8]\n"
        "str b3, [x11, #641]\n"
        "str b8, [sp, #124]!\n"
        "str b15, [sp, x8]\n"
        "ldur b0, [sp, #-3]\n"
        "ldr h7, [x11, #4440]\n"
        "ldr h25, [sp, #172]!\n"
        "ldr h25, [x9, x20, lsl #1]\n"
        "str h19, [x17, #882]\n"
        "str h7, [x5, #-36]!\n"
        "str h27, [x8, x8, lsl #1]\n"
        "ldur h17, [sp, #-3]\n"
        "ldr s30, [sp, #3396]\n"
        "ldr s23, [sp, #-225]!\n"
        "ldr s19, [x14, x4, lsl #2]\n"
        "str s4, [x29, #7084]\n"
        "str s30, [sp, #117]!\n"
        "str s18, [x5, x27, lsl #2]\n"
        "ldur s24, [sp, #-3]\n"
        "ldr d25, [x3, #17672]\n"
        "ldr d18, [x21, #-247]!\n"
        "ldr d8, [x12, x14, lsl #3]\n"
        "str d1, [sp, #27672]\n"
        "str d17, [sp, #162]!\n"
        "str d25, [sp, x1, lsl #3]\n"
        "ldur d6, [x15, #-3]\n"
        "ldr q0, [x25, #14560]\n"
        "ldr q8, [sp, #21]!\n"
        "ldr q22, [sp, x26, lsl #4]\n"
        "str q15, [x29, #13824]\n"
        "str q22, [x27, #-137]!\n"
        "str q2, [sp, x13, lsl #4]\n"
        "ldur q22, [x8, #-3]\n"
        "ldxr x19, [sp]\n"
        "ldaxr x12, [sp]\n"
        "ldar x24, [sp]\n"
        "stlr x14, [x25]\n"
        "stxr w20, x19, [x16]\n"
        "stlxr w1, x10, [x21]\n"
        "ldxp x28, x16, [sp]\n"
        "ldaxp x28, x10, [x24]\n"
        "stxp w18, x0, x15, [x28]\n"
        "stlxp w12, x20, x26, [x30]\n"
        "ldxr w12, [x22]\n"
        "ldaxr w3, [sp]\n"
        "ldar w10, [x21]\n"
        "stlr w25, [sp]\n"
        "stxr w23, w15, [sp]\n"
        "stlxr w20, w24, [x21]\n"
        "ldxp w13, w14, [x12]\n"
        "ldaxp w18, w15, [sp]\n"
        "stxp w26, w4, w4, [sp]\n"
        "stlxp w13, w3, w25, [x0]\n"
        "ldxrb w30, [sp]\n"
        "ldaxrb w24, [sp]\n"
        "stxrb w29, w4, [x4]\n"
        "stlxrb w30, w8, [sp]\n"
        "ldarb w12, [x25]\n"
        "stlrb w17, [sp]\n"
        "ldxrh w0, [x17]\n"
        "ldaxrh w13, [x29]\n"
        "stxrh w21, w5, [x10]\n"
        "stlxrh w2, w24, [x17]\n"
        "ldarh w5, [x12]\n"
        "stlrh w16, [sp]\n"
        "clrex\n"
        "clrex #5\n"
        "cas x1, x29, [x16]\n"
        "casb w22, w19, [x20]\n"
        "cash w24, w14, [x3]\n"
        "casa x17, x3, [sp]\n"
        "casab w16, w7, [x24]\n"
        "casah w27, w9, [sp]\n"
        "casl w5, w19, [sp]\n"
        "caslb w20, w14, [sp]\n"
        "caslh w22, w12, [sp]\n"
        "casal w15, w26, [sp]\n"
        "casalb w8, w18, [x8]\n"
        "casalh w19, w2, [sp]\n"
        "swp x4, x8, [sp]\n"
        "swpb w8, w18, [x14]\n"
        "swph w4, w30, [x8]\n"
        "swpa x25, x18, [x17]\n"
        "swpab w22, w27, [x7]\n"
        "swpah w12, w22, [x6]\n"
        "swpl w2, w4, [x25]\n"
        "swplb w12, w12, [x13]\n"
        "swplh w4, w21, [x17]\n"
        "swpal w27, w12, [sp]\n"
        "swpalb w23, w12, [x11]\n"
        "swpalh w22, w4, [sp]\n"
        "ldadd w2, w16, [sp]\n"
        "ldaddb w14, w0, [x9]\n"
        "ldaddh w24, w14, [x8]\n"
        "ldadda x25, x10, [x5]\n"
        "ldaddab w20, w26, [x3]\n"
        "ldaddah w20, w18, [x7]\n"
        "ldaddl w12, w3, [x29]\n"
        "ldaddlb w12, w21, [x29]\n"
        "ldaddlh w18, w8, [x23]\n"
        "ldaddal w25, w6, [sp]\n"
        "ldaddalb w17, w19, [sp]\n"
        "ldaddalh w5, w21, [x21]\n"
        "ldclr w30, w8, [sp]\n"
        "ldclrb w15, w3, [x21]\n"
        "ldclrh w0, w14, [x24]\n"
        "ldclra x12, x26, [sp]\n"
        "ldclrab w3, w2, [x21]\n"
        "ldclrah w26, w14, [x6]\n"
        "ldclrl w6, w27, [sp]\n"
        "ldclrlb w6, w7, [x11]\n"
        "ldclrlh w1, w14, [x1]\n"
        "ldclral w27, w29, [sp]\n"
        "ldclralb w18, w16, [sp]\n"
        "ldclralh w12, w25, [sp]\n"
        "ldeor x28, x11, [x15]\n"
        "ldeorb w29, w27, [x15]\n"
        "ldeorh w18, w23, [x10]\n"
        "ldeora w17, w27, [x8]\n"
        "ldeorab w25, w25, [sp]\n"
        "ldeorah w16, w25, [x0]\n"
        "ldeorl x16, x0, [sp]\n"
        "ldeorlb w11, w24, [x17]\n"
        "ldeorlh w18, w2, [x15]\n"
        "ldeoral w14, w10, [x16]\n"
        "ldeoralb w29, w10, [x11]\n"
        "ldeoralh w18, w4, [x18]\n"
        "ldset x27, x16, [sp]\n"
        "ldsetb w27, w8, [sp]\n"
        "ldseth w2, w24, [x20]\n"
        "ldseta w8, w24, [sp]\n"
        "ldsetab w19, w2, [x2]\n"
        "ldsetah w30, w8, [x13]\n"
        "ldsetl x17, x23, [sp]\n"
        "ldsetlb w3, w8, [sp]\n"
        "ldsetlh w16, w9, [x25]\n"
        "ldsetal w2, w17, [sp]\n"
        "ldsetalb w27, w16, [x4]\n"
        "ldsetalh w11, w25, [x23]\n"
        "ldsmax x10, x13, [x23]\n"
        "ldsmaxb w1, w22, [sp]\n"
        "ldsmaxh w28, w6, [x7]\n"
        "ldsmaxa w18, w16, [sp]\n"
        "ldsmaxab w25, w1, [sp]\n"
        "ldsmaxah w30, w19, [sp]\n"
        "ldsmaxl x29, x20, [x0]\n"
        "ldsmaxlb w1, w5, [x8]\n"
        "ldsmaxlh w24, w28, [x2]\n"
        "ldsmaxal x16, x6, [sp]\n"
        "ldsmaxalb w15, w16, [sp]\n"
        "ldsmaxalh w20, w2, [x6]\n"
        "ldsmin x19, x9, [sp]\n"
        "ldsminb w11, w0, [x15]\n"
        "ldsminh w21, w20, [sp]\n"
        "ldsmina w18, w10, [x10]\n"
        "ldsminab w6, w22, [sp]\n"
        "ldsminah w26, w19, [sp]\n"
        "ldsminl w23, w18, [sp]\n"
        "ldsminlb w5, w26, [sp]\n"
        "ldsminlh w25, w12, [sp]\n"
        "ldsminal x0, x19, [x24]\n"
        "ldsminalb w14, w28, [x11]\n"
        "ldsminalh w6, w22, [sp]\n"
        "ldumax w20, w4, [sp]\n"
        "ldumaxb w3, w11, [x27]\n"
        "ldumaxh w17, w23, [sp]\n"
        "ldumaxa x10, x10, [x9]\n"
        "ldumaxab w2, w10, [x3]\n"
        "ldumaxah w22, w9, [sp]\n"
        "ldumaxl w7, w0, [x30]\n"
        "ldumaxlb w24, w16, [sp]\n"
        "ldumaxlh w12, w13, [sp]\n"
        "ldumaxal w27, w2, [x30]\n"
        "ldumaxalb w7, w19, [x1]\n"
        "ldumaxalh w7, w22, [sp]\n"
        "ldumin x4, x23, [sp]\n"
        "lduminb w22, w21, [sp]\n"
        "lduminh w21, w4, [sp]\n"
        "ldumina x10, x27, [sp]\n"
        "lduminab w19, w3, [sp]\n"
        "lduminah w21, w8, [x13]\n"
        "lduminl w24, w2, [sp]\n"
        "lduminlb w7, w27, [sp]\n"
        "lduminlh w25, w7, [x1]\n"
        "lduminal w16, w16, [x5]\n"
        "lduminalb w1, w28, [x2]\n"
        "lduminalh w1, w13, [x23]\n"
        "stadd x0, [sp]\n"
        "staddl x0, [x19]\n"
        "stclr w15, [sp]\n"
        "stclrl x17, [sp]\n"
        "steor w5, [sp]\n"
        "steorl w7, [sp]\n"
        "stset x10, [x13]\n"
        "stsetl x5, [x16]\n"
        "stsmax w6, [x7]\n"
        "stsmaxl x25, [x3]\n"
        "stsmin w28, [x23]\n"
        "stsminl x14, [x4]\n"
        "stumax x9, [sp]\n"
        "stumaxl x2, [x14]\n"
        "stumin w21, [x5]\n"
        "stuminl x1, [x23]\n"
        "ldapr x2, [x30]\n"
        "ldaprb w30, [sp]\n"
        "nop\n"
        "yield\n"
        "wfe\n"
        "wfi\n"
        "sev\n"
        "sevl\n"
        "isb\n"
        "svc #0\n"
        "svc #0x80\n"
        "brk #1\n"
        "hint #9\n"
        "udf #3\n"
        "dmb sy\n"
        "dsb sy\n"
        "dmb ish\n"
        "dsb ish\n"
        "dmb ishld\n"
        "dsb ishld\n"
        "dmb ishst\n"
        "dsb ishst\n"
        "dmb osh\n"
        "dsb osh\n"
        "dmb nsh\n"
        "dsb nsh\n"
        "dmb ld\n"
        "dsb ld\n"
        "dmb st\n"
        "dsb st\n"
        "mrs x16, tpidr_el0\n"
        "mrs x13, tpidrro_el0\n"
        "mrs x27, fpcr\n"
        "mrs x7, fpsr\n"
        "mrs x23, nzcv\n"
        "mrs x1, cntvct_el0\n"
        "mrs x23, cntfrq_el0\n"
        "mrs x8, dczid_el0\n"
        "mrs x24, ctr_el0\n"
        "mrs x6, midr_el1\n"
        "msr tpidr_el0, x10\n"
        "msr fpcr, x11\n"
        "msr fpsr, x11\n"
        "msr nzcv, x14\n"
        "mrs x1, s3_3_c13_c0_2\n"
        "fmul s24, s24, s5\n"
        "fdiv s27, s15, s31\n"
        "fadd s21, s11, s7\n"
        "fsub s15, s4, s27\n"
        "fmax s17, s19, s21\n"
        "fmin s23, s26, s29\n"
        "fmaxnm s23, s22, s20\n"
        "fminnm s25, s30, s1\n"
        "fnmul s23, s8, s19\n"
        "fmov s10, s19\n"
        "fabs s8, s9\n"
        "fneg s10, s29\n"
        "fsqrt s9, s8\n"
        "frintn s10, s5\n"
        "frintp s16, s15\n"
        "frintm s22, s20\n"
        "frintz s10, s17\n"
        "frinta s30, s19\n"
        "frintx s4, s27\n"
        "frinti s9, s22\n"
        "fmadd s28, s6, s9, s20\n"
        "fmsub s4, s11, s30, s2\n"
        "fnmadd s2, s12, s22, s23\n"
        "fnmsub s22, s23, s21, s7\n"
        "fcmp s11, s24\n"
        "fcmpe s2, s17\n"
        "fcmp s13, #0.0\n"
        "fcmpe s3, #0.0\n"
        "fccmp s15, s19, #3, hi\n"
        "fcsel s25, s15, s23, ne\n"
        "fmov s14, #1.0\n"
        "fmov s18, #-0.5\n"
        "fmov s0, #2.0\n"
        "fmov s12, #0.125\n"
        "fmov s6, #31.0\n"
        "fmov s8, #-1.25\n"
        "fmov s14, #3.0\n"
        "fcvtns x28, s17\n"
        "fcvtnu x7, s4\n"
        "fcvtas x16, s27\n"
        "fcvtau w14, s30\n"
        "fcvtps x27, s22\n"
        "fcvtpu x25, s4\n"
        "fcvtms x7, s9\n"
        "fcvtmu x6, s1\n"
        "fcvtzs x11, s11\n"
        "fcvtzu x11, s5\n"
        "scvtf s15, w27\n"
        "ucvtf s13, x20\n"
        "fmul d12, d21, d10\n"
        "fdiv d1, d13, d20\n"
        "fadd d30, d2, d3\n"
        "fsub d23, d31, d22\n"
        "fmax d8, d31, d4\n"
        "fmin d20, d19, d20\n"
        "fmaxnm d5, d30, d21\n"
        "fminnm d26, d4, d16\n"
        "fnmul d4, d20, d1\n"
        "fmov d11, d20\n"
        "fabs d14, d20\n"
        "fneg d16, d16\n"
        "fsqrt d19, d31\n"
        "frintn d26, d0\n"
        "frintp d18, d10\n"
        "frintm d18, d3\n"
        "frintz d7, d27\n"
        "frinta d27, d13\n"
        "frintx d17, d22\n"
        "frinti d31, d18\n"
        "fmadd d16, d11, d20, d9\n"
        "fmsub d22, d6, d25, d22\n"
        "fnmadd d12, d25, d28, d9\n"
        "fnmsub d30, d15, d2, d15\n"
        "fcmp d5, d4\n"
        "fcmpe d2, d30\n"
        "fcmp d30, #0.0\n"
        "fcmpe d20, #0.0\n"
        "fccmp d10, d31, #3, ge\n"
        "fcsel d0, d24, d28, lo\n"
        "fmov d23, #1.0\n"
        "fmov d3, #-0.5\n"
        "fmov d23, #2.0\n"
        "fmov d22, #0.125\n"
        "fmov d28, #31.0\n"
        "fmov d15, #-1.25\n"
        "fmov d19, #3.0\n"
        "fcvtns w14, d22\n"
        "fcvtnu x4, d28\n"
        "fcvtas w1, d23\n"
        "fcvtau w25, d11\n"
        "fcvtps w15, d0\n"
        "fcvtpu w28, d3\n"
        "fcvtms x5, d13\n"
        "fcvtmu w14, d7\n"
        "fcvtzs x4, d10\n"
        "fcvtzu x5, d19\n"
        "scvtf d14, w13\n"
        "ucvtf d29, x17\n"
        "fmov s19, w5\n"
        "fmov w16, s19\n"
        "fmov d13, x9\n"
        "fmov x21, d9\n"
        "fmov d1, xzr\n"
        "fmov v3.d[1], x4\n"
        "fmov x5, v6.d[1]\n"
        "fcvt d1, s2\n"
        "fcvt s3, d4\n"
        "fcvt h5, s6\n"
        "fcvt s7, h8\n"
        "fcvt h1, d2\n"
        "fcvt d3, h4\n"
        "add v0.4s, v21.4s, v7.4s\n"
        "sub v24.8h, v11.8h, v28.8h\n"
        "cmeq v28.8h, v23.8h, v13.8h\n"
        "cmgt v5.8b, v6.8b, v6.8b\n"
        "cmge v24.2s, v8.2s, v28.2s\n"
        "cmhi v11.8h, v30.8h, v28.8h\n"
        "cmhs v2.2s, v12.2s, v28.2s\n"
        "addp v24.8h, v18.8h, v22.8h\n"
        "ushl v11.2d, v17.2d, v11.2d\n"
        "sshl v1.2d, v3.2d, v4.2d\n"
        "cmtst v14.2s, v28.2s, v20.2s\n"
        "mul v21.8h, v6.8h, v24.8h\n"
        "umax v29.8b, v17.8b, v26.8b\n"
        "umin v21.8h, v6.8h, v10.8h\n"
        "smax v27.8h, v30.8h, v9.8h\n"
        "smin v9.4h, v22.4h, v8.4h\n"
        "uqadd v12.2s, v14.2s, v13.2s\n"
        "sqadd v9.8h, v6.8h, v6.8h\n"
        "uqsub v3.8h, v29.8h, v9.8h\n"
        "sqsub v20.4h, v17.4h, v25.4h\n"
        "and v24.8b, v31.8b, v28.8b\n"
        "orr v19.16b, v24.16b, v20.16b\n"
        "eor v11.16b, v6.16b, v31.16b\n"
        "bic v28.8b, v9.8b, v29.8b\n"
        "orn v7.8b, v20.8b, v20.8b\n"
        "bsl v21.16b, v20.16b, v29.16b\n"
        "bit v31.16b, v25.16b, v13.16b\n"
        "bif v15.8b, v12.8b, v15.8b\n"
        "fadd v20.2s, v3.2s, v21.2s\n"
        "fsub v1.4s, v22.4s, v23.4s\n"
        "fmul v26.4s, v13.4s, v18.4s\n"
        "fdiv v20.2s, v25.2s, v24.2s\n"
        "fmax v11.2d, v0.2d, v24.2d\n"
        "fmin v22.2d, v14.2d, v14.2d\n"
        "fmaxnm v20.2s, v24.2s, v13.2s\n"
        "fminnm v18.2d, v6.2d, v27.2d\n"
        "mov v0.16b, v22.16b\n"
        "mov v5.8b, v26.8b\n"
        "not v9.16b, v7.16b\n"
        "mvn v11.8b, v21.8b\n"
        "cnt v9.8b, v24.8b\n"
        "rev64 v27.4s, v20.4s\n"
        "neg v17.2d, v13.2d\n"
        "abs v12.8h, v10.8h\n"
        "addv b1, v10.16b\n"
        "addv h2, v10.4h\n"
        "addv s3, v9.4s\n"
        "uaddlv h4, v7.8b\n"
        "umaxv b5, v28.16b\n"
        "uminv s6, v8.4s\n"
        "dup v27.4s, w4\n"
        "dup v21.2d, x19\n"
        "dup v20.4s, v3.s[2]\n"
        "dup v8.16b, v7.b[9]\n"
        "mov v1.s[2], w3\n"
        "mov v4.d[1], x5\n"
        "mov v6.b[3], v7.b[9]\n"
        "ins v8.h[1], w9\n"
        "mov w1, v2.s[3]\n"
        "mov x3, v4.d[0]\n"
        "umov w5, v6.b[2]\n"
        "umov w7, v8.h[7]\n"
        "smov x9, v10.b[1]\n"
        "smov w11, v12.h[3]\n"
        "smov x13, v14.s[1]\n"
        "mov s15, v16.s[3]\n"
        "mov d1, v2.d[1]\n"
        "movi d1, #0\n"
        "movi v1.16b, #183\n"
        "movi v11.8b, #115\n"
        "movi v2.2d, #0\n"
        "ext v15.16b, v31.16b, v31.16b, #3\n"
        "ext v2.8b, v5.8b, v8.8b, #5\n"
        "ld1 {v7.2s}, [x15]\n"
        "st1 {v7.2s}, [sp]\n"
        "ld1 {v7.2s}, [x22], #8\n"
        "st1 {v7.2s}, [sp], x2\n"
        "ld1 {v7.8h, v8.8h}, [x15]\n"
        "st1 {v7.8h, v8.8h}, [sp]\n"
        "ld1 {v7.8h, v8.8h}, [x28], #32\n"
        "st1 {v7.8h, v8.8h}, [x23], x6\n"
        "ld1 {v7.4s, v8.4s, v9.4s}, [x29]\n"
        "st1 {v7.4s, v8.4s, v9.4s}, [sp]\n"
        "ld1 {v7.4s, v8.4s, v9.4s}, [sp], #48\n"
        "st1 {v7.4s, v8.4s, v9.4s}, [x26], x2\n"
        "ld1 {v7.2d, v8.2d, v9.2d, v10.2d}, [x3]\n"
        "st1 {v7.2d, v8.2d, v9.2d, v10.2d}, [sp]\n"
        "ld1 {v7.2d, v8.2d, v9.2d, v10.2d}, [x10], #64\n"
        "st1 {v7.2d, v8.2d, v9.2d, v10.2d}, [sp], x21\n"
        "add x30, x2, #4095\n"
        "add x30, x26, #7, lsl #12\n"
        "add x8, x19, x18\n"
        "add xzr, xzr, xzr, asr #8\n"
        "add x30, x28, w27, uxtb #2\n"
        "add x30, sp, x29, sxtx\n"
        "adds x13, x17, #1\n"
        "adds x13, x7, #1, lsl #12\n"
        "adds x10, xzr, x16\n"
        "adds xzr, xzr, xzr, asr #11\n"
        "adds x13, sp, w5, sxtb #3\n"
        "adds x13, x20, x15, sxtx\n"
        "sub sp, sp, #4096\n"
        "sub xzr, x15, xzr\n"
        "sub x26, xzr, xzr, lsr #9\n"
        "sub sp, sp, w9, sxtw #1\n"
        "sub sp, sp, x29, lsl #2\n"
        "subs x19, sp, #0\n"
        "subs x19, x29, #1, lsl #12\n"
        "subs x1, x8, x21\n"
        "subs x24, xzr, x7, lsl #13\n"
        "subs x19, x28, w1, uxtw #2\n"
        "subs x19, x5, x21, uxtx\n"
        "cmp x2, #552\n"
        "cmn x0, x23\n"
        "cmp x11, x8, lsl #3\n"
        "neg x4, x26\n"
        "negs x29, x5, lsl #2\n"
        "adc x23, x16, x12\n"
        "adcs x25, x4, xzr\n"
        "sbc x30, xzr, x15\n"
        "sbcs xzr, x17, xzr\n"
        "ngc x24, x12\n"
        "ngcs x27, x19\n"
        "and x22, x15, #0xff00\n"
        "orr sp, x26, #0xff\n"
        "eor x14, xzr, #0x8000000000000000\n"
        "ands xzr, xzr, #0xf0f0f0f0f0f0f0f\n"
        "and xzr, x20, x22\n"
        "and x21, x8, x4, lsr #3\n"
        "orr x14, x16, x7\n"
        "orr xzr, xzr, x2, asr #8\n"
        "eor xzr, x8, x4\n"
        "eor xzr, x5, x16, lsr #3\n"
        "ands x3, x5, x3\n"
        "ands xzr, xzr, xzr, lsr #21\n"
        "bic x29, xzr, x13\n"
        "bic x18, xzr, x29, lsl #21\n"
        "orn xzr, x11, x16\n"
        "orn xzr, xzr, x9, ror #3\n"
        "eon xzr, x6, xzr\n"
        "eon xzr, xzr, x14, lsl #0\n"
        "bics x9, xzr, x29\n"
        "bics xzr, x6, xzr, ror #22\n"
        "tst x14, #0x5555555555555555\n"
        "tst x24, x11\n"
        "mvn x12, x28\n"
        "mov x3, x8\n"
        "mov sp, x3\n"
        "mov x3, sp\n"
        "mov x19, #1\n"
        "mov x10, #65536\n"
        "mov x20, #4294901760\n"
        "mov x12, #-1\n"
        "mov x30, #-2\n"
        "mov x6, #-65536\n"
        "mov x22, #20014547599360\n"
        "mov x3, #18446462598732840960\n"
        "mov x0, #6148914691236517205\n"
        "movz x19, #61652, lsl #16\n"
        "movk x24, #5659\n"
        "movn x23, #65267\n"
        "movk x9, #46869, lsl #48\n"
        "lsl x30, x14, #10\n"
        "lsl x25, x11, x8\n"
        "lsr x15, x16, #56\n"
        "lsr x15, x23, x30\n"
        "asr x23, x25, #27\n"
        "asr x29, x15, x26\n"
        "ror x21, x9, #26\n"
        "ror x7, x5, x15\n"
        "lslv x19, x8, x17\n"
        "lsrv x13, x22, x21\n"
        "asrv x22, x29, x2\n"
        "rorv x18, x23, x26\n"
        "udiv x18, x3, x2\n"
        "sdiv x11, x5, x17\n"
        "ubfx x25, x13, #9, #5\n"
        "sbfx x2, x29, #51, #11\n"
        "bfxil x20, x1, #51, #3\n"
        "ubfiz x9, x12, #62, #1\n"
        "sbfiz x21, x28, #45, #11\n"
        "bfi x5, x16, #28, #19\n"
        "ubfm x3, x4, #54, #12\n"
        "sbfm x10, x16, #31, #32\n"
        "bfm x5, x28, #20, #59\n"
        "bfc x30, #3, #5\n"
        "extr x22, x7, x12, #45\n"
        "madd x25, x24, x18, x23\n"
        "msub x14, x14, x23, xzr\n"
        "mul x28, x23, x5\n"
        "mneg x12, x16, x1\n"
        "rbit x15, x8\n"
        "rev16 x12, x8\n"
        "rev x22, x23\n"
        "clz x29, x13\n"
        "cls x22, x20\n"
        "csel xzr, xzr, x22, pl\n"
        "csinc x17, xzr, x28, hi\n"
        "csinv xzr, x20, x26, eq\n"
        "csneg x12, xzr, x28, hs\n"
        "cset x12, mi\n"
        "csetm x8, mi\n"
        "cinc x15, x25, cc\n"
        "cinv x0, x19, lt\n"
        "cneg x15, x8, lo\n"
        "ccmp x14, x22, #6, cs\n"
        "ccmp x11, #0, #15, cs\n"
        "ccmn x24, x18, #15, hi\n"
        "ccmn x14, #17, #14, eq\n"
        "cbz x2, 1f\n"
        "cbnz x19, 1b\n"
        "tbz x24, #44, 1f\n"
        "tbnz x5, #51, 1b\n"
        "add w8, w1, #4096\n"
        "add w8, wsp, #7, lsl #12\n"
        "add w4, wzr, w30\n"
        "add w11, wzr, wzr, lsl #27\n"
        "add w8, wsp, w15, sxtw #1\n"
        "adds wzr, wsp, #4095\n"
        "adds wzr, wsp, #7, lsl #12\n"
        "adds wzr, w16, w16\n"
        "adds w12, w25, w9, lsl #30\n"
        "adds wzr, w14, w16, uxtw #0\n"
        "sub w30, wsp, #4095\n"
        "sub w30, wsp, #7, lsl #12\n"
        "sub wzr, wzr, wzr\n"
        "sub wzr, w14, wzr, asr #6\n"
        "sub w30, wsp, w10, uxth #0\n"
        "subs w28, w15, #4095\n"
        "subs w28, wsp, #1, lsl #12\n"
        "subs w25, wzr, wzr\n"
        "subs wzr, wzr, wzr, asr #27\n"
        "subs w28, wsp, w17, sxth #0\n"
        "cmp wsp, #3237\n"
        "cmn w15, wzr\n"
        "cmp w14, w15, lsl #3\n"
        "neg w2, w28\n"
        "negs w29, w5, lsl #2\n"
        "adc wzr, w4, wzr\n"
        "adcs wzr, wzr, wzr\n"
        "sbc wzr, wzr, w15\n"
        "sbcs wzr, wzr, w3\n"
        "ngc w25, w6\n"
        "ngcs w10, w8\n"
        "and wsp, wzr, #0xff00\n"
        "orr w10, w7, #0xff\n"
        "eor wsp, w20, #0x55555555\n"
        "ands w5, wzr, #0xf0f0f0f\n"
        "and wzr, w16, wzr\n"
        "and w7, wzr, wzr, lsl #18\n"
        "orr wzr, w1, w14, lsl #29\n"
        "eor w29, wzr, wzr\n"
        "eor w25, w8, wzr, lsr #5\n"
        "ands w0, w13, wzr\n"
        "ands wzr, w7, w15, lsl #4\n"
        "bic w22, w1, w17\n"
        "bic wzr, w21, wzr, ror #0\n"
        "orn w4, w7, w14\n"
        "orn w25, w19, wzr, asr #19\n"
        "eon wzr, w28, w16\n"
        "eon w26, w20, wzr, lsr #9\n"
        "bics wzr, wzr, wzr\n"
        "bics wzr, w25, w17, ror #8\n"
        "tst w9, #0xff00ff\n"
        "tst w24, w15\n"
        "mvn w8, w7\n"
        "mov w13, w22\n"
        "mov wsp, w4\n"
        "mov w22, wsp\n"
        "mov w17, #0\n"
        "mov w21, #1\n"
        "mov w3, #65535\n"
        "mov w0, #65536\n"
        "mov w19, #4294901760\n"
        "mov w24, #-2\n"
        "mov w6, #-65536\n"
        "mov w6, #1431655765\n"
        "movz w6, #51311, lsl #16\n"
        "movk w18, #5125\n"
        "movn w20, #18100\n"
        "lsl w20, w0, #24\n"
        "lsl w8, w22, w23\n"
        "lsr w15, w17, #2\n"
        "lsr w23, w24, w27\n"
        "asr w26, w7, #27\n"
        "asr w4, w19, w10\n"
        "ror w1, w22, #7\n"
        "ror w3, w4, w20\n"
        "lslv w22, w17, w29\n"
        "lsrv w5, w24, w30\n"
        "asrv w2, w21, w28\n"
        "rorv w27, w14, w20\n"
        "udiv w9, w30, w6\n"
        "sdiv w5, w26, w10\n"
        "ubfx w26, w8, #22, #9\n"
        "sbfx w2, w13, #18, #7\n"
        "bfxil w23, w1, #21, #8\n"
        "ubfiz w21, w27, #9, #4\n"
        "sbfiz w22, w23, #20, #5\n"
        "bfi w29, w0, #30, #1\n"
        "ubfm w13, w10, #16, #25\n"
        "sbfm w18, w16, #12, #27\n"
        "bfm w24, w4, #10, #28\n"
        "bfc w27, #3, #5\n"
        "extr w14, w11, w12, #30\n"
        "madd w19, w8, w19, wzr\n"
        "msub w14, w6, w24, wzr\n"
        "mul w9, w2, w5\n"
        "mneg w11, w19, w20\n"
        "rbit w27, w15\n"
        "rev16 w7, w24\n"
        "rev w19, w20\n"
        "clz w21, w18\n"
        "cls w28, w4\n"
        "csel wzr, w27, wzr, hs\n"
        "csinc w0, wzr, wzr, vs\n"
        "csinv wzr, wzr, wzr, le\n"
        "csneg w26, w9, w6, lo\n"
        "cset w19, ge\n"
        "csetm w23, cs\n"
        "cinc w20, w18, gt\n"
        "cinv w29, w8, cs\n"
        "cneg w15, w15, pl\n"
        "ccmp w4, w18, #9, pl\n"
        "ccmp w6, #21, #12, lt\n"
        "ccmn w7, w20, #6, ne\n"
        "ccmn w8, #15, #4, ge\n"
        "cbz w26, 1f\n"
        "cbnz w13, 1b\n"
        "tbz w3, #29, 1f\n"
        "tbnz w12, #25, 1b\n"
        "sxtb x12, w9\n"
        "sxth x7, w1\n"
        "sxtw x17, w16\n"
        "uxtb w26, w28\n"
        "uxth w2, w19\n"
        "rev32 x30, x17\n"
        "rev64 x21, x0\n"
        "smaddl x1, w12, w22, x13\n"
        "smsubl x12, w7, w16, x8\n"
        "umaddl x3, w11, w16, x11\n"
        "umsubl x16, w30, w25, x15\n"
        "smull x18, w2, w22\n"
        "umull x14, w23, w22\n"
        "smnegl x7, w8, w0\n"
        "umnegl x0, w15, w1\n"
        "smulh x4, x20, x4\n"
        "umulh x6, x10, x7\n"
        "crc32b w17, w1, w19\n"
        "crc32h w4, w20, w9\n"
        "crc32w w30, w24, w3\n"
        "crc32cb w20, w17, w17\n"
        "crc32ch w2, w21, w21\n"
        "crc32cw w4, w13, w22\n"
        "crc32x w4, w1, x9\n"
        "crc32cx w16, w21, x8\n"
        "br x15\n"
        "blr x1\n"
        "ret x30\n"
        "adr x17, 1b\n"
        "adr x11, 1f\n"
        "ldr x24, [x10]\n"
        "ldr x19, [x11, #22808]\n"
        "ldr x11, [sp, #255]\n"
        "ldr x28, [x9, #-256]!\n"
        "ldr x1, [sp], #-256\n"
        "ldr x11, [x21, x27]\n"
        "ldr x21, [sp, x13, lsl #3]\n"
        "ldr x13, [x7, w5, uxtw]\n"
        "ldr x0, [sp, w21, sxtw #3]\n"
        "ldr x5, [x9, x28, sxtx]\n"
        "str x27, [x1]\n"
        "str x18, [x25, #26400]\n"
        "str x2, [sp, #-256]\n"
        "str x29, [x19, #-16]!\n"
        "str x7, [x17], #-256\n"
        "str x0, [sp, x2]\n"
        "str x16, [sp, x18, lsl #3]\n"
        "str x20, [x7, w16, sxtw]\n"
        "str x13, [x29, w4, sxtw #3]\n"
        "str x13, [sp, x23, sxtx]\n"
        "ldur x11, [x1, #194]\n"
        "stur x30, [sp, #-90]\n"
        "ldr w15, [x19]\n"
        "ldr w27, [x11, #752]\n"
        "ldr w8, [x22, #-8]\n"
        "ldr w20, [sp, #255]!\n"
        "ldr w15, [x14], #255\n"
        "ldr w13, [x8, x11]\n"
        "ldr w24, [x20, x26, lsl #2]\n"
        "ldr w12, [x19, w13, sxtw]\n"
        "ldr w29, [x26, w29, sxtw #2]\n"
        "ldr w17, [sp, x21, sxtx]\n"
        "str w8, [sp]\n"
        "str w22, [sp, #15128]\n"
        "str w29, [x21, #-8]\n"
        "str w14, [sp, #255]!\n"
        "str w3, [x16], #255\n"
        "str w3, [sp, x19]\n"
        "str w16, [x14, x30, lsl #2]\n"
        "str w23, [sp, w14, sxtw]\n"
        "str w29, [sp, w5, sxtw #2]\n"
        "str w4, [x7, x13, sxtx]\n"
        "ldur w14, [x15, #160]\n"
        "stur w8, [sp, #158]\n"
        "ldrb w9, [sp, #2568]\n"
        "ldrb w21, [sp, #-215]!\n"
        "ldrb w6, [x14], #-139\n"
        "ldrb w0, [sp, x10]\n"
        "ldrb w19, [sp, x5, lsl #0]\n"
        "strb w23, [sp, #659]\n"
        "strb w16, [sp, #155]!\n"
        "strb w19, [x29], #199\n"
        "strb w28, [x3, x11]\n"
        "strb w0, [sp, x4, lsl #0]\n"
        "ldrh w22, [x22, #154]\n"
        "ldrh w10, [sp, #-251]!\n"
        "ldrh w28, [x15], #-28\n"
        "ldrh w23, [sp, x5]\n"
        "ldrh w10, [sp, x18, lsl #1]\n"
        "strh w19, [sp, #7624]\n"
        "strh w24, [x8, #-38]!\n"
        "strh w18, [x11], #114\n"
        "strh w27, [x11, x28]\n"
        "strh w7, [x17, x6, lsl #1]\n"
        "ldrsb w7, [sp, #150]\n"
        "ldrsb x27, [x5, x12]\n"
        "ldrsh w23, [x11, #172]\n"
        "ldrsh x22, [x19, x27]\n"
        "ldrsw x15, [x19, #4376]\n"
        "ldrsw x29, [x6], #4\n"
        "ldrsw x0, [x19, x14, lsl #2]\n"
        "ldurb w24, [x20, #-67]\n"
        "sturb w8, [x12, #-246]\n"
        "ldurh w11, [x28, #56]\n"
        "sturh w1, [sp, #227]\n"
        "ldursb w12, [x3, #217]\n"
        "ldursh w5, [sp, #-192]\n"
        "ldursw x8, [sp, #-4]\n"
        "prfm pldl1keep, [x17]\n"
        "prfm pstl2strm, [x2, #64]\n"
        "prfum pldl3keep, [x29, #-1]\n"
        "ldp x25, x28, [sp, #72]\n"
        "ldp x29, x0, [x17, #-360]!\n"
        "ldp x13, x29, [x4], #112\n"
        "stp x14, x29, [x6, #176]\n"
        "stp x14, x25, [x1, #-48]!\n"
        "stp x27, x19, [sp], #-296\n"
        "ldnp x27, x6, [x4, #24]\n"
        "stnp x0, x10, [sp]\n"
        "ldp w3, w11, [x24, #-120]\n"
        "ldp w11, w4, [sp, #-56]!\n"
        "ldp w9, w17, [x24], #-240\n"
        "stp w10, w20, [x4, #-208]\n"
        "stp w4, w23, [sp, #108]!\n"
        "stp w15, w21, [sp], #-192\n"
        "ldnp w28, w20, [x4, #12]\n"
        "stnp w0, w26, [x2]\n"
        "ldpsw x11, x28, [x27, #8]\n"
        "ldp s19, s8, [x2, #-24]\n"
        "stp s21, s5, [x28, #188]\n"
        "ldp d13, d26, [x20, #168]\n"
        "stp d27, d31, [sp, #-136]\n"
        "ldp q7, q2, [sp, #-432]\n"
        "stp q26, q14, [sp, #-528]\n"
        "ldr b5, [x16, #1628]\n"
        "ldr b4, [sp, #-124]!\n"
        "ldr b15, [sp, x21]\n"
        "str b25, [x14, #2569]\n"
        "str b14, [sp, #-147]!\n"
        "str b25, [x0, x23]\n"
        "ldur b16, [x11, #-3]\n"
        "ldr h21, [x13, #712]\n"
        "ldr h8, [x5, #-81]!\n"
        "ldr h25, [x4, x20, lsl #1]\n"
        "str h19, [sp, #7594]\n"
        "str h0, [sp, #-255]!\n"
        "str h6, [x26, x6, lsl #1]\n"
        "ldur h27, [x0, #-3]\n"
        "ldr s24, [sp, #5064]\n"
        "ldr s11, [sp, #7]!\n"
        "ldr s14, [sp, x9, lsl #2]\n"
        "str s26, [sp, #12656]\n"
        "str s18, [x12, #174]!\n"
        "str s18, [sp, x16, lsl #2]\n"
        "ldur s27, [sp, #-3]\n"
        "ldr d4, [x18, #16736]\n"
        "ldr d26, [sp, #-165]!\n"
        "ldr d30, [sp, x5, lsl #3]\n"
        "str d13, [x11, #14544]\n"
        "str d13, [x13, #-61]!\n"
        "str d24, [x28, x27, lsl #3]\n"
        "ldur d4, [sp, #-3]\n"
        "ldr q9, [x24, #48960]\n"
        "ldr q11, [x7, #190]!\n"
        "ldr q2, [sp, x17, lsl #4]\n"
        "str q19, [x7, #52224]\n"
        "str q14, [x28, #-22]!\n"
        "str q16, [sp, x19, lsl #4]\n"
        "ldur q16, [x5, #-3]\n"
        "ldxr x20, [sp]\n"
        "ldaxr x16, [x29]\n"
        "ldar x1, [sp]\n"
        "stlr x6, [sp]\n"
        "stxr w30, x9, [x23]\n"
        "stlxr w14, x10, [sp]\n"
        "ldxp x1, x6, [sp]\n"
        "ldaxp x23, x14, [sp]\n"
        "stxp w1, x23, x10, [sp]\n"
        "stlxp w20, x7, x5, [sp]\n"
        "ldxr w4, [sp]\n"
        "ldaxr w2, [x21]\n"
        "ldar w24, [x25]\n"
        "stlr w13, [x28]\n"
        "stxr w30, w17, [x18]\n"
        "stlxr w0, w0, [x11]\n"
        "ldxp w21, w20, [sp]\n"
        "ldaxp w13, w30, [sp]\n"
        "stxp w3, w26, w0, [x2]\n"
        "stlxp w0, w23, w16, [x27]\n"
        "ldxrb w4, [sp]\n"
        "ldaxrb w8, [x3]\n"
        "stxrb w20, w18, [sp]\n"
        "stlxrb w28, w8, [sp]\n"
        "ldarb w21, [x28]\n"
        "stlrb w13, [x0]\n"
        "ldxrh w6, [x28]\n"
        "ldaxrh w28, [sp]\n"
        "stxrh w17, w27, [sp]\n"
        "stlxrh w3, w7, [x18]\n"
        "ldarh w1, [x6]\n"
        "stlrh w21, [x26]\n"
        "cas w26, w0, [x5]\n"
        "casb w20, w7, [sp]\n"
        "cash w28, w20, [x6]\n"
        "casa w30, w7, [sp]\n"
        "casab w5, w6, [sp]\n"
        "casah w2, w7, [sp]\n"
        "casl w9, w17, [sp]\n"
        "caslb w1, w24, [x9]\n"
        "caslh w16, w21, [sp]\n"
        "casal x29, x19, [sp]\n"
        "casalb w26, w3, [x30]\n"
        "casalh w7, w29, [x19]\n"
        "swp x2, x19, [sp]\n"
        "swpb w12, w6, [sp]\n"
        "swph w4, w13, [x30]\n"
        "swpa x10, x9, [x13]\n"
        "swpab w8, w15, [sp]\n"
        "swpah w23, w29, [x26]\n"
        "swpl x6, x9, [sp]\n"
        "swplb w24, w7, [x6]\n"
        "swplh w1, w0, [sp]\n"
        "swpal x18, x29, [sp]\n"
        "swpalb w30, w0, [x24]\n"
        "swpalh w29, w27, [x29]\n"
        "ldadd w2, w28, [sp]\n"
        "ldaddb w17, w29, [x8]\n"
        "ldaddh w25, w5, [sp]\n"
        "ldadda x19, x2, [x5]\n"
        "ldaddab w21, w3, [sp]\n"
        "ldaddah w20, w28, [x18]\n"
        "ldaddl w7, w26, [sp]\n"
        "ldaddlb w7, w28, [sp]\n"
        "ldaddlh w27, w3, [x5]\n"
        "ldaddal x27, x8, [sp]\n"
        "ldaddalb w8, w19, [x19]\n"
        "ldaddalh w12, w7, [x18]\n"
        "ldclr x15, x14, [sp]\n"
        "ldclrb w27, w10, [x22]\n"
        "ldclrh w13, w14, [x23]\n"
        "ldclra w13, w17, [x20]\n"
        "ldclrab w25, w16, [sp]\n"
        "ldclrah w14, w9, [sp]\n"
        "ldclrl w24, w21, [x28]\n"
        "ldclrlb w7, w27, [x3]\n"
        "ldclrlh w22, w4, [sp]\n"
        "ldclral w1, w21, [x21]\n"
        "ldclralb w1, w1, [x29]\n"
        "ldclralh w29, w29, [sp]\n"
        "ldeor w26, w5, [sp]\n"
        "ldeorb w21, w2, [sp]\n"
        "ldeorh w19, w27, [x5]\n"
        "ldeora x7, x0, [x20]\n"
        "ldeorab w1, w26, [x7]\n"
        "ldeorah w28, w16, [sp]\n"
        "ldeorl x24, x30, [x0]\n"
        "ldeorlb w27, w16, [x19]\n"
        "ldeorlh w3, w14, [x14]\n"
        "ldeoral w9, w19, [x30]\n"
        "ldeoralb w27, w3, [x16]\n"
        "ldeoralh w23, w25, [sp]\n"
        "ldset w12, w10, [x19]\n"
        "ldsetb w7, w4, [sp]\n"
        "ldseth w1, w28, [x25]\n"
        "ldseta w1, w6, [x11]\n"
        "ldsetab w21, w15, [x12]\n"
        "ldsetah w19, w20, [x13]\n"
        "ldsetl x16, x16, [x7]\n"
        "ldsetlb w25, w8, [sp]\n"
        "ldsetlh w21, w8, [x14]\n"
        "ldsetal x0, x19, [x27]\n"
        "ldsetalb w23, w18, [x29]\n"
        "ldsetalh w13, w13, [sp]\n"
        "ldsmax w8, w13, [x24]\n"
        "ldsmaxb w0, w5, [sp]\n"
        "ldsmaxh w30, w26, [sp]\n"
        "ldsmaxa x29, x10, [x1]\n"
        "ldsmaxab w23, w9, [x5]\n"
        "ldsmaxah w15, w16, [sp]\n"
        "ldsmaxl x10, x16, [sp]\n"
        "ldsmaxlb w12, w2, [x12]\n"
        "ldsmaxlh w16, w4, [sp]\n"
        "ldsmaxal x24, x27, [x21]\n"
        "ldsmaxalb w12, w14, [sp]\n"
        "ldsmaxalh w26, w15, [sp]\n"
        "ldsmin w11, w20, [x13]\n"
        "ldsminb w2, w0, [sp]\n"
        "ldsminh w9, w26, [x30]\n"
        "ldsmina w24, w9, [x12]\n"
        "ldsminab w13, w25, [sp]\n"
        "ldsminah w12, w8, [sp]\n"
        "ldsminl x9, x0, [x6]\n"
        "ldsminlb w10, w25, [x8]\n"
        "ldsminlh w22, w10, [sp]\n"
        "ldsminal w0, w1, [x3]\n"
        "ldsminalb w3, w12, [x9]\n"
        "ldsminalh w12, w30, [sp]\n"
        "ldumax w10, w2, [sp]\n"
        "ldumaxb w22, w17, [sp]\n"
        "ldumaxh w22, w8, [x26]\n"
        "ldumaxa x7, x20, [sp]\n"
        "ldumaxab w6, w21, [sp]\n"
        "ldumaxah w16, w8, [sp]\n"
        "ldumaxl x18, x29, [sp]\n"
        "ldumaxlb w3, w26, [x5]\n"
        "ldumaxlh w25, w17, [x7]\n"
        "ldumaxal w20, w2, [sp]\n"
        "ldumaxalb w10, w16, [x24]\n"
        "ldumaxalh w7, w3, [sp]\n"
        "ldumin w14, w20, [sp]\n"
        "lduminb w4, w14, [x14]\n"
        "lduminh w25, w16, [x12]\n"
        "ldumina w7, w19, [sp]\n"
        "lduminab w4, w28, [sp]\n"
        "lduminah w21, w28, [x21]\n"
        "lduminl x11, x22, [x26]\n"
        "lduminlb w13, w10, [x17]\n"
        "lduminlh w21, w4, [sp]\n"
        "lduminal w7, w20, [x18]\n"
        "lduminalb w17, w22, [sp]\n"
        "lduminalh w17, w6, [x0]\n"
        "stadd w10, [sp]\n"
        "staddl x18, [sp]\n"
        "stclr w10, [x4]\n"
        "stclrl w18, [sp]\n"
        "steor w28, [sp]\n"
        "steorl w5, [x28]\n"
        "stset x3, [x14]\n"
        "stsetl x2, [x9]\n"
        "stsmax x13, [x3]\n"
        "stsmaxl x12, [sp]\n"
        "stsmin w7, [sp]\n"
        "stsminl w6, [x2]\n"
        "stumax w2, [x11]\n"
        "stumaxl x22, [sp]\n"
        "stumin w29, [sp]\n"
        "stuminl w16, [x8]\n"
        "ldapr x0, [x16]\n"
        "ldaprb w10, [x17]\n"
        "mrs x7, tpidr_el0\n"
        "mrs x26, tpidrro_el0\n"
        "mrs x7, fpcr\n"
        "mrs x24, nzcv\n"
        "mrs x13, cntvct_el0\n"
        "mrs x22, cntfrq_el0\n"
        "mrs x13, dczid_el0\n"
        "mrs x9, ctr_el0\n"
        "mrs x4, midr_el1\n"
        "msr tpidr_el0, x21\n"
        "msr fpcr, x9\n"
        "msr fpsr, x21\n"
        "msr nzcv, x0\n"
        "fmul s27, s29, s8\n"
        "fdiv s19, s21, s6\n"
        "fadd s9, s25, s23\n"
        "fsub s2, s26, s31\n"
        "fmax s24, s5, s5\n"
        "fmin s31, s29, s30\n"
        "fmaxnm s2, s11, s15\n"
        "fminnm s22, s25, s20\n"
        "fnmul s20, s16, s29\n"
        "fmov s26, s26\n"
        "fneg s12, s13\n"
        "fsqrt s13, s6\n"
        "frintn s2, s3\n"
        "frintp s26, s4\n"
        "frintm s25, s29\n"
        "frintz s11, s20\n"
        "frinta s24, s8\n"
        "frintx s0, s1\n"
        "frinti s25, s26\n"
        "fmadd s9, s11, s19, s9\n"
        "fmsub s6, s7, s11, s13\n"
        "fnmadd s13, s12, s7, s21\n"
        "fnmsub s2, s15, s21, s2\n"
        "fcmp s9, s21\n"
        "fcmpe s2, s18\n"
        "fcmp s15, #0.0\n"
        "fcmpe s8, #0.0\n"
        "fccmp s15, s15, #3, gt\n"
        "fcsel s3, s25, s19, le\n"
        "fmov s2, #1.0\n"
        "fmov s11, #-0.5\n"
        "fmov s15, #2.0\n"
        "fmov s5, #0.125\n"
        "fmov s22, #31.0\n"
        "fmov s1, #-1.25\n"
        "fmov s30, #3.0\n"
        "fcvtns w0, s27\n"
        "fcvtnu x12, s21\n"
        "fcvtas w15, s6\n"
        "fcvtau x25, s4\n"
        "fcvtps w20, s14\n"
        "fcvtpu x15, s28\n"
        "fcvtms w10, s28\n"
        "fcvtmu x24, s19\n"
        "fcvtzs x19, s18\n"
        "fcvtzu w29, s31\n"
        "scvtf s19, w3\n"
        "ucvtf s1, w28\n"
        "fmul d27, d7, d20\n"
        "fdiv d3, d19, d1\n"
        "fadd d13, d9, d22\n"
        "fsub d22, d9, d5\n"
        "fmax d17, d7, d4\n"
        "fmin d9, d15, d0\n"
        "fmaxnm d18, d10, d2\n"
        "fminnm d24, d22, d5\n"
        "fnmul d19, d31, d29\n"
        "fmov d10, d3\n"
        "fabs d15, d25\n"
        "fneg d16, d21\n"
        "fsqrt d11, d29\n"
        "frintn d21, d5\n"
        "frintp d18, d19\n"
        "frintm d13, d4\n"
        "frintz d4, d21\n"
        "frinta d30, d0\n"
        "frintx d8, d23\n"
        "frinti d23, d18\n"
        "fmadd d30, d0, d9, d22\n"
        "fmsub d10, d17, d16, d16\n"
        "fnmadd d9, d1, d26, d8\n"
        "fnmsub d25, d24, d7, d22\n"
        "fcmp d26, d20\n"
        "fcmpe d16, d20\n"
        "fcmp d31, #0.0\n"
        "fcmpe d2, #0.0\n"
        "fccmp d19, d25, #3, vc\n"
        "fcsel d4, d4, d30, cc\n"
        "fmov d11, #1.0\n"
        "fmov d7, #-0.5\n"
        "fmov d26, #2.0\n"
        "fmov d16, #0.125\n"
        "fmov d24, #31.0\n"
        "fmov d1, #-1.25\n"
        "fmov d28, #3.0\n"
        "fcvtns w1, d8\n"
        "fcvtas w27, d1\n"
        "fcvtau x23, d15\n"
        "fcvtps w16, d4\n"
        "fcvtpu w9, d9\n"
        "fcvtms x27, d29\n"
        "fcvtmu x28, d18\n"
        "fcvtzs x23, d4\n"
        "fcvtzu x1, d15\n"
        "scvtf d16, x0\n"
        "ucvtf d3, w16\n"
        "fmov s16, w12\n"
        "fmov w14, s0\n"
        "fmov d22, x19\n"
        "fmov x21, d22\n"
        "add v16.16b, v0.16b, v3.16b\n"
        "sub v25.4h, v1.4h, v15.4h\n"
        "cmeq v7.8h, v29.8h, v1.8h\n"
        "cmgt v28.4s, v21.4s, v10.4s\n"
        "cmge v20.8h, v18.8h, v20.8h\n"
        "cmhi v9.4h, v25.4h, v19.4h\n"
        "cmhs v17.8b, v20.8b, v26.8b\n"
        "addp v18.2s, v10.2s, v5.2s\n"
        "ushl v8.2d, v19.2d, v25.2d\n"
        "sshl v31.4h, v4.4h, v16.4h\n"
        "cmtst v29.4s, v18.4s, v7.4s\n"
        "mul v17.4s, v19.4s, v6.4s\n"
        "umax v6.2s, v26.2s, v2.2s\n"
        "umin v26.8b, v13.8b, v20.8b\n"
        "smax v29.16b, v18.16b, v29.16b\n"
        "smin v23.4h, v25.4h, v21.4h\n"
        "uqadd v30.8b, v15.8b, v30.8b\n"
        "sqadd v12.4s, v28.4s, v13.4s\n"
        "uqsub v25.8b, v9.8b, v25.8b\n"
        "sqsub v24.2s, v10.2s, v25.2s\n"
        "and v0.16b, v24.16b, v26.16b\n"
        "orr v3.16b, v19.16b, v6.16b\n"
        "eor v16.8b, v18.8b, v0.8b\n"
        "bic v21.8b, v1.8b, v23.8b\n"
        "orn v24.16b, v27.16b, v14.16b\n"
        "bsl v2.8b, v31.8b, v28.8b\n"
        "bit v12.8b, v27.8b, v30.8b\n"
        "bif v6.8b, v4.8b, v27.8b\n"
        "fadd v27.4s, v20.4s, v17.4s\n"
        "fsub v12.2d, v27.2d, v5.2d\n"
        "fmul v7.2d, v12.2d, v0.2d\n"
        "fdiv v28.2d, v5.2d, v26.2d\n"
        "fmax v11.2d, v4.2d, v22.2d\n"
        "fmin v30.4s, v24.4s, v17.4s\n"
        "fmaxnm v12.2s, v0.2s, v30.2s\n"
        "fminnm v3.2d, v8.2d, v25.2d\n"
        "mov v30.16b, v27.16b\n"
        "mov v23.8b, v10.8b\n"
        "not v12.16b, v18.16b\n"
        "mvn v25.8b, v26.8b\n"
        "cnt v3.8b, v26.8b\n"
        "rev64 v26.4s, v12.4s\n"
        "neg v25.2d, v23.2d\n"
        "abs v8.8h, v31.8h\n"
        "addv b1, v12.16b\n"
        "addv h2, v21.4h\n"
        "addv s3, v5.4s\n"
        "uaddlv h4, v23.8b\n"
        "umaxv b5, v2.16b\n"
        "uminv s6, v7.4s\n"
        "dup v8.4s, w21\n"
        "dup v10.2d, x14\n"
        "dup v17.4s, v3.s[2]\n"
        "dup v28.16b, v7.b[9]\n"
        "movi v6.16b, #58\n"
        "movi v20.8b, #0\n"
        "ext v31.16b, v19.16b, v16.16b, #3\n"
        "ext v13.8b, v12.8b, v18.8b, #5\n"
        "ld1 {v7.8h}, [x5]\n"
        "st1 {v7.8h}, [sp]\n"
        "ld1 {v7.8h}, [sp], #16\n"
        "st1 {v7.8h}, [x25], x27\n"
        "ld1 {v7.4h, v8.4h}, [sp]\n"
        "st1 {v7.4h, v8.4h}, [sp]\n"
        "ld1 {v7.4h, v8.4h}, [sp], #16\n"
        "st1 {v7.4h, v8.4h}, [sp], x24\n"
        "ld1 {v7.4s, v8.4s, v9.4s}, [sp]\n"
        "st1 {v7.4s, v8.4s, v9.4s}, [sp], x3\n"
        "ld1 {v7.2d, v8.2d, v9.2d, v10.2d}, [sp]\n"
        "st1 {v7.2d, v8.2d, v9.2d, v10.2d}, [x12]\n"
        "ld1 {v7.2d, v8.2d, v9.2d, v10.2d}, [sp], #64\n"
        "st1 {v7.2d, v8.2d, v9.2d, v10.2d}, [sp], x0\n"
        "add x7, sp, #16773120\n"
        "add x7, x2, #7, lsl #12\n"
        "add x8, xzr, xzr\n"
        "add x12, x7, xzr, asr #0\n"
        "add x7, x21, w5, sxth #0\n"
        "add x7, x9, x26, sxtx\n"
        "adds xzr, sp, #4096\n"
        "adds xzr, sp, #1, lsl #12\n"
        "adds xzr, x3, xzr\n"
        "adds xzr, xzr, xzr, lsr #16\n"
        "adds xzr, sp, w18, uxtw #4\n"
        "adds xzr, sp, x18, uxtx\n"
        "sub sp, x21, #4095\n"
        "sub sp, x19, #7, lsl #12\n"
        "sub x30, x22, xzr\n"
        "sub x9, xzr, xzr, lsl #11\n"
        "sub sp, x25, w13, uxth #0\n"
        "sub sp, sp, x24, sxtx\n"
        "subs x27, x1, #4096\n"
        "subs x27, sp, #7, lsl #12\n"
        "subs x16, xzr, x0\n"
        "subs x3, x30, xzr, asr #8\n"
        "subs x27, x4, w27, uxtw #2\n"
        "subs x27, x11, x28, sxtx\n"
        "cmp sp, #3163\n"
        "cmn x20, xzr\n"
        "cmp x13, x20, lsl #3\n"
        "neg x23, x22\n"
        "negs x7, x29, lsl #2\n"
        "adc xzr, xzr, xzr\n"
        "adcs xzr, xzr, xzr\n"
        "sbc x19, xzr, xzr\n"
        "sbcs xzr, xzr, xzr\n"
        "ngc x0, x18\n"
        "ngcs x1, x30\n"
        "and x21, xzr, #0xf0f0f0f0f0f0f0f\n"
        "orr sp, xzr, #0x5555555555555555\n"
        "eor x11, xzr, #0xf0f0f0f0f0f0f0f\n"
        "ands xzr, x3, #0x5555555555555555\n"
        "and x9, xzr, xzr\n"
        "and xzr, x20, xzr, asr #21\n"
        "orr x26, x14, xzr\n"
        "orr x23, xzr, x8, lsl #1\n"
        "eor x16, x10, xzr\n"
        "eor x10, xzr, x18, ror #9\n"
        "ands xzr, xzr, xzr\n"
        "ands xzr, xzr, xzr, lsr #6\n"
        "bic xzr, xzr, x17\n"
        "bic xzr, xzr, xzr, lsr #27\n"
        "orn xzr, x3, x1\n"
        "orn x28, xzr, x18, lsl #15\n"
        "eon x23, xzr, x7\n"
        "eon xzr, x1, x10, asr #27\n"
        "bics xzr, x25, x11\n"
        "bics x6, x17, x5, asr #29\n"
        "tst x28, #0x5555555555555555\n"
        "tst x26, x29\n"
        "mvn x0, x15\n"
        "mov x20, x18\n"
        "mov sp, x27\n"
        "mov x12, sp\n"
        "mov x1, #0\n"
        "mov x24, #1\n"
        "mov x8, #65535\n"
        "mov x7, #65536\n"
        "mov x8, #4294901760\n"
        "mov x19, #-1\n"
        "mov x16, #-2\n"
        "mov x16, #-65536\n"
        "mov x13, #20014547599360\n"
        "mov x1, #18446462598732840960\n"
        "mov x15, #6148914691236517205\n"
        "movz x10, #237, lsl #16\n"
        "movk x27, #7187\n"
        "movn x24, #16628\n"
        "movk x1, #16335, lsl #48\n"
        "lsl x1, x2, #31\n"
        "lsl x1, x27, x22\n"
        "lsr x2, x16, #33\n"
        "lsr x15, x10, x5\n"
        "asr x10, x2, #23\n"
        "asr x12, x20, x12\n"
        "ror x18, x9, #24\n"
        "ror x8, x6, x10\n"
        "lslv x13, x3, x4\n"
        "lsrv x17, x0, x22\n"
        "asrv x23, x12, x25\n"
        "rorv x2, x18, x5\n"
        "udiv x1, x11, x14\n"
        "sdiv x19, x20, x25\n"
        "ubfx x12, x20, #34, #26\n"
        "sbfx x19, x28, #2, #28\n"
        "bfxil x11, x20, #3, #32\n"
        "ubfiz x22, x10, #48, #14\n"
        "sbfiz x22, x13, #60, #4\n"
        "bfi x7, x6, #1, #35\n"
        "ubfm x8, x22, #9, #54\n"
        "sbfm x7, x13, #16, #3\n"
        "bfm x29, x10, #47, #33\n"
        "bfc x3, #3, #5\n"
        "extr x14, x22, x3, #48\n"
        "madd x21, x3, x23, x10\n"
        "msub x25, x18, x22, xzr\n"
        "mul x4, x7, x24\n"
        "mneg x12, x1, x16\n"
        "rbit x2, x18\n"
        "rev16 x3, x21\n"
        "rev x28, x12\n"
        "clz x5, x26\n"
        "cls x0, x10\n"
        "csel x26, x0, xzr, vc\n"
        "csinc xzr, x25, x1, hs\n"
        "csinv x17, x29, xzr, lo\n"
        "csneg x26, x7, x20, gt\n"
        "cset x19, ge\n"
        "csetm x8, ls\n"
        "cinc x19, x12, ls\n"
        "cinv x17, x13, cs\n"
        "cneg x12, x16, pl\n"
        "ccmp x29, x13, #5, lt\n"
        "ccmp x22, #30, #4, ge\n"
        "ccmn x28, x28, #4, lo\n"
        "ccmn x3, #31, #15, gt\n"
        "cbz x18, 1f\n"
        "cbnz x23, 1b\n"
        "tbz x27, #23, 1f\n"
        "tbnz x4, #34, 1b\n"
        "add w24, wsp, #1\n"
        "add w24, wsp, #7, lsl #12\n"
        "add wzr, w28, w9\n"
        "add wzr, wzr, w6, asr #11\n"
        "add w24, wsp, w15, uxth #3\n"
        "adds wzr, w22, #4096\n"
        "adds wzr, w18, #7, lsl #12\n"
        "adds w23, wzr, w25\n"
        "adds w14, w28, w20, lsl #27\n"
        "adds wzr, w8, w28, uxth #2\n"
        "sub w4, w19, #4095\n"
        "sub w4, w5, #7, lsl #12\n"
        "sub wzr, w2, w3\n"
        "sub wzr, wzr, wzr, lsl #0\n"
        "sub w4, wsp, w13, sxtb #3\n"
        "subs w2, wsp, #4096\n"
        "subs w2, wsp, #1, lsl #12\n"
        "subs wzr, wzr, wzr\n"
        "subs wzr, wzr, wzr, lsr #9\n"
        "subs w2, wsp, w3, uxtw #4\n"
        "cmp w17, #4045\n"
        "cmn wzr, wzr\n"
        "cmp w21, w23, lsl #3\n"
        "neg w18, w23\n"
        "negs w5, w20, lsl #2\n"
        "adc w20, wzr, wzr\n"
        "adcs w3, w29, wzr\n"
        "sbc wzr, wzr, w20\n"
        "sbcs wzr, w9, w22\n"
        "ngc w3, w22\n"
        "ngcs w5, w24\n"
        "and wsp, w11, #0xf0f0f0f\n"
        "orr wsp, w1, #0xff\n"
        "eor wsp, wzr, #0xfffffffe\n"
        "ands wzr, w14, #0xfffffffe\n"
        "and w15, wzr, w18\n"
        "and w21, w18, w5, ror #21\n"
        "orr wzr, w26, w18\n"
        "orr wzr, w25, w17, lsr #6\n"
        "eor wzr, wzr, wzr\n"
        "eor wzr, wzr, wzr, ror #16\n"
        "ands wzr, wzr, w0\n"
        "ands w23, w3, w15, ror #6\n"
        "bic w6, wzr, wzr\n"
        "bic w30, wzr, w25, lsl #0\n"
        "orn w11, w16, w15\n"
        "orn wzr, wzr, wzr, lsr #2\n"
        "eon w27, w22, wzr\n"
        "eon w25, w29, w27, ror #22\n"
        "bics w24, wzr, w3\n"
        "bics w13, w27, w22, ror #15\n"
        "tst w15, #0xff00\n"
        "tst w17, w27\n"
        "mvn w13, w6\n"
        "mov w20, w15\n"
        "mov wsp, w9\n"
        "mov w0, wsp\n"
        "mov w14, #0\n"
        "mov w14, #1\n"
        "mov w24, #65535\n"
        "mov w22, #65536\n"
        "mov w14, #-1\n"
        "mov w5, #-2\n"
        "mov w14, #-65536\n"
        "mov w28, #1431655765\n"
        "movz w1, #33636, lsl #16\n"
        "movk w11, #48624\n"
        "movn w14, #47386\n"
        "lsl w19, w12, #8\n"
        "lsl w0, w25, w6\n"
        "lsr w8, w25, #12\n"
        "lsr w4, w27, w14\n"
        "asr w17, w6, #6\n"
        "asr w6, w0, w5\n"
        "ror w18, w12, #17\n"
        "ror w5, w20, w0\n"
        "lslv w4, w3, w19\n"
        "lsrv w5, w14, w15\n"
        "asrv w5, w1, w26\n"
        "rorv w0, w12, w14\n"
        "udiv w10, w13, w1\n"
        "sdiv w22, w22, w1\n"
        "ubfx w12, w1, #7, #13\n"
        "sbfx w0, w29, #15, #8\n"
        "bfxil w3, w12, #7, #16\n"
        "ubfiz w5, w10, #6, #20\n"
        "sbfiz w11, w29, #3, #4\n"
        "bfi w1, w25, #19, #12\n"
        "ubfm w9, w8, #29, #19\n"
        "sbfm w15, w7, #17, #1\n"
        "bfm w26, w10, #22, #20\n"
        "bfc w2, #3, #5\n"
        "extr w1, w21, w13, #5\n"
        "madd w18, w19, w0, w3\n"
        "msub w21, w2, w0, w5\n"
        "mul w15, w1, w6\n"
        "mneg w20, w16, w28\n"
        "rbit w10, w6\n"
        "rev16 w28, w24\n"
        "rev w15, w10\n"
        "clz w25, w15\n"
        "cls w29, w11\n"
        "csel w30, wzr, wzr, cs\n"
        "csinc wzr, wzr, wzr, hi\n"
        "csinv wzr, wzr, wzr, lo\n"
        "csneg wzr, w14, wzr, ls\n"
        "cset w8, lo\n"
        "csetm w16, ge\n"
        "cinc w30, w15, ne\n"
        "cinv w4, w5, eq\n"
        "cneg w26, w14, cs\n"
        "ccmp w24, w22, #3, hi\n"
        "ccmp w7, #3, #1, gt\n"
        "ccmn w14, w28, #10, ls\n"
        "ccmn w0, #4, #6, ge\n"
        "cbz w25, 1f\n"
        "cbnz w30, 1b\n"
        "tbz w29, #6, 1f\n"
        "tbnz w10, #19, 1b\n"
        "sxtb x2, w26\n"
        "sxth w7, w22\n"
        "sxtw x1, w4\n"
        "uxtb w29, w20\n"
        "uxth w4, w18\n"
        "rev32 x0, x3\n"
        "rev64 x7, x9\n"
        "smaddl x30, w6, w7, x27\n"
        "smsubl x17, w16, w13, x16\n"
        "umaddl x24, w29, w19, x10\n"
        "umsubl x25, w28, w17, x25\n"
        "smull x6, w14, w5\n"
        "umull x19, w2, w1\n"
        "smnegl x26, w25, w3\n"
        "umnegl x19, w0, w27\n"
        "smulh x30, x3, x6\n"
        "umulh x30, x28, x8\n"
        "crc32b w2, w3, w14\n"
        "crc32h w12, w7, w26\n"
        "crc32w w26, w21, w19\n"
        "crc32cb w3, w20, w15\n"
        "crc32ch w24, w21, w27\n"
        "crc32cw w22, w11, w12\n"
        "crc32x w19, w21, x14\n"
        "crc32cx w24, w3, x9\n"
        "br x26\n"
        "blr x30\n"
        "ret x19\n"
        "adr x14, 1b\n"
        "adr x26, 1f\n"
        "ldr x12, [x6]\n"
        "ldr x17, [sp, #19608]\n"
        "ldr x23, [x20, #3]\n"
        "ldr x11, [sp, #-256]!\n"
        "ldr x29, [sp], #255\n"
        "ldr x25, [x20, x19]\n"
        "ldr x16, [x6, x11, lsl #3]\n"
        "ldr x26, [sp, w7, sxtw]\n"
        "ldr x14, [sp, w6, sxtw #3]\n"
        "ldr x10, [x5, x23, sxtx]\n"
        "str x6, [sp]\n"
        "str x29, [sp, #24096]\n"
        "str x30, [sp, #255]\n"
        "str x28, [sp, #255]!\n"
        "str x18, [x29], #8\n"
        "str x30, [x29, x5]\n"
        "str x28, [x3, x23, lsl #3]\n"
        "str x4, [x9, w20, sxtw]\n"
        "str x1, [x6, w8, sxtw #3]\n"
        "str x15, [x13, x11, sxtx]\n"
        "ldur x14, [x23, #38]\n"
        "stur x4, [sp, #50]\n"
        "ldr w29, [sp]\n"
        "ldr w2, [x6, #15968]\n"
        "ldr w24, [sp, #255]\n"
        "ldr w25, [sp, #-16]!\n"
        "ldr w13, [sp], #255\n"
        "ldr w16, [x17, x11]\n"
        "ldr w30, [sp, x9, lsl #2]\n"
        "ldr w0, [sp, w28, sxtw]\n"
        "ldr w25, [sp, w23, sxtw #2]\n"
        "ldr w7, [x29, x0, sxtx]\n"
        "str w4, [x20]\n"
        "str w17, [x0, #1644]\n"
        "str w0, [sp, #3]\n"
        "str w30, [sp, #-256]!\n"
        "str w15, [x5], #-256\n"
        "str w8, [sp, x26]\n"
        "str w1, [x19, x14, lsl #2]\n"
        "str w9, [x8, w21, sxtw]\n"
        "str w13, [sp, w10, sxtw #2]\n"
        "str w1, [sp, x1, sxtx]\n"
        "ldur w20, [x21, #-3]\n"
        "stur w30, [sp, #89]\n"
        "ldrb w17, [sp, #136]\n"
        "ldrb w5, [x1, #241]!\n"
        "ldrb w20, [sp], #222\n"
        "ldrb w16, [sp, x11]\n"
        "ldrb w16, [x20, x9, lsl #0]\n"
        "strb w5, [x2, #832]\n"
        "strb w13, [sp, #198]!\n"
        "strb w14, [sp], #207\n"
        "strb w9, [x30, x18]\n"
        "strb w10, [x4, x13, lsl #0]\n"
        "ldrh w15, [x27, #7518]\n"
        "ldrh w29, [sp, #-225]!\n"
        "ldrh w10, [x18], #243\n"
        "ldrh w4, [sp, x3]\n"
        "ldrh w13, [x21, x11, lsl #1]\n"
        "strh w1, [sp, #7336]\n"
        "strh w6, [sp, #-71]!\n"
        "strh w20, [sp], #67\n"
        "strh w1, [x8, x30]\n"
        "strh w1, [sp, x10, lsl #1]\n"
        "ldrsb w17, [x9, #32]\n"
        "ldrsb x13, [x28, x15]\n"
        "ldrsh x6, [x23, #178]\n"
        "ldrsh w23, [x20, x20]\n"
        "ldrsw x26, [x26, #9244]\n"
        "ldrsw x30, [x22], #4\n"
        "ldrsw x13, [sp, x14, lsl #2]\n"
        "ldurb w8, [x9, #-78]\n"
        "sturb w7, [x15, #-109]\n"
        "ldurh w4, [x28, #222]\n"
        "sturh w21, [x12, #-111]\n"
        "ldursb w12, [x1, #-76]\n"
        "ldursh w9, [x6, #-106]\n"
        "ldursw x1, [x16, #-4]\n"
        "prfm pstl2strm, [x6, #64]\n"
        "ldp x24, x3, [sp, #-144]\n"
        "ldp x0, x8, [x3, #-280]!\n"
        "ldp x4, x9, [sp], #208\n"
        "stp x30, x29, [x19, #-120]\n"
        "stp x0, x30, [x11, #464]!\n"
        "stp x7, x2, [sp], #-304\n"
        "ldnp x22, x27, [sp, #24]\n"
        "stnp x0, x23, [sp]\n"
        "ldp w18, w13, [sp, #196]\n"
        "ldp w4, w17, [sp, #-52]!\n"
        "ldp w24, w19, [x9], #48\n"
        "stp w30, w27, [x26, #68]\n"
        "stp w8, w26, [x6, #-204]!\n"
        "stp w19, w1, [x13], #-140\n"
        "ldnp w18, w0, [x5, #12]\n"
        "stnp w25, w12, [sp]\n"
        "ldpsw x16, x22, [sp, #8]\n"
        "ldp s5, s29, [sp, #76]\n"
        "stp s9, s13, [sp, #124]\n"
        "ldp d20, d22, [sp, #184]\n"
        "stp d18, d19, [x8, #-264]\n"
        "ldp q14, q21, [x23, #128]\n"
        "stp q27, q17, [x14, #928]\n"
        "ldr b21, [x17, #3617]\n"
        "ldr b3, [sp, #179]!\n"
        "ldr b18, [x1, x27]\n"
        "str b24, [x12, #612]\n"
        "str b23, [x16, #-200]!\n"
        "str b31, [x28, x13]\n"
        "ldur b24, [sp, #-3]\n"
        "ldr h0, [sp, #6078]\n"
        "ldr h25, [sp, #-150]!\n"
        "ldr h31, [sp, x7, lsl #1]\n"
        "str h0, [x11, #2]\n"
        "str h8, [x28, #-42]!\n"
        "str h20, [sp, x6, lsl #1]\n"
        "ldur h1, [sp, #-3]\n"
        "ldr s28, [sp, #6980]\n"
        "ldr s28, [x10, #131]!\n"
        "ldr s14, [x15, x28, lsl #2]\n"
        "str s19, [x9, #3844]\n"
        "str s11, [x19, #185]!\n"
        "str s1, [sp, x12, lsl #2]\n"
        "ldur s0, [x15, #-3]\n"
        "ldr d30, [x8, #25472]\n"
        "ldr d13, [sp, #49]!\n"
        "ldr d27, [x9, x20, lsl #3]\n"
        "str d5, [sp, #21040]\n"
        "str d4, [x27, #76]!\n"
        "str d30, [sp, x20, lsl #3]\n"
        "ldur d6, [sp, #-3]\n"
        "ldr q26, [x28, #42288]\n"
        "ldr q12, [sp, #-166]!\n"
        "ldr q3, [x20, x28, lsl #4]\n"
        "str q15, [x18, #23584]\n"
        "str q14, [sp, #149]!\n"
        "str q18, [sp, x21, lsl #4]\n"
        "ldur q14, [sp, #-3]\n"
        "ldxr x10, [x28]\n"
        "ldaxr x18, [sp]\n"
        "ldar x19, [sp]\n"
        "stlr x10, [x24]\n"
        "stxr w14, x1, [sp]\n"
        "stlxr w27, x9, [x20]\n"
        "ldxp x15, x27, [x27]\n"
        "ldaxp x5, x26, [sp]\n"
        "stxp w3, x29, x29, [sp]\n"
        "stlxp w26, x6, x10, [x5]\n"
        "ldxr w10, [x15]\n"
        "ldaxr w12, [x28]\n"
        "ldar w7, [sp]\n"
        "stlr w12, [x7]\n"
        "stxr w9, w8, [sp]\n"
        "stlxr w1, w22, [sp]\n"
        "ldxp w23, w3, [x6]\n"
        "ldaxp w9, w14, [x5]\n"
        "stxp w7, w15, w22, [x6]\n"
        "stlxp w25, w21, w11, [x19]\n"
        "ldxrb w5, [sp]\n"
        "ldaxrb w19, [sp]\n"
        "stxrb w19, w5, [sp]\n"
        "stlxrb w14, w11, [x1]\n"
        "ldarb w25, [sp]\n"
        "stlrb w15, [x0]\n"
        "ldxrh w8, [sp]\n"
        "ldaxrh w21, [x15]\n"
        "stxrh w12, w26, [sp]\n"
        "stlxrh w11, w12, [sp]\n"
        "ldarh w24, [x5]\n"
        "stlrh w22, [sp]\n"
        "cas w4, w18, [x10]\n"
        "casb w4, w27, [sp]\n"
        "cash w4, w27, [sp]\n"
        "casa x1, x18, [x11]\n"
        "casab w22, w24, [sp]\n"
        "casah w21, w28, [sp]\n"
        "casl w26, w29, [sp]\n"
        "caslb w13, w21, [sp]\n"
        "caslh w11, w18, [x5]\n"
        "casal x6, x18, [sp]\n"
        "casalb w10, w17, [sp]\n"
        "casalh w15, w5, [sp]\n"
        "swp x29, x13, [sp]\n"
        "swpb w1, w19, [x15]\n"
        "swph w21, w5, [x3]\n"
        "swpa w3, w29, [sp]\n"
        "swpab w2, w15, [sp]\n"
        "swpah w8, w3, [x6]\n"
        "swpl x13, x4, [sp]\n"
        "swplb w5, w8, [x18]\n"
        "swplh w7, w8, [sp]\n"
        "swpal w28, w3, [x3]\n"
        "swpalb w7, w14, [x10]\n"
        "swpalh w24, w4, [x5]\n"
        "ldadd w8, w16, [sp]\n"
        "ldaddb w14, w23, [sp]\n"
        "ldaddh w13, w8, [sp]\n"
        "ldadda w12, w4, [x1]\n"
        "ldaddab w20, w0, [x6]\n"
        "ldaddah w30, w6, [sp]\n"
        "ldaddl x20, x17, [sp]\n"
        "ldaddlb w5, w6, [x30]\n"
        "ldaddlh w23, w9, [x4]\n"
        "ldaddal w21, w26, [x19]\n"
        "ldaddalb w2, w19, [x2]\n"
        "ldaddalh w22, w2, [sp]\n"
        "ldclr w17, w21, [sp]\n"
        "ldclrb w22, w26, [x0]\n"
        "ldclrh w22, w13, [x5]\n"
        "ldclra x16, x23, [sp]\n"
        "ldclrab w25, w6, [x12]\n"
        "ldclrah w14, w29, [sp]\n"
        "ldclrl w14, w14, [x7]\n"
        "ldclrlb w7, w5, [sp]\n"
        "ldclrlh w10, w18, [x3]\n"
        "ldclral x0, x12, [sp]\n"
        "ldclralb w14, w18, [sp]\n"
        "ldclralh w19, w12, [x21]\n"
        "ldeor w13, w20, [x9]\n"
        "ldeorb w14, w16, [x16]\n"
        "ldeorh w30, w18, [x30]\n"
        "ldeora x4, x9, [x16]\n"
        "ldeorab w16, w9, [x3]\n"
        "ldeorah w15, w25, [sp]\n"
        "ldeorl x0, x15, [x19]\n"
        "ldeorlb w16, w27, [x15]\n"
        "ldeorlh w21, w9, [sp]\n"
        "ldeoral x24, x9, [x25]\n"
        "ldeoralb w18, w2, [x12]\n"
        "ldeoralh w21, w6, [x7]\n"
        "ldset x12, x16, [x23]\n"
        "ldsetb w2, w30, [sp]\n"
        "ldseth w30, w16, [sp]\n"
        "ldseta x21, x15, [x29]\n"
        "ldsetab w22, w10, [sp]\n"
        "ldsetah w10, w19, [x30]\n"
        "ldsetl x15, x23, [sp]\n"
        "ldsetlb w23, w8, [sp]\n"
        "ldsetlh w8, w23, [sp]\n"
        "ldsetal x6, x23, [x10]\n"
        "ldsetalb w18, w1, [sp]\n"
        "ldsetalh w5, w10, [x3]\n"
        "ldsmax w14, w8, [sp]\n"
        "ldsmaxb w10, w12, [x13]\n"
        "ldsmaxh w26, w24, [sp]\n"
        "ldsmaxa w12, w8, [x16]\n"
        "ldsmaxab w28, w6, [x24]\n"
        "ldsmaxah w23, w27, [sp]\n"
        "ldsmaxl x12, x16, [sp]\n"
        "ldsmaxlb w15, w12, [x1]\n"
        "ldsmaxlh w6, w18, [x28]\n"
        "ldsmaxal w4, w12, [x24]\n"
        "ldsmaxalb w19, w6, [sp]\n"
        "ldsmaxalh w4, w29, [sp]\n"
        "ldsmin w5, w6, [sp]\n"
        "ldsminb w20, w14, [x16]\n"
        "ldsminh w0, w18, [sp]\n"
        "ldsmina x12, x6, [sp]\n"
        "ldsminab w6, w28, [x12]\n"
        "ldsminah w27, w30, [x10]\n"
        "ldsminl w2, w10, [sp]\n"
        "ldsminlb w26, w17, [sp]\n"
        "ldsminlh w20, w9, [x26]\n"
        "ldsminal w24, w8, [x26]\n"
        "ldsminalb w11, w6, [x9]\n"
        "ldsminalh w6, w28, [x6]\n"
        "ldumax x8, x1, [sp]\n"
        "ldumaxb w9, w26, [x30]\n"
        "ldumaxh w6, w9, [x11]\n"
        "ldumaxa x28, x24, [sp]\n"
        "ldumaxab w24, w18, [sp]\n"
        "ldumaxah w19, w18, [x3]\n"
        "ldumaxl x10, x11, [x19]\n"
        "ldumaxlb w1, w3, [sp]\n"
        "ldumaxlh w2, w13, [sp]\n"
        "ldumaxal w0, w15, [sp]\n"
        "ldumaxalb w5, w15, [x0]\n"
        "ldumaxalh w11, w27, [sp]\n"
        "ldumin x27, x16, [sp]\n"
        "lduminb w11, w22, [x11]\n"
        "lduminh w10, w15, [sp]\n"
        "ldumina w0, w15, [sp]\n"
        "lduminab w17, w3, [x21]\n"
        "lduminah w25, w30, [sp]\n"
        "lduminl x17, x0, [x26]\n"
        "lduminlb w28, w0, [x19]\n"
        "lduminlh w5, w3, [sp]\n"
        "lduminal x15, x25, [sp]\n"
        "lduminalb w28, w20, [x17]\n"
        "lduminalh w24, w2, [x21]\n"
        "stadd x3, [x8]\n"
        "staddl w0, [x25]\n"
        "stclr x14, [x1]\n"
        "stclrl w29, [x23]\n"
        "steor x14, [sp]\n"
        "steorl w22, [x30]\n"
        "stset w22, [x17]\n"
        "stsetl x18, [x13]\n"
        "stsmax w11, [sp]\n"
        "stsmaxl w4, [sp]\n"
        "stsmin w9, [x2]\n"
        "stsminl w17, [sp]\n"
        "stumax x12, [sp]\n"
        "stumaxl x11, [x24]\n"
        "stumin x2, [sp]\n"
        "stuminl w19, [sp]\n"
        "ldapr x30, [sp]\n"
        "ldaprb w26, [x6]\n"
        "mrs x20, tpidr_el0\n"
        "mrs x11, tpidrro_el0\n"
        "mrs x0, fpcr\n"
        "mrs x19, fpsr\n"
        "mrs x11, cntvct_el0\n"
        "mrs x14, cntfrq_el0\n"
        "mrs x30, dczid_el0\n"
        "mrs x1, ctr_el0\n"
        "mrs x20, midr_el1\n"
        "msr tpidr_el0, x20\n"
        "msr fpcr, x14\n"
        "msr fpsr, x16\n"
        "msr nzcv, x21\n"
        "fmul s16, s31, s31\n"
        "fdiv s12, s19, s5\n"
        "fadd s4, s31, s28\n"
        "fsub s25, s10, s12\n"
        "fmax s16, s8, s16\n"
        "fmin s0, s11, s12\n"
        "fmaxnm s15, s29, s7\n"
        "fminnm s18, s23, s12\n"
        "fnmul s6, s0, s20\n"
        "fmov s28, s19\n"
        "fabs s31, s2\n"
        "fneg s20, s18\n"
        "fsqrt s11, s29\n"
        "frintn s31, s19\n"
        "frintp s30, s21\n"
        "frintm s15, s4\n"
        "frintz s12, s2\n"
        "frinta s23, s4\n"
        "frintx s22, s0\n"
        "frinti s11, s2\n"
        "fmadd s30, s10, s20, s6\n"
        "fmsub s10, s23, s15, s30\n"
        "fnmadd s3, s0, s14, s20\n"
        "fnmsub s9, s18, s15, s22\n"
        "fcmp s22, s24\n"
        "fcmpe s9, s29\n"
        "fcmp s27, #0.0\n"
        "fcmpe s25, #0.0\n"
        "fccmp s3, s29, #3, ge\n"
        "fcsel s1, s5, s17, lt\n"
        "fmov s19, #1.0\n"
        "fmov s23, #-0.5\n"
        "fmov s7, #2.0\n"
        "fmov s0, #0.125\n"
        "fmov s11, #31.0\n"
        "fmov s6, #-1.25\n"
        "fmov s26, #3.0\n"
        "fcvtns w12, s27\n"
        "fcvtnu w21, s4\n"
        "fcvtas w1, s16\n"
        "fcvtau w5, s27\n"
        "fcvtps w11, s26\n"
        "fcvtpu w29, s14\n"
        "fcvtms w11, s14\n"
        "fcvtmu w26, s24\n"
        "fcvtzs x11, s18\n"
        "fcvtzu w7, s23\n"
        "scvtf s0, x5\n"
        "ucvtf s31, x4\n"
        "fmul d30, d28, d10\n"
        "fdiv d25, d16, d24\n"
        "fadd d25, d5, d13\n"
        "fsub d24, d6, d13\n"
        "fmax d23, d22, d16\n"
        "fmin d19, d24, d27\n"
        "fmaxnm d9, d22, d4\n"
        "fminnm d18, d20, d17\n"
        "fnmul d12, d12, d25\n"
        "fmov d13, d13\n"
        "fabs d14, d23\n"
        "fneg d16, d28\n"
        "fsqrt d15, d7\n"
        "frintn d7, d1\n"
        "frintp d26, d2\n"
        "frintm d19, d19\n"
        "frintz d22, d8\n"
        "frinta d8, d16\n"
        "frintx d11, d30\n"
        "frinti d24, d16\n"
        "fmadd d15, d10, d30, d5\n"
        "fmsub d12, d22, d0, d31\n"
        "fnmadd d3, d26, d27, d7\n"
        "fnmsub d25, d20, d26, d19\n"
        "fcmp d22, d25\n"
        "fcmpe d4, d29\n"
        "fcmp d12, #0.0\n"
        "fcmpe d4, #0.0\n"
        "fccmp d16, d28, #3, hs\n"
        "fcsel d16, d4, d15, ls\n"
        "fmov d17, #1.0\n"
        "fmov d15, #-0.5\n"
        "fmov d20, #2.0\n"
        "fmov d26, #0.125\n"
        "fmov d3, #31.0\n"
        "fmov d4, #-1.25\n"
        "fmov d0, #3.0\n"
        "fcvtns x19, d14\n"
        "fcvtnu w3, d12\n"
        "fcvtas x17, d5\n"
        "fcvtau x16, d29\n"
        "fcvtps x3, d20\n"
        "fcvtpu x25, d26\n"
        "fcvtms w3, d2\n"
        "fcvtmu x2, d19\n"
        "fcvtzs w27, d12\n"
        "fcvtzu x5, d5\n"
        "scvtf d22, x15\n"
        "ucvtf d9, x7\n"
        "fmov s31, w8\n"
        "fmov w12, s3\n"
        "fmov d14, x19\n"
        "fmov x9, d22\n"
        "add v11.8b, v10.8b, v1.8b\n"
        "sub v7.8h, v15.8h, v6.8h\n"
        "cmeq v11.2d, v8.2d, v20.2d\n"
        "cmgt v13.4s, v27.4s, v1.4s\n"
        "cmge v5.4h, v28.4h, v15.4h\n"
        "cmhi v29.2s, v15.2s, v7.2s\n"
        "cmhs v10.8h, v14.8h, v17.8h\n"
        "addp v8.8b, v29.8b, v26.8b\n"
        "ushl v24.8b, v7.8b, v15.8b\n"
        "sshl v26.2s, v10.2s, v5.2s\n"
        "cmtst v30.8b, v1.8b, v11.8b\n"
        "mul v20.16b, v21.16b, v30.16b\n"
        "umax v29.4s, v18.4s, v4.4s\n"
        "umin v22.8b, v3.8b, v1.8b\n"
        "smax v20.4h, v2.4h, v4.4h\n"
        "smin v16.16b, v7.16b, v1.16b\n"
        "uqadd v30.4h, v0.4h, v26.4h\n"
        "sqadd v24.4s, v3.4s, v16.4s\n"
        "uqsub v9.4h, v22.4h, v13.4h\n"
        "sqsub v4.4s, v20.4s, v19.4s\n"
        "and v9.16b, v24.16b, v16.16b\n"
        "orr v13.8b, v31.8b, v5.8b\n"
        "eor v19.16b, v4.16b, v20.16b\n"
        "bic v12.8b, v14.8b, v25.8b\n"
        "orn v18.16b, v9.16b, v9.16b\n"
        "bsl v15.8b, v11.8b, v29.8b\n"
        "bit v26.16b, v25.16b, v28.16b\n"
        "bif v25.16b, v24.16b, v19.16b\n"
        "fadd v20.2s, v6.2s, v5.2s\n"
        "fsub v27.4s, v16.4s, v29.4s\n"
        "fmul v6.4s, v14.4s, v29.4s\n"
        "fdiv v30.2d, v10.2d, v6.2d\n"
        "fmax v24.2d, v31.2d, v13.2d\n"
        "fmin v28.2d, v19.2d, v1.2d\n"
        "fmaxnm v18.2d, v18.2d, v4.2d\n"
        "fminnm v12.2s, v2.2s, v2.2s\n"
        "mov v30.16b, v26.16b\n"
        "mov v24.8b, v24.8b\n"
        "not v27.16b, v0.16b\n"
        "mvn v9.8b, v0.8b\n"
        "cnt v24.8b, v26.8b\n"
        "rev64 v28.4s, v6.4s\n"
        "neg v15.2d, v11.2d\n"
        "abs v11.8h, v23.8h\n"
        "addv b1, v7.16b\n"
        "addv h2, v22.4h\n"
        "addv s3, v22.4s\n"
        "uaddlv h4, v1.8b\n"
        "umaxv b5, v19.16b\n"
        "uminv s6, v23.4s\n"
        "dup v3.4s, w20\n"
        "dup v17.2d, x15\n"
        "dup v21.4s, v3.s[2]\n"
        "dup v0.16b, v7.b[9]\n"
        "movi v13.16b, #193\n"
        "movi v2.8b, #129\n"
        "ext v0.16b, v5.16b, v15.16b, #3\n"
        "ext v20.8b, v30.8b, v4.8b, #5\n"
        "ld1 {v7.8h}, [sp]\n"
        "ld1 {v7.8h}, [x27], #16\n"
        "st1 {v7.8h}, [sp], x29\n"
        "ld1 {v7.2s, v8.2s}, [sp]\n"
        "st1 {v7.2s, v8.2s}, [x7]\n"
        "ld1 {v7.2s, v8.2s}, [x13], #16\n"
        "st1 {v7.2s, v8.2s}, [x2], x27\n"
        "ld1 {v7.8b, v8.8b, v9.8b}, [x7]\n"
        "st1 {v7.8b, v8.8b, v9.8b}, [sp]\n"
        "ld1 {v7.8b, v8.8b, v9.8b}, [sp], #24\n"
        "st1 {v7.8b, v8.8b, v9.8b}, [x25], x13\n"
        "ld1 {v7.16b, v8.16b, v9.16b, v10.16b}, [x30]\n"
        "st1 {v7.16b, v8.16b, v9.16b, v10.16b}, [x10]\n"
        "ld1 {v7.16b, v8.16b, v9.16b, v10.16b}, [x15], #64\n"
        "st1 {v7.16b, v8.16b, v9.16b, v10.16b}, [x12], x26\n"
        "0:\n1:\n2:\n3:\n4:\n"
        ".globl axiom_asm_table_end\n"
        "axiom_asm_table_end:\n"
        ".text\n");

extern const unsigned char axiom_asm_table[], axiom_asm_table_end[];

/* each line: its bytes */
static const char *const want[] = {
    "04054091",  /* add x4, x8, #4096 */
    "e41f4091",  /* add x4, sp, #7, lsl #12 */
    "3f031f8b",  /* add xzr, x25, xzr */
    "bf5d5f8b",  /* add xzr, x13, xzr, lsr #23 */
    "240b328b",  /* add x4, x25, w18, uxtb #2 */
    "0408008b",  /* add x4, x0, x0, lsl #2 */
    "f10700b1",  /* adds x17, sp, #1 */
    "b10540b1",  /* adds x17, x13, #1, lsl #12 */
    "ff0311ab",  /* adds xzr, xzr, x17 */
    "ab765fab",  /* adds x11, x21, xzr, lsr #29 */
    "f1a33aab",  /* adds x17, sp, w26, sxth #0 */
    "f16b23ab",  /* adds x17, sp, x3, lsl #2 */
    "1ffe3fd1",  /* sub sp, x16, #4095 */
    "ff1f40d1",  /* sub sp, sp, #7, lsl #12 */
    "f2011fcb",  /* sub x18, x15, xzr */
    "ed439fcb",  /* sub x13, xzr, xzr, asr #16 */
    "7f8830cb",  /* sub sp, x3, w16, sxtb #2 */
    "ff612fcb",  /* sub sp, x15, x15, uxtx */
    "9f0600f1",  /* subs xzr, x20, #1 */
    "1f0640f1",  /* subs xzr, x16, #1, lsl #12 */
    "38021feb",  /* subs x24, x17, xzr */
    "ff6755eb",  /* subs xzr, xzr, x21, lsr #25 */
    "7fa730eb",  /* subs xzr, x27, w16, sxth #1 */
    "bfe12feb",  /* subs xzr, x13, x15, sxtx */
    "5fea34f1",  /* cmp x18, #3386 */
    "ff031fab",  /* cmn xzr, xzr */
    "1f0c11eb",  /* cmp x0, x17, lsl #3 */
    "f10313cb",  /* neg x17, x19 */
    "f90b13eb",  /* negs x25, x19, lsl #2 */
    "7f02199a",  /* adc xzr, x19, x25 */
    "34021bba",  /* adcs x20, x17, x27 */
    "3f0002da",  /* sbc xzr, x1, x2 */
    "ee031ffa",  /* sbcs x14, xzr, xzr */
    "e30319da",  /* ngc x3, x25 */
    "f30305fa",  /* ngcs x19, x5 */
    "5ff00092",  /* and sp, x2, #0x5555555555555555 */
    "e8cf00b2",  /* orr x8, xzr, #0xf0f0f0f0f0f0f0f */
    "ff1f78d2",  /* eor sp, xzr, #0xff00 */
    "ff0340f2",  /* ands xzr, xzr, #0x1 */
    "1901088a",  /* and x25, x8, x8 */
    "5f13c78a",  /* and xzr, x26, x7, ror #4 */
    "e10311aa",  /* orr x1, xzr, x17 */
    "ff04dfaa",  /* orr xzr, x7, xzr, ror #1 */
    "9f001cca",  /* eor xzr, x4, x28 */
    "69375fca",  /* eor x9, x27, xzr, lsr #13 */
    "9f0011ea",  /* ands xzr, x4, x17 */
    "f21bc5ea",  /* ands x18, xzr, x5, ror #6 */
    "eb03328a",  /* bic x11, xzr, x18 */
    "ef2b3f8a",  /* bic x15, xzr, xzr, lsl #10 */
    "ff0320aa",  /* orn xzr, xzr, x0 */
    "3f237faa",  /* orn xzr, x25, xzr, lsr #8 */
    "f5033fca",  /* eon x21, xzr, xzr */
    "1f1322ca",  /* eon xzr, x24, x2, lsl #4 */
    "a5033fea",  /* bics x5, x29, xzr */
    "ff1faaea",  /* bics xzr, xzr, x10, asr #7 */
    "7f0341f2",  /* tst x27, #0x8000000000000000 */
    "9f0012ea",  /* tst x4, x18 */
    "f10338aa",  /* mvn x17, x24 */
    "e3030aaa",  /* mov x3, x10 */
    "3f000091",  /* mov sp, x1 */
    "ed030091",  /* mov x13, sp */
    "020080d2",  /* mov x2, #0 */
    "2c0080d2",  /* mov x12, #1 */
    "fbff9fd2",  /* mov x27, #65535 */
    "3900a0d2",  /* mov x25, #65536 */
    "e4ffbfd2",  /* mov x4, #4294901760 */
    "1a008092",  /* mov x26, #-1 */
    "24008092",  /* mov x4, #-2 */
    "eaff9f92",  /* mov x10, #-65536 */
    "8346c2d2",  /* mov x3, #20014547599360 */
    "f3ffffd2",  /* mov x19, #18446462598732840960 */
    "f2f300b2",  /* mov x18, #6148914691236517205 */
    "d931b8d2",  /* movz x25, #49550, lsl #16 */
    "42518ef2",  /* movk x2, #29322 */
    "523b8592",  /* movn x18, #10714 */
    "1e12f1f2",  /* movk x30, #34960, lsl #48 */
    "8bb36dd3",  /* lsl x11, x28, #19 */
    "3222dd9a",  /* lsl x18, x17, x29 */
    "c3fd7ad3",  /* lsr x3, x14, #58 */
    "6824d99a",  /* lsr x8, x3, x25 */
    "41ff5393",  /* asr x1, x26, #19 */
    "602ad59a",  /* asr x0, x19, x21 */
    "406cc293",  /* ror x0, x2, #27 */
    "432fdc9a",  /* ror x3, x26, x28 */
    "3920c69a",  /* lslv x25, x1, x6 */
    "2727d29a",  /* lsrv x7, x25, x18 */
    "ad28c39a",  /* asrv x13, x5, x3 */
    "ae2cd59a",  /* rorv x14, x5, x21 */
    "a708d79a",  /* udiv x7, x5, x23 */
    "7b0ccd9a",  /* sdiv x27, x3, x13 */
    "9ef97ad3",  /* ubfx x30, x12, #58, #5 */
    "3af97a93",  /* sbfx x26, x9, #58, #5 */
    "f69150b3",  /* bfxil x22, x15, #16, #21 */
    "86527ad3",  /* ubfiz x6, x20, #6, #21 */
    "00c87e93",  /* sbfiz x0, x0, #2, #51 */
    "3d0542b3",  /* bfi x29, x9, #62, #2 */
    "8ecd68d3",  /* ubfm x14, x12, #40, #51 */
    "42e86893",  /* sbfm x2, x2, #40, #58 */
    "03f15bb3",  /* bfm x3, x8, #27, #60 */
    "f5137db3",  /* bfc x21, #3, #5 */
    "0b69c593",  /* extr x11, x8, x5, #26 */
    "c92c079b",  /* madd x9, x6, x7, x11 */
    "1afd029b",  /* msub x26, x8, x2, xzr */
    "827e129b",  /* mul x2, x20, x18 */
    "54fd1e9b",  /* mneg x20, x10, x30 */
    "8701c0da",  /* rbit x7, x12 */
    "3e05c0da",  /* rev16 x30, x9 */
    "410dc0da",  /* rev x1, x10 */
    "4511c0da",  /* clz x5, x10 */
    "7917c0da",  /* cls x25, x27 */
    "ff53839a",  /* csel xzr, xzr, x3, pl */
    "27678c9a",  /* csinc x7, x25, x12, vs */
    "f17280da",  /* csinv x17, x23, x0, vc */
    "ff279bda",  /* csneg xzr, xzr, x27, hs */
    "f0979f9a",  /* cset x16, hi */
    "e2239fda",  /* csetm x2, lo */
    "0527989a",  /* cinc x5, x24, cc */
    "44939ada",  /* cinv x4, x26, hi */
    "696483da",  /* cneg x9, x3, vc */
    "86305cfa",  /* ccmp x4, x28, #6, cc */
    "2a4a42fa",  /* ccmp x17, #2, #10, mi */
    "ad3049ba",  /* ccmn x5, x9, #13, lo */
    "28284fba",  /* ccmn x1, #15, #8, cs */
    "f51d01b4",  /* cbz x21, 1f */
    "9ef0ffb5",  /* cbnz x30, 1b */
    "ae1db9b6",  /* tbz x14, #55, 1f */
    "51f007b7",  /* tbnz x17, #32, 1b */
    "ff030011",  /* add wsp, wsp, #0 */
    "ff074011",  /* add wsp, wsp, #1, lsl #12 */
    "ff03120b",  /* add wzr, wzr, w18 */
    "5f22120b",  /* add wzr, w18, w18, lsl #8 */
    "ffaf2c0b",  /* add wsp, wsp, w12, sxth #3 */
    "e5030031",  /* adds w5, wsp, #0 */
    "e51f4031",  /* adds w5, wsp, #7, lsl #12 */
    "fd031f2b",  /* adds w29, wzr, wzr */
    "fe1f9f2b",  /* adds w30, wzr, wzr, asr #7 */
    "25a8382b",  /* adds w5, w1, w24, sxth #2 */
    "e5ff3f51",  /* sub w5, wsp, #4095 */
    "e5074051",  /* sub w5, wsp, #1, lsl #12 */
    "7f021b4b",  /* sub wzr, w19, w27 */
    "bf781f4b",  /* sub wzr, w5, wzr, lsl #30 */
    "45ce2f4b",  /* sub w5, w18, w15, sxtw #3 */
    "9ffd7f71",  /* subs wzr, w12, #16773120 */
    "ff064071",  /* subs wzr, w23, #1, lsl #12 */
    "9f02086b",  /* subs wzr, w20, w8 */
    "1e7b4e6b",  /* subs w30, w24, w14, lsr #30 */
    "ff2b2c6b",  /* subs wzr, wsp, w12, uxth #2 */
    "df753e71",  /* cmp w14, #3997 */
    "df031f2b",  /* cmn w30, wzr */
    "7f0e116b",  /* cmp w19, w17, lsl #3 */
    "ed031d4b",  /* neg w13, w29 */
    "e30b156b",  /* negs w3, w21, lsl #2 */
    "ff03171a",  /* adc wzr, wzr, w23 */
    "fe03123a",  /* adcs w30, wzr, w18 */
    "e003065a",  /* sbc w0, wzr, w6 */
    "29021f7a",  /* sbcs w9, w17, wzr */
    "f203185a",  /* ngc w18, w24 */
    "e8031a7a",  /* ngcs w8, w26 */
    "3f9f0012",  /* and wsp, w25, #0xff00ff */
    "bff10132",  /* orr wsp, w13, #0xaaaaaaaa */
    "ff1f1852",  /* eor wsp, wzr, #0xff00 */
    "7c9c0072",  /* ands w28, w3, #0xff00ff */
    "e9031f0a",  /* and w9, wzr, wzr */
    "ff5bc00a",  /* and wzr, wzr, w0, ror #22 */
    "ff031f2a",  /* orr wzr, wzr, wzr */
    "9f51912a",  /* orr wzr, w12, w17, asr #20 */
    "f3031f4a",  /* eor w19, wzr, wzr */
    "df428e4a",  /* eor wzr, w22, w14, asr #16 */
    "ff031f6a",  /* ands wzr, wzr, wzr */
    "fb351f6a",  /* ands w27, w15, wzr, lsl #13 */
    "f7033f0a",  /* bic w23, wzr, wzr */
    "1b77ba0a",  /* bic w27, w24, w26, asr #29 */
    "ff033f2a",  /* orn wzr, wzr, wzr */
    "ff41ae2a",  /* orn wzr, w15, w14, asr #16 */
    "ff033f4a",  /* eon wzr, wzr, wzr */
    "0052f64a",  /* eon w0, w16, w22, ror #20 */
    "ff03266a",  /* bics wzr, wzr, w6 */
    "1f41ff6a",  /* bics wzr, w8, wzr, ror #16 */
    "3f1e0072",  /* tst w17, #0xff */
    "bf00096a",  /* tst w5, w9 */
    "f403372a",  /* mvn w20, w23 */
    "f6031a2a",  /* mov w22, w26 */
    "3f020011",  /* mov wsp, w17 */
    "e8030011",  /* mov w8, wsp */
    "0b008052",  /* mov w11, #0 */
    "33008052",  /* mov w19, #1 */
    "f7ff9f52",  /* mov w23, #65535 */
    "2700a052",  /* mov w7, #65536 */
    "ecffbf52",  /* mov w12, #4294901760 */
    "11008012",  /* mov w17, #-1 */
    "2c008012",  /* mov w12, #-2 */
    "e5ffbf52",  /* mov w5, #-65536 */
    "eff30032",  /* mov w15, #1431655765 */
    "199cb052",  /* movz w25, #34016, lsl #16 */
    "fb189572",  /* movk w27, #43207 */
    "763a8e12",  /* movn w22, #29139 */
    "c82f0c53",  /* lsl w8, w30, #20 */
    "f620db1a",  /* lsl w22, w7, w27 */
    "157c1c53",  /* lsr w21, w0, #28 */
    "7c27d31a",  /* lsr w28, w27, w19 */
    "4c7d1e13",  /* asr w12, w10, #30 */
    "ad2bd81a",  /* asr w13, w29, w24 */
    "27279913",  /* ror w7, w25, #9 */
    "462cd41a",  /* ror w6, w2, w20 */
    "b720db1a",  /* lslv w23, w5, w27 */
    "d225d21a",  /* lsrv w18, w14, w18 */
    "bd2bd71a",  /* asrv w29, w29, w23 */
    "642ede1a",  /* rorv w4, w19, w30 */
    "c809d01a",  /* udiv w8, w14, w16 */
    "850cd81a",  /* sdiv w5, w4, w24 */
    "dc4a0453",  /* ubfx w28, w22, #4, #15 */
    "095f0b13",  /* sbfx w9, w24, #11, #13 */
    "c3360733",  /* bfxil w3, w22, #7, #7 */
    "35050a53",  /* ubfiz w21, w9, #22, #2 */
    "87291d13",  /* sbfiz w7, w12, #3, #11 */
    "7d141133",  /* bfi w29, w3, #15, #6 */
    "21340153",  /* ubfm w1, w1, #1, #13 */
    "35701f13",  /* sbfm w21, w1, #31, #28 */
    "aa1e1133",  /* bfm w10, w21, #17, #7 */
    "f3131d33",  /* bfc w19, #3, #5 */
    "b6388313",  /* extr w22, w5, w3, #14 */
    "ec7c0f1b",  /* madd w12, w7, w15, wzr */
    "b8fc071b",  /* msub w24, w5, w7, wzr */
    "2e7e121b",  /* mul w14, w17, w18 */
    "ccfc0e1b",  /* mneg w12, w6, w14 */
    "1601c05a",  /* rbit w22, w8 */
    "ea05c05a",  /* rev16 w10, w15 */
    "7208c05a",  /* rev w18, w3 */
    "dd10c05a",  /* clz w29, w6 */
    "2214c05a",  /* cls w2, w1 */
    "e0739f1a",  /* csel w0, wzr, wzr, vc */
    "9d059c1a",  /* csinc w29, w12, w28, eq */
    "9f609f5a",  /* csinv wzr, w4, wzr, vs */
    "e4179c5a",  /* csneg w4, wzr, w28, ne */
    "f1079f1a",  /* cset w17, ne */
    "f0239f5a",  /* csetm w16, cc */
    "a1779d1a",  /* cinc w1, w29, vs */
    "78a0835a",  /* cinv w24, w3, lt */
    "c214865a",  /* cneg w2, w6, eq */
    "e461547a",  /* ccmp w15, w20, #4, vs */
    "aeaa4c7a",  /* ccmp w21, #12, #14, ge */
    "4861543a",  /* ccmn w10, w20, #8, vs */
    "871a4f3a",  /* ccmn w20, #15, #7, ne */
    "f20e0134",  /* cbz w18, 1f */
    "9de1ff35",  /* cbnz w29, 1b */
    "b90e5936",  /* tbz w25, #11, 1f */
    "4be1df37",  /* tbnz w11, #27, 1b */
    "911e0013",  /* sxtb w17, w20 */
    "813f0013",  /* sxth w1, w28 */
    "2b7e4093",  /* sxtw x11, w17 */
    "2d1e0053",  /* uxtb w13, w17 */
    "c63e0053",  /* uxth w6, w22 */
    "3c0ac0da",  /* rev32 x28, x17 */
    "ad0fc0da",  /* rev64 x13, x29 */
    "5520369b",  /* smaddl x21, w2, w22, x8 */
    "77e2379b",  /* smsubl x23, w19, w23, x24 */
    "020da59b",  /* umaddl x2, w8, w5, x3 */
    "2498bd9b",  /* umsubl x4, w1, w29, x6 */
    "bb7d3b9b",  /* smull x27, w13, w27 */
    "217cb49b",  /* umull x1, w1, w20 */
    "a2ff3a9b",  /* smnegl x2, w29, w26 */
    "f0fdb09b",  /* umnegl x16, w15, w16 */
    "6b7c4a9b",  /* smulh x11, x3, x10 */
    "817cd19b",  /* umulh x1, x4, x17 */
    "c141d51a",  /* crc32b w1, w14, w21 */
    "8447cc1a",  /* crc32h w4, w28, w12 */
    "d84adc1a",  /* crc32w w24, w22, w28 */
    "dc51c01a",  /* crc32cb w28, w14, w0 */
    "1756c81a",  /* crc32ch w23, w16, w8 */
    "0259d91a",  /* crc32cw w2, w8, w25 */
    "4a4cc99a",  /* crc32x w10, w2, x9 */
    "615fcc9a",  /* crc32cx w1, w27, x12 */
    "00deff54",  /* b.eq 1b */
    "200b0154",  /* b.eq 1f */
    "c1ddff54",  /* b.ne 1b */
    "e10a0154",  /* b.ne 1f */
    "82ddff54",  /* b.cs 1b */
    "a20a0154",  /* b.cs 1f */
    "42ddff54",  /* b.hs 1b */
    "620a0154",  /* b.hs 1f */
    "03ddff54",  /* b.cc 1b */
    "230a0154",  /* b.cc 1f */
    "c3dcff54",  /* b.lo 1b */
    "e3090154",  /* b.lo 1f */
    "84dcff54",  /* b.mi 1b */
    "a4090154",  /* b.mi 1f */
    "45dcff54",  /* b.pl 1b */
    "65090154",  /* b.pl 1f */
    "06dcff54",  /* b.vs 1b */
    "26090154",  /* b.vs 1f */
    "c7dbff54",  /* b.vc 1b */
    "e7080154",  /* b.vc 1f */
    "88dbff54",  /* b.hi 1b */
    "a8080154",  /* b.hi 1f */
    "49dbff54",  /* b.ls 1b */
    "69080154",  /* b.ls 1f */
    "0adbff54",  /* b.ge 1b */
    "2a080154",  /* b.ge 1f */
    "cbdaff54",  /* b.lt 1b */
    "eb070154",  /* b.lt 1f */
    "8cdaff54",  /* b.gt 1b */
    "ac070154",  /* b.gt 1f */
    "4ddaff54",  /* b.le 1b */
    "6d070154",  /* b.le 1f */
    "3a080014",  /* b 1f */
    "cffeff97",  /* bl 1b */
    "20001fd6",  /* br x1 */
    "e0023fd6",  /* blr x23 */
    "c0035fd6",  /* ret */
    "00015fd6",  /* ret x8 */
    "4ad9ff10",  /* adr x10, 1b */
    "77060110",  /* adr x23, 1f */
    "e40340f9",  /* ldr x4, [sp] */
    "f90b4cf9",  /* ldr x25, [sp, #6160] */
    "4d835ff8",  /* ldr x13, [x26, #-8] */
    "eaff4ff8",  /* ldr x10, [sp, #255]! */
    "fe0750f8",  /* ldr x30, [sp], #-256 */
    "e46b70f8",  /* ldr x4, [sp, x16] */
    "f17a7cf8",  /* ldr x17, [x23, x28, lsl #3] */
    "3ac966f8",  /* ldr x26, [x9, w6, sxtw] */
    "ecdb63f8",  /* ldr x12, [sp, w3, sxtw #3] */
    "6de972f8",  /* ldr x13, [x11, x18, sxtx] */
    "e20300f9",  /* str x2, [sp] */
    "fa7335f9",  /* str x26, [sp, #27360] */
    "e93300f8",  /* str x9, [sp, #3] */
    "ea0e10f8",  /* str x10, [x23, #-256]! */
    "e48700f8",  /* str x4, [sp], #8 */
    "f26b28f8",  /* str x18, [sp, x8] */
    "ef7b3df8",  /* str x15, [sp, x29, lsl #3] */
    "97493df8",  /* str x23, [x12, w29, uxtw] */
    "e4db32f8",  /* str x4, [sp, w18, sxtw #3] */
    "1be936f8",  /* str x27, [x8, x22, sxtx] */
    "f22347f8",  /* ldur x18, [sp, #114] */
    "fec309f8",  /* stur x30, [sp, #156] */
    "e90340b9",  /* ldr w9, [sp] */
    "11ba43b9",  /* ldr w17, [x16, #952] */
    "04815fb8",  /* ldr w4, [x8, #-8] */
    "7dfc4fb8",  /* ldr w29, [x3, #255]! */
    "fe0650b8",  /* ldr w30, [x23], #-256 */
    "f16b76b8",  /* ldr w17, [sp, x22] */
    "e37b62b8",  /* ldr w3, [sp, x2, lsl #2] */
    "544a7bb8",  /* ldr w20, [x18, w27, uxtw] */
    "79db74b8",  /* ldr w25, [x27, w20, sxtw #2] */
    "faeb60b8",  /* ldr w26, [sp, x0, sxtx] */
    "f20300b9",  /* str w18, [sp] */
    "f6271cb9",  /* str w22, [sp, #7204] */
    "fc831fb8",  /* str w28, [sp, #-8] */
    "ed0f1fb8",  /* str w13, [sp, #-16]! */
    "f98500b8",  /* str w25, [x15], #8 */
    "cd6837b8",  /* str w13, [x6, x23] */
    "f17b30b8",  /* str w17, [sp, x16, lsl #2] */
    "fcc92cb8",  /* str w28, [x15, w12, sxtw] */
    "e1db20b8",  /* str w1, [sp, w0, sxtw #2] */
    "e6eb36b8",  /* str w6, [sp, x22, sxtx] */
    "96a257b8",  /* ldur w22, [x20, #-134] */
    "fa1302b8",  /* stur w26, [sp, #33] */
    "f0676739",  /* ldrb w16, [sp, #2521] */
    "2e6d4c38",  /* ldrb w14, [x9, #198]! */
    "92244038",  /* ldrb w18, [x4], #2 */
    "f46b7738",  /* ldrb w20, [sp, x23] */
    "557a6b38",  /* ldrb w21, [x18, x11, lsl #0] */
    "ed5b0239",  /* strb w13, [sp, #150] */
    "5c4c1038",  /* strb w28, [x2, #-252]! */
    "ec670138",  /* strb w12, [sp], #22 */
    "f96b3438",  /* strb w25, [sp, x20] */
    "f77b3838",  /* strb w23, [sp, x24, lsl #0] */
    "eaeb4e79",  /* ldrh w10, [sp, #1908] */
    "6f9d4a78",  /* ldrh w15, [x11, #169]! */
    "04a44078",  /* ldrh w4, [x0], #10 */
    "6b6b7278",  /* ldrh w11, [x27, x18] */
    "f97b6878",  /* ldrh w25, [sp, x8, lsl #1] */
    "fedb3579",  /* strh w30, [sp, #6892] */
    "f67f0578",  /* strh w22, [sp, #87]! */
    "f8c71d78",  /* strh w24, [sp], #-36 */
    "f66b3e78",  /* strh w22, [sp, x30] */
    "ec7b2278",  /* strh w12, [sp, x2, lsl #1] */
    "c6eb8039",  /* ldrsb x6, [x30, #58] */
    "0369ef38",  /* ldrsb w3, [x8, x15] */
    "8301c079",  /* ldrsh w3, [x12, #0] */
    "d36bb178",  /* ldrsh x19, [x30, x17] */
    "e663acb9",  /* ldrsw x6, [sp, #11360] */
    "c14780b8",  /* ldrsw x1, [x30], #4 */
    "f77bbab8",  /* ldrsw x23, [sp, x26, lsl #2] */
    "f5f24038",  /* ldurb w21, [x23, #15] */
    "15b10e38",  /* sturb w21, [x8, #235] */
    "39b35d78",  /* ldurh w25, [x25, #-37] */
    "95f20878",  /* sturh w21, [x20, #143] */
    "e3d3c238",  /* ldursb w3, [sp, #45] */
    "f523c978",  /* ldursh w21, [sp, #146] */
    "e3c39fb8",  /* ldursw x3, [sp, #-4] */
    "41cfff58",  /* ldr x1, 1b */
    "62fc0018",  /* ldr w2, 1f */
    "03cfff98",  /* ldrsw x3, 1b */
    "24fc005c",  /* ldr d4, 1f */
    "c5ceff9c",  /* ldr q5, 1b */
    "e6fb001c",  /* ldr s6, 1f */
    "600080f9",  /* prfm pldl1keep, [x3] */
    "932080f9",  /* prfm pstl2strm, [x4, #64] */
    "40ceffd8",  /* prfm pldl1keep, 1b */
    "84f19ff8",  /* prfum pldl3keep, [x12, #-1] */
    "d37275a9",  /* ldp x19, x28, [x22, #-176] */
    "f023dfa9",  /* ldp x16, x8, [sp, #496]! */
    "3473cba8",  /* ldp x20, x28, [x25], #176 */
    "7b3c0ca9",  /* stp x27, x15, [x3, #192] */
    "fd73a7a9",  /* stp x29, x28, [sp, #-400]! */
    "f1d3aca8",  /* stp x17, x20, [sp], #-312 */
    "e7c341a8",  /* ldnp x7, x16, [sp, #24] */
    "b61d00a8",  /* stnp x22, x7, [x13] */
    "e4236729",  /* ldp w4, w8, [sp, #-200] */
    "71ead429",  /* ldp w17, w26, [x19, #164]! */
    "e823c228",  /* ldp w8, w8, [sp], #16 */
    "ef1b1c29",  /* stp w15, w6, [sp, #224] */
    "a7a89929",  /* stp w7, w10, [x5, #204]! */
    "f113b128",  /* stp w17, w4, [sp], #-120 */
    "34e34128",  /* ldnp w20, w24, [x25, #12] */
    "ea4f0028",  /* stnp w10, w19, [sp] */
    "8a0c4169",  /* ldpsw x10, x3, [x4, #8] */
    "5038762d",  /* ldp s16, s14, [x2, #-80] */
    "473a072d",  /* stp s7, s14, [x18, #56] */
    "1b50476d",  /* ldp d27, d20, [x0, #112] */
    "ee96036d",  /* stp d14, d5, [x23, #56] */
    "f5c762ad",  /* ldp q21, q17, [sp, #-944] */
    "67d52ead",  /* stp q7, q21, [x11, #-560] */
    "9043453d",  /* ldr b16, [x28, #336] */
    "569c563c",  /* ldr b22, [x2, #-151]! */
    "5369683c",  /* ldr b19, [x10, x8] */
    "63050a3d",  /* str b3, [x11, #641] */
    "e8cf073c",  /* str b8, [sp, #124]! */
    "ef6b283c",  /* str b15, [sp, x8] */
    "e0d35f3c",  /* ldur b0, [sp, #-3] */
    "67b1627d",  /* ldr h7, [x11, #4440] */
    "f9cf4a7c",  /* ldr h25, [sp, #172]! */
    "3979747c",  /* ldr h25, [x9, x20, lsl #1] */
    "33e6067d",  /* str h19, [x17, #882] */
    "a7cc1d7c",  /* str h7, [x5, #-36]! */
    "1b79287c",  /* str h27, [x8, x8, lsl #1] */
    "f1d35f7c",  /* ldur h17, [sp, #-3] */
    "fe474dbd",  /* ldr s30, [sp, #3396] */
    "f7ff51bc",  /* ldr s23, [sp, #-225]! */
    "d37964bc",  /* ldr s19, [x14, x4, lsl #2] */
    "a4af1bbd",  /* str s4, [x29, #7084] */
    "fe5f07bc",  /* str s30, [sp, #117]! */
    "b2783bbc",  /* str s18, [x5, x27, lsl #2] */
    "f8d35fbc",  /* ldur s24, [sp, #-3] */
    "798462fd",  /* ldr d25, [x3, #17672] */
    "b29e50fc",  /* ldr d18, [x21, #-247]! */
    "88796efc",  /* ldr d8, [x12, x14, lsl #3] */
    "e10f36fd",  /* str d1, [sp, #27672] */
    "f12f0afc",  /* str d17, [sp, #162]! */
    "f97b21fc",  /* str d25, [sp, x1, lsl #3] */
    "e6d15ffc",  /* ldur d6, [x15, #-3] */
    "203bce3d",  /* ldr q0, [x25, #14560] */
    "e85fc13c",  /* ldr q8, [sp, #21]! */
    "f67bfa3c",  /* ldr q22, [sp, x26, lsl #4] */
    "af838d3d",  /* str q15, [x29, #13824] */
    "767f973c",  /* str q22, [x27, #-137]! */
    "e27bad3c",  /* str q2, [sp, x13, lsl #4] */
    "16d1df3c",  /* ldur q22, [x8, #-3] */
    "f37f5fc8",  /* ldxr x19, [sp] */
    "ecff5fc8",  /* ldaxr x12, [sp] */
    "f8ffdfc8",  /* ldar x24, [sp] */
    "2eff9fc8",  /* stlr x14, [x25] */
    "137e14c8",  /* stxr w20, x19, [x16] */
    "aafe01c8",  /* stlxr w1, x10, [x21] */
    "fc437fc8",  /* ldxp x28, x16, [sp] */
    "1cab7fc8",  /* ldaxp x28, x10, [x24] */
    "803f32c8",  /* stxp w18, x0, x15, [x28] */
    "d4eb2cc8",  /* stlxp w12, x20, x26, [x30] */
    "cc7e5f88",  /* ldxr w12, [x22] */
    "e3ff5f88",  /* ldaxr w3, [sp] */
    "aafedf88",  /* ldar w10, [x21] */
    "f9ff9f88",  /* stlr w25, [sp] */
    "ef7f1788",  /* stxr w23, w15, [sp] */
    "b8fe1488",  /* stlxr w20, w24, [x21] */
    "8d397f88",  /* ldxp w13, w14, [x12] */
    "f2bf7f88",  /* ldaxp w18, w15, [sp] */
    "e4133a88",  /* stxp w26, w4, w4, [sp] */
    "03e42d88",  /* stlxp w13, w3, w25, [x0] */
    "fe7f5f08",  /* ldxrb w30, [sp] */
    "f8ff5f08",  /* ldaxrb w24, [sp] */
    "847c1d08",  /* stxrb w29, w4, [x4] */
    "e8ff1e08",  /* stlxrb w30, w8, [sp] */
    "2cffdf08",  /* ldarb w12, [x25] */
    "f1ff9f08",  /* stlrb w17, [sp] */
    "207e5f48",  /* ldxrh w0, [x17] */
    "adff5f48",  /* ldaxrh w13, [x29] */
    "457d1548",  /* stxrh w21, w5, [x10] */
    "38fe0248",  /* stlxrh w2, w24, [x17] */
    "85fddf48",  /* ldarh w5, [x12] */
    "f0ff9f48",  /* stlrh w16, [sp] */
    "5f3f03d5",  /* clrex */
    "5f3503d5",  /* clrex #5 */
    "1d7ea1c8",  /* cas x1, x29, [x16] */
    "937eb608",  /* casb w22, w19, [x20] */
    "6e7cb848",  /* cash w24, w14, [x3] */
    "e37ff1c8",  /* casa x17, x3, [sp] */
    "077ff008",  /* casab w16, w7, [x24] */
    "e97ffb48",  /* casah w27, w9, [sp] */
    "f3ffa588",  /* casl w5, w19, [sp] */
    "eeffb408",  /* caslb w20, w14, [sp] */
    "ecffb648",  /* caslh w22, w12, [sp] */
    "faffef88",  /* casal w15, w26, [sp] */
    "12fde808",  /* casalb w8, w18, [x8] */
    "e2fff348",  /* casalh w19, w2, [sp] */
    "e88324f8",  /* swp x4, x8, [sp] */
    "d2812838",  /* swpb w8, w18, [x14] */
    "1e812478",  /* swph w4, w30, [x8] */
    "3282b9f8",  /* swpa x25, x18, [x17] */
    "fb80b638",  /* swpab w22, w27, [x7] */
    "d680ac78",  /* swpah w12, w22, [x6] */
    "248362b8",  /* swpl w2, w4, [x25] */
    "ac816c38",  /* swplb w12, w12, [x13] */
    "35826478",  /* swplh w4, w21, [x17] */
    "ec83fbb8",  /* swpal w27, w12, [sp] */
    "6c81f738",  /* swpalb w23, w12, [x11] */
    "e483f678",  /* swpalh w22, w4, [sp] */
    "f00322b8",  /* ldadd w2, w16, [sp] */
    "20012e38",  /* ldaddb w14, w0, [x9] */
    "0e013878",  /* ldaddh w24, w14, [x8] */
    "aa00b9f8",  /* ldadda x25, x10, [x5] */
    "7a00b438",  /* ldaddab w20, w26, [x3] */
    "f200b478",  /* ldaddah w20, w18, [x7] */
    "a3036cb8",  /* ldaddl w12, w3, [x29] */
    "b5036c38",  /* ldaddlb w12, w21, [x29] */
    "e8027278",  /* ldaddlh w18, w8, [x23] */
    "e603f9b8",  /* ldaddal w25, w6, [sp] */
    "f303f138",  /* ldaddalb w17, w19, [sp] */
    "b502e578",  /* ldaddalh w5, w21, [x21] */
    "e8133eb8",  /* ldclr w30, w8, [sp] */
    "a3122f38",  /* ldclrb w15, w3, [x21] */
    "0e132078",  /* ldclrh w0, w14, [x24] */
    "fa13acf8",  /* ldclra x12, x26, [sp] */
    "a212a338",  /* ldclrab w3, w2, [x21] */
    "ce10ba78",  /* ldclrah w26, w14, [x6] */
    "fb1366b8",  /* ldclrl w6, w27, [sp] */
    "67116638",  /* ldclrlb w6, w7, [x11] */
    "2e106178",  /* ldclrlh w1, w14, [x1] */
    "fd13fbb8",  /* ldclral w27, w29, [sp] */
    "f013f238",  /* ldclralb w18, w16, [sp] */
    "f913ec78",  /* ldclralh w12, w25, [sp] */
    "eb213cf8",  /* ldeor x28, x11, [x15] */
    "fb213d38",  /* ldeorb w29, w27, [x15] */
    "57213278",  /* ldeorh w18, w23, [x10] */
    "1b21b1b8",  /* ldeora w17, w27, [x8] */
    "f923b938",  /* ldeorab w25, w25, [sp] */
    "1920b078",  /* ldeorah w16, w25, [x0] */
    "e02370f8",  /* ldeorl x16, x0, [sp] */
    "38226b38",  /* ldeorlb w11, w24, [x17] */
    "e2217278",  /* ldeorlh w18, w2, [x15] */
    "0a22eeb8",  /* ldeoral w14, w10, [x16] */
    "6a21fd38",  /* ldeoralb w29, w10, [x11] */
    "4422f278",  /* ldeoralh w18, w4, [x18] */
    "f0333bf8",  /* ldset x27, x16, [sp] */
    "e8333b38",  /* ldsetb w27, w8, [sp] */
    "98322278",  /* ldseth w2, w24, [x20] */
    "f833a8b8",  /* ldseta w8, w24, [sp] */
    "4230b338",  /* ldsetab w19, w2, [x2] */
    "a831be78",  /* ldsetah w30, w8, [x13] */
    "f73371f8",  /* ldsetl x17, x23, [sp] */
    "e8336338",  /* ldsetlb w3, w8, [sp] */
    "29337078",  /* ldsetlh w16, w9, [x25] */
    "f133e2b8",  /* ldsetal w2, w17, [sp] */
    "9030fb38",  /* ldsetalb w27, w16, [x4] */
    "f932eb78",  /* ldsetalh w11, w25, [x23] */
    "ed422af8",  /* ldsmax x10, x13, [x23] */
    "f6432138",  /* ldsmaxb w1, w22, [sp] */
    "e6403c78",  /* ldsmaxh w28, w6, [x7] */
    "f043b2b8",  /* ldsmaxa w18, w16, [sp] */
    "e143b938",  /* ldsmaxab w25, w1, [sp] */
    "f343be78",  /* ldsmaxah w30, w19, [sp] */
    "14407df8",  /* ldsmaxl x29, x20, [x0] */
    "05416138",  /* ldsmaxlb w1, w5, [x8] */
    "5c407878",  /* ldsmaxlh w24, w28, [x2] */
    "e643f0f8",  /* ldsmaxal x16, x6, [sp] */
    "f043ef38",  /* ldsmaxalb w15, w16, [sp] */
    "c240f478",  /* ldsmaxalh w20, w2, [x6] */
    "e95333f8",  /* ldsmin x19, x9, [sp] */
    "e0512b38",  /* ldsminb w11, w0, [x15] */
    "f4533578",  /* ldsminh w21, w20, [sp] */
    "4a51b2b8",  /* ldsmina w18, w10, [x10] */
    "f653a638",  /* ldsminab w6, w22, [sp] */
    "f353ba78",  /* ldsminah w26, w19, [sp] */
    "f25377b8",  /* ldsminl w23, w18, [sp] */
    "fa536538",  /* ldsminlb w5, w26, [sp] */
    "ec537978",  /* ldsminlh w25, w12, [sp] */
    "1353e0f8",  /* ldsminal x0, x19, [x24] */
    "7c51ee38",  /* ldsminalb w14, w28, [x11] */
    "f653e678",  /* ldsminalh w6, w22, [sp] */
    "e46334b8",  /* ldumax w20, w4, [sp] */
    "6b632338",  /* ldumaxb w3, w11, [x27] */
    "f7633178",  /* ldumaxh w17, w23, [sp] */
    "2a61aaf8",  /* ldumaxa x10, x10, [x9] */
    "6a60a238",  /* ldumaxab w2, w10, [x3] */
    "e963b678",  /* ldumaxah w22, w9, [sp] */
    "c06367b8",  /* ldumaxl w7, w0, [x30] */
    "f0637838",  /* ldumaxlb w24, w16, [sp] */
    "ed636c78",  /* ldumaxlh w12, w13, [sp] */
    "c263fbb8",  /* ldumaxal w27, w2, [x30] */
    "3360e738",  /* ldumaxalb w7, w19, [x1] */
    "f663e778",  /* ldumaxalh w7, w22, [sp] */
    "f77324f8",  /* ldumin x4, x23, [sp] */
    "f5733638",  /* lduminb w22, w21, [sp] */
    "e4733578",  /* lduminh w21, w4, [sp] */
    "fb73aaf8",  /* ldumina x10, x27, [sp] */
    "e373b338",  /* lduminab w19, w3, [sp] */
    "a871b578",  /* lduminah w21, w8, [x13] */
    "e27378b8",  /* lduminl w24, w2, [sp] */
    "fb736738",  /* lduminlb w7, w27, [sp] */
    "27707978",  /* lduminlh w25, w7, [x1] */
    "b070f0b8",  /* lduminal w16, w16, [x5] */
    "5c70e138",  /* lduminalb w1, w28, [x2] */
    "ed72e178",  /* lduminalh w1, w13, [x23] */
    "ff0320f8",  /* stadd x0, [sp] */
    "7f0260f8",  /* staddl x0, [x19] */
    "ff132fb8",  /* stclr w15, [sp] */
    "ff1371f8",  /* stclrl x17, [sp] */
    "ff2325b8",  /* steor w5, [sp] */
    "ff2367b8",  /* steorl w7, [sp] */
    "bf312af8",  /* stset x10, [x13] */
    "1f3265f8",  /* stsetl x5, [x16] */
    "ff4026b8",  /* stsmax w6, [x7] */
    "7f4079f8",  /* stsmaxl x25, [x3] */
    "ff523cb8",  /* stsmin w28, [x23] */
    "9f506ef8",  /* stsminl x14, [x4] */
    "ff6329f8",  /* stumax x9, [sp] */
    "df6162f8",  /* stumaxl x2, [x14] */
    "bf7035b8",  /* stumin w21, [x5] */
    "ff7261f8",  /* stuminl x1, [x23] */
    "c2c3bff8",  /* ldapr x2, [x30] */
    "fec3bf38",  /* ldaprb w30, [sp] */
    "1f2003d5",  /* nop */
    "3f2003d5",  /* yield */
    "5f2003d5",  /* wfe */
    "7f2003d5",  /* wfi */
    "9f2003d5",  /* sev */
    "bf2003d5",  /* sevl */
    "df3f03d5",  /* isb */
    "010000d4",  /* svc #0 */
    "011000d4",  /* svc #0x80 */
    "200020d4",  /* brk #1 */
    "3f2103d5",  /* hint #9 */
    "03000000",  /* udf #3 */
    "bf3f03d5",  /* dmb sy */
    "9f3f03d5",  /* dsb sy */
    "bf3b03d5",  /* dmb ish */
    "9f3b03d5",  /* dsb ish */
    "bf3903d5",  /* dmb ishld */
    "9f3903d5",  /* dsb ishld */
    "bf3a03d5",  /* dmb ishst */
    "9f3a03d5",  /* dsb ishst */
    "bf3303d5",  /* dmb osh */
    "9f3303d5",  /* dsb osh */
    "bf3703d5",  /* dmb nsh */
    "9f3703d5",  /* dsb nsh */
    "bf3d03d5",  /* dmb ld */
    "9f3d03d5",  /* dsb ld */
    "bf3e03d5",  /* dmb st */
    "9f3e03d5",  /* dsb st */
    "50d03bd5",  /* mrs x16, tpidr_el0 */
    "6dd03bd5",  /* mrs x13, tpidrro_el0 */
    "1b443bd5",  /* mrs x27, fpcr */
    "27443bd5",  /* mrs x7, fpsr */
    "17423bd5",  /* mrs x23, nzcv */
    "41e03bd5",  /* mrs x1, cntvct_el0 */
    "17e03bd5",  /* mrs x23, cntfrq_el0 */
    "e8003bd5",  /* mrs x8, dczid_el0 */
    "38003bd5",  /* mrs x24, ctr_el0 */
    "060038d5",  /* mrs x6, midr_el1 */
    "4ad01bd5",  /* msr tpidr_el0, x10 */
    "0b441bd5",  /* msr fpcr, x11 */
    "2b441bd5",  /* msr fpsr, x11 */
    "0e421bd5",  /* msr nzcv, x14 */
    "41d03bd5",  /* mrs x1, s3_3_c13_c0_2 */
    "180b251e",  /* fmul s24, s24, s5 */
    "fb193f1e",  /* fdiv s27, s15, s31 */
    "7529271e",  /* fadd s21, s11, s7 */
    "8f383b1e",  /* fsub s15, s4, s27 */
    "714a351e",  /* fmax s17, s19, s21 */
    "575b3d1e",  /* fmin s23, s26, s29 */
    "d76a341e",  /* fmaxnm s23, s22, s20 */
    "d97b211e",  /* fminnm s25, s30, s1 */
    "1789331e",  /* fnmul s23, s8, s19 */
    "6a42201e",  /* fmov s10, s19 */
    "28c1201e",  /* fabs s8, s9 */
    "aa43211e",  /* fneg s10, s29 */
    "09c1211e",  /* fsqrt s9, s8 */
    "aa40241e",  /* frintn s10, s5 */
    "f0c1241e",  /* frintp s16, s15 */
    "9642251e",  /* frintm s22, s20 */
    "2ac2251e",  /* frintz s10, s17 */
    "7e42261e",  /* frinta s30, s19 */
    "6443271e",  /* frintx s4, s27 */
    "c9c2271e",  /* frinti s9, s22 */
    "dc50091f",  /* fmadd s28, s6, s9, s20 */
    "64891e1f",  /* fmsub s4, s11, s30, s2 */
    "825d361f",  /* fnmadd s2, s12, s22, s23 */
    "f69e351f",  /* fnmsub s22, s23, s21, s7 */
    "6021381e",  /* fcmp s11, s24 */
    "5020311e",  /* fcmpe s2, s17 */
    "a821201e",  /* fcmp s13, #0.0 */
    "7820201e",  /* fcmpe s3, #0.0 */
    "e385331e",  /* fccmp s15, s19, #3, hi */
    "f91d371e",  /* fcsel s25, s15, s23, ne */
    "0e102e1e",  /* fmov s14, #1.0 */
    "12103c1e",  /* fmov s18, #-0.5 */
    "0010201e",  /* fmov s0, #2.0 */
    "0c10281e",  /* fmov s12, #0.125 */
    "06f0271e",  /* fmov s6, #31.0 */
    "08903e1e",  /* fmov s8, #-1.25 */
    "0e10211e",  /* fmov s14, #3.0 */
    "3c02209e",  /* fcvtns x28, s17 */
    "8700219e",  /* fcvtnu x7, s4 */
    "7003249e",  /* fcvtas x16, s27 */
    "ce03251e",  /* fcvtau w14, s30 */
    "db02289e",  /* fcvtps x27, s22 */
    "9900299e",  /* fcvtpu x25, s4 */
    "2701309e",  /* fcvtms x7, s9 */
    "2600319e",  /* fcvtmu x6, s1 */
    "6b01389e",  /* fcvtzs x11, s11 */
    "ab00399e",  /* fcvtzu x11, s5 */
    "6f03221e",  /* scvtf s15, w27 */
    "8d02239e",  /* ucvtf s13, x20 */
    "ac0a6a1e",  /* fmul d12, d21, d10 */
    "a119741e",  /* fdiv d1, d13, d20 */
    "5e28631e",  /* fadd d30, d2, d3 */
    "f73b761e",  /* fsub d23, d31, d22 */
    "e84b641e",  /* fmax d8, d31, d4 */
    "745a741e",  /* fmin d20, d19, d20 */
    "c56b751e",  /* fmaxnm d5, d30, d21 */
    "9a78701e",  /* fminnm d26, d4, d16 */
    "848a611e",  /* fnmul d4, d20, d1 */
    "8b42601e",  /* fmov d11, d20 */
    "8ec2601e",  /* fabs d14, d20 */
    "1042611e",  /* fneg d16, d16 */
    "f3c3611e",  /* fsqrt d19, d31 */
    "1a40641e",  /* frintn d26, d0 */
    "52c1641e",  /* frintp d18, d10 */
    "7240651e",  /* frintm d18, d3 */
    "67c3651e",  /* frintz d7, d27 */
    "bb41661e",  /* frinta d27, d13 */
    "d142671e",  /* frintx d17, d22 */
    "5fc2671e",  /* frinti d31, d18 */
    "7025541f",  /* fmadd d16, d11, d20, d9 */
    "d6d8591f",  /* fmsub d22, d6, d25, d22 */
    "2c277c1f",  /* fnmadd d12, d25, d28, d9 */
    "febd621f",  /* fnmsub d30, d15, d2, d15 */
    "a020641e",  /* fcmp d5, d4 */
    "50207e1e",  /* fcmpe d2, d30 */
    "c823601e",  /* fcmp d30, #0.0 */
    "9822601e",  /* fcmpe d20, #0.0 */
    "43a57f1e",  /* fccmp d10, d31, #3, ge */
    "003f7c1e",  /* fcsel d0, d24, d28, lo */
    "17106e1e",  /* fmov d23, #1.0 */
    "03107c1e",  /* fmov d3, #-0.5 */
    "1710601e",  /* fmov d23, #2.0 */
    "1610681e",  /* fmov d22, #0.125 */
    "1cf0671e",  /* fmov d28, #31.0 */
    "0f907e1e",  /* fmov d15, #-1.25 */
    "1310611e",  /* fmov d19, #3.0 */
    "ce02601e",  /* fcvtns w14, d22 */
    "8403619e",  /* fcvtnu x4, d28 */
    "e102641e",  /* fcvtas w1, d23 */
    "7901651e",  /* fcvtau w25, d11 */
    "0f00681e",  /* fcvtps w15, d0 */
    "7c00691e",  /* fcvtpu w28, d3 */
    "a501709e",  /* fcvtms x5, d13 */
    "ee00711e",  /* fcvtmu w14, d7 */
    "4401789e",  /* fcvtzs x4, d10 */
    "6502799e",  /* fcvtzu x5, d19 */
    "ae01621e",  /* scvtf d14, w13 */
    "3d02639e",  /* ucvtf d29, x17 */
    "b300271e",  /* fmov s19, w5 */
    "7002261e",  /* fmov w16, s19 */
    "2d01679e",  /* fmov d13, x9 */
    "3501669e",  /* fmov x21, d9 */
    "e103679e",  /* fmov d1, xzr */
    "8300af9e",  /* fmov v3.d[1], x4 */
    "c500ae9e",  /* fmov x5, v6.d[1] */
    "41c0221e",  /* fcvt d1, s2 */
    "8340621e",  /* fcvt s3, d4 */
    "c5c0231e",  /* fcvt h5, s6 */
    "0741e21e",  /* fcvt s7, h8 */
    "41c0631e",  /* fcvt h1, d2 */
    "83c0e21e",  /* fcvt d3, h4 */
    "a086a74e",  /* add v0.4s, v21.4s, v7.4s */
    "78857c6e",  /* sub v24.8h, v11.8h, v28.8h */
    "fc8e6d6e",  /* cmeq v28.8h, v23.8h, v13.8h */
    "c534260e",  /* cmgt v5.8b, v6.8b, v6.8b */
    "183dbc0e",  /* cmge v24.2s, v8.2s, v28.2s */
    "cb377c6e",  /* cmhi v11.8h, v30.8h, v28.8h */
    "823dbc2e",  /* cmhs v2.2s, v12.2s, v28.2s */
    "58be764e",  /* addp v24.8h, v18.8h, v22.8h */
    "2b46eb6e",  /* ushl v11.2d, v17.2d, v11.2d */
    "6144e44e",  /* sshl v1.2d, v3.2d, v4.2d */
    "8e8fb40e",  /* cmtst v14.2s, v28.2s, v20.2s */
    "d59c784e",  /* mul v21.8h, v6.8h, v24.8h */
    "3d663a2e",  /* umax v29.8b, v17.8b, v26.8b */
    "d56c6a6e",  /* umin v21.8h, v6.8h, v10.8h */
    "db67694e",  /* smax v27.8h, v30.8h, v9.8h */
    "c96e680e",  /* smin v9.4h, v22.4h, v8.4h */
    "cc0dad2e",  /* uqadd v12.2s, v14.2s, v13.2s */
    "c90c664e",  /* sqadd v9.8h, v6.8h, v6.8h */
    "a32f696e",  /* uqsub v3.8h, v29.8h, v9.8h */
    "342e790e",  /* sqsub v20.4h, v17.4h, v25.4h */
    "f81f3c0e",  /* and v24.8b, v31.8b, v28.8b */
    "131fb44e",  /* orr v19.16b, v24.16b, v20.16b */
    "cb1c3f6e",  /* eor v11.16b, v6.16b, v31.16b */
    "3c1d7d0e",  /* bic v28.8b, v9.8b, v29.8b */
    "871ef40e",  /* orn v7.8b, v20.8b, v20.8b */
    "951e7d6e",  /* bsl v21.16b, v20.16b, v29.16b */
    "3f1fad6e",  /* bit v31.16b, v25.16b, v13.16b */
    "8f1def2e",  /* bif v15.8b, v12.8b, v15.8b */
    "74d4350e",  /* fadd v20.2s, v3.2s, v21.2s */
    "c1d6b74e",  /* fsub v1.4s, v22.4s, v23.4s */
    "badd326e",  /* fmul v26.4s, v13.4s, v18.4s */
    "34ff382e",  /* fdiv v20.2s, v25.2s, v24.2s */
    "0bf4784e",  /* fmax v11.2d, v0.2d, v24.2d */
    "d6f5ee4e",  /* fmin v22.2d, v14.2d, v14.2d */
    "14c72d0e",  /* fmaxnm v20.2s, v24.2s, v13.2s */
    "d2c4fb4e",  /* fminnm v18.2d, v6.2d, v27.2d */
    "c01eb64e",  /* mov v0.16b, v22.16b */
    "451fba0e",  /* mov v5.8b, v26.8b */
    "e958206e",  /* not v9.16b, v7.16b */
    "ab5a202e",  /* mvn v11.8b, v21.8b */
    "095b200e",  /* cnt v9.8b, v24.8b */
    "9b0aa04e",  /* rev64 v27.4s, v20.4s */
    "b1b9e06e",  /* neg v17.2d, v13.2d */
    "4cb9604e",  /* abs v12.8h, v10.8h */
    "41b9314e",  /* addv b1, v10.16b */
    "42b9710e",  /* addv h2, v10.4h */
    "23b9b14e",  /* addv s3, v9.4s */
    "e438302e",  /* uaddlv h4, v7.8b */
    "85ab306e",  /* umaxv b5, v28.16b */
    "06a9b16e",  /* uminv s6, v8.4s */
    "9b0c044e",  /* dup v27.4s, w4 */
    "750e084e",  /* dup v21.2d, x19 */
    "7404144e",  /* dup v20.4s, v3.s[2] */
    "e804134e",  /* dup v8.16b, v7.b[9] */
    "611c144e",  /* mov v1.s[2], w3 */
    "a41c184e",  /* mov v4.d[1], x5 */
    "e64c076e",  /* mov v6.b[3], v7.b[9] */
    "281d064e",  /* ins v8.h[1], w9 */
    "413c1c0e",  /* mov w1, v2.s[3] */
    "833c084e",  /* mov x3, v4.d[0] */
    "c53c050e",  /* umov w5, v6.b[2] */
    "073d1e0e",  /* umov w7, v8.h[7] */
    "492d034e",  /* smov x9, v10.b[1] */
    "8b2d0e0e",  /* smov w11, v12.h[3] */
    "cd2d0c4e",  /* smov x13, v14.s[1] */
    "0f061c5e",  /* mov s15, v16.s[3] */
    "4104185e",  /* mov d1, v2.d[1] */
    "01e4002f",  /* movi d1, #0 */
    "e1e6054f",  /* movi v1.16b, #183 */
    "6be6030f",  /* movi v11.8b, #115 */
    "02e4006f",  /* movi v2.2d, #0 */
    "ef1b1f6e",  /* ext v15.16b, v31.16b, v31.16b, #3 */
    "a228082e",  /* ext v2.8b, v5.8b, v8.8b, #5 */
    "e779400c",  /* ld1 {v7.2s}, [x15] */
    "e77b000c",  /* st1 {v7.2s}, [sp] */
    "c77adf0c",  /* ld1 {v7.2s}, [x22], #8 */
    "e77b820c",  /* st1 {v7.2s}, [sp], x2 */
    "e7a5404c",  /* ld1 {v7.8h, v8.8h}, [x15] */
    "e7a7004c",  /* st1 {v7.8h, v8.8h}, [sp] */
    "87a7df4c",  /* ld1 {v7.8h, v8.8h}, [x28], #32 */
    "e7a6864c",  /* st1 {v7.8h, v8.8h}, [x23], x6 */
    "a76b404c",  /* ld1 {v7.4s, v8.4s, v9.4s}, [x29] */
    "e76b004c",  /* st1 {v7.4s, v8.4s, v9.4s}, [sp] */
    "e76bdf4c",  /* ld1 {v7.4s, v8.4s, v9.4s}, [sp], #48 */
    "476b824c",  /* st1 {v7.4s, v8.4s, v9.4s}, [x26], x2 */
    "672c404c",  /* ld1 {v7.2d, v8.2d, v9.2d, v10.2d}, [x3] */
    "e72f004c",  /* st1 {v7.2d, v8.2d, v9.2d, v10.2d}, [sp] */
    "472ddf4c",  /* ld1 {v7.2d, v8.2d, v9.2d, v10.2d}, [x10], #64 */
    "e72f954c",  /* st1 {v7.2d, v8.2d, v9.2d, v10.2d}, [sp], x21 */
    "5efc3f91",  /* add x30, x2, #4095 */
    "5e1f4091",  /* add x30, x26, #7, lsl #12 */
    "6802128b",  /* add x8, x19, x18 */
    "ff239f8b",  /* add xzr, xzr, xzr, asr #8 */
    "9e0b3b8b",  /* add x30, x28, w27, uxtb #2 */
    "fee33d8b",  /* add x30, sp, x29, sxtx */
    "2d0600b1",  /* adds x13, x17, #1 */
    "ed0440b1",  /* adds x13, x7, #1, lsl #12 */
    "ea0310ab",  /* adds x10, xzr, x16 */
    "ff2f9fab",  /* adds xzr, xzr, xzr, asr #11 */
    "ed8f25ab",  /* adds x13, sp, w5, sxtb #3 */
    "8de22fab",  /* adds x13, x20, x15, sxtx */
    "ff0740d1",  /* sub sp, sp, #4096 */
    "ff011fcb",  /* sub xzr, x15, xzr */
    "fa275fcb",  /* sub x26, xzr, xzr, lsr #9 */
    "ffc729cb",  /* sub sp, sp, w9, sxtw #1 */
    "ff6b3dcb",  /* sub sp, sp, x29, lsl #2 */
    "f30300f1",  /* subs x19, sp, #0 */
    "b30740f1",  /* subs x19, x29, #1, lsl #12 */
    "010115eb",  /* subs x1, x8, x21 */
    "f83707eb",  /* subs x24, xzr, x7, lsl #13 */
    "934b21eb",  /* subs x19, x28, w1, uxtw #2 */
    "b36035eb",  /* subs x19, x5, x21, uxtx */
    "5fa008f1",  /* cmp x2, #552 */
    "1f0017ab",  /* cmn x0, x23 */
    "7f0d08eb",  /* cmp x11, x8, lsl #3 */
    "e4031acb",  /* neg x4, x26 */
    "fd0b05eb",  /* negs x29, x5, lsl #2 */
    "17020c9a",  /* adc x23, x16, x12 */
    "99001fba",  /* adcs x25, x4, xzr */
    "fe030fda",  /* sbc x30, xzr, x15 */
    "3f021ffa",  /* sbcs xzr, x17, xzr */
    "f8030cda",  /* ngc x24, x12 */
    "fb0313fa",  /* ngcs x27, x19 */
    "f61d7892",  /* and x22, x15, #0xff00 */
    "5f1f40b2",  /* orr sp, x26, #0xff */
    "ee0341d2",  /* eor x14, xzr, #0x8000000000000000 */
    "ffcf00f2",  /* ands xzr, xzr, #0xf0f0f0f0f0f0f0f */
    "9f02168a",  /* and xzr, x20, x22 */
    "150d448a",  /* and x21, x8, x4, lsr #3 */
    "0e0207aa",  /* orr x14, x16, x7 */
    "ff2382aa",  /* orr xzr, xzr, x2, asr #8 */
    "1f0104ca",  /* eor xzr, x8, x4 */
    "bf0c50ca",  /* eor xzr, x5, x16, lsr #3 */
    "a30003ea",  /* ands x3, x5, x3 */
    "ff575fea",  /* ands xzr, xzr, xzr, lsr #21 */
    "fd032d8a",  /* bic x29, xzr, x13 */
    "f2573d8a",  /* bic x18, xzr, x29, lsl #21 */
    "7f0130aa",  /* orn xzr, x11, x16 */
    "ff0fe9aa",  /* orn xzr, xzr, x9, ror #3 */
    "df003fca",  /* eon xzr, x6, xzr */
    "ff032eca",  /* eon xzr, xzr, x14, lsl #0 */
    "e9033dea",  /* bics x9, xzr, x29 */
    "df58ffea",  /* bics xzr, x6, xzr, ror #22 */
    "dff100f2",  /* tst x14, #0x5555555555555555 */
    "1f030bea",  /* tst x24, x11 */
    "ec033caa",  /* mvn x12, x28 */
    "e30308aa",  /* mov x3, x8 */
    "7f000091",  /* mov sp, x3 */
    "e3030091",  /* mov x3, sp */
    "330080d2",  /* mov x19, #1 */
    "2a00a0d2",  /* mov x10, #65536 */
    "f4ffbfd2",  /* mov x20, #4294901760 */
    "0c008092",  /* mov x12, #-1 */
    "3e008092",  /* mov x30, #-2 */
    "e6ff9f92",  /* mov x6, #-65536 */
    "9646c2d2",  /* mov x22, #20014547599360 */
    "e3ffffd2",  /* mov x3, #18446462598732840960 */
    "e0f300b2",  /* mov x0, #6148914691236517205 */
    "931abed2",  /* movz x19, #61652, lsl #16 */
    "78c382f2",  /* movk x24, #5659 */
    "77de9f92",  /* movn x23, #65267 */
    "a9e2f6f2",  /* movk x9, #46869, lsl #48 */
    "ded576d3",  /* lsl x30, x14, #10 */
    "7921c89a",  /* lsl x25, x11, x8 */
    "0ffe78d3",  /* lsr x15, x16, #56 */
    "ef26de9a",  /* lsr x15, x23, x30 */
    "37ff5b93",  /* asr x23, x25, #27 */
    "fd29da9a",  /* asr x29, x15, x26 */
    "3569c993",  /* ror x21, x9, #26 */
    "a72ccf9a",  /* ror x7, x5, x15 */
    "1321d19a",  /* lslv x19, x8, x17 */
    "cd26d59a",  /* lsrv x13, x22, x21 */
    "b62bc29a",  /* asrv x22, x29, x2 */
    "f22eda9a",  /* rorv x18, x23, x26 */
    "7208c29a",  /* udiv x18, x3, x2 */
    "ab0cd19a",  /* sdiv x11, x5, x17 */
    "b93549d3",  /* ubfx x25, x13, #9, #5 */
    "a2f77393",  /* sbfx x2, x29, #51, #11 */
    "34d473b3",  /* bfxil x20, x1, #51, #3 */
    "890142d3",  /* ubfiz x9, x12, #62, #1 */
    "952b5393",  /* sbfiz x21, x28, #45, #11 */
    "054a64b3",  /* bfi x5, x16, #28, #19 */
    "833076d3",  /* ubfm x3, x4, #54, #12 */
    "0a825f93",  /* sbfm x10, x16, #31, #32 */
    "85ef54b3",  /* bfm x5, x28, #20, #59 */
    "fe137db3",  /* bfc x30, #3, #5 */
    "f6b4cc93",  /* extr x22, x7, x12, #45 */
    "195f129b",  /* madd x25, x24, x18, x23 */
    "cefd179b",  /* msub x14, x14, x23, xzr */
    "fc7e059b",  /* mul x28, x23, x5 */
    "0cfe019b",  /* mneg x12, x16, x1 */
    "0f01c0da",  /* rbit x15, x8 */
    "0c05c0da",  /* rev16 x12, x8 */
    "f60ec0da",  /* rev x22, x23 */
    "bd11c0da",  /* clz x29, x13 */
    "9616c0da",  /* cls x22, x20 */
    "ff53969a",  /* csel xzr, xzr, x22, pl */
    "f1879c9a",  /* csinc x17, xzr, x28, hi */
    "9f029ada",  /* csinv xzr, x20, x26, eq */
    "ec279cda",  /* csneg x12, xzr, x28, hs */
    "ec579f9a",  /* cset x12, mi */
    "e8539fda",  /* csetm x8, mi */
    "2f27999a",  /* cinc x15, x25, cc */
    "60a293da",  /* cinv x0, x19, lt */
    "0f2588da",  /* cneg x15, x8, lo */
    "c62156fa",  /* ccmp x14, x22, #6, cs */
    "6f2940fa",  /* ccmp x11, #0, #15, cs */
    "0f8352ba",  /* ccmn x24, x18, #15, hi */
    "ce0951ba",  /* ccmn x14, #17, #14, eq */
    "22b100b4",  /* cbz x2, 1f */
    "d383ffb5",  /* cbnz x19, 1b */
    "f8b060b6",  /* tbz x24, #44, 1f */
    "85839fb7",  /* tbnz x5, #51, 1b */
    "28044011",  /* add w8, w1, #4096 */
    "e81f4011",  /* add w8, wsp, #7, lsl #12 */
    "e4031e0b",  /* add w4, wzr, w30 */
    "eb6f1f0b",  /* add w11, wzr, wzr, lsl #27 */
    "e8c72f0b",  /* add w8, wsp, w15, sxtw #1 */
    "ffff3f31",  /* adds wzr, wsp, #4095 */
    "ff1f4031",  /* adds wzr, wsp, #7, lsl #12 */
    "1f02102b",  /* adds wzr, w16, w16 */
    "2c7b092b",  /* adds w12, w25, w9, lsl #30 */
    "df41302b",  /* adds wzr, w14, w16, uxtw #0 */
    "feff3f51",  /* sub w30, wsp, #4095 */
    "fe1f4051",  /* sub w30, wsp, #7, lsl #12 */
    "ff031f4b",  /* sub wzr, wzr, wzr */
    "df199f4b",  /* sub wzr, w14, wzr, asr #6 */
    "fe232a4b",  /* sub w30, wsp, w10, uxth #0 */
    "fcfd3f71",  /* subs w28, w15, #4095 */
    "fc074071",  /* subs w28, wsp, #1, lsl #12 */
    "f9031f6b",  /* subs w25, wzr, wzr */
    "ff6f9f6b",  /* subs wzr, wzr, wzr, asr #27 */
    "fca3316b",  /* subs w28, wsp, w17, sxth #0 */
    "ff973271",  /* cmp wsp, #3237 */
    "ff011f2b",  /* cmn w15, wzr */
    "df0d0f6b",  /* cmp w14, w15, lsl #3 */
    "e2031c4b",  /* neg w2, w28 */
    "fd0b056b",  /* negs w29, w5, lsl #2 */
    "9f001f1a",  /* adc wzr, w4, wzr */
    "ff031f3a",  /* adcs wzr, wzr, wzr */
    "ff030f5a",  /* sbc wzr, wzr, w15 */
    "ff03037a",  /* sbcs wzr, wzr, w3 */
    "f903065a",  /* ngc w25, w6 */
    "ea03087a",  /* ngcs w10, w8 */
    "ff1f1812",  /* and wsp, wzr, #0xff00 */
    "ea1c0032",  /* orr w10, w7, #0xff */
    "9ff20052",  /* eor wsp, w20, #0x55555555 */
    "e5cf0072",  /* ands w5, wzr, #0xf0f0f0f */
    "1f021f0a",  /* and wzr, w16, wzr */
    "e74b1f0a",  /* and w7, wzr, wzr, lsl #18 */
    "3f740e2a",  /* orr wzr, w1, w14, lsl #29 */
    "fd031f4a",  /* eor w29, wzr, wzr */
    "19155f4a",  /* eor w25, w8, wzr, lsr #5 */
    "a0011f6a",  /* ands w0, w13, wzr */
    "ff100f6a",  /* ands wzr, w7, w15, lsl #4 */
    "3600310a",  /* bic w22, w1, w17 */
    "bf02ff0a",  /* bic wzr, w21, wzr, ror #0 */
    "e4002e2a",  /* orn w4, w7, w14 */
    "794ebf2a",  /* orn w25, w19, wzr, asr #19 */
    "9f03304a",  /* eon wzr, w28, w16 */
    "9a267f4a",  /* eon w26, w20, wzr, lsr #9 */
    "ff033f6a",  /* bics wzr, wzr, wzr */
    "3f23f16a",  /* bics wzr, w25, w17, ror #8 */
    "3f9d0072",  /* tst w9, #0xff00ff */
    "1f030f6a",  /* tst w24, w15 */
    "e803272a",  /* mvn w8, w7 */
    "ed03162a",  /* mov w13, w22 */
    "9f000011",  /* mov wsp, w4 */
    "f6030011",  /* mov w22, wsp */
    "11008052",  /* mov w17, #0 */
    "35008052",  /* mov w21, #1 */
    "e3ff9f52",  /* mov w3, #65535 */
    "2000a052",  /* mov w0, #65536 */
    "f3ffbf52",  /* mov w19, #4294901760 */
    "38008012",  /* mov w24, #-2 */
    "e6ffbf52",  /* mov w6, #-65536 */
    "e6f30032",  /* mov w6, #1431655765 */
    "e60db952",  /* movz w6, #51311, lsl #16 */
    "b2808272",  /* movk w18, #5125 */
    "94d68812",  /* movn w20, #18100 */
    "141c0853",  /* lsl w20, w0, #24 */
    "c822d71a",  /* lsl w8, w22, w23 */
    "2f7e0253",  /* lsr w15, w17, #2 */
    "1727db1a",  /* lsr w23, w24, w27 */
    "fa7c1b13",  /* asr w26, w7, #27 */
    "642aca1a",  /* asr w4, w19, w10 */
    "c11e9613",  /* ror w1, w22, #7 */
    "832cd41a",  /* ror w3, w4, w20 */
    "3622dd1a",  /* lslv w22, w17, w29 */
    "0527de1a",  /* lsrv w5, w24, w30 */
    "a22adc1a",  /* asrv w2, w21, w28 */
    "db2dd41a",  /* rorv w27, w14, w20 */
    "c90bc61a",  /* udiv w9, w30, w6 */
    "450fca1a",  /* sdiv w5, w26, w10 */
    "1a791653",  /* ubfx w26, w8, #22, #9 */
    "a2611213",  /* sbfx w2, w13, #18, #7 */
    "37701533",  /* bfxil w23, w1, #21, #8 */
    "750f1753",  /* ubfiz w21, w27, #9, #4 */
    "f6120c13",  /* sbfiz w22, w23, #20, #5 */
    "1d000233",  /* bfi w29, w0, #30, #1 */
    "4d651053",  /* ubfm w13, w10, #16, #25 */
    "126e0c13",  /* sbfm w18, w16, #12, #27 */
    "98700a33",  /* bfm w24, w4, #10, #28 */
    "fb131d33",  /* bfc w27, #3, #5 */
    "6e798c13",  /* extr w14, w11, w12, #30 */
    "137d131b",  /* madd w19, w8, w19, wzr */
    "cefc181b",  /* msub w14, w6, w24, wzr */
    "497c051b",  /* mul w9, w2, w5 */
    "6bfe141b",  /* mneg w11, w19, w20 */
    "fb01c05a",  /* rbit w27, w15 */
    "0707c05a",  /* rev16 w7, w24 */
    "930ac05a",  /* rev w19, w20 */
    "5512c05a",  /* clz w21, w18 */
    "9c14c05a",  /* cls w28, w4 */
    "7f239f1a",  /* csel wzr, w27, wzr, hs */
    "e0679f1a",  /* csinc w0, wzr, wzr, vs */
    "ffd39f5a",  /* csinv wzr, wzr, wzr, le */
    "3a35865a",  /* csneg w26, w9, w6, lo */
    "f3b79f1a",  /* cset w19, ge */
    "f7339f5a",  /* csetm w23, cs */
    "54d6921a",  /* cinc w20, w18, gt */
    "1d31885a",  /* cinv w29, w8, cs */
    "ef458f5a",  /* cneg w15, w15, pl */
    "8950527a",  /* ccmp w4, w18, #9, pl */
    "ccb8557a",  /* ccmp w6, #21, #12, lt */
    "e610543a",  /* ccmn w7, w20, #6, ne */
    "04a94f3a",  /* ccmn w8, #15, #4, ge */
    "7aa20034",  /* cbz w26, 1f */
    "0d75ff35",  /* cbnz w13, 1b */
    "23a2e836",  /* tbz w3, #29, 1f */
    "cc74cf37",  /* tbnz w12, #25, 1b */
    "2c1d4093",  /* sxtb x12, w9 */
    "273c4093",  /* sxth x7, w1 */
    "117e4093",  /* sxtw x17, w16 */
    "9a1f0053",  /* uxtb w26, w28 */
    "623e0053",  /* uxth w2, w19 */
    "3e0ac0da",  /* rev32 x30, x17 */
    "150cc0da",  /* rev64 x21, x0 */
    "8135369b",  /* smaddl x1, w12, w22, x13 */
    "eca0309b",  /* smsubl x12, w7, w16, x8 */
    "632db09b",  /* umaddl x3, w11, w16, x11 */
    "d0bfb99b",  /* umsubl x16, w30, w25, x15 */
    "527c369b",  /* smull x18, w2, w22 */
    "ee7eb69b",  /* umull x14, w23, w22 */
    "07fd209b",  /* smnegl x7, w8, w0 */
    "e0fda19b",  /* umnegl x0, w15, w1 */
    "847e449b",  /* smulh x4, x20, x4 */
    "467dc79b",  /* umulh x6, x10, x7 */
    "3140d31a",  /* crc32b w17, w1, w19 */
    "8446c91a",  /* crc32h w4, w20, w9 */
    "1e4bc31a",  /* crc32w w30, w24, w3 */
    "3452d11a",  /* crc32cb w20, w17, w17 */
    "a256d51a",  /* crc32ch w2, w21, w21 */
    "a459d61a",  /* crc32cw w4, w13, w22 */
    "244cc99a",  /* crc32x w4, w1, x9 */
    "b05ec89a",  /* crc32cx w16, w21, x8 */
    "e0011fd6",  /* br x15 */
    "20003fd6",  /* blr x1 */
    "c0035fd6",  /* ret x30 */
    "3171ff10",  /* adr x17, 1b */
    "4b9e0010",  /* adr x11, 1f */
    "580140f9",  /* ldr x24, [x10] */
    "738d6cf9",  /* ldr x19, [x11, #22808] */
    "ebf34ff8",  /* ldr x11, [sp, #255] */
    "3c0d50f8",  /* ldr x28, [x9, #-256]! */
    "e10750f8",  /* ldr x1, [sp], #-256 */
    "ab6a7bf8",  /* ldr x11, [x21, x27] */
    "f57b6df8",  /* ldr x21, [sp, x13, lsl #3] */
    "ed4865f8",  /* ldr x13, [x7, w5, uxtw] */
    "e0db75f8",  /* ldr x0, [sp, w21, sxtw #3] */
    "25e97cf8",  /* ldr x5, [x9, x28, sxtx] */
    "3b0000f9",  /* str x27, [x1] */
    "329333f9",  /* str x18, [x25, #26400] */
    "e20310f8",  /* str x2, [sp, #-256] */
    "7d0e1ff8",  /* str x29, [x19, #-16]! */
    "270610f8",  /* str x7, [x17], #-256 */
    "e06b22f8",  /* str x0, [sp, x2] */
    "f07b32f8",  /* str x16, [sp, x18, lsl #3] */
    "f4c830f8",  /* str x20, [x7, w16, sxtw] */
    "addb24f8",  /* str x13, [x29, w4, sxtw #3] */
    "edeb37f8",  /* str x13, [sp, x23, sxtx] */
    "2b204cf8",  /* ldur x11, [x1, #194] */
    "fe631af8",  /* stur x30, [sp, #-90] */
    "6f0240b9",  /* ldr w15, [x19] */
    "7bf142b9",  /* ldr w27, [x11, #752] */
    "c8825fb8",  /* ldr w8, [x22, #-8] */
    "f4ff4fb8",  /* ldr w20, [sp, #255]! */
    "cff54fb8",  /* ldr w15, [x14], #255 */
    "0d696bb8",  /* ldr w13, [x8, x11] */
    "987a7ab8",  /* ldr w24, [x20, x26, lsl #2] */
    "6cca6db8",  /* ldr w12, [x19, w13, sxtw] */
    "5ddb7db8",  /* ldr w29, [x26, w29, sxtw #2] */
    "f1eb75b8",  /* ldr w17, [sp, x21, sxtx] */
    "e80300b9",  /* str w8, [sp] */
    "f61b3bb9",  /* str w22, [sp, #15128] */
    "bd821fb8",  /* str w29, [x21, #-8] */
    "eeff0fb8",  /* str w14, [sp, #255]! */
    "03f60fb8",  /* str w3, [x16], #255 */
    "e36b33b8",  /* str w3, [sp, x19] */
    "d0793eb8",  /* str w16, [x14, x30, lsl #2] */
    "f7cb2eb8",  /* str w23, [sp, w14, sxtw] */
    "fddb25b8",  /* str w29, [sp, w5, sxtw #2] */
    "e4e82db8",  /* str w4, [x7, x13, sxtx] */
    "ee014ab8",  /* ldur w14, [x15, #160] */
    "e8e309b8",  /* stur w8, [sp, #158] */
    "e9236839",  /* ldrb w9, [sp, #2568] */
    "f59f5238",  /* ldrb w21, [sp, #-215]! */
    "c6555738",  /* ldrb w6, [x14], #-139 */
    "e06b6a38",  /* ldrb w0, [sp, x10] */
    "f37b6538",  /* ldrb w19, [sp, x5, lsl #0] */
    "f74f0a39",  /* strb w23, [sp, #659] */
    "f0bf0938",  /* strb w16, [sp, #155]! */
    "b3770c38",  /* strb w19, [x29], #199 */
    "7c682b38",  /* strb w28, [x3, x11] */
    "e07b2438",  /* strb w0, [sp, x4, lsl #0] */
    "d6364179",  /* ldrh w22, [x22, #154] */
    "ea5f5078",  /* ldrh w10, [sp, #-251]! */
    "fc455e78",  /* ldrh w28, [x15], #-28 */
    "f76b6578",  /* ldrh w23, [sp, x5] */
    "ea7b7278",  /* ldrh w10, [sp, x18, lsl #1] */
    "f3933b79",  /* strh w19, [sp, #7624] */
    "18ad1d78",  /* strh w24, [x8, #-38]! */
    "72250778",  /* strh w18, [x11], #114 */
    "7b693c78",  /* strh w27, [x11, x28] */
    "277a2678",  /* strh w7, [x17, x6, lsl #1] */
    "e75bc239",  /* ldrsb w7, [sp, #150] */
    "bb68ac38",  /* ldrsb x27, [x5, x12] */
    "7759c179",  /* ldrsh w23, [x11, #172] */
    "766abb78",  /* ldrsh x22, [x19, x27] */
    "6f1a91b9",  /* ldrsw x15, [x19, #4376] */
    "dd4480b8",  /* ldrsw x29, [x6], #4 */
    "607aaeb8",  /* ldrsw x0, [x19, x14, lsl #2] */
    "98d25b38",  /* ldurb w24, [x20, #-67] */
    "88a11038",  /* sturb w8, [x12, #-246] */
    "8b834378",  /* ldurh w11, [x28, #56] */
    "e1330e78",  /* sturh w1, [sp, #227] */
    "6c90cd38",  /* ldursb w12, [x3, #217] */
    "e503d478",  /* ldursh w5, [sp, #-192] */
    "e8c39fb8",  /* ldursw x8, [sp, #-4] */
    "200280f9",  /* prfm pldl1keep, [x17] */
    "532080f9",  /* prfm pstl2strm, [x2, #64] */
    "a4f39ff8",  /* prfum pldl3keep, [x29, #-1] */
    "f9f344a9",  /* ldp x25, x28, [sp, #72] */
    "3d82e9a9",  /* ldp x29, x0, [x17, #-360]! */
    "8d74c7a8",  /* ldp x13, x29, [x4], #112 */
    "ce740ba9",  /* stp x14, x29, [x6, #176] */
    "2e64bda9",  /* stp x14, x25, [x1, #-48]! */
    "fbcfada8",  /* stp x27, x19, [sp], #-296 */
    "9b9841a8",  /* ldnp x27, x6, [x4, #24] */
    "e02b00a8",  /* stnp x0, x10, [sp] */
    "032f7129",  /* ldp w3, w11, [x24, #-120] */
    "eb13f929",  /* ldp w11, w4, [sp, #-56]! */
    "0947e228",  /* ldp w9, w17, [x24], #-240 */
    "8a502629",  /* stp w10, w20, [x4, #-208] */
    "e4df8d29",  /* stp w4, w23, [sp, #108]! */
    "ef57a828",  /* stp w15, w21, [sp], #-192 */
    "9cd04128",  /* ldnp w28, w20, [x4, #12] */
    "40680028",  /* stnp w0, w26, [x2] */
    "6b734169",  /* ldpsw x11, x28, [x27, #8] */
    "53207d2d",  /* ldp s19, s8, [x2, #-24] */
    "9597172d",  /* stp s21, s5, [x28, #188] */
    "8dea4a6d",  /* ldp d13, d26, [x20, #168] */
    "fbff376d",  /* stp d27, d31, [sp, #-136] */
    "e78b72ad",  /* ldp q7, q2, [sp, #-432] */
    "fabb2fad",  /* stp q26, q14, [sp, #-528] */
    "0572593d",  /* ldr b5, [x16, #1628] */
    "e44f583c",  /* ldr b4, [sp, #-124]! */
    "ef6b753c",  /* ldr b15, [sp, x21] */
    "d925283d",  /* str b25, [x14, #2569] */
    "eedf163c",  /* str b14, [sp, #-147]! */
    "1968373c",  /* str b25, [x0, x23] */
    "70d15f3c",  /* ldur b16, [x11, #-3] */
    "b591457d",  /* ldr h21, [x13, #712] */
    "a8fc5a7c",  /* ldr h8, [x5, #-81]! */
    "9978747c",  /* ldr h25, [x4, x20, lsl #1] */
    "f3573b7d",  /* str h19, [sp, #7594] */
    "e01f107c",  /* str h0, [sp, #-255]! */
    "467b267c",  /* str h6, [x26, x6, lsl #1] */
    "1bd05f7c",  /* ldur h27, [x0, #-3] */
    "f8cb53bd",  /* ldr s24, [sp, #5064] */
    "eb7f40bc",  /* ldr s11, [sp, #7]! */
    "ee7b69bc",  /* ldr s14, [sp, x9, lsl #2] */
    "fa7331bd",  /* str s26, [sp, #12656] */
    "92ed0abc",  /* str s18, [x12, #174]! */
    "f27b30bc",  /* str s18, [sp, x16, lsl #2] */
    "fbd35fbc",  /* ldur s27, [sp, #-3] */
    "44b260fd",  /* ldr d4, [x18, #16736] */
    "fabf55fc",  /* ldr d26, [sp, #-165]! */
    "fe7b65fc",  /* ldr d30, [sp, x5, lsl #3] */
    "6d691cfd",  /* str d13, [x11, #14544] */
    "ad3d1cfc",  /* str d13, [x13, #-61]! */
    "987b3bfc",  /* str d24, [x28, x27, lsl #3] */
    "e4d35ffc",  /* ldur d4, [sp, #-3] */
    "09d3ef3d",  /* ldr q9, [x24, #48960] */
    "ebeccb3c",  /* ldr q11, [x7, #190]! */
    "e27bf13c",  /* ldr q2, [sp, x17, lsl #4] */
    "f300b33d",  /* str q19, [x7, #52224] */
    "8eaf9e3c",  /* str q14, [x28, #-22]! */
    "f07bb33c",  /* str q16, [sp, x19, lsl #4] */
    "b0d0df3c",  /* ldur q16, [x5, #-3] */
    "f47f5fc8",  /* ldxr x20, [sp] */
    "b0ff5fc8",  /* ldaxr x16, [x29] */
    "e1ffdfc8",  /* ldar x1, [sp] */
    "e6ff9fc8",  /* stlr x6, [sp] */
    "e97e1ec8",  /* stxr w30, x9, [x23] */
    "eaff0ec8",  /* stlxr w14, x10, [sp] */
    "e11b7fc8",  /* ldxp x1, x6, [sp] */
    "f7bb7fc8",  /* ldaxp x23, x14, [sp] */
    "f72b21c8",  /* stxp w1, x23, x10, [sp] */
    "e79734c8",  /* stlxp w20, x7, x5, [sp] */
    "e47f5f88",  /* ldxr w4, [sp] */
    "a2fe5f88",  /* ldaxr w2, [x21] */
    "38ffdf88",  /* ldar w24, [x25] */
    "8dff9f88",  /* stlr w13, [x28] */
    "517e1e88",  /* stxr w30, w17, [x18] */
    "60fd0088",  /* stlxr w0, w0, [x11] */
    "f5537f88",  /* ldxp w21, w20, [sp] */
    "edfb7f88",  /* ldaxp w13, w30, [sp] */
    "5a002388",  /* stxp w3, w26, w0, [x2] */
    "77c32088",  /* stlxp w0, w23, w16, [x27] */
    "e47f5f08",  /* ldxrb w4, [sp] */
    "68fc5f08",  /* ldaxrb w8, [x3] */
    "f27f1408",  /* stxrb w20, w18, [sp] */
    "e8ff1c08",  /* stlxrb w28, w8, [sp] */
    "95ffdf08",  /* ldarb w21, [x28] */
    "0dfc9f08",  /* stlrb w13, [x0] */
    "867f5f48",  /* ldxrh w6, [x28] */
    "fcff5f48",  /* ldaxrh w28, [sp] */
    "fb7f1148",  /* stxrh w17, w27, [sp] */
    "47fe0348",  /* stlxrh w3, w7, [x18] */
    "c1fcdf48",  /* ldarh w1, [x6] */
    "55ff9f48",  /* stlrh w21, [x26] */
    "a07cba88",  /* cas w26, w0, [x5] */
    "e77fb408",  /* casb w20, w7, [sp] */
    "d47cbc48",  /* cash w28, w20, [x6] */
    "e77ffe88",  /* casa w30, w7, [sp] */
    "e67fe508",  /* casab w5, w6, [sp] */
    "e77fe248",  /* casah w2, w7, [sp] */
    "f1ffa988",  /* casl w9, w17, [sp] */
    "38fda108",  /* caslb w1, w24, [x9] */
    "f5ffb048",  /* caslh w16, w21, [sp] */
    "f3fffdc8",  /* casal x29, x19, [sp] */
    "c3fffa08",  /* casalb w26, w3, [x30] */
    "7dfee748",  /* casalh w7, w29, [x19] */
    "f38322f8",  /* swp x2, x19, [sp] */
    "e6832c38",  /* swpb w12, w6, [sp] */
    "cd832478",  /* swph w4, w13, [x30] */
    "a981aaf8",  /* swpa x10, x9, [x13] */
    "ef83a838",  /* swpab w8, w15, [sp] */
    "5d83b778",  /* swpah w23, w29, [x26] */
    "e98366f8",  /* swpl x6, x9, [sp] */
    "c7807838",  /* swplb w24, w7, [x6] */
    "e0836178",  /* swplh w1, w0, [sp] */
    "fd83f2f8",  /* swpal x18, x29, [sp] */
    "0083fe38",  /* swpalb w30, w0, [x24] */
    "bb83fd78",  /* swpalh w29, w27, [x29] */
    "fc0322b8",  /* ldadd w2, w28, [sp] */
    "1d013138",  /* ldaddb w17, w29, [x8] */
    "e5033978",  /* ldaddh w25, w5, [sp] */
    "a200b3f8",  /* ldadda x19, x2, [x5] */
    "e303b538",  /* ldaddab w21, w3, [sp] */
    "5c02b478",  /* ldaddah w20, w28, [x18] */
    "fa0367b8",  /* ldaddl w7, w26, [sp] */
    "fc036738",  /* ldaddlb w7, w28, [sp] */
    "a3007b78",  /* ldaddlh w27, w3, [x5] */
    "e803fbf8",  /* ldaddal x27, x8, [sp] */
    "7302e838",  /* ldaddalb w8, w19, [x19] */
    "4702ec78",  /* ldaddalh w12, w7, [x18] */
    "ee132ff8",  /* ldclr x15, x14, [sp] */
    "ca123b38",  /* ldclrb w27, w10, [x22] */
    "ee122d78",  /* ldclrh w13, w14, [x23] */
    "9112adb8",  /* ldclra w13, w17, [x20] */
    "f013b938",  /* ldclrab w25, w16, [sp] */
    "e913ae78",  /* ldclrah w14, w9, [sp] */
    "951378b8",  /* ldclrl w24, w21, [x28] */
    "7b106738",  /* ldclrlb w7, w27, [x3] */
    "e4137678",  /* ldclrlh w22, w4, [sp] */
    "b512e1b8",  /* ldclral w1, w21, [x21] */
    "a113e138",  /* ldclralb w1, w1, [x29] */
    "fd13fd78",  /* ldclralh w29, w29, [sp] */
    "e5233ab8",  /* ldeor w26, w5, [sp] */
    "e2233538",  /* ldeorb w21, w2, [sp] */
    "bb203378",  /* ldeorh w19, w27, [x5] */
    "8022a7f8",  /* ldeora x7, x0, [x20] */
    "fa20a138",  /* ldeorab w1, w26, [x7] */
    "f023bc78",  /* ldeorah w28, w16, [sp] */
    "1e2078f8",  /* ldeorl x24, x30, [x0] */
    "70227b38",  /* ldeorlb w27, w16, [x19] */
    "ce216378",  /* ldeorlh w3, w14, [x14] */
    "d323e9b8",  /* ldeoral w9, w19, [x30] */
    "0322fb38",  /* ldeoralb w27, w3, [x16] */
    "f923f778",  /* ldeoralh w23, w25, [sp] */
    "6a322cb8",  /* ldset w12, w10, [x19] */
    "e4332738",  /* ldsetb w7, w4, [sp] */
    "3c332178",  /* ldseth w1, w28, [x25] */
    "6631a1b8",  /* ldseta w1, w6, [x11] */
    "8f31b538",  /* ldsetab w21, w15, [x12] */
    "b431b378",  /* ldsetah w19, w20, [x13] */
    "f03070f8",  /* ldsetl x16, x16, [x7] */
    "e8337938",  /* ldsetlb w25, w8, [sp] */
    "c8317578",  /* ldsetlh w21, w8, [x14] */
    "7333e0f8",  /* ldsetal x0, x19, [x27] */
    "b233f738",  /* ldsetalb w23, w18, [x29] */
    "ed33ed78",  /* ldsetalh w13, w13, [sp] */
    "0d4328b8",  /* ldsmax w8, w13, [x24] */
    "e5432038",  /* ldsmaxb w0, w5, [sp] */
    "fa433e78",  /* ldsmaxh w30, w26, [sp] */
    "2a40bdf8",  /* ldsmaxa x29, x10, [x1] */
    "a940b738",  /* ldsmaxab w23, w9, [x5] */
    "f043af78",  /* ldsmaxah w15, w16, [sp] */
    "f0436af8",  /* ldsmaxl x10, x16, [sp] */
    "82416c38",  /* ldsmaxlb w12, w2, [x12] */
    "e4437078",  /* ldsmaxlh w16, w4, [sp] */
    "bb42f8f8",  /* ldsmaxal x24, x27, [x21] */
    "ee43ec38",  /* ldsmaxalb w12, w14, [sp] */
    "ef43fa78",  /* ldsmaxalh w26, w15, [sp] */
    "b4512bb8",  /* ldsmin w11, w20, [x13] */
    "e0532238",  /* ldsminb w2, w0, [sp] */
    "da532978",  /* ldsminh w9, w26, [x30] */
    "8951b8b8",  /* ldsmina w24, w9, [x12] */
    "f953ad38",  /* ldsminab w13, w25, [sp] */
    "e853ac78",  /* ldsminah w12, w8, [sp] */
    "c05069f8",  /* ldsminl x9, x0, [x6] */
    "19516a38",  /* ldsminlb w10, w25, [x8] */
    "ea537678",  /* ldsminlh w22, w10, [sp] */
    "6150e0b8",  /* ldsminal w0, w1, [x3] */
    "2c51e338",  /* ldsminalb w3, w12, [x9] */
    "fe53ec78",  /* ldsminalh w12, w30, [sp] */
    "e2632ab8",  /* ldumax w10, w2, [sp] */
    "f1633638",  /* ldumaxb w22, w17, [sp] */
    "48633678",  /* ldumaxh w22, w8, [x26] */
    "f463a7f8",  /* ldumaxa x7, x20, [sp] */
    "f563a638",  /* ldumaxab w6, w21, [sp] */
    "e863b078",  /* ldumaxah w16, w8, [sp] */
    "fd6372f8",  /* ldumaxl x18, x29, [sp] */
    "ba606338",  /* ldumaxlb w3, w26, [x5] */
    "f1607978",  /* ldumaxlh w25, w17, [x7] */
    "e263f4b8",  /* ldumaxal w20, w2, [sp] */
    "1063ea38",  /* ldumaxalb w10, w16, [x24] */
    "e363e778",  /* ldumaxalh w7, w3, [sp] */
    "f4732eb8",  /* ldumin w14, w20, [sp] */
    "ce712438",  /* lduminb w4, w14, [x14] */
    "90713978",  /* lduminh w25, w16, [x12] */
    "f373a7b8",  /* ldumina w7, w19, [sp] */
    "fc73a438",  /* lduminab w4, w28, [sp] */
    "bc72b578",  /* lduminah w21, w28, [x21] */
    "56736bf8",  /* lduminl x11, x22, [x26] */
    "2a726d38",  /* lduminlb w13, w10, [x17] */
    "e4737578",  /* lduminlh w21, w4, [sp] */
    "5472e7b8",  /* lduminal w7, w20, [x18] */
    "f673f138",  /* lduminalb w17, w22, [sp] */
    "0670f178",  /* lduminalh w17, w6, [x0] */
    "ff032ab8",  /* stadd w10, [sp] */
    "ff0372f8",  /* staddl x18, [sp] */
    "9f102ab8",  /* stclr w10, [x4] */
    "ff1372b8",  /* stclrl w18, [sp] */
    "ff233cb8",  /* steor w28, [sp] */
    "9f2365b8",  /* steorl w5, [x28] */
    "df3123f8",  /* stset x3, [x14] */
    "3f3162f8",  /* stsetl x2, [x9] */
    "7f402df8",  /* stsmax x13, [x3] */
    "ff436cf8",  /* stsmaxl x12, [sp] */
    "ff5327b8",  /* stsmin w7, [sp] */
    "5f5066b8",  /* stsminl w6, [x2] */
    "7f6122b8",  /* stumax w2, [x11] */
    "ff6376f8",  /* stumaxl x22, [sp] */
    "ff733db8",  /* stumin w29, [sp] */
    "1f7170b8",  /* stuminl w16, [x8] */
    "00c2bff8",  /* ldapr x0, [x16] */
    "2ac2bf38",  /* ldaprb w10, [x17] */
    "47d03bd5",  /* mrs x7, tpidr_el0 */
    "7ad03bd5",  /* mrs x26, tpidrro_el0 */
    "07443bd5",  /* mrs x7, fpcr */
    "18423bd5",  /* mrs x24, nzcv */
    "4de03bd5",  /* mrs x13, cntvct_el0 */
    "16e03bd5",  /* mrs x22, cntfrq_el0 */
    "ed003bd5",  /* mrs x13, dczid_el0 */
    "29003bd5",  /* mrs x9, ctr_el0 */
    "040038d5",  /* mrs x4, midr_el1 */
    "55d01bd5",  /* msr tpidr_el0, x21 */
    "09441bd5",  /* msr fpcr, x9 */
    "35441bd5",  /* msr fpsr, x21 */
    "00421bd5",  /* msr nzcv, x0 */
    "bb0b281e",  /* fmul s27, s29, s8 */
    "b31a261e",  /* fdiv s19, s21, s6 */
    "292b371e",  /* fadd s9, s25, s23 */
    "423b3f1e",  /* fsub s2, s26, s31 */
    "b848251e",  /* fmax s24, s5, s5 */
    "bf5b3e1e",  /* fmin s31, s29, s30 */
    "62692f1e",  /* fmaxnm s2, s11, s15 */
    "367b341e",  /* fminnm s22, s25, s20 */
    "148a3d1e",  /* fnmul s20, s16, s29 */
    "5a43201e",  /* fmov s26, s26 */
    "ac41211e",  /* fneg s12, s13 */
    "cdc0211e",  /* fsqrt s13, s6 */
    "6240241e",  /* frintn s2, s3 */
    "9ac0241e",  /* frintp s26, s4 */
    "b943251e",  /* frintm s25, s29 */
    "8bc2251e",  /* frintz s11, s20 */
    "1841261e",  /* frinta s24, s8 */
    "2040271e",  /* frintx s0, s1 */
    "59c3271e",  /* frinti s25, s26 */
    "6925131f",  /* fmadd s9, s11, s19, s9 */
    "e6b40b1f",  /* fmsub s6, s7, s11, s13 */
    "8d55271f",  /* fnmadd s13, s12, s7, s21 */
    "e289351f",  /* fnmsub s2, s15, s21, s2 */
    "2021351e",  /* fcmp s9, s21 */
    "5020321e",  /* fcmpe s2, s18 */
    "e821201e",  /* fcmp s15, #0.0 */
    "1821201e",  /* fcmpe s8, #0.0 */
    "e3c52f1e",  /* fccmp s15, s15, #3, gt */
    "23df331e",  /* fcsel s3, s25, s19, le */
    "02102e1e",  /* fmov s2, #1.0 */
    "0b103c1e",  /* fmov s11, #-0.5 */
    "0f10201e",  /* fmov s15, #2.0 */
    "0510281e",  /* fmov s5, #0.125 */
    "16f0271e",  /* fmov s22, #31.0 */
    "01903e1e",  /* fmov s1, #-1.25 */
    "1e10211e",  /* fmov s30, #3.0 */
    "6003201e",  /* fcvtns w0, s27 */
    "ac02219e",  /* fcvtnu x12, s21 */
    "cf00241e",  /* fcvtas w15, s6 */
    "9900259e",  /* fcvtau x25, s4 */
    "d401281e",  /* fcvtps w20, s14 */
    "8f03299e",  /* fcvtpu x15, s28 */
    "8a03301e",  /* fcvtms w10, s28 */
    "7802319e",  /* fcvtmu x24, s19 */
    "5302389e",  /* fcvtzs x19, s18 */
    "fd03391e",  /* fcvtzu w29, s31 */
    "7300221e",  /* scvtf s19, w3 */
    "8103231e",  /* ucvtf s1, w28 */
    "fb08741e",  /* fmul d27, d7, d20 */
    "631a611e",  /* fdiv d3, d19, d1 */
    "2d29761e",  /* fadd d13, d9, d22 */
    "3639651e",  /* fsub d22, d9, d5 */
    "f148641e",  /* fmax d17, d7, d4 */
    "e959601e",  /* fmin d9, d15, d0 */
    "5269621e",  /* fmaxnm d18, d10, d2 */
    "d87a651e",  /* fminnm d24, d22, d5 */
    "f38b7d1e",  /* fnmul d19, d31, d29 */
    "6a40601e",  /* fmov d10, d3 */
    "2fc3601e",  /* fabs d15, d25 */
    "b042611e",  /* fneg d16, d21 */
    "abc3611e",  /* fsqrt d11, d29 */
    "b540641e",  /* frintn d21, d5 */
    "72c2641e",  /* frintp d18, d19 */
    "8d40651e",  /* frintm d13, d4 */
    "a4c2651e",  /* frintz d4, d21 */
    "1e40661e",  /* frinta d30, d0 */
    "e842671e",  /* frintx d8, d23 */
    "57c2671e",  /* frinti d23, d18 */
    "1e58491f",  /* fmadd d30, d0, d9, d22 */
    "2ac2501f",  /* fmsub d10, d17, d16, d16 */
    "29207a1f",  /* fnmadd d9, d1, d26, d8 */
    "19db671f",  /* fnmsub d25, d24, d7, d22 */
    "4023741e",  /* fcmp d26, d20 */
    "1022741e",  /* fcmpe d16, d20 */
    "e823601e",  /* fcmp d31, #0.0 */
    "5820601e",  /* fcmpe d2, #0.0 */
    "6376791e",  /* fccmp d19, d25, #3, vc */
    "843c7e1e",  /* fcsel d4, d4, d30, cc */
    "0b106e1e",  /* fmov d11, #1.0 */
    "07107c1e",  /* fmov d7, #-0.5 */
    "1a10601e",  /* fmov d26, #2.0 */
    "1010681e",  /* fmov d16, #0.125 */
    "18f0671e",  /* fmov d24, #31.0 */
    "01907e1e",  /* fmov d1, #-1.25 */
    "1c10611e",  /* fmov d28, #3.0 */
    "0101601e",  /* fcvtns w1, d8 */
    "3b00641e",  /* fcvtas w27, d1 */
    "f701659e",  /* fcvtau x23, d15 */
    "9000681e",  /* fcvtps w16, d4 */
    "2901691e",  /* fcvtpu w9, d9 */
    "bb03709e",  /* fcvtms x27, d29 */
    "5c02719e",  /* fcvtmu x28, d18 */
    "9700789e",  /* fcvtzs x23, d4 */
    "e101799e",  /* fcvtzu x1, d15 */
    "1000629e",  /* scvtf d16, x0 */
    "0302631e",  /* ucvtf d3, w16 */
    "9001271e",  /* fmov s16, w12 */
    "0e00261e",  /* fmov w14, s0 */
    "7602679e",  /* fmov d22, x19 */
    "d502669e",  /* fmov x21, d22 */
    "1084234e",  /* add v16.16b, v0.16b, v3.16b */
    "39846f2e",  /* sub v25.4h, v1.4h, v15.4h */
    "a78f616e",  /* cmeq v7.8h, v29.8h, v1.8h */
    "bc36aa4e",  /* cmgt v28.4s, v21.4s, v10.4s */
    "543e744e",  /* cmge v20.8h, v18.8h, v20.8h */
    "2937732e",  /* cmhi v9.4h, v25.4h, v19.4h */
    "913e3a2e",  /* cmhs v17.8b, v20.8b, v26.8b */
    "52bda50e",  /* addp v18.2s, v10.2s, v5.2s */
    "6846f96e",  /* ushl v8.2d, v19.2d, v25.2d */
    "9f44700e",  /* sshl v31.4h, v4.4h, v16.4h */
    "5d8ea74e",  /* cmtst v29.4s, v18.4s, v7.4s */
    "719ea64e",  /* mul v17.4s, v19.4s, v6.4s */
    "4667a22e",  /* umax v6.2s, v26.2s, v2.2s */
    "ba6d342e",  /* umin v26.8b, v13.8b, v20.8b */
    "5d663d4e",  /* smax v29.16b, v18.16b, v29.16b */
    "376f750e",  /* smin v23.4h, v25.4h, v21.4h */
    "fe0d3e2e",  /* uqadd v30.8b, v15.8b, v30.8b */
    "8c0fad4e",  /* sqadd v12.4s, v28.4s, v13.4s */
    "392d392e",  /* uqsub v25.8b, v9.8b, v25.8b */
    "582db90e",  /* sqsub v24.2s, v10.2s, v25.2s */
    "001f3a4e",  /* and v0.16b, v24.16b, v26.16b */
    "631ea64e",  /* orr v3.16b, v19.16b, v6.16b */
    "501e202e",  /* eor v16.8b, v18.8b, v0.8b */
    "351c770e",  /* bic v21.8b, v1.8b, v23.8b */
    "781fee4e",  /* orn v24.16b, v27.16b, v14.16b */
    "e21f7c2e",  /* bsl v2.8b, v31.8b, v28.8b */
    "6c1fbe2e",  /* bit v12.8b, v27.8b, v30.8b */
    "861cfb2e",  /* bif v6.8b, v4.8b, v27.8b */
    "9bd6314e",  /* fadd v27.4s, v20.4s, v17.4s */
    "6cd7e54e",  /* fsub v12.2d, v27.2d, v5.2d */
    "87dd606e",  /* fmul v7.2d, v12.2d, v0.2d */
    "bcfc7a6e",  /* fdiv v28.2d, v5.2d, v26.2d */
    "8bf4764e",  /* fmax v11.2d, v4.2d, v22.2d */
    "1ef7b14e",  /* fmin v30.4s, v24.4s, v17.4s */
    "0cc43e0e",  /* fmaxnm v12.2s, v0.2s, v30.2s */
    "03c5f94e",  /* fminnm v3.2d, v8.2d, v25.2d */
    "7e1fbb4e",  /* mov v30.16b, v27.16b */
    "571daa0e",  /* mov v23.8b, v10.8b */
    "4c5a206e",  /* not v12.16b, v18.16b */
    "595b202e",  /* mvn v25.8b, v26.8b */
    "435b200e",  /* cnt v3.8b, v26.8b */
    "9a09a04e",  /* rev64 v26.4s, v12.4s */
    "f9bae06e",  /* neg v25.2d, v23.2d */
    "e8bb604e",  /* abs v8.8h, v31.8h */
    "81b9314e",  /* addv b1, v12.16b */
    "a2ba710e",  /* addv h2, v21.4h */
    "a3b8b14e",  /* addv s3, v5.4s */
    "e43a302e",  /* uaddlv h4, v23.8b */
    "45a8306e",  /* umaxv b5, v2.16b */
    "e6a8b16e",  /* uminv s6, v7.4s */
    "a80e044e",  /* dup v8.4s, w21 */
    "ca0d084e",  /* dup v10.2d, x14 */
    "7104144e",  /* dup v17.4s, v3.s[2] */
    "fc04134e",  /* dup v28.16b, v7.b[9] */
    "46e7014f",  /* movi v6.16b, #58 */
    "14e4000f",  /* movi v20.8b, #0 */
    "7f1a106e",  /* ext v31.16b, v19.16b, v16.16b, #3 */
    "8d29122e",  /* ext v13.8b, v12.8b, v18.8b, #5 */
    "a774404c",  /* ld1 {v7.8h}, [x5] */
    "e777004c",  /* st1 {v7.8h}, [sp] */
    "e777df4c",  /* ld1 {v7.8h}, [sp], #16 */
    "27779b4c",  /* st1 {v7.8h}, [x25], x27 */
    "e7a7400c",  /* ld1 {v7.4h, v8.4h}, [sp] */
    "e7a7000c",  /* st1 {v7.4h, v8.4h}, [sp] */
    "e7a7df0c",  /* ld1 {v7.4h, v8.4h}, [sp], #16 */
    "e7a7980c",  /* st1 {v7.4h, v8.4h}, [sp], x24 */
    "e76b404c",  /* ld1 {v7.4s, v8.4s, v9.4s}, [sp] */
    "e76b834c",  /* st1 {v7.4s, v8.4s, v9.4s}, [sp], x3 */
    "e72f404c",  /* ld1 {v7.2d, v8.2d, v9.2d, v10.2d}, [sp] */
    "872d004c",  /* st1 {v7.2d, v8.2d, v9.2d, v10.2d}, [x12] */
    "e72fdf4c",  /* ld1 {v7.2d, v8.2d, v9.2d, v10.2d}, [sp], #64 */
    "e72f804c",  /* st1 {v7.2d, v8.2d, v9.2d, v10.2d}, [sp], x0 */
    "e7ff7f91",  /* add x7, sp, #16773120 */
    "471c4091",  /* add x7, x2, #7, lsl #12 */
    "e8031f8b",  /* add x8, xzr, xzr */
    "ec009f8b",  /* add x12, x7, xzr, asr #0 */
    "a7a2258b",  /* add x7, x21, w5, sxth #0 */
    "27e13a8b",  /* add x7, x9, x26, sxtx */
    "ff0740b1",  /* adds xzr, sp, #4096 */
    "ff0740b1",  /* adds xzr, sp, #1, lsl #12 */
    "7f001fab",  /* adds xzr, x3, xzr */
    "ff435fab",  /* adds xzr, xzr, xzr, lsr #16 */
    "ff5332ab",  /* adds xzr, sp, w18, uxtw #4 */
    "ff6332ab",  /* adds xzr, sp, x18, uxtx */
    "bffe3fd1",  /* sub sp, x21, #4095 */
    "7f1e40d1",  /* sub sp, x19, #7, lsl #12 */
    "de021fcb",  /* sub x30, x22, xzr */
    "e92f1fcb",  /* sub x9, xzr, xzr, lsl #11 */
    "3f232dcb",  /* sub sp, x25, w13, uxth #0 */
    "ffe338cb",  /* sub sp, sp, x24, sxtx */
    "3b0440f1",  /* subs x27, x1, #4096 */
    "fb1f40f1",  /* subs x27, sp, #7, lsl #12 */
    "f00300eb",  /* subs x16, xzr, x0 */
    "c3239feb",  /* subs x3, x30, xzr, asr #8 */
    "9b483beb",  /* subs x27, x4, w27, uxtw #2 */
    "7be13ceb",  /* subs x27, x11, x28, sxtx */
    "ff6f31f1",  /* cmp sp, #3163 */
    "9f021fab",  /* cmn x20, xzr */
    "bf0d14eb",  /* cmp x13, x20, lsl #3 */
    "f70316cb",  /* neg x23, x22 */
    "e70b1deb",  /* negs x7, x29, lsl #2 */
    "ff031f9a",  /* adc xzr, xzr, xzr */
    "ff031fba",  /* adcs xzr, xzr, xzr */
    "f3031fda",  /* sbc x19, xzr, xzr */
    "ff031ffa",  /* sbcs xzr, xzr, xzr */
    "e00312da",  /* ngc x0, x18 */
    "e1031efa",  /* ngcs x1, x30 */
    "f5cf0092",  /* and x21, xzr, #0xf0f0f0f0f0f0f0f */
    "fff300b2",  /* orr sp, xzr, #0x5555555555555555 */
    "ebcf00d2",  /* eor x11, xzr, #0xf0f0f0f0f0f0f0f */
    "7ff000f2",  /* ands xzr, x3, #0x5555555555555555 */
    "e9031f8a",  /* and x9, xzr, xzr */
    "9f569f8a",  /* and xzr, x20, xzr, asr #21 */
    "da011faa",  /* orr x26, x14, xzr */
    "f70708aa",  /* orr x23, xzr, x8, lsl #1 */
    "50011fca",  /* eor x16, x10, xzr */
    "ea27d2ca",  /* eor x10, xzr, x18, ror #9 */
    "ff031fea",  /* ands xzr, xzr, xzr */
    "ff1b5fea",  /* ands xzr, xzr, xzr, lsr #6 */
    "ff03318a",  /* bic xzr, xzr, x17 */
    "ff6f7f8a",  /* bic xzr, xzr, xzr, lsr #27 */
    "7f0021aa",  /* orn xzr, x3, x1 */
    "fc3f32aa",  /* orn x28, xzr, x18, lsl #15 */
    "f70327ca",  /* eon x23, xzr, x7 */
    "3f6caaca",  /* eon xzr, x1, x10, asr #27 */
    "3f032bea",  /* bics xzr, x25, x11 */
    "2676a5ea",  /* bics x6, x17, x5, asr #29 */
    "9ff300f2",  /* tst x28, #0x5555555555555555 */
    "5f031dea",  /* tst x26, x29 */
    "e0032faa",  /* mvn x0, x15 */
    "f40312aa",  /* mov x20, x18 */
    "7f030091",  /* mov sp, x27 */
    "ec030091",  /* mov x12, sp */
    "010080d2",  /* mov x1, #0 */
    "380080d2",  /* mov x24, #1 */
    "e8ff9fd2",  /* mov x8, #65535 */
    "2700a0d2",  /* mov x7, #65536 */
    "e8ffbfd2",  /* mov x8, #4294901760 */
    "13008092",  /* mov x19, #-1 */
    "30008092",  /* mov x16, #-2 */
    "f0ff9f92",  /* mov x16, #-65536 */
    "8d46c2d2",  /* mov x13, #20014547599360 */
    "e1ffffd2",  /* mov x1, #18446462598732840960 */
    "eff300b2",  /* mov x15, #6148914691236517205 */
    "aa1da0d2",  /* movz x10, #237, lsl #16 */
    "7b8283f2",  /* movk x27, #7187 */
    "981e8892",  /* movn x24, #16628 */
    "e1f9e7f2",  /* movk x1, #16335, lsl #48 */
    "418061d3",  /* lsl x1, x2, #31 */
    "6123d69a",  /* lsl x1, x27, x22 */
    "02fe61d3",  /* lsr x2, x16, #33 */
    "4f25c59a",  /* lsr x15, x10, x5 */
    "4afc5793",  /* asr x10, x2, #23 */
    "8c2acc9a",  /* asr x12, x20, x12 */
    "3261c993",  /* ror x18, x9, #24 */
    "c82cca9a",  /* ror x8, x6, x10 */
    "6d20c49a",  /* lslv x13, x3, x4 */
    "1124d69a",  /* lsrv x17, x0, x22 */
    "9729d99a",  /* asrv x23, x12, x25 */
    "422ec59a",  /* rorv x2, x18, x5 */
    "6109ce9a",  /* udiv x1, x11, x14 */
    "930ed99a",  /* sdiv x19, x20, x25 */
    "8cee62d3",  /* ubfx x12, x20, #34, #26 */
    "93774293",  /* sbfx x19, x28, #2, #28 */
    "8b8a43b3",  /* bfxil x11, x20, #3, #32 */
    "563550d3",  /* ubfiz x22, x10, #48, #14 */
    "b60d4493",  /* sbfiz x22, x13, #60, #4 */
    "c7887fb3",  /* bfi x7, x6, #1, #35 */
    "c8da49d3",  /* ubfm x8, x22, #9, #54 */
    "a70d5093",  /* sbfm x7, x13, #16, #3 */
    "5d856fb3",  /* bfm x29, x10, #47, #33 */
    "e3137db3",  /* bfc x3, #3, #5 */
    "cec2c393",  /* extr x14, x22, x3, #48 */
    "7528179b",  /* madd x21, x3, x23, x10 */
    "59fe169b",  /* msub x25, x18, x22, xzr */
    "e47c189b",  /* mul x4, x7, x24 */
    "2cfc109b",  /* mneg x12, x1, x16 */
    "4202c0da",  /* rbit x2, x18 */
    "a306c0da",  /* rev16 x3, x21 */
    "9c0dc0da",  /* rev x28, x12 */
    "4513c0da",  /* clz x5, x26 */
    "4015c0da",  /* cls x0, x10 */
    "1a709f9a",  /* csel x26, x0, xzr, vc */
    "3f27819a",  /* csinc xzr, x25, x1, hs */
    "b1339fda",  /* csinv x17, x29, xzr, lo */
    "fac494da",  /* csneg x26, x7, x20, gt */
    "f3b79f9a",  /* cset x19, ge */
    "e8839fda",  /* csetm x8, ls */
    "93858c9a",  /* cinc x19, x12, ls */
    "b1318dda",  /* cinv x17, x13, cs */
    "0c4690da",  /* cneg x12, x16, pl */
    "a5b34dfa",  /* ccmp x29, x13, #5, lt */
    "c4aa5efa",  /* ccmp x22, #30, #4, ge */
    "84335cba",  /* ccmn x28, x28, #4, lo */
    "6fc85fba",  /* ccmn x3, #31, #15, gt */
    "125100b4",  /* cbz x18, 1f */
    "b723ffb5",  /* cbnz x23, 1b */
    "db50b836",  /* tbz x27, #23, 1f */
    "642317b7",  /* tbnz x4, #34, 1b */
    "f8070011",  /* add w24, wsp, #1 */
    "f81f4011",  /* add w24, wsp, #7, lsl #12 */
    "9f03090b",  /* add wzr, w28, w9 */
    "ff2f860b",  /* add wzr, wzr, w6, asr #11 */
    "f82f2f0b",  /* add w24, wsp, w15, uxth #3 */
    "df064031",  /* adds wzr, w22, #4096 */
    "5f1e4031",  /* adds wzr, w18, #7, lsl #12 */
    "f703192b",  /* adds w23, wzr, w25 */
    "8e6f142b",  /* adds w14, w28, w20, lsl #27 */
    "1f293c2b",  /* adds wzr, w8, w28, uxth #2 */
    "64fe3f51",  /* sub w4, w19, #4095 */
    "a41c4051",  /* sub w4, w5, #7, lsl #12 */
    "5f00034b",  /* sub wzr, w2, w3 */
    "ff031f4b",  /* sub wzr, wzr, wzr, lsl #0 */
    "e48f2d4b",  /* sub w4, wsp, w13, sxtb #3 */
    "e2074071",  /* subs w2, wsp, #4096 */
    "e2074071",  /* subs w2, wsp, #1, lsl #12 */
    "ff031f6b",  /* subs wzr, wzr, wzr */
    "ff275f6b",  /* subs wzr, wzr, wzr, lsr #9 */
    "e253236b",  /* subs w2, wsp, w3, uxtw #4 */
    "3f363f71",  /* cmp w17, #4045 */
    "ff031f2b",  /* cmn wzr, wzr */
    "bf0e176b",  /* cmp w21, w23, lsl #3 */
    "f203174b",  /* neg w18, w23 */
    "e50b146b",  /* negs w5, w20, lsl #2 */
    "f4031f1a",  /* adc w20, wzr, wzr */
    "a3031f3a",  /* adcs w3, w29, wzr */
    "ff03145a",  /* sbc wzr, wzr, w20 */
    "3f01167a",  /* sbcs wzr, w9, w22 */
    "e303165a",  /* ngc w3, w22 */
    "e503187a",  /* ngcs w5, w24 */
    "7fcd0012",  /* and wsp, w11, #0xf0f0f0f */
    "3f1c0032",  /* orr wsp, w1, #0xff */
    "ff7b1f52",  /* eor wsp, wzr, #0xfffffffe */
    "df791f72",  /* ands wzr, w14, #0xfffffffe */
    "ef03120a",  /* and w15, wzr, w18 */
    "5556c50a",  /* and w21, w18, w5, ror #21 */
    "5f03122a",  /* orr wzr, w26, w18 */
    "3f1b512a",  /* orr wzr, w25, w17, lsr #6 */
    "ff031f4a",  /* eor wzr, wzr, wzr */
    "ff43df4a",  /* eor wzr, wzr, wzr, ror #16 */
    "ff03006a",  /* ands wzr, wzr, w0 */
    "7718cf6a",  /* ands w23, w3, w15, ror #6 */
    "e6033f0a",  /* bic w6, wzr, wzr */
    "fe03390a",  /* bic w30, wzr, w25, lsl #0 */
    "0b022f2a",  /* orn w11, w16, w15 */
    "ff0b7f2a",  /* orn wzr, wzr, wzr, lsr #2 */
    "db023f4a",  /* eon w27, w22, wzr */
    "b95bfb4a",  /* eon w25, w29, w27, ror #22 */
    "f803236a",  /* bics w24, wzr, w3 */
    "6d3ff66a",  /* bics w13, w27, w22, ror #15 */
    "ff1d1872",  /* tst w15, #0xff00 */
    "3f021b6a",  /* tst w17, w27 */
    "ed03262a",  /* mvn w13, w6 */
    "f4030f2a",  /* mov w20, w15 */
    "3f010011",  /* mov wsp, w9 */
    "e0030011",  /* mov w0, wsp */
    "0e008052",  /* mov w14, #0 */
    "2e008052",  /* mov w14, #1 */
    "f8ff9f52",  /* mov w24, #65535 */
    "3600a052",  /* mov w22, #65536 */
    "0e008012",  /* mov w14, #-1 */
    "25008012",  /* mov w5, #-2 */
    "eeffbf52",  /* mov w14, #-65536 */
    "fcf30032",  /* mov w28, #1431655765 */
    "816cb052",  /* movz w1, #33636, lsl #16 */
    "0bbe9772",  /* movk w11, #48624 */
    "4e239712",  /* movn w14, #47386 */
    "935d1853",  /* lsl w19, w12, #8 */
    "2023c61a",  /* lsl w0, w25, w6 */
    "287f0c53",  /* lsr w8, w25, #12 */
    "6427ce1a",  /* lsr w4, w27, w14 */
    "d17c0613",  /* asr w17, w6, #6 */
    "0628c51a",  /* asr w6, w0, w5 */
    "92458c13",  /* ror w18, w12, #17 */
    "852ec01a",  /* ror w5, w20, w0 */
    "6420d31a",  /* lslv w4, w3, w19 */
    "c525cf1a",  /* lsrv w5, w14, w15 */
    "2528da1a",  /* asrv w5, w1, w26 */
    "802dce1a",  /* rorv w0, w12, w14 */
    "aa09c11a",  /* udiv w10, w13, w1 */
    "d60ec11a",  /* sdiv w22, w22, w1 */
    "2c4c0753",  /* ubfx w12, w1, #7, #13 */
    "a05b0f13",  /* sbfx w0, w29, #15, #8 */
    "83590733",  /* bfxil w3, w12, #7, #16 */
    "454d1a53",  /* ubfiz w5, w10, #6, #20 */
    "ab0f1d13",  /* sbfiz w11, w29, #3, #4 */
    "212f0d33",  /* bfi w1, w25, #19, #12 */
    "094d1d53",  /* ubfm w9, w8, #29, #19 */
    "ef041113",  /* sbfm w15, w7, #17, #1 */
    "5a511633",  /* bfm w26, w10, #22, #20 */
    "e2131d33",  /* bfc w2, #3, #5 */
    "a1168d13",  /* extr w1, w21, w13, #5 */
    "720e001b",  /* madd w18, w19, w0, w3 */
    "5594001b",  /* msub w21, w2, w0, w5 */
    "2f7c061b",  /* mul w15, w1, w6 */
    "14fe1c1b",  /* mneg w20, w16, w28 */
    "ca00c05a",  /* rbit w10, w6 */
    "1c07c05a",  /* rev16 w28, w24 */
    "4f09c05a",  /* rev w15, w10 */
    "f911c05a",  /* clz w25, w15 */
    "7d15c05a",  /* cls w29, w11 */
    "fe239f1a",  /* csel w30, wzr, wzr, cs */
    "ff879f1a",  /* csinc wzr, wzr, wzr, hi */
    "ff339f5a",  /* csinv wzr, wzr, wzr, lo */
    "df959f5a",  /* csneg wzr, w14, wzr, ls */
    "e8279f1a",  /* cset w8, lo */
    "f0b39f5a",  /* csetm w16, ge */
    "fe058f1a",  /* cinc w30, w15, ne */
    "a410855a",  /* cinv w4, w5, eq */
    "da358e5a",  /* cneg w26, w14, cs */
    "0383567a",  /* ccmp w24, w22, #3, hi */
    "e1c8437a",  /* ccmp w7, #3, #1, gt */
    "ca915c3a",  /* ccmn w14, w28, #10, ls */
    "06a8443a",  /* ccmn w0, #4, #6, ge */
    "39420034",  /* cbz w25, 1f */
    "de14ff35",  /* cbnz w30, 1b */
    "fd413036",  /* tbz w29, #6, 1f */
    "8a149f37",  /* tbnz w10, #19, 1b */
    "421f4093",  /* sxtb x2, w26 */
    "c73e0013",  /* sxth w7, w22 */
    "817c4093",  /* sxtw x1, w4 */
    "9d1e0053",  /* uxtb w29, w20 */
    "443e0053",  /* uxth w4, w18 */
    "6008c0da",  /* rev32 x0, x3 */
    "270dc0da",  /* rev64 x7, x9 */
    "de6c279b",  /* smaddl x30, w6, w7, x27 */
    "11c22d9b",  /* smsubl x17, w16, w13, x16 */
    "b82bb39b",  /* umaddl x24, w29, w19, x10 */
    "99e7b19b",  /* umsubl x25, w28, w17, x25 */
    "c67d259b",  /* smull x6, w14, w5 */
    "537ca19b",  /* umull x19, w2, w1 */
    "3aff239b",  /* smnegl x26, w25, w3 */
    "13fcbb9b",  /* umnegl x19, w0, w27 */
    "7e7c469b",  /* smulh x30, x3, x6 */
    "9e7fc89b",  /* umulh x30, x28, x8 */
    "6240ce1a",  /* crc32b w2, w3, w14 */
    "ec44da1a",  /* crc32h w12, w7, w26 */
    "ba4ad31a",  /* crc32w w26, w21, w19 */
    "8352cf1a",  /* crc32cb w3, w20, w15 */
    "b856db1a",  /* crc32ch w24, w21, w27 */
    "7659cc1a",  /* crc32cw w22, w11, w12 */
    "b34ece9a",  /* crc32x w19, w21, x14 */
    "785cc99a",  /* crc32cx w24, w3, x9 */
    "40031fd6",  /* br x26 */
    "c0033fd6",  /* blr x30 */
    "60025fd6",  /* ret x19 */
    "ee10ff10",  /* adr x14, 1b */
    "1a3e0010",  /* adr x26, 1f */
    "cc0040f9",  /* ldr x12, [x6] */
    "f14f66f9",  /* ldr x17, [sp, #19608] */
    "973240f8",  /* ldr x23, [x20, #3] */
    "eb0f50f8",  /* ldr x11, [sp, #-256]! */
    "fdf74ff8",  /* ldr x29, [sp], #255 */
    "996a73f8",  /* ldr x25, [x20, x19] */
    "d0786bf8",  /* ldr x16, [x6, x11, lsl #3] */
    "facb67f8",  /* ldr x26, [sp, w7, sxtw] */
    "eedb66f8",  /* ldr x14, [sp, w6, sxtw #3] */
    "aae877f8",  /* ldr x10, [x5, x23, sxtx] */
    "e60300f9",  /* str x6, [sp] */
    "fd132ff9",  /* str x29, [sp, #24096] */
    "fef30ff8",  /* str x30, [sp, #255] */
    "fcff0ff8",  /* str x28, [sp, #255]! */
    "b28700f8",  /* str x18, [x29], #8 */
    "be6b25f8",  /* str x30, [x29, x5] */
    "7c7837f8",  /* str x28, [x3, x23, lsl #3] */
    "24c934f8",  /* str x4, [x9, w20, sxtw] */
    "c1d828f8",  /* str x1, [x6, w8, sxtw #3] */
    "afe92bf8",  /* str x15, [x13, x11, sxtx] */
    "ee6242f8",  /* ldur x14, [x23, #38] */
    "e42303f8",  /* stur x4, [sp, #50] */
    "fd0340b9",  /* ldr w29, [sp] */
    "c2607eb9",  /* ldr w2, [x6, #15968] */
    "f8f34fb8",  /* ldr w24, [sp, #255] */
    "f90f5fb8",  /* ldr w25, [sp, #-16]! */
    "edf74fb8",  /* ldr w13, [sp], #255 */
    "306a6bb8",  /* ldr w16, [x17, x11] */
    "fe7b69b8",  /* ldr w30, [sp, x9, lsl #2] */
    "e0cb7cb8",  /* ldr w0, [sp, w28, sxtw] */
    "f9db77b8",  /* ldr w25, [sp, w23, sxtw #2] */
    "a7eb60b8",  /* ldr w7, [x29, x0, sxtx] */
    "840200b9",  /* str w4, [x20] */
    "116c06b9",  /* str w17, [x0, #1644] */
    "e03300b8",  /* str w0, [sp, #3] */
    "fe0f10b8",  /* str w30, [sp, #-256]! */
    "af0410b8",  /* str w15, [x5], #-256 */
    "e86b3ab8",  /* str w8, [sp, x26] */
    "617a2eb8",  /* str w1, [x19, x14, lsl #2] */
    "09c935b8",  /* str w9, [x8, w21, sxtw] */
    "eddb2ab8",  /* str w13, [sp, w10, sxtw #2] */
    "e1eb21b8",  /* str w1, [sp, x1, sxtx] */
    "b4d25fb8",  /* ldur w20, [x21, #-3] */
    "fe9305b8",  /* stur w30, [sp, #89] */
    "f1234239",  /* ldrb w17, [sp, #136] */
    "251c4f38",  /* ldrb w5, [x1, #241]! */
    "f4e74d38",  /* ldrb w20, [sp], #222 */
    "f06b6b38",  /* ldrb w16, [sp, x11] */
    "907a6938",  /* ldrb w16, [x20, x9, lsl #0] */
    "45000d39",  /* strb w5, [x2, #832] */
    "ed6f0c38",  /* strb w13, [sp, #198]! */
    "eef70c38",  /* strb w14, [sp], #207 */
    "c96b3238",  /* strb w9, [x30, x18] */
    "8a782d38",  /* strb w10, [x4, x13, lsl #0] */
    "6fbf7a79",  /* ldrh w15, [x27, #7518] */
    "fdff5178",  /* ldrh w29, [sp, #-225]! */
    "4a364f78",  /* ldrh w10, [x18], #243 */
    "e46b6378",  /* ldrh w4, [sp, x3] */
    "ad7a6b78",  /* ldrh w13, [x21, x11, lsl #1] */
    "e1533979",  /* strh w1, [sp, #7336] */
    "e69f1b78",  /* strh w6, [sp, #-71]! */
    "f4370478",  /* strh w20, [sp], #67 */
    "01693e78",  /* strh w1, [x8, x30] */
    "e17b2a78",  /* strh w1, [sp, x10, lsl #1] */
    "3181c039",  /* ldrsb w17, [x9, #32] */
    "8d6baf38",  /* ldrsb x13, [x28, x15] */
    "e6668179",  /* ldrsh x6, [x23, #178] */
    "976af478",  /* ldrsh w23, [x20, x20] */
    "5a1fa4b9",  /* ldrsw x26, [x26, #9244] */
    "de4680b8",  /* ldrsw x30, [x22], #4 */
    "ed7baeb8",  /* ldrsw x13, [sp, x14, lsl #2] */
    "28215b38",  /* ldurb w8, [x9, #-78] */
    "e7311938",  /* sturb w7, [x15, #-109] */
    "84e34d78",  /* ldurh w4, [x28, #222] */
    "95111978",  /* sturh w21, [x12, #-111] */
    "2c40db38",  /* ldursb w12, [x1, #-76] */
    "c960d978",  /* ldursh w9, [x6, #-106] */
    "01c29fb8",  /* ldursw x1, [x16, #-4] */
    "d32080f9",  /* prfm pstl2strm, [x6, #64] */
    "f80f77a9",  /* ldp x24, x3, [sp, #-144] */
    "60a0eea9",  /* ldp x0, x8, [x3, #-280]! */
    "e427cda8",  /* ldp x4, x9, [sp], #208 */
    "7ef638a9",  /* stp x30, x29, [x19, #-120] */
    "60799da9",  /* stp x0, x30, [x11, #464]! */
    "e70bada8",  /* stp x7, x2, [sp], #-304 */
    "f6ef41a8",  /* ldnp x22, x27, [sp, #24] */
    "e05f00a8",  /* stnp x0, x23, [sp] */
    "f2b75829",  /* ldp w18, w13, [sp, #196] */
    "e4c7f929",  /* ldp w4, w17, [sp, #-52]! */
    "384dc628",  /* ldp w24, w19, [x9], #48 */
    "5eef0829",  /* stp w30, w27, [x26, #68] */
    "c8e8a629",  /* stp w8, w26, [x6, #-204]! */
    "b385ae28",  /* stp w19, w1, [x13], #-140 */
    "b2804128",  /* ldnp w18, w0, [x5, #12] */
    "f9330028",  /* stnp w25, w12, [sp] */
    "f05b4169",  /* ldpsw x16, x22, [sp, #8] */
    "e5f7492d",  /* ldp s5, s29, [sp, #76] */
    "e9b70f2d",  /* stp s9, s13, [sp, #124] */
    "f4db4b6d",  /* ldp d20, d22, [sp, #184] */
    "12cd2f6d",  /* stp d18, d19, [x8, #-264] */
    "ee5644ad",  /* ldp q14, q21, [x23, #128] */
    "db451dad",  /* stp q27, q17, [x14, #928] */
    "3586783d",  /* ldr b21, [x17, #3617] */
    "e33f4b3c",  /* ldr b3, [sp, #179]! */
    "32687b3c",  /* ldr b18, [x1, x27] */
    "9891093d",  /* str b24, [x12, #612] */
    "178e133c",  /* str b23, [x16, #-200]! */
    "9f6b2d3c",  /* str b31, [x28, x13] */
    "f8d35f3c",  /* ldur b24, [sp, #-3] */
    "e07f6f7d",  /* ldr h0, [sp, #6078] */
    "f9af567c",  /* ldr h25, [sp, #-150]! */
    "ff7b677c",  /* ldr h31, [sp, x7, lsl #1] */
    "6005007d",  /* str h0, [x11, #2] */
    "886f1d7c",  /* str h8, [x28, #-42]! */
    "f47b267c",  /* str h20, [sp, x6, lsl #1] */
    "e1d35f7c",  /* ldur h1, [sp, #-3] */
    "fc475bbd",  /* ldr s28, [sp, #6980] */
    "5c3d48bc",  /* ldr s28, [x10, #131]! */
    "ee797cbc",  /* ldr s14, [x15, x28, lsl #2] */
    "33050fbd",  /* str s19, [x9, #3844] */
    "6b9e0bbc",  /* str s11, [x19, #185]! */
    "e17b2cbc",  /* str s1, [sp, x12, lsl #2] */
    "e0d15fbc",  /* ldur s0, [x15, #-3] */
    "1ec171fd",  /* ldr d30, [x8, #25472] */
    "ed1f43fc",  /* ldr d13, [sp, #49]! */
    "3b7974fc",  /* ldr d27, [x9, x20, lsl #3] */
    "e51b29fd",  /* str d5, [sp, #21040] */
    "64cf04fc",  /* str d4, [x27, #76]! */
    "fe7b34fc",  /* str d30, [sp, x20, lsl #3] */
    "e6d35ffc",  /* ldur d6, [sp, #-3] */
    "9a4fe93d",  /* ldr q26, [x28, #42288] */
    "ecafd53c",  /* ldr q12, [sp, #-166]! */
    "837afc3c",  /* ldr q3, [x20, x28, lsl #4] */
    "4f0a973d",  /* str q15, [x18, #23584] */
    "ee5f893c",  /* str q14, [sp, #149]! */
    "f27bb53c",  /* str q18, [sp, x21, lsl #4] */
    "eed3df3c",  /* ldur q14, [sp, #-3] */
    "8a7f5fc8",  /* ldxr x10, [x28] */
    "f2ff5fc8",  /* ldaxr x18, [sp] */
    "f3ffdfc8",  /* ldar x19, [sp] */
    "0aff9fc8",  /* stlr x10, [x24] */
    "e17f0ec8",  /* stxr w14, x1, [sp] */
    "89fe1bc8",  /* stlxr w27, x9, [x20] */
    "6f6f7fc8",  /* ldxp x15, x27, [x27] */
    "e5eb7fc8",  /* ldaxp x5, x26, [sp] */
    "fd7723c8",  /* stxp w3, x29, x29, [sp] */
    "a6a83ac8",  /* stlxp w26, x6, x10, [x5] */
    "ea7d5f88",  /* ldxr w10, [x15] */
    "8cff5f88",  /* ldaxr w12, [x28] */
    "e7ffdf88",  /* ldar w7, [sp] */
    "ecfc9f88",  /* stlr w12, [x7] */
    "e87f0988",  /* stxr w9, w8, [sp] */
    "f6ff0188",  /* stlxr w1, w22, [sp] */
    "d70c7f88",  /* ldxp w23, w3, [x6] */
    "a9b87f88",  /* ldaxp w9, w14, [x5] */
    "cf582788",  /* stxp w7, w15, w22, [x6] */
    "75ae3988",  /* stlxp w25, w21, w11, [x19] */
    "e57f5f08",  /* ldxrb w5, [sp] */
    "f3ff5f08",  /* ldaxrb w19, [sp] */
    "e57f1308",  /* stxrb w19, w5, [sp] */
    "2bfc0e08",  /* stlxrb w14, w11, [x1] */
    "f9ffdf08",  /* ldarb w25, [sp] */
    "0ffc9f08",  /* stlrb w15, [x0] */
    "e87f5f48",  /* ldxrh w8, [sp] */
    "f5fd5f48",  /* ldaxrh w21, [x15] */
    "fa7f0c48",  /* stxrh w12, w26, [sp] */
    "ecff0b48",  /* stlxrh w11, w12, [sp] */
    "b8fcdf48",  /* ldarh w24, [x5] */
    "f6ff9f48",  /* stlrh w22, [sp] */
    "527da488",  /* cas w4, w18, [x10] */
    "fb7fa408",  /* casb w4, w27, [sp] */
    "fb7fa448",  /* cash w4, w27, [sp] */
    "727de1c8",  /* casa x1, x18, [x11] */
    "f87ff608",  /* casab w22, w24, [sp] */
    "fc7ff548",  /* casah w21, w28, [sp] */
    "fdffba88",  /* casl w26, w29, [sp] */
    "f5ffad08",  /* caslb w13, w21, [sp] */
    "b2fcab48",  /* caslh w11, w18, [x5] */
    "f2ffe6c8",  /* casal x6, x18, [sp] */
    "f1ffea08",  /* casalb w10, w17, [sp] */
    "e5ffef48",  /* casalh w15, w5, [sp] */
    "ed833df8",  /* swp x29, x13, [sp] */
    "f3812138",  /* swpb w1, w19, [x15] */
    "65803578",  /* swph w21, w5, [x3] */
    "fd83a3b8",  /* swpa w3, w29, [sp] */
    "ef83a238",  /* swpab w2, w15, [sp] */
    "c380a878",  /* swpah w8, w3, [x6] */
    "e4836df8",  /* swpl x13, x4, [sp] */
    "48826538",  /* swplb w5, w8, [x18] */
    "e8836778",  /* swplh w7, w8, [sp] */
    "6380fcb8",  /* swpal w28, w3, [x3] */
    "4e81e738",  /* swpalb w7, w14, [x10] */
    "a480f878",  /* swpalh w24, w4, [x5] */
    "f00328b8",  /* ldadd w8, w16, [sp] */
    "f7032e38",  /* ldaddb w14, w23, [sp] */
    "e8032d78",  /* ldaddh w13, w8, [sp] */
    "2400acb8",  /* ldadda w12, w4, [x1] */
    "c000b438",  /* ldaddab w20, w0, [x6] */
    "e603be78",  /* ldaddah w30, w6, [sp] */
    "f10374f8",  /* ldaddl x20, x17, [sp] */
    "c6036538",  /* ldaddlb w5, w6, [x30] */
    "89007778",  /* ldaddlh w23, w9, [x4] */
    "7a02f5b8",  /* ldaddal w21, w26, [x19] */
    "5300e238",  /* ldaddalb w2, w19, [x2] */
    "e203f678",  /* ldaddalh w22, w2, [sp] */
    "f51331b8",  /* ldclr w17, w21, [sp] */
    "1a103638",  /* ldclrb w22, w26, [x0] */
    "ad103678",  /* ldclrh w22, w13, [x5] */
    "f713b0f8",  /* ldclra x16, x23, [sp] */
    "8611b938",  /* ldclrab w25, w6, [x12] */
    "fd13ae78",  /* ldclrah w14, w29, [sp] */
    "ee106eb8",  /* ldclrl w14, w14, [x7] */
    "e5136738",  /* ldclrlb w7, w5, [sp] */
    "72106a78",  /* ldclrlh w10, w18, [x3] */
    "ec13e0f8",  /* ldclral x0, x12, [sp] */
    "f213ee38",  /* ldclralb w14, w18, [sp] */
    "ac12f378",  /* ldclralh w19, w12, [x21] */
    "34212db8",  /* ldeor w13, w20, [x9] */
    "10222e38",  /* ldeorb w14, w16, [x16] */
    "d2233e78",  /* ldeorh w30, w18, [x30] */
    "0922a4f8",  /* ldeora x4, x9, [x16] */
    "6920b038",  /* ldeorab w16, w9, [x3] */
    "f923af78",  /* ldeorah w15, w25, [sp] */
    "6f2260f8",  /* ldeorl x0, x15, [x19] */
    "fb217038",  /* ldeorlb w16, w27, [x15] */
    "e9237578",  /* ldeorlh w21, w9, [sp] */
    "2923f8f8",  /* ldeoral x24, x9, [x25] */
    "8221f238",  /* ldeoralb w18, w2, [x12] */
    "e620f578",  /* ldeoralh w21, w6, [x7] */
    "f0322cf8",  /* ldset x12, x16, [x23] */
    "fe332238",  /* ldsetb w2, w30, [sp] */
    "f0333e78",  /* ldseth w30, w16, [sp] */
    "af33b5f8",  /* ldseta x21, x15, [x29] */
    "ea33b638",  /* ldsetab w22, w10, [sp] */
    "d333aa78",  /* ldsetah w10, w19, [x30] */
    "f7336ff8",  /* ldsetl x15, x23, [sp] */
    "e8337738",  /* ldsetlb w23, w8, [sp] */
    "f7336878",  /* ldsetlh w8, w23, [sp] */
    "5731e6f8",  /* ldsetal x6, x23, [x10] */
    "e133f238",  /* ldsetalb w18, w1, [sp] */
    "6a30e578",  /* ldsetalh w5, w10, [x3] */
    "e8432eb8",  /* ldsmax w14, w8, [sp] */
    "ac412a38",  /* ldsmaxb w10, w12, [x13] */
    "f8433a78",  /* ldsmaxh w26, w24, [sp] */
    "0842acb8",  /* ldsmaxa w12, w8, [x16] */
    "0643bc38",  /* ldsmaxab w28, w6, [x24] */
    "fb43b778",  /* ldsmaxah w23, w27, [sp] */
    "f0436cf8",  /* ldsmaxl x12, x16, [sp] */
    "2c406f38",  /* ldsmaxlb w15, w12, [x1] */
    "92436678",  /* ldsmaxlh w6, w18, [x28] */
    "0c43e4b8",  /* ldsmaxal w4, w12, [x24] */
    "e643f338",  /* ldsmaxalb w19, w6, [sp] */
    "fd43e478",  /* ldsmaxalh w4, w29, [sp] */
    "e65325b8",  /* ldsmin w5, w6, [sp] */
    "0e523438",  /* ldsminb w20, w14, [x16] */
    "f2532078",  /* ldsminh w0, w18, [sp] */
    "e653acf8",  /* ldsmina x12, x6, [sp] */
    "9c51a638",  /* ldsminab w6, w28, [x12] */
    "5e51bb78",  /* ldsminah w27, w30, [x10] */
    "ea5362b8",  /* ldsminl w2, w10, [sp] */
    "f1537a38",  /* ldsminlb w26, w17, [sp] */
    "49537478",  /* ldsminlh w20, w9, [x26] */
    "4853f8b8",  /* ldsminal w24, w8, [x26] */
    "2651eb38",  /* ldsminalb w11, w6, [x9] */
    "dc50e678",  /* ldsminalh w6, w28, [x6] */
    "e16328f8",  /* ldumax x8, x1, [sp] */
    "da632938",  /* ldumaxb w9, w26, [x30] */
    "69612678",  /* ldumaxh w6, w9, [x11] */
    "f863bcf8",  /* ldumaxa x28, x24, [sp] */
    "f263b838",  /* ldumaxab w24, w18, [sp] */
    "7260b378",  /* ldumaxah w19, w18, [x3] */
    "6b626af8",  /* ldumaxl x10, x11, [x19] */
    "e3636138",  /* ldumaxlb w1, w3, [sp] */
    "ed636278",  /* ldumaxlh w2, w13, [sp] */
    "ef63e0b8",  /* ldumaxal w0, w15, [sp] */
    "0f60e538",  /* ldumaxalb w5, w15, [x0] */
    "fb63eb78",  /* ldumaxalh w11, w27, [sp] */
    "f0733bf8",  /* ldumin x27, x16, [sp] */
    "76712b38",  /* lduminb w11, w22, [x11] */
    "ef732a78",  /* lduminh w10, w15, [sp] */
    "ef73a0b8",  /* ldumina w0, w15, [sp] */
    "a372b138",  /* lduminab w17, w3, [x21] */
    "fe73b978",  /* lduminah w25, w30, [sp] */
    "407371f8",  /* lduminl x17, x0, [x26] */
    "60727c38",  /* lduminlb w28, w0, [x19] */
    "e3736578",  /* lduminlh w5, w3, [sp] */
    "f973eff8",  /* lduminal x15, x25, [sp] */
    "3472fc38",  /* lduminalb w28, w20, [x17] */
    "a272f878",  /* lduminalh w24, w2, [x21] */
    "1f0123f8",  /* stadd x3, [x8] */
    "3f0360b8",  /* staddl w0, [x25] */
    "3f102ef8",  /* stclr x14, [x1] */
    "ff127db8",  /* stclrl w29, [x23] */
    "ff232ef8",  /* steor x14, [sp] */
    "df2376b8",  /* steorl w22, [x30] */
    "3f3236b8",  /* stset w22, [x17] */
    "bf3172f8",  /* stsetl x18, [x13] */
    "ff432bb8",  /* stsmax w11, [sp] */
    "ff4364b8",  /* stsmaxl w4, [sp] */
    "5f5029b8",  /* stsmin w9, [x2] */
    "ff5371b8",  /* stsminl w17, [sp] */
    "ff632cf8",  /* stumax x12, [sp] */
    "1f636bf8",  /* stumaxl x11, [x24] */
    "ff7322f8",  /* stumin x2, [sp] */
    "ff7373b8",  /* stuminl w19, [sp] */
    "fec3bff8",  /* ldapr x30, [sp] */
    "dac0bf38",  /* ldaprb w26, [x6] */
    "54d03bd5",  /* mrs x20, tpidr_el0 */
    "6bd03bd5",  /* mrs x11, tpidrro_el0 */
    "00443bd5",  /* mrs x0, fpcr */
    "33443bd5",  /* mrs x19, fpsr */
    "4be03bd5",  /* mrs x11, cntvct_el0 */
    "0ee03bd5",  /* mrs x14, cntfrq_el0 */
    "fe003bd5",  /* mrs x30, dczid_el0 */
    "21003bd5",  /* mrs x1, ctr_el0 */
    "140038d5",  /* mrs x20, midr_el1 */
    "54d01bd5",  /* msr tpidr_el0, x20 */
    "0e441bd5",  /* msr fpcr, x14 */
    "30441bd5",  /* msr fpsr, x16 */
    "15421bd5",  /* msr nzcv, x21 */
    "f00b3f1e",  /* fmul s16, s31, s31 */
    "6c1a251e",  /* fdiv s12, s19, s5 */
    "e42b3c1e",  /* fadd s4, s31, s28 */
    "59392c1e",  /* fsub s25, s10, s12 */
    "1049301e",  /* fmax s16, s8, s16 */
    "60592c1e",  /* fmin s0, s11, s12 */
    "af6b271e",  /* fmaxnm s15, s29, s7 */
    "f27a2c1e",  /* fminnm s18, s23, s12 */
    "0688341e",  /* fnmul s6, s0, s20 */
    "7c42201e",  /* fmov s28, s19 */
    "5fc0201e",  /* fabs s31, s2 */
    "5442211e",  /* fneg s20, s18 */
    "abc3211e",  /* fsqrt s11, s29 */
    "7f42241e",  /* frintn s31, s19 */
    "bec2241e",  /* frintp s30, s21 */
    "8f40251e",  /* frintm s15, s4 */
    "4cc0251e",  /* frintz s12, s2 */
    "9740261e",  /* frinta s23, s4 */
    "1640271e",  /* frintx s22, s0 */
    "4bc0271e",  /* frinti s11, s2 */
    "5e19141f",  /* fmadd s30, s10, s20, s6 */
    "eafa0f1f",  /* fmsub s10, s23, s15, s30 */
    "03502e1f",  /* fnmadd s3, s0, s14, s20 */
    "49da2f1f",  /* fnmsub s9, s18, s15, s22 */
    "c022381e",  /* fcmp s22, s24 */
    "30213d1e",  /* fcmpe s9, s29 */
    "6823201e",  /* fcmp s27, #0.0 */
    "3823201e",  /* fcmpe s25, #0.0 */
    "63a43d1e",  /* fccmp s3, s29, #3, ge */
    "a1bc311e",  /* fcsel s1, s5, s17, lt */
    "13102e1e",  /* fmov s19, #1.0 */
    "17103c1e",  /* fmov s23, #-0.5 */
    "0710201e",  /* fmov s7, #2.0 */
    "0010281e",  /* fmov s0, #0.125 */
    "0bf0271e",  /* fmov s11, #31.0 */
    "06903e1e",  /* fmov s6, #-1.25 */
    "1a10211e",  /* fmov s26, #3.0 */
    "6c03201e",  /* fcvtns w12, s27 */
    "9500211e",  /* fcvtnu w21, s4 */
    "0102241e",  /* fcvtas w1, s16 */
    "6503251e",  /* fcvtau w5, s27 */
    "4b03281e",  /* fcvtps w11, s26 */
    "dd01291e",  /* fcvtpu w29, s14 */
    "cb01301e",  /* fcvtms w11, s14 */
    "1a03311e",  /* fcvtmu w26, s24 */
    "4b02389e",  /* fcvtzs x11, s18 */
    "e702391e",  /* fcvtzu w7, s23 */
    "a000229e",  /* scvtf s0, x5 */
    "9f00239e",  /* ucvtf s31, x4 */
    "9e0b6a1e",  /* fmul d30, d28, d10 */
    "191a781e",  /* fdiv d25, d16, d24 */
    "b9286d1e",  /* fadd d25, d5, d13 */
    "d8386d1e",  /* fsub d24, d6, d13 */
    "d74a701e",  /* fmax d23, d22, d16 */
    "135b7b1e",  /* fmin d19, d24, d27 */
    "c96a641e",  /* fmaxnm d9, d22, d4 */
    "927a711e",  /* fminnm d18, d20, d17 */
    "8c89791e",  /* fnmul d12, d12, d25 */
    "ad41601e",  /* fmov d13, d13 */
    "eec2601e",  /* fabs d14, d23 */
    "9043611e",  /* fneg d16, d28 */
    "efc0611e",  /* fsqrt d15, d7 */
    "2740641e",  /* frintn d7, d1 */
    "5ac0641e",  /* frintp d26, d2 */
    "7342651e",  /* frintm d19, d19 */
    "16c1651e",  /* frintz d22, d8 */
    "0842661e",  /* frinta d8, d16 */
    "cb43671e",  /* frintx d11, d30 */
    "18c2671e",  /* frinti d24, d16 */
    "4f155e1f",  /* fmadd d15, d10, d30, d5 */
    "ccfe401f",  /* fmsub d12, d22, d0, d31 */
    "431f7b1f",  /* fnmadd d3, d26, d27, d7 */
    "99ce7a1f",  /* fnmsub d25, d20, d26, d19 */
    "c022791e",  /* fcmp d22, d25 */
    "90207d1e",  /* fcmpe d4, d29 */
    "8821601e",  /* fcmp d12, #0.0 */
    "9820601e",  /* fcmpe d4, #0.0 */
    "03267c1e",  /* fccmp d16, d28, #3, hs */
    "909c6f1e",  /* fcsel d16, d4, d15, ls */
    "11106e1e",  /* fmov d17, #1.0 */
    "0f107c1e",  /* fmov d15, #-0.5 */
    "1410601e",  /* fmov d20, #2.0 */
    "1a10681e",  /* fmov d26, #0.125 */
    "03f0671e",  /* fmov d3, #31.0 */
    "04907e1e",  /* fmov d4, #-1.25 */
    "0010611e",  /* fmov d0, #3.0 */
    "d301609e",  /* fcvtns x19, d14 */
    "8301611e",  /* fcvtnu w3, d12 */
    "b100649e",  /* fcvtas x17, d5 */
    "b003659e",  /* fcvtau x16, d29 */
    "8302689e",  /* fcvtps x3, d20 */
    "5903699e",  /* fcvtpu x25, d26 */
    "4300701e",  /* fcvtms w3, d2 */
    "6202719e",  /* fcvtmu x2, d19 */
    "9b01781e",  /* fcvtzs w27, d12 */
    "a500799e",  /* fcvtzu x5, d5 */
    "f601629e",  /* scvtf d22, x15 */
    "e900639e",  /* ucvtf d9, x7 */
    "1f01271e",  /* fmov s31, w8 */
    "6c00261e",  /* fmov w12, s3 */
    "6e02679e",  /* fmov d14, x19 */
    "c902669e",  /* fmov x9, d22 */
    "4b85210e",  /* add v11.8b, v10.8b, v1.8b */
    "e785666e",  /* sub v7.8h, v15.8h, v6.8h */
    "0b8df46e",  /* cmeq v11.2d, v8.2d, v20.2d */
    "6d37a14e",  /* cmgt v13.4s, v27.4s, v1.4s */
    "853f6f0e",  /* cmge v5.4h, v28.4h, v15.4h */
    "fd35a72e",  /* cmhi v29.2s, v15.2s, v7.2s */
    "ca3d716e",  /* cmhs v10.8h, v14.8h, v17.8h */
    "a8bf3a0e",  /* addp v8.8b, v29.8b, v26.8b */
    "f8442f2e",  /* ushl v24.8b, v7.8b, v15.8b */
    "5a45a50e",  /* sshl v26.2s, v10.2s, v5.2s */
    "3e8c2b0e",  /* cmtst v30.8b, v1.8b, v11.8b */
    "b49e3e4e",  /* mul v20.16b, v21.16b, v30.16b */
    "5d66a46e",  /* umax v29.4s, v18.4s, v4.4s */
    "766c212e",  /* umin v22.8b, v3.8b, v1.8b */
    "5464640e",  /* smax v20.4h, v2.4h, v4.4h */
    "f06c214e",  /* smin v16.16b, v7.16b, v1.16b */
    "1e0c7a2e",  /* uqadd v30.4h, v0.4h, v26.4h */
    "780cb04e",  /* sqadd v24.4s, v3.4s, v16.4s */
    "c92e6d2e",  /* uqsub v9.4h, v22.4h, v13.4h */
    "842eb34e",  /* sqsub v4.4s, v20.4s, v19.4s */
    "091f304e",  /* and v9.16b, v24.16b, v16.16b */
    "ed1fa50e",  /* orr v13.8b, v31.8b, v5.8b */
    "931c346e",  /* eor v19.16b, v4.16b, v20.16b */
    "cc1d790e",  /* bic v12.8b, v14.8b, v25.8b */
    "321de94e",  /* orn v18.16b, v9.16b, v9.16b */
    "6f1d7d2e",  /* bsl v15.8b, v11.8b, v29.8b */
    "3a1fbc6e",  /* bit v26.16b, v25.16b, v28.16b */
    "191ff36e",  /* bif v25.16b, v24.16b, v19.16b */
    "d4d4250e",  /* fadd v20.2s, v6.2s, v5.2s */
    "1bd6bd4e",  /* fsub v27.4s, v16.4s, v29.4s */
    "c6dd3d6e",  /* fmul v6.4s, v14.4s, v29.4s */
    "5efd666e",  /* fdiv v30.2d, v10.2d, v6.2d */
    "f8f76d4e",  /* fmax v24.2d, v31.2d, v13.2d */
    "7cf6e14e",  /* fmin v28.2d, v19.2d, v1.2d */
    "52c6644e",  /* fmaxnm v18.2d, v18.2d, v4.2d */
    "4cc4a20e",  /* fminnm v12.2s, v2.2s, v2.2s */
    "5e1fba4e",  /* mov v30.16b, v26.16b */
    "181fb80e",  /* mov v24.8b, v24.8b */
    "1b58206e",  /* not v27.16b, v0.16b */
    "0958202e",  /* mvn v9.8b, v0.8b */
    "585b200e",  /* cnt v24.8b, v26.8b */
    "dc08a04e",  /* rev64 v28.4s, v6.4s */
    "6fb9e06e",  /* neg v15.2d, v11.2d */
    "ebba604e",  /* abs v11.8h, v23.8h */
    "e1b8314e",  /* addv b1, v7.16b */
    "c2ba710e",  /* addv h2, v22.4h */
    "c3bab14e",  /* addv s3, v22.4s */
    "2438302e",  /* uaddlv h4, v1.8b */
    "65aa306e",  /* umaxv b5, v19.16b */
    "e6aab16e",  /* uminv s6, v23.4s */
    "830e044e",  /* dup v3.4s, w20 */
    "f10d084e",  /* dup v17.2d, x15 */
    "7504144e",  /* dup v21.4s, v3.s[2] */
    "e004134e",  /* dup v0.16b, v7.b[9] */
    "2de4064f",  /* movi v13.16b, #193 */
    "22e4040f",  /* movi v2.8b, #129 */
    "a0180f6e",  /* ext v0.16b, v5.16b, v15.16b, #3 */
    "d42b042e",  /* ext v20.8b, v30.8b, v4.8b, #5 */
    "e777404c",  /* ld1 {v7.8h}, [sp] */
    "6777df4c",  /* ld1 {v7.8h}, [x27], #16 */
    "e7779d4c",  /* st1 {v7.8h}, [sp], x29 */
    "e7ab400c",  /* ld1 {v7.2s, v8.2s}, [sp] */
    "e7a8000c",  /* st1 {v7.2s, v8.2s}, [x7] */
    "a7a9df0c",  /* ld1 {v7.2s, v8.2s}, [x13], #16 */
    "47a89b0c",  /* st1 {v7.2s, v8.2s}, [x2], x27 */
    "e760400c",  /* ld1 {v7.8b, v8.8b, v9.8b}, [x7] */
    "e763000c",  /* st1 {v7.8b, v8.8b, v9.8b}, [sp] */
    "e763df0c",  /* ld1 {v7.8b, v8.8b, v9.8b}, [sp], #24 */
    "27638d0c",  /* st1 {v7.8b, v8.8b, v9.8b}, [x25], x13 */
    "c723404c",  /* ld1 {v7.16b, v8.16b, v9.16b, v10.16b}, [x30] */
    "4721004c",  /* st1 {v7.16b, v8.16b, v9.16b, v10.16b}, [x10] */
    "e721df4c",  /* ld1 {v7.16b, v8.16b, v9.16b, v10.16b}, [x15], #64 */
    "87219a4c",  /* st1 {v7.16b, v8.16b, v9.16b, v10.16b}, [x12], x26 */
};

static int hexval(char c) { return c <= '9' ? c - '0' : c - 'a' + 10; }

int main(void) {
    const unsigned char *p = axiom_asm_table;
    int bad = 0, n = (int)(sizeof want / sizeof want[0]);
    for (int i = 0; i < n; i++) {
        const char *w = want[i];
        int ok = 1;
        for (int k = 0; w[k] != 0; k += 2) {
            if (p >= axiom_asm_table_end || *p != hexval(w[k]) * 16 + hexval(w[k + 1])) ok = 0;
            p++;
        }
        if (!ok && bad++ < 5) {
            print_str("mismatch at line ", 17);
            print_i64(i);
        }
    }
    print_i64(bad);
    print_i64(p == axiom_asm_table_end);
    return 0;
}
#else
int main(void) {
    print_i64(0);
    print_i64(1);
    return 0;
}
#endif
/* expect: 0 */
/* expect: 1 */
