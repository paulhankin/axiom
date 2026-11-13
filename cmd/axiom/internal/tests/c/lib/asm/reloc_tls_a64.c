/* flags: --c-std=gnu17 */
/* native-only */
/* linux-only */
/* (a hosted program: linked by the compiler's own linker) */
/* AArch64 thread-local sequences of every code model, relaxed to local exec
   by the linker for an executable's own variables: the large model's
   initial exec (:gottprel_g1:, :gottprel_g0_nc: and a load at the GOT
   base) and TLS descriptor (:tlsdesc_off_g1:, :tlsdesc_off_g0_nc:,
   .tlsdescldr, .tlsdescadd, .tlsdesccall), the tiny model's descriptor (a
   literal ldr and adr of :tlsdesc:), and local dynamic (:tlsldm: and
   :tlsldm_lo12_nc:, then :dtprel_*: offsets in the module's block; and adr of
   :tlsldm:). Elsewhere the same values are printed. */
#include "print.h"

typedef unsigned long long u64;
typedef long long i64;

#ifdef __AXIOM_ASM_AARCH64__
_Thread_local i64 tv = 5;
_Thread_local i64 tv2[4] = {1, 2, 3, 4};

int main(void) {
    u64 p;
    /* large model, initial exec: the slot's offset from the GOT */
    asm("movz %0, #:gottprel_g1:tv\n\tmovk %0, #:gottprel_g0_nc:tv\n\t"
        "adrp x9, _GLOBAL_OFFSET_TABLE_\n\tadd x9, x9, :lo12:_GLOBAL_OFFSET_TABLE_\n\t"
        "ldr %0, [x9, %0]\n\tmrs x9, tpidr_el0\n\tadd %0, %0, x9"
        : "=r"(p)
        :
        : "x9");
    print_i64(p == (u64)&tv);
    /* large model, a TLS descriptor */
    asm volatile("movz x0, #:tlsdesc_off_g1:tv2\n\tmovk x0, #:tlsdesc_off_g0_nc:tv2\n\t"
                 "adrp x9, _GLOBAL_OFFSET_TABLE_\n\tadd x9, x9, :lo12:_GLOBAL_OFFSET_TABLE_\n\t"
                 ".tlsdescldr tv2\n\tldr x1, [x9, x0]\n\t.tlsdescadd tv2\n\tadd x0, x9, x0\n\t"
                 ".tlsdesccall tv2\n\tblr x1\n\tmrs x9, tpidr_el0\n\tadd %0, x0, x9"
                 : "=r"(p)
                 :
                 : "x0", "x1", "x9", "x30", "memory");
    print_i64(p == (u64)&tv2[0]);
    /* tiny model, a TLS descriptor */
    asm volatile("ldr x1, :tlsdesc:tv2+16\n\tadr x0, :tlsdesc:tv2+16\n\t.tlsdesccall tv2+16\n\tblr x1\n\t"
                 "mrs x9, tpidr_el0\n\tadd %0, x0, x9"
                 : "=r"(p)
                 :
                 : "x0", "x1", "x9", "x30", "memory");
    print_i64(p == (u64)&tv2[2]);
    /* local dynamic: the module's block, then a variable's offset in it */
    asm volatile("adrp x0, :tlsldm:tv\n\tadd x0, x0, :tlsldm_lo12_nc:tv\n\tbl __tls_get_addr\n\tnop\n\t"
                 "add x0, x0, :dtprel_lo12:tv2\n\tmov %0, x0"
                 : "=r"(p)
                 :
                 : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
                   "x16", "x17", "x18", "x30", "memory", "cc");
    print_i64(p == (u64)&tv2[0]);
    asm volatile("adr x0, :tlsldm:tv\n\tbl __tls_get_addr\n\tnop\n\tmovz x9, #:dtprel_g0:tv\n\tadd %0, x0, x9"
                 : "=r"(p)
                 :
                 : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
                   "x16", "x17", "x18", "x30", "memory", "cc");
    print_i64(p == (u64)&tv);
    print_i64(*(i64 *)p);
    return 0;
}
#else
int main(void) {
    for (int i = 0; i < 5; i++) print_i64(1);
    print_i64(5);
    return 0;
}
#endif
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 5 */
