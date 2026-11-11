/* va_arg of structures and unions of every size class (in one register,
   two, a floating-point aggregate, by reference), mixed with scalars, and
   enough of them that some come from the stack. */
#include <stdarg.h>
#include "host.h"

struct c3 { char c[3]; };
struct ii { int a, b; };
struct ff { float a, b; };
struct f3 { float a[3]; };
struct dd { double a, b; };
struct di { double d; int i; };
struct ll { long long a, b; };
struct big { int v[10]; };
union ud { double d; long long l; };

static long long take(const char *f, ...) {
    va_list ap;
    long long s = 0;
    va_start(ap, f);
    for (; *f; f++) {
        switch (*f) {
        case 'c': { struct c3 x = va_arg(ap, struct c3); s = s * 10 + x.c[0] + x.c[1] + x.c[2]; break; }
        case 'i': { struct ii x = va_arg(ap, struct ii); s = s * 10 + x.a + x.b; break; }
        case 'f': { struct ff x = va_arg(ap, struct ff); s = s * 10 + (long long)(x.a + x.b); break; }
        case '3': { struct f3 x = va_arg(ap, struct f3); s = s * 10 + (long long)(x.a[0] + x.a[1] + x.a[2]); break; }
        case 'd': { struct dd x = va_arg(ap, struct dd); s = s * 10 + (long long)(x.a + x.b); break; }
        case 'm': { struct di x = va_arg(ap, struct di); s = s * 10 + (long long)x.d + x.i; break; }
        case 'l': { struct ll x = va_arg(ap, struct ll); s = s * 10 + x.a + x.b; break; }
        case 'b': { struct big x = va_arg(ap, struct big); s = s * 10 + x.v[0] + x.v[9]; break; }
        case 'u': { union ud x = va_arg(ap, union ud); s = s * 10 + (long long)x.d; break; }
        case 'n': s = s * 10 + va_arg(ap, int); break;
        case 'x': s = s * 10 + (long long)va_arg(ap, double); break;
        }
    }
    va_end(ap);
    return s;
}

int main(void) {
    struct c3 c = { { 1, 1, 1 } };
    struct ii i = { 2, 2 };
    struct ff f = { 1.5f, 2.5f };
    struct f3 f3 = { { 1.0f, 2.0f, 2.0f } };
    struct dd d = { 2.25, 3.75 };
    struct di m = { 4.0, 2 };
    struct ll l = { 3, 4 };
    struct big b = { { 4, 0, 0, 0, 0, 0, 0, 0, 0, 5 } };
    union ud u;
    u.d = 8.0;
    print_i64(take("cifd", c, i, f, d));
    print_i64(take("3mlb", f3, m, l, b));
    print_i64(take("u", u));
    print_i64(take("nnnnndl", 1, 2, 3, 4, 5, d, l));
    print_i64(take("xxxxxxxdf3", 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, d, f, f3));
    print_i64(take("lllnd", l, l, l, 9, d));
    print_i64(take("bnbn", b, 1, b, 2));
    return 0;
}

/* expect: 3446 */
/* expect: 5679 */
/* expect: 8 */
/* expect: 1234567 */
/* expect: 1234567645 */
/* expect: 77796 */
/* expect: 9192 */
