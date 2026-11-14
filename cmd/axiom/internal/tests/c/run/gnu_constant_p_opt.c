/* flags: --c-std=gnu17 */
/* __builtin_constant_p of an operand that is not a constant expression
   (a GNU dialect): 0 where a constant expression needs its value (a static
   initializer chooses by it), and in code 1 when the optimizer has made the
   operand a constant (a local object initialized with one, an inlined
   function's parameter), else 0; never 1 at -O0. An operand with side
   effects is 0 and is not evaluated; the address of an object or function
   is not a constant for it (the link decides it), a string literal's is. */
#include "host.h"

#ifdef __OPTIMIZE__
#define OPT 1
#else
#define OPT 0
#endif

static int y = 5;
int global;
static const int tab[] = { __builtin_constant_p(y) ? y : -1, __builtin_constant_p(3), !__builtin_constant_p(y) };

static inline __attribute__((always_inline)) int f(int n) {
    return __builtin_constant_p(n) ? 100 + n : n;
}

int g(int n) { return f(n); }

int main(void) {
    int size = sizeof(int);
    volatile int v = 3;
    int w = v;
    int k = 0;
    char buf[4];
    print_i64(tab[0] * 100 + tab[1] * 10 + tab[2]);
    print_i64(__builtin_constant_p(size) == OPT);
    print_i64((__builtin_constant_p(size) && size == 4) == OPT);
    print_i64(__builtin_constant_p(w) + __builtin_constant_p(v) + __builtin_constant_p(k++) + k);
    print_i64(f(7) == (OPT ? 107 : 7));
    print_i64(g(w));
    if (__builtin_constant_p(size))
        print_i64(OPT);
    else
        print_i64(!OPT);
    print_i64(__builtin_constant_p(&global) + __builtin_constant_p(buf) + __builtin_constant_p(main));
    print_i64(__builtin_constant_p("s") + __builtin_constant_p((char *)0) + __builtin_constant_p(1.5));
    return 0;
}
/* expect: -89 */
/* expect: 1 */
/* expect: 1 */
/* expect: 0 */
/* expect: 1 */
/* expect: 3 */
/* expect: 1 */
/* expect: 0 */
/* expect: 3 */
