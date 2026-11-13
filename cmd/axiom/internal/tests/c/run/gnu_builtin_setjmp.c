/* flags: --c-std=gnu17 */
/* native-only */
/* __builtin_setjmp and __builtin_longjmp (a buffer of five words; the
   jump from a deeper call while the function that called setjmp runs),
   and __builtin_frame_address and __builtin_return_address of level 1 (the
   caller's frame record). Not on WebAssembly (C3300). */
#include "host.h"

static void *jb[5];
static int depth;
__attribute__((noinline)) static void jumper(int x) {
    depth++;
    if (x > 0)
        jumper(x - 1);
    __builtin_longjmp(jb, 1);
}
__attribute__((noinline)) static void *frame1(void) { return __builtin_frame_address(1); }
__attribute__((noinline)) static void *ret1(void) { return __builtin_return_address(1); }
__attribute__((noinline)) static int is_my_frame(void) { return frame1() == __builtin_frame_address(0); }

int main(void) {
    volatile int n = 0;
    if (__builtin_setjmp(jb) == 0) {
        n = 1;
        jumper(3);
        n = 99;
    }
    print_i64(n * 10 + depth);
/* expect: 14 */
    print_i64(is_my_frame());
/* expect: 1 */
    print_i64(ret1() != 0);
/* expect: 1 */
    return 0;
}
