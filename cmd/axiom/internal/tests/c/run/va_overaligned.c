/* flags: --c-std=c23 */
/* va_arg of structures aligned to more than 16 bytes: on x86-64 System V
   such an argument on the stack is at an address aligned as its type is
   (the caller aligns its stack pointer for the call); AAPCS64, Windows and
   macOS pass them by reference, the copy aligned to 16 at most. */
#include "host.h"
#include <stdarg.h>

struct A { alignas(32) int x; int y; };
struct B { alignas(64) char c[3]; };

static int va(int n, ...) {
    va_list ap;
    va_start(ap);
    struct A s = va_arg(ap, struct A);
    struct B t = va_arg(ap, struct B);
    int k = va_arg(ap, int);
    va_end(ap);
    return n + s.x + s.y * 10 + t.c[2] * 100 + k * 1000;
}

int main(void) {
    struct A s = { 1, 2 };
    struct B t = { { 0, 0, 3 } };
    print_i64(va(0, s, t, 9));
    return 0;
}
/* expect: 9321 */
