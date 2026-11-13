/* flags: --c-std=gnu17 */
/* The attributes optimize (a function's own level for the optimizer: not
   inlined, nor anything into it, below 2) and target (extensions its code
   may use besides the target's: lowering/c_target.c); an extension no target
   has is the warning C4052, and is ignored. */
/* expect-warning: C4052 */
#include "host.h"
__attribute__((optimize("O0"))) static int slow(int x) {
    int s = 0;
    for (int i = 0; i < x; i++)
        s += i;
    return s;
}
__attribute__((optimize(3))) static int fast(int x) {
    int s = 0;
    for (int i = 0; i < x; i++)
        s += i * 3;
    return s;
}
__attribute__((optimize("-Os"), target("nosuchext"))) static int odd(int x) { return x + 1; }
static volatile int ten = 10;
int main(void) {
    print_i64(slow(ten) + fast(ten) + odd(ten));
    return 0;
}
/* expect: 191 */
