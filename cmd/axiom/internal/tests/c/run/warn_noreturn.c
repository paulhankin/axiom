/* flags: --c-std=c23 */
/* expect-warning: C4053 */
/* A function declared _Noreturn or [[noreturn]] that can return is warned
   about (C11 6.7.4p12, C23 6.7.13: recommended practice); one whose every
   path ends in a call of such a function, or loops forever, is not. The
   calls themselves work as usual where they are not reached. */
#include "host.h"

[[noreturn]] static void forever(void) { for (;;) { } }
_Noreturn static void fail(int code) { if (code) forever(); }
[[noreturn]] static void stop(int code) { if (code) fail(code); forever(); }

static int check(int x) {
    if (x < 0) {
        stop(x);
    }
    return x * 2;
}

int main(void) {
    print_i64(check(21));
    return 0;
}
/* expect: 42 */
