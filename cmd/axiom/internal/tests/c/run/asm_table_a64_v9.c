/* flags: --c-std=gnu17 */
/* native-only */
/* The AArch64 assembler against a byte table, one line per form: bc.cond (FEAT_HBC), the system-level-cache prefetch operations, rprfm (FEAT_RPRFM), the checked pointer arithmetic (FEAT_CPA), FEAT_PAuth_LR, FAMINMAX, FP8 (the conversions, fscale, fdot, the fmlal family) and LUT in Advanced SIMD, brb (FEAT_BRBE), tlbi of FEAT_TLBIW and the nXS forms of the granule-protection ones, and fpmr; the bytes were checked against the system assembler in development. */
#include "host.h"

#ifdef __AXIOM_ASM_AARCH64__
__asm__(".section .rodata.axiom_asm_table, \"a\"\n"
        ".globl axiom_asm_table\n"
        "axiom_asm_table:\n"
        "1:\n"
        "bc.eq 1b\n"
        "bc.ne 2f\n"
        "bc.al 1b\n"
        "2:\n"
        "prfm pldslckeep, [x1, #8]\n"
        "prfm pldslcstrm, [x2]\n"
        "prfm plislckeep, [x3, x4, lsl #3]\n"
        "prfm plislcstrm, [x5, w6, sxtw]\n"
        "prfm pstslckeep, [sp, #32760]\n"
        "prfum pstslcstrm, [x5, #-3]\n"
        "rprfm pldkeep, x1, [x2]\n"
        "rprfm pstkeep, x3, [sp]\n"
        "rprfm pldstrm, x4, [x5]\n"
        "rprfm pststrm, xzr, [x6]\n"
        "rprfm #63, x7, [x8]\n"
        "addpt x0, x1, x2\n"
        "addpt sp, sp, x3, lsl #7\n"
        "subpt x4, sp, x5, lsl #1\n"
        "subpt x6, x7, xzr\n"
        "maddpt x8, x9, x10, x11\n"
        "msubpt x12, x13, x14, xzr\n"
        "3:\n"
        "autia171615\n"
        "autib171615\n"
        "pacm\n"
        "pacnbiasppc\n"
        "pacnbibsppc\n"
        "paciasppc\n"
        "pacibsppc\n"
        "pacia171615\n"
        "pacib171615\n"
        "autiasppc 3b\n"
        "autibsppc 3b\n"
        "retaasppc 3b\n"
        "retabsppc 3b\n"
        "autiasppcr x3\n"
        "autibsppcr x30\n"
        "retaasppcr x5\n"
        "retabsppcr x9\n"
        "brb iall\n"
        "brb inj\n"
        "tlbi vmallws2e1\n"
        "tlbi vmallws2e1is\n"
        "tlbi vmallws2e1os\n"
        "tlbi vmallws2e1nxs\n"
        "tlbi vmallws2e1isnxs\n"
        "tlbi vmallws2e1osnxs\n"
        "tlbi paallnxs\n"
        "tlbi paallosnxs\n"
        "tlbi rpaosnxs, x1\n"
        "tlbi rpalosnxs, x2\n"
        "mrs x3, fpmr\n"
        "msr fpmr, x4\n"
        "fdot v1.4h, v2.8b, v3.2b[0]\n"
        "fdot v1.4h, v2.8b, v3.2b[1]\n"
        "fdot v1.4h, v2.8b, v3.2b[2]\n"
        "fdot v1.4h, v2.8b, v3.2b[4]\n"
        "fdot v1.8h, v2.16b, v3.2b[0]\n"
        "fdot v1.2s, v2.8b, v3.4b[0]\n"
        "fdot v1.2s, v2.8b, v3.4b[1]\n"
        "fdot v1.2s, v2.8b, v3.4b[2]\n"
        "fdot v1.2s, v2.8b, v31.4b[0]\n"
        "fdot v1.4h, v2.8b, v3.8b\n"
        "fdot v1.2s, v2.8b, v3.8b\n"
        "fmlalb v1.8h, v2.16b, v3.b[0]\n"
        "fmlalb v1.8h, v2.16b, v3.b[1]\n"
        "fmlalb v1.8h, v2.16b, v3.b[2]\n"
        "fmlalb v1.8h, v2.16b, v3.b[4]\n"
        "fmlalb v1.8h, v2.16b, v3.b[8]\n"
        "fmlalb v1.8h, v2.16b, v7.b[0]\n"
        "fmlalt v1.8h, v2.16b, v3.b[0]\n"
        "fmlalb v1.8h, v2.16b, v3.16b\n"
        "fmlalt v1.8h, v2.16b, v3.16b\n"
        "fmlallbb v1.4s, v2.16b, v3.16b\n"
        "fmlallbt v1.4s, v2.16b, v3.16b\n"
        "fmlalltb v1.4s, v2.16b, v3.16b\n"
        "fmlalltt v1.4s, v2.16b, v3.16b\n"
        "fmlallbb v1.4s, v2.16b, v3.b[0]\n"
        "fmlallbb v1.4s, v2.16b, v3.b[15]\n"
        "fmlallbt v1.4s, v2.16b, v3.b[0]\n"
        "fmlalltb v1.4s, v2.16b, v3.b[0]\n"
        "fmlalltt v1.4s, v2.16b, v3.b[0]\n"
        "luti2 v1.16b, {v4.16b}, v5[0]\n"
        "luti2 v1.16b, {v4.16b}, v5[1]\n"
        "luti2 v1.16b, {v4.16b}, v5[2]\n"
        "luti2 v1.8h, {v4.8h}, v5[0]\n"
        "luti2 v1.8h, {v4.8h}, v5[1]\n"
        "luti2 v1.8h, {v4.8h}, v5[4]\n"
        "luti4 v1.16b, {v4.16b}, v5[0]\n"
        "luti4 v1.16b, {v4.16b}, v5[1]\n"
        "luti4 v1.8h, {v4.8h, v5.8h}, v5[0]\n"
        "luti4 v1.8h, {v4.8h, v5.8h}, v5[1]\n"
        "luti4 v1.8h, {v4.8h, v5.8h}, v5[2]\n"
        "luti4 v1.8h, {v31.8h, v0.8h}, v5[0]\n"
        "f1cvtl v1.8h, v2.8b\n"
        "f1cvtl2 v1.8h, v2.16b\n"
        "f2cvtl v1.8h, v2.8b\n"
        "f2cvtl2 v1.8h, v2.16b\n"
        "bf1cvtl v1.8h, v2.8b\n"
        "bf1cvtl2 v1.8h, v2.16b\n"
        "bf2cvtl v1.8h, v2.8b\n"
        "bf2cvtl2 v1.8h, v2.16b\n"
        "fcvtn v1.8b, v2.4h, v3.4h\n"
        "fcvtn v1.16b, v2.8h, v3.8h\n"
        "fcvtn v1.8b, v2.4s, v3.4s\n"
        "fcvtn2 v1.16b, v2.4s, v3.4s\n"
        "fscale v1.4h, v2.4h, v3.4h\n"
        "fscale v1.8h, v2.8h, v3.8h\n"
        "fscale v1.2s, v2.2s, v3.2s\n"
        "fscale v1.4s, v2.4s, v3.4s\n"
        "fscale v1.2d, v2.2d, v3.2d\n"
        "famax v1.4h, v2.4h, v3.4h\n"
        "famax v1.8h, v2.8h, v3.8h\n"
        "famax v1.2s, v2.2s, v3.2s\n"
        "famax v1.4s, v2.4s, v3.4s\n"
        "famax v1.2d, v2.2d, v3.2d\n"
        "famin v1.4h, v2.4h, v3.4h\n"
        "famin v1.8h, v2.8h, v3.8h\n"
        "famin v1.2s, v2.2s, v3.2s\n"
        "famin v1.4s, v2.4s, v3.4s\n"
        "famin v1.2d, v2.2d, v3.2d\n"
        ".globl axiom_asm_table_end\n"
        "axiom_asm_table_end:\n"
        ".text\n");

