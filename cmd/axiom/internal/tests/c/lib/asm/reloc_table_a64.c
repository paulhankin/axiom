/* flags: --c-std=gnu17 */
/* native-only */
/* linux-only */
/* AArch64 relocations of file-scope asm, linked by the compiler's linker:
   each line's word (the system assembler's, with the field the relocation
   fills zero) and the field as the ELF for the Arm 64-bit Architecture ABI
   computes it from the symbol's address, the addend and the place:
   adrp's page difference, :lo12: (scaled by the access), the absolute,
   signed and pc-relative move-wide groups (movz or movn by the sign), adr,
   the 19-bit literal and conditional branch offsets, tbz's 14 bits and
   b/bl's 26. */
#include "print.h"

typedef unsigned long long u64;
typedef long long i64;

#ifdef __AXIOM_ASM_AARCH64__
__asm__(".data\n.globl rt_d\n.p2align 4\nrt_d:\n\t.zero 8192\n"
        ".text\n.globl rt_tab\n.p2align 2\nrt_tab:\n"
        "\tadrp x1, rt_d\n"
        "\tadrp x2, :pg_hi21_nc:rt_d\n"
        "\tadd x3, x3, :lo12:rt_d\n"
        "\tadrp x1, rt_d+8\n"
        "\tadrp x2, :pg_hi21_nc:rt_d+8\n"
        "\tadd x3, x3, :lo12:rt_d+8\n"
        "\tadrp x1, rt_d-16\n"
        "\tadrp x2, :pg_hi21_nc:rt_d-16\n"
        "\tadd x3, x3, :lo12:rt_d-16\n"
        "\tadrp x1, rt_d+4108\n"
        "\tadrp x2, :pg_hi21_nc:rt_d+4108\n"
        "\tadd x3, x3, :lo12:rt_d+4108\n"
        "\tldrb w4, [x9, :lo12:rt_d+3]\n"
        "\tldrh w5, [x9, :lo12:rt_d+32]\n"
        "\tldr w6, [x9, :lo12:rt_d+64]\n"
        "\tldr x7, [x9, :lo12:rt_d+128]\n"
        "\tldr q8, [x9, :lo12:rt_d+256]\n"
        "\tmovk x10, #:abs_g0_nc:rt_d+0\n"
        "\tmovz x10, #:abs_g1:rt_d+8\n"
        "\tmovk x10, #:abs_g1_nc:rt_d+8\n"
        "\tmovz x10, #:abs_g2:rt_d+16\n"
        "\tmovk x10, #:abs_g2_nc:rt_d+16\n"
        "\tmovz x10, #:abs_g3:rt_d+24\n"
        "\tmovz x11, #:abs_g1_s:rt_d\n"
        "\tmovz x11, #:abs_g2_s:rt_d\n"
        "\tmovz x12, #:prel_g0:rt_t+0\n"
        "\tmovk x12, #:prel_g0_nc:rt_t+0\n"
        "\tmovz x12, #:prel_g1:rt_t+4\n"
        "\tmovk x12, #:prel_g1_nc:rt_t+4\n"
        "\tmovz x12, #:prel_g2:rt_t+8\n"
        "\tmovk x12, #:prel_g2_nc:rt_t+8\n"
        "\tmovz x12, #:prel_g3:rt_t+12\n"
        "\tadr x13, rt_t+8\n"
        "\tadr x13, rt_d\n"
        "\tldr x14, rt_t\n"
        "\tldr w14, rt_t+4\n"
        "\tldrsw x14, rt_t+8\n"
        "\tldr q14, rt_t\n"
        "\tprfm pldl1keep, rt_t+16\n"
        "\tb.ne rt_t+4\n"
        "\tcbz x15, rt_t\n"
        "\tcbnz w15, rt_t+12\n"
        "\ttbz x15, #33, rt_t\n"
        "\ttbnz w15, #3, rt_t+8\n"
        "\tb rt_t+4\n"
        "\tbl rt_t\n"
        ".globl rt_t\nrt_t:\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n");
