/* flags: --c-std=gnu17 */
/* Forms the GNU dialects accept that ISO C does not: an empty declaration
   at file scope; __complex__ and __complex for _Complex, and _Complex alone
   for double _Complex; __real and __imag for __real__ and __imag__; ~ of a
   complex value, its conjugate, and __builtin_conj, creal and cimag (with f
   and l); __builtin_classify_type; a
   __builtin_constant_p in the condition of __builtin_choose_expr; the
   address of an enclosing function's local label taken in a nested
   function, for that function's goto *; return of a void expression in a
   void function. */
#include "host.h"

;
int glob;;

static __complex__ double zd = 1.0 + 2.0i;
static __complex float zf = 3.0f - 4.0if;
static _Complex za = 5.0 + 6.0i;

static int cls(void) {
    char c = 0;
    _Bool b = 0;
    int a[2];
    struct { int x; } st;
    union { int x; } un;
    float f = 0;
    return __builtin_classify_type(c) * 1000000 + __builtin_classify_type(b) * 100000 +
           __builtin_classify_type(a) * 10000 + __builtin_classify_type(f) * 1000 +
           __builtin_classify_type(zd) * 100 + __builtin_classify_type(st) * 10 +
           (__builtin_classify_type(un) - 10);
}

static int y;
static int z = __builtin_choose_expr(!__builtin_constant_p(y), 3, 4);

static int jump(int n) {
    __label__ one, two;
    void *pick(int k) {
        if (k == 1)
            return &&one;
        return &&two;
    }
    goto *pick(n);
one:
    return 10;
two:
    return 20;
}

static void nothing(void) {}
static void calls_nothing(void) { return nothing(); }

int main(void) {
    __complex__ double c = ~zd;
    print_f64(__real c + __imag c);
    print_f64(__builtin_creal(zd) * 10 + __builtin_cimagf(zf) + __builtin_cimag(__builtin_conj(za)));
    print_f64(__imag__ __builtin_conjl(zd) + (sizeof za == sizeof(double _Complex)));
    print_i64(cls());
    print_i64(z + __builtin_choose_expr(__builtin_constant_p(3), 10, 20));
    print_i64(7 + glob);
    print_i64(jump(1) + jump(2));
    calls_nothing();
    return 0;
}
/* expect: -1 */
/* expect: 0 */
/* expect: -1 */
/* expect: 1459023 */
/* expect: 13 */
/* expect: 7 */
/* expect: 30 */
