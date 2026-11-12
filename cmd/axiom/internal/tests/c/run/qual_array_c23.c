/* flags: --c-std=c23 */
/* C23: an array type is qualified as its element type is (6.7.4.1p3), so
   a pointer to an array converts to a pointer to an array of more qualified
   elements, the operands of ?: may point to arrays whose elements differ in
   qualifiers (the result points to an array of elements with the
   qualifiers of both: 6.5.16p6), and the qualifiers of such an array are
   kept when the other operand points to void. */
#include "host.h"

static int second(const int (*p)[3]) { return (*p)[1]; }

int main(void) {
    int x[2][3] = { { 1, 2, 3 }, { 4, 5, 6 } };
    const int (*cp)[3] = x + 1;
    volatile int (*vp)[3] = x;
    int c = 1;
    const int (*r)[3] = c ? x : cp;
    const int (*r2)[3] = c ? cp : x;
    const volatile int (*r3)[3] = c ? cp : vp;
    void *v = x;
    const void *cv = c ? cp : v;
    static_assert(_Generic(c ? x : cp, const int (*)[3]: 1, default: 0));
    static_assert(_Generic(c ? cp : vp, const volatile int (*)[3]: 1, default: 0));
    static_assert(_Generic(c ? cp : v, const void *: 1, default: 0));
    static_assert(_Generic(c ? cp : nullptr, const int (*)[3]: 1, default: 0));
    typedef const int CA[3];
    static_assert(_Generic((CA *)0, const int (*)[3]: 1, default: 0));
    print_i64(second(x) + second(cp) * 10);
    print_i64((*r)[0] + (*r2)[0] * 10 + (*r3)[2] * 100 + (cv == (void *)cp) * 1000);
    print_i64((c ? x : cp) - cp);
    print_i64((cp == x + 1) + (cp > x) * 2);
    return 0;
}
/* expect: 52 */
/* expect: 1641 */
/* expect: -1 */
/* expect: 3 */
