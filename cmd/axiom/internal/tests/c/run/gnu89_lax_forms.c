/* flags: --c-std=gnu89 */
/* expect-warning: C4146 */
/* gnu89 takes bit-fields of any integer type (as C99 lets an implementation)
   and a return with a value in a function returning void (the value is
   evaluated and discarded: C4146). */
#include "host.h"

struct s {
    unsigned short h : 9;
    unsigned long long w : 40;
    enum { A, B } e : 2;
};

static int n;

static int bump(void) {
    n++;
    return n;
}

static void f(void) {
    return bump();
}

int main(void) {
    struct s v;
    v.h = 511;
    v.w = 0xffffffffffULL;
    v.e = B;
    f();
    print_i64(v.h + (v.w == 0xffffffffffULL) + v.e + n);
    return 0;
}
/* expect: 514 */
