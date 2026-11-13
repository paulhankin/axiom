/* flags: --c-std=c11 */
/* Sums of carries (an unsigned compare's 0 or 1) and of other sums, which
   x86-64 computes with adc (x64.ax: mark_carry, gen_carry); a carry
   added to a chained sum was once taken for the chained operand. */
#include "host.h"
static unsigned long long f(unsigned long long a, unsigned long long b, unsigned long long p, unsigned long long q,
                            unsigned long long r, unsigned long long s) {
    return (unsigned long long)(p < q) + ((unsigned long long)(r < s) + (a + b));
}
int main(void) {
    print_u64(f(1, 2, 3, 4, 6, 5) + f(10, 20, 4, 3, 5, 6) * 100);
    return 0;
}
/* expect: 3104 */
