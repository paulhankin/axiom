/* flags: --c-std=gnu17 --features=+exception-handling */
/* wasm-only */
/* __builtin_setjmp and __builtin_longjmp on WebAssembly with its
   exception-handling extension (as <setjmp.h>'s, cgen_sjlj.ax). */
#include "host.h"

static void *jb[5];
static int depth;
__attribute__((noinline)) static void jumper(int x) {
    depth++;
    if (x > 0)
        jumper(x - 1);
    __builtin_longjmp(jb, 1);
}

int main(void) {
    volatile int n = 0;
    if (__builtin_setjmp(jb) == 0) {
        n = 1;
        jumper(3);
        n = 99;
    }
    print_i64(n * 10 + depth);
/* expect: 14 */
    return 0;
}
