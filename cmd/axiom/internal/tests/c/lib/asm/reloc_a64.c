/* flags: --c-std=gnu17 */
/* native-only */
/* linux-only */
/* (a hosted program: linked by the compiler's own linker) */
/* AArch64 relocation operators in inline and file-scope asm, linked by our
   linker and run: page-relative addressing (adrp, :lo12:), the GOT (:got:,
   :got_lo12:, a GOT literal), absolute and pc-relative move-wide groups,
   adr, a literal load and the branches (b.cond, cbz, tbz) to symbols of
   other statements, and thread-locals: local exec (:tprel_hi12:,
   :tprel_lo12_nc:, a load at :tprel_lo12_nc:, :tprel_g1:/:tprel_g0_nc:),
   initial exec, TLS descriptors and general dynamic (relaxed to local exec by
   the linker),
   and :dtprel_g0: (the offset in the module's block: the thread pointer's
   offset less the 16-byte control block). */
#include "print.h"

typedef unsigned long long u64;
typedef long long i64;

#ifdef __AXIOM_ASM_AARCH64__
_Thread_local i64 tv = 5;
_Thread_local i64 tv2[4] = {1, 2, 3, 4};
i64 gv = 42;

i64 helper(void) { return 7; }

/* functions with branches and literal loads to global labels (relocations: a
   global label may be another unit's) */
__asm__(".text\n"
        ".globl rel_lit\n.p2align 3\nrel_lit:\n\t.quad 99\n"
        ".globl rel_yes\nrel_yes:\n\tmov x0, #1\n\tret\n"
        ".globl rel_bcond\nrel_bcond:\n\tcmp x0, #3\n\tb.eq rel_yes\n\tmov x0, #0\n\tret\n"
        ".globl rel_cbz\nrel_cbz:\n\tcbz x0, rel_yes\n\tmov x0, #0\n\tret\n"
        ".globl rel_tbz\nrel_tbz:\n\ttbnz x0, #5, rel_yes\n\tmov x0, #0\n\tret\n"
        ".globl rel_ldr\nrel_ldr:\n\tldr x0, rel_lit\n\tret\n");
i64 rel_bcond(i64);
i64 rel_cbz(i64);
i64 rel_tbz(i64);
i64 rel_ldr(void);

int main(void) {
    i64 a;
    u64 p;
    /* page-relative: adrp and :lo12: */
    asm("adrp %0, gv\n\tldr %0, [%0, :lo12:gv]" : "=r"(a));
    print_i64(a);
    asm("adrp %0, gv\n\tadd %0, %0, :lo12:gv" : "=r"(p));
    print_i64(p == (u64)&gv);
    /* through the GOT, and a GOT literal */
    asm("adrp %0, :got:gv\n\tldr %0, [%0, :got_lo12:gv]" : "=r"(p));
    print_i64(p == (u64)&gv);
    asm("ldr %0, :got:gv" : "=r"(p));
    print_i64(p == (u64)&gv);
    /* the absolute move-wide groups */
    asm("movz %0, #:abs_g3:gv\n\tmovk %0, #:abs_g2_nc:gv\n\tmovk %0, #:abs_g1_nc:gv\n\tmovk %0, #:abs_g0_nc:gv"
        : "=r"(p));
    print_i64(p == (u64)&gv);
    /* pc-relative ones: gv - (the movz), the movk's addend making up its place */
    asm("1:\n\tmovz %0, #:prel_g1:gv\n\tmovk %0, #:prel_g0_nc:gv+4\n\tadr x9, 1b\n\tadd %0, %0, x9"
        : "=r"(p)
        :
        : "x9");
    print_i64(p == (u64)&gv);
    /* adr to another function */
    asm("adr %0, helper" : "=r"(p));
    print_i64(p == (u64)helper);
    /* branches and a literal load to other statements' labels */
    print_i64(rel_bcond(3) * 10 + rel_bcond(4));
    print_i64(rel_cbz(0) * 10 + rel_cbz(1));
    print_i64(rel_tbz(32) * 10 + rel_tbz(31));
    print_i64(rel_ldr());
    /* local exec */
    asm("mrs %0, tpidr_el0\n\tadd %0, %0, :tprel_hi12:tv\n\tadd %0, %0, :tprel_lo12_nc:tv\n\tldr %0, [%0]"
        : "=r"(a));
    print_i64(a);
    asm("mrs %0, tpidr_el0\n\tadd %0, %0, :tprel_hi12:tv2\n\tldr %0, [%0, :tprel_lo12_nc:tv2+16]" : "=r"(a));
    print_i64(a);
    asm("movz %0, #:tprel_g1:tv2\n\tmovk %0, #:tprel_g0_nc:tv2+8\n\tmrs x9, tpidr_el0\n\tldr %0, [x9, %0]"
        : "=r"(a)
        :
        : "x9");
    print_i64(a);
    /* initial exec and a TLS descriptor (relaxed), writing through the address */
    asm("adrp %0, :gottprel:tv\n\tldr %0, [%0, :gottprel_lo12:tv]\n\tmrs x9, tpidr_el0\n\tadd %0, %0, x9"
        : "=r"(p)
        :
        : "x9");
    print_i64(p == (u64)&tv);
    asm volatile("adrp x0, :tlsdesc:tv2\n\tldr x1, [x0, :tlsdesc_lo12:tv2]\n\tadd x0, x0, :tlsdesc_lo12:tv2\n\t"
                 ".tlsdesccall tv2\n\tblr x1\n\tmrs x9, tpidr_el0\n\tadd %0, x0, x9"
                 : "=r"(p)
                 :
                 : "x0", "x1", "x9", "x30", "memory");
    print_i64(p == (u64)&tv2[0]);
    /* general dynamic (relaxed: the call becomes the thread pointer's addition) */
    asm volatile("adrp x0, :tlsgd:tv\n\tadd x0, x0, :tlsgd_lo12:tv\n\tbl __tls_get_addr\n\tnop\n\tmov %0, x0"
                 : "=r"(p)
                 :
                 : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
                   "x16", "x17", "x18", "x30", "memory", "cc");
    print_i64(p == (u64)&tv);
    /* the offset in the module's block: the thread pointer's less 16 */
    {
        i64 d, t;
        asm("movz %0, #:dtprel_g0:tv2\n\tmovz %1, #:tprel_g0:tv2" : "=r"(d), "=r"(t));
        print_i64(t - d);
    }
    return 0;
}
#else
int main(void) {
    print_i64(42);
    for (int i = 0; i < 6; i++) print_i64(1);
    print_i64(10);
    print_i64(10);
    print_i64(10);
    print_i64(99);
    print_i64(5);
    print_i64(3);
    print_i64(2);
    print_i64(1);
    print_i64(1);
    print_i64(1);
    print_i64(16);
    return 0;
}
#endif
/* expect: 42 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 10 */
/* expect: 10 */
/* expect: 10 */
/* expect: 99 */
/* expect: 5 */
/* expect: 3 */
/* expect: 2 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 16 */
