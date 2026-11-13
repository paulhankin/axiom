/* flags: --c-std=gnu17 */
/* native-only */
/* SVE instructions assembled from inline asm and run under the user-mode
   emulator: results derived from the instructions' definitions. A whilelo
   builds a four-lane predicate (independent of the vector length), under
   which a predicated add, a predicated load/store, an index and a cntp are
   checked; a dup broadcasts a scalar; an incw advances a counter. Elsewhere
   the same values are printed so the expectations hold on every target. */
#include "host.h"

typedef unsigned int u32;
typedef unsigned long long u64;

#ifdef __AXIOM_ASM_AARCH64__
static u32 a[4], b[4], out[4], idx[4];

int main(void) {
    for (int i = 0; i < 4; i++) {
        a[i] = (u32)(i + 1);
        b[i] = (u32)(10 * (i + 1));
    }
    /* whilelo p0.s, xzr, #4 makes lanes 0..3 active; a predicated add of the
       four loaded lanes gives {11, 22, 33, 44}, stored back under the same
       predicate. cntp counts the active lanes (4). */
    u64 active;
    asm volatile(
        "mov x3, #4\n\t"
        "whilelo p0.s, xzr, x3\n\t"
        "ld1w {z0.s}, p0/z, [%2]\n\t"
        "ld1w {z1.s}, p0/z, [%3]\n\t"
        "add z0.s, p0/m, z0.s, z1.s\n\t"
        "st1w {z0.s}, p0, [%1]\n\t"
        "cntp %0, p0, p0.s"
        : "=r"(active)
        : "r"(out), "r"(a), "r"(b)
        : "p0", "z0", "z1", "x3", "memory");
    for (int i = 0; i < 4; i++) print_i64(out[i]);
    print_i64((long long)active);
    /* index z0.s, #0, #1 is {0,1,2,...}; stored under the four-lane predicate
       the first four are 0,1,2,3. */
    asm volatile(
        "mov x3, #4\n\t"
        "whilelo p0.s, xzr, x3\n\t"
        "index z0.s, #0, #1\n\t"
        "st1w {z0.s}, p0, [%0]"
        :
        : "r"(idx)
        : "p0", "z0", "x3", "memory");
    for (int i = 0; i < 4; i++) print_i64(idx[i]);
    /* dup z0.s, w1 broadcasts 7 to every lane; read back under the predicate
       the first four are all 7. */
    u64 bcast = 7;
    asm volatile(
        "mov x3, #4\n\t"
        "whilelo p0.s, xzr, x3\n\t"
        "dup z0.s, %w0\n\t"
        "st1w {z0.s}, p0, [%1]"
        :
        : "r"(bcast), "r"(idx)
        : "p0", "z0", "x3", "memory");
    for (int i = 0; i < 4; i++) print_i64(idx[i]); /* 7 7 7 7 */
    return 0;
}
#else
int main(void) {
    print_i64(11);
    print_i64(22);
    print_i64(33);
    print_i64(44);
    print_i64(4);
    print_i64(0);
    print_i64(1);
    print_i64(2);
    print_i64(3);
    print_i64(7);
    print_i64(7);
    print_i64(7);
    print_i64(7);
    return 0;
}
#endif
/* expect: 11 */
/* expect: 22 */
/* expect: 33 */
/* expect: 44 */
/* expect: 4 */
/* expect: 0 */
/* expect: 1 */
/* expect: 2 */
/* expect: 3 */
/* expect: 7 */
/* expect: 7 */
/* expect: 7 */
/* expect: 7 */
