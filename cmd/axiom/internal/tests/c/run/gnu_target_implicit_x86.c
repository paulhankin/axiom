/* flags: --c-std=gnu17 --c-warnings=error */
/* x86-64-only */
/* The GNU compilers' names of the extensions x86-64 code always has here
   (the baseline's sse and sse2, mmx, fxsr; rdrnd, rdseed and xsave, whose
   intrinsics every program has) are taken in the pragma and the attribute
   target without a diagnostic, beside the extensions that are asked for. */
#include "host.h"
#pragma GCC push_options
#pragma GCC target("sse2,ssse3,rdrnd")
static int twice(int x) { return x * 2; }
#pragma GCC pop_options
__attribute__((target("sse,sse2,fxsr,mmx,xsave,avx2"))) static int thrice(int x) { return x * 3; }
__attribute__((target("rdseed"))) static int once(int x) { return x; }
int main(void) {
    print_i64(twice(2) + thrice(3) + once(4));
    return 0;
}
/* expect: 17 */
