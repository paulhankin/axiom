/* flags: --c-std=gnu17 */
/* native-only */
/* Inline assembly for what C cannot say: the processor's identification and
   counters (cpuid, rdtsc; AArch64's system registers), and atomic
   operations written in assembly (lock cmpxchg; ldaxr/stlxr). */
#include "host.h"

typedef long long i64;
typedef unsigned long long u64;

#ifdef __AXIOM_ASM_X86_64__
static int vendor_ok(void) {
    unsigned a, b, c, d;
    asm volatile("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "a"(0), "c"(0));
    return (a > 0) && (b != 0);
}
static u64 counter(void) {
    unsigned lo, hi;
    asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return ((u64)hi << 32) | lo;
}
static int cas(int *p, int expected, int desired) {
    int prev;
    asm volatile("lock cmpxchgl %2, %1" : "=a"(prev), "+m"(*p) : "r"(desired), "0"(expected) : "memory", "cc");
    return prev;
}
static i64 fetch_add(i64 *p, i64 v) {
    asm volatile("lock xaddq %0, %1" : "+r"(v), "+m"(*p) : : "memory", "cc");
    return v;
}
#else
static int vendor_ok(void) {
    u64 z;
    asm volatile("mrs %0, dczid_el0" : "=r"(z));
    return (z & ~(u64)0x1f) == 0;
}
static u64 counter(void) {
    u64 v;
    asm volatile("isb\n\tmrs %0, cntvct_el0" : "=r"(v));
    return v;
}
static int cas(int *p, int expected, int desired) {
    int prev, fail;
    asm volatile("1: ldaxr %w0, %2\n\tcmp %w0, %w3\n\tb.ne 2f\n\tstlxr %w1, %w4, %2\n\tcbnz %w1, 1b\n2:"
                 : "=&r"(prev), "=&r"(fail), "+Q"(*p) : "r"(expected), "r"(desired) : "memory", "cc");
    return prev;
}
static i64 fetch_add(i64 *p, i64 v) {
    i64 old, sum;
    int fail;
    asm volatile("1: ldaxr %0, %3\n\tadd %1, %0, %4\n\tstlxr %w2, %1, %3\n\tcbnz %w2, 1b"
                 : "=&r"(old), "=&r"(sum), "=&r"(fail), "+Q"(*p) : "r"(v) : "memory");
    return old;
}
#endif

int main(void) {
    print_i64(vendor_ok());
    u64 t0 = counter();
    u64 t1 = counter();
    print_i64(t1 >= t0);
    int x = 5;
    print_i64(cas(&x, 5, 8));
    print_i64(x);
    print_i64(cas(&x, 5, 9));
    print_i64(x);
    i64 total = 100;
    print_i64(fetch_add(&total, 23));
    print_i64(total);
    return 0;
}
/* expect: 1 */
/* expect: 1 */
/* expect: 5 */
/* expect: 8 */
/* expect: 8 */
/* expect: 8 */
/* expect: 100 */
/* expect: 123 */