extern const unsigned rt_tab[];
extern char rt_d[], rt_t[];

struct row { unsigned word; int kind, n, text; i64 addend; };
static const struct row rows[] = {
    {0x90000001, 0, 0, 0, 0},  /* adrp x1, rt_d */
    {0x90000002, 0, 0, 0, 0},  /* adrp x2, :pg_hi21_nc:rt_d */
    {0x91000063, 1, 0, 0, 0},  /* add x3, x3, :lo12:rt_d */
    {0x90000001, 0, 0, 0, 8},  /* adrp x1, rt_d+8 */
    {0x90000002, 0, 0, 0, 8},  /* adrp x2, :pg_hi21_nc:rt_d+8 */
    {0x91000063, 1, 0, 0, 8},  /* add x3, x3, :lo12:rt_d+8 */
    {0x90000001, 0, 0, 0, -16},  /* adrp x1, rt_d-16 */
    {0x90000002, 0, 0, 0, -16},  /* adrp x2, :pg_hi21_nc:rt_d-16 */
    {0x91000063, 1, 0, 0, -16},  /* add x3, x3, :lo12:rt_d-16 */
    {0x90000001, 0, 0, 0, 4108},  /* adrp x1, rt_d+4108 */
    {0x90000002, 0, 0, 0, 4108},  /* adrp x2, :pg_hi21_nc:rt_d+4108 */
    {0x91000063, 1, 0, 0, 4108},  /* add x3, x3, :lo12:rt_d+4108 */
    {0x39400124, 1, 0, 0, 3},  /* ldrb w4, [x9, :lo12:rt_d+3] */
    {0x79400125, 1, 1, 0, 32},  /* ldrh w5, [x9, :lo12:rt_d+32] */
    {0xb9400126, 1, 2, 0, 64},  /* ldr w6, [x9, :lo12:rt_d+64] */
    {0xf9400127, 1, 3, 0, 128},  /* ldr x7, [x9, :lo12:rt_d+128] */
    {0x3dc00128, 1, 4, 0, 256},  /* ldr q8, [x9, :lo12:rt_d+256] */
    {0xf280000a, 2, 0, 0, 0},  /* movk x10, #:abs_g0_nc:rt_d+0 */
    {0xd2a0000a, 2, 1, 0, 8},  /* movz x10, #:abs_g1:rt_d+8 */
    {0xf2a0000a, 2, 1, 0, 8},  /* movk x10, #:abs_g1_nc:rt_d+8 */
    {0xd2c0000a, 2, 2, 0, 16},  /* movz x10, #:abs_g2:rt_d+16 */
    {0xf2c0000a, 2, 2, 0, 16},  /* movk x10, #:abs_g2_nc:rt_d+16 */
    {0xd2e0000a, 2, 3, 0, 24},  /* movz x10, #:abs_g3:rt_d+24 */
    {0xd2a0000b, 3, 1, 0, 0},  /* movz x11, #:abs_g1_s:rt_d */
    {0xd2c0000b, 3, 2, 0, 0},  /* movz x11, #:abs_g2_s:rt_d */
    {0xd280000c, 4, 0, 1, 0},  /* movz x12, #:prel_g0:rt_t+0 */
    {0xf280000c, 4, 0, 1, 0},  /* movk x12, #:prel_g0_nc:rt_t+0 */
    {0xd2a0000c, 4, 1, 1, 4},  /* movz x12, #:prel_g1:rt_t+4 */
    {0xf2a0000c, 4, 1, 1, 4},  /* movk x12, #:prel_g1_nc:rt_t+4 */
    {0xd2c0000c, 4, 2, 1, 8},  /* movz x12, #:prel_g2:rt_t+8 */
    {0xf2c0000c, 4, 2, 1, 8},  /* movk x12, #:prel_g2_nc:rt_t+8 */
    {0xd2e0000c, 4, 3, 1, 12},  /* movz x12, #:prel_g3:rt_t+12 */
    {0x1000000d, 5, 0, 1, 8},  /* adr x13, rt_t+8 */
    {0x1000000d, 5, 0, 0, 0},  /* adr x13, rt_d */
    {0x5800000e, 6, 0, 1, 0},  /* ldr x14, rt_t */
    {0x1800000e, 6, 0, 1, 4},  /* ldr w14, rt_t+4 */
    {0x9800000e, 6, 0, 1, 8},  /* ldrsw x14, rt_t+8 */
    {0x9c00000e, 6, 0, 1, 0},  /* ldr q14, rt_t */
    {0xd8000000, 6, 0, 1, 16},  /* prfm pldl1keep, rt_t+16 */
    {0x54000001, 6, 0, 1, 4},  /* b.ne rt_t+4 */
    {0xb400000f, 6, 0, 1, 0},  /* cbz x15, rt_t */
    {0x3500000f, 6, 0, 1, 12},  /* cbnz w15, rt_t+12 */
    {0xb608000f, 7, 0, 1, 0},  /* tbz x15, #33, rt_t */
    {0x3718000f, 7, 0, 1, 8},  /* tbnz w15, #3, rt_t+8 */
    {0x14000000, 8, 0, 1, 4},  /* b rt_t+4 */
    {0x94000000, 8, 0, 1, 0},  /* bl rt_t */
};