extern const unsigned char axiom_asm_table[], axiom_asm_table_end[];
static const char *const want[] = {
    "10000054",  /* bc.eq 1b */
    "51000054",  /* bc.ne 2f */
    "deffff54",  /* bc.al 1b */
    "260480f9",  /* prfm pldslckeep, [x1, #8] */
    "470080f9",  /* prfm pldslcstrm, [x2] */
    "6e78a4f8",  /* prfm plislckeep, [x3, x4, lsl #3] */
    "afc8a6f8",  /* prfm plislcstrm, [x5, w6, sxtw] */
    "f6ffbff9",  /* prfm pstslckeep, [sp, #32760] */
    "b7d09ff8",  /* prfum pstslcstrm, [x5, #-3] */
    "5848a1f8",  /* rprfm pldkeep, x1, [x2] */
    "f94ba3f8",  /* rprfm pstkeep, x3, [sp] */
    "bc48a4f8",  /* rprfm pldstrm, x4, [x5] */
    "dd48bff8",  /* rprfm pststrm, xzr, [x6] */
    "1ff9a7f8",  /* rprfm #63, x7, [x8] */
    "2020029a",  /* addpt x0, x1, x2 */
    "ff3f039a",  /* addpt sp, sp, x3, lsl #7 */
    "e42705da",  /* subpt x4, sp, x5, lsl #1 */
    "e6201fda",  /* subpt x6, x7, xzr */
    "282d6a9b",  /* maddpt x8, x9, x10, x11 */
    "acfd6e9b",  /* msubpt x12, x13, x14, xzr */
    "febbc1da",  /* autia171615 */
    "febfc1da",  /* autib171615 */
    "ff2403d5",  /* pacm */
    "fe83c1da",  /* pacnbiasppc */
    "fe87c1da",  /* pacnbibsppc */
    "fea3c1da",  /* paciasppc */
    "fea7c1da",  /* pacibsppc */
    "fe8bc1da",  /* pacia171615 */
    "fe8fc1da",  /* pacib171615 */
    "3f0180f3",  /* autiasppc 3b */
    "5f01a0f3",  /* autibsppc 3b */
    "7f010055",  /* retaasppc 3b */
    "9f012055",  /* retabsppc 3b */
    "7e90c1da",  /* autiasppcr x3 */
    "de97c1da",  /* autibsppcr x30 */
    "e50b5fd6",  /* retaasppcr x5 */
    "e90f5fd6",  /* retabsppcr x9 */
    "9f7209d5",  /* brb iall */
    "bf7209d5",  /* brb inj */
    "5f860cd5",  /* tlbi vmallws2e1 */
    "5f820cd5",  /* tlbi vmallws2e1is */
    "5f850cd5",  /* tlbi vmallws2e1os */
    "5f960cd5",  /* tlbi vmallws2e1nxs */
    "5f920cd5",  /* tlbi vmallws2e1isnxs */
    "5f950cd5",  /* tlbi vmallws2e1osnxs */
    "9f970ed5",  /* tlbi paallnxs */
    "9f910ed5",  /* tlbi paallosnxs */
    "61940ed5",  /* tlbi rpaosnxs, x1 */
    "e2940ed5",  /* tlbi rpalosnxs, x2 */
    "43443bd5",  /* mrs x3, fpmr */
    "44441bd5",  /* msr fpmr, x4 */
    "4100430f",  /* fdot v1.4h, v2.8b, v3.2b[0] */
    "4100530f",  /* fdot v1.4h, v2.8b, v3.2b[1] */
    "4100630f",  /* fdot v1.4h, v2.8b, v3.2b[2] */
    "4108430f",  /* fdot v1.4h, v2.8b, v3.2b[4] */
    "4100434f",  /* fdot v1.8h, v2.16b, v3.2b[0] */
    "4100030f",  /* fdot v1.2s, v2.8b, v3.4b[0] */
    "4100230f",  /* fdot v1.2s, v2.8b, v3.4b[1] */
    "4108030f",  /* fdot v1.2s, v2.8b, v3.4b[2] */
    "41001f0f",  /* fdot v1.2s, v2.8b, v31.4b[0] */
    "41fc430e",  /* fdot v1.4h, v2.8b, v3.8b */
    "41fc030e",  /* fdot v1.2s, v2.8b, v3.8b */
    "4100c30f",  /* fmlalb v1.8h, v2.16b, v3.b[0] */
    "4100cb0f",  /* fmlalb v1.8h, v2.16b, v3.b[1] */
    "4100d30f",  /* fmlalb v1.8h, v2.16b, v3.b[2] */
    "4100e30f",  /* fmlalb v1.8h, v2.16b, v3.b[4] */
    "4108c30f",  /* fmlalb v1.8h, v2.16b, v3.b[8] */
    "4100c70f",  /* fmlalb v1.8h, v2.16b, v7.b[0] */
    "4100c34f",  /* fmlalt v1.8h, v2.16b, v3.b[0] */
    "41fcc30e",  /* fmlalb v1.8h, v2.16b, v3.16b */
    "41fcc34e",  /* fmlalt v1.8h, v2.16b, v3.16b */
    "41c4030e",  /* fmlallbb v1.4s, v2.16b, v3.16b */
    "41c4430e",  /* fmlallbt v1.4s, v2.16b, v3.16b */
    "41c4034e",  /* fmlalltb v1.4s, v2.16b, v3.16b */
    "41c4434e",  /* fmlalltt v1.4s, v2.16b, v3.16b */
    "4180032f",  /* fmlallbb v1.4s, v2.16b, v3.b[0] */
    "41883b2f",  /* fmlallbb v1.4s, v2.16b, v3.b[15] */
    "4180432f",  /* fmlallbt v1.4s, v2.16b, v3.b[0] */
    "4180036f",  /* fmlalltb v1.4s, v2.16b, v3.b[0] */
    "4180436f",  /* fmlalltt v1.4s, v2.16b, v3.b[0] */
    "8110854e",  /* luti2 v1.16b, {v4.16b}, v5[0] */
    "8130854e",  /* luti2 v1.16b, {v4.16b}, v5[1] */
    "8150854e",  /* luti2 v1.16b, {v4.16b}, v5[2] */
    "8100c54e",  /* luti2 v1.8h, {v4.8h}, v5[0] */
    "8110c54e",  /* luti2 v1.8h, {v4.8h}, v5[1] */
    "8140c54e",  /* luti2 v1.8h, {v4.8h}, v5[4] */
    "8120454e",  /* luti4 v1.16b, {v4.16b}, v5[0] */
    "8160454e",  /* luti4 v1.16b, {v4.16b}, v5[1] */
    "8110454e",  /* luti4 v1.8h, {v4.8h, v5.8h}, v5[0] */
    "8130454e",  /* luti4 v1.8h, {v4.8h, v5.8h}, v5[1] */
    "8150454e",  /* luti4 v1.8h, {v4.8h, v5.8h}, v5[2] */
    "e113454e",  /* luti4 v1.8h, {v31.8h, v0.8h}, v5[0] */
    "4178212e",  /* f1cvtl v1.8h, v2.8b */
    "4178216e",  /* f1cvtl2 v1.8h, v2.16b */
    "4178612e",  /* f2cvtl v1.8h, v2.8b */
    "4178616e",  /* f2cvtl2 v1.8h, v2.16b */
    "4178a12e",  /* bf1cvtl v1.8h, v2.8b */
    "4178a16e",  /* bf1cvtl2 v1.8h, v2.16b */
    "4178e12e",  /* bf2cvtl v1.8h, v2.8b */
    "4178e16e",  /* bf2cvtl2 v1.8h, v2.16b */
    "41f4430e",  /* fcvtn v1.8b, v2.4h, v3.4h */
    "41f4434e",  /* fcvtn v1.16b, v2.8h, v3.8h */
    "41f4030e",  /* fcvtn v1.8b, v2.4s, v3.4s */
    "41f4034e",  /* fcvtn2 v1.16b, v2.4s, v3.4s */
    "413cc32e",  /* fscale v1.4h, v2.4h, v3.4h */
    "413cc36e",  /* fscale v1.8h, v2.8h, v3.8h */
    "41fca32e",  /* fscale v1.2s, v2.2s, v3.2s */
    "41fca36e",  /* fscale v1.4s, v2.4s, v3.4s */
    "41fce36e",  /* fscale v1.2d, v2.2d, v3.2d */
    "411cc30e",  /* famax v1.4h, v2.4h, v3.4h */
    "411cc34e",  /* famax v1.8h, v2.8h, v3.8h */
    "41dca30e",  /* famax v1.2s, v2.2s, v3.2s */
    "41dca34e",  /* famax v1.4s, v2.4s, v3.4s */
    "41dce34e",  /* famax v1.2d, v2.2d, v3.2d */
    "411cc32e",  /* famin v1.4h, v2.4h, v3.4h */
    "411cc36e",  /* famin v1.8h, v2.8h, v3.8h */
    "41dca32e",  /* famin v1.2s, v2.2s, v3.2s */
    "41dca36e",  /* famin v1.4s, v2.4s, v3.4s */
    "41dce36e",  /* famin v1.2d, v2.2d, v3.2d */
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
