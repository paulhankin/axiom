/* flags: --c-std=gnu17 */
/* native-only */
/* The x86-64 constraint 'A': a 128-bit value in the rdx:rax pair (rdx its
   high half) as an output, an input, '+' and tied with "0"; a value of up
   to 64 bits in rax. */
#include "host.h"

#ifdef __AXIOM_ASM_X86_64__
typedef unsigned __int128 u128;
static u128 mul(unsigned long long a, unsigned long long b) { u128 r; __asm__("mulq %2" : "=A"(r) : "a"(a), "rm"(b)); return r; }
static unsigned long long hi_of(u128 v) { unsigned long long h; __asm__("mov %%rdx, %0" : "=r"(h) : "A"(v)); return h; }
static u128 inc(u128 v) { __asm__("addq $1, %%rax\n\tadcq $0, %%rdx" : "+A"(v)); return v; }
static u128 tied(u128 v) { u128 r; __asm__("notq %%rax\n\tnotq %%rdx" : "=A"(r) : "0"(v)); return r; }
static long long small(long long v) { long long r; __asm__("lea 1(%1), %0" : "=A"(r) : "r"(v)); return r; }
static void show(u128 r) { print_u64((unsigned long long)(r >> 64)); print_u64((unsigned long long)r); }
int main(void) {
    show(mul(0xffffffffffffffffULL, 3));
    print_u64(hi_of(((u128)0x1234 << 64) | 5));
    show(inc(((u128)7 << 64) | 0xffffffffffffffffULL));
    show(tied(((u128)1 << 64) | 2));
    print_i64(small(41));
    return 0;
}
#else
int main(void) {
    print_u64(2); print_u64(18446744073709551613ULL); print_u64(4660); print_u64(8); print_u64(0);
    print_u64(18446744073709551614ULL); print_u64(18446744073709551613ULL); print_i64(42);
    return 0;
}
#endif
/* expect: 2 */
/* expect: 18446744073709551613 */
/* expect: 4660 */
/* expect: 8 */
/* expect: 0 */
/* expect: 18446744073709551614 */
/* expect: 18446744073709551613 */
/* expect: 42 */