/* the field's mask and its value for kind k at place p for target s */
static unsigned field(const struct row *r, u64 p, u64 s, unsigned *mask) {
    u64 x = s + (u64)r->addend;
    i64 d = (i64)(x - p);
    switch (r->kind) {
    case 0: { /* adrp: immlo 30:29, immhi 23:5 */
        u64 pg = ((x >> 12) - (p >> 12)) & 0x1fffff;
        *mask = 0x60ffffe0;
        return (unsigned)(((pg & 3) << 29) | ((pg >> 2) << 5));
    }
    case 1: *mask = 0x3ffc00; return (unsigned)(((x & 0xfff) >> r->n) << 10);
    case 2: *mask = 0x1fffe0; return (unsigned)(((x >> (16 * r->n)) & 0xffff) << 5);
    case 3: case 4: { /* a signed value: movz (opc 10) for >= 0, movn (00) of its complement */
        i64 v = r->kind == 3 ? (i64)x : d;
        int movk = (r->word >> 29 & 3) == 3;
        *mask = movk ? 0x1fffe0 : 0x601fffe0;
        if (movk) return (unsigned)((((u64)v >> (16 * r->n)) & 0xffff) << 5);
        if (v < 0) return (unsigned)((((u64)~v >> (16 * r->n)) & 0xffff) << 5);
        return (unsigned)((2u << 29) | ((((u64)v >> (16 * r->n)) & 0xffff) << 5));
    }
    case 5: *mask = 0x60ffffe0; return (unsigned)((((u64)d & 3) << 29) | ((((u64)d >> 2) & 0x7ffff) << 5));
    case 6: *mask = 0xffffe0; return (unsigned)((((u64)d >> 2) & 0x7ffff) << 5);
    case 7: *mask = 0x7ffe0; return (unsigned)((((u64)d >> 2) & 0x3fff) << 5);
    default: *mask = 0x3ffffff; return (unsigned)(((u64)d >> 2) & 0x3ffffff);
    }
}

int main(void) {
    int bad = 0, n = (int)(sizeof rows / sizeof rows[0]);
    for (int i = 0; i < n; i++) {
        unsigned mask, want = field(&rows[i], (u64)&rt_tab[i], rows[i].text ? (u64)rt_t : (u64)rt_d, &mask);
        unsigned got = rt_tab[i];
        if ((got & mask) != want || (got & ~mask) != (rows[i].word & ~mask)) {
            if (bad++ < 5) print_i64(i);
        }
    }
    print_i64(bad);
    print_i64(n);
    return 0;
}
#else
int main(void) {
    print_i64(0);
    print_i64(46);
    return 0;
}
#endif
/* expect: 0 */
/* expect: 46 */
