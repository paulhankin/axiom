/* Functions: prototypes, recursion, mutual recursion, pointers to
   functions, arguments converted to parameter types, old-style
   declarations. */
#include "host.h"

static int fact(int n) { return n <= 1 ? 1 : n * fact(n - 1); }
static int is_even(unsigned n);
static int is_odd(unsigned n) { return n == 0 ? 0 : is_even(n - 1); }
static int is_even(unsigned n) { return n == 0 ? 1 : is_odd(n - 1); }
static int twice(int x) { return 2 * x; }
static int inc(int x) { return x + 1; }
static int apply(int (*f)(int), int x) { return f(x); }
static int (*pick(int which))(int) { return which ? twice : inc; }
static double half(double x) { return x / 2; }
static unsigned char low(unsigned char c) { return c; }
static short neg(short s) { return -s; }
static _Bool nz(int x) { return x; }
static long long wide(long long a, int b, signed char c) { return a * b + c; }
static void set(int *p, int v) { *p = v; }
int old();
int old(a, b) int a; int b; { return a - b; }
typedef int (*binop)(int, int);
static int addi(int a, int b) { return a + b; }
static int subi(int a, int b) { return a - b; }

int main(void) {
    int (*fp)(int) = twice;
    binop ops[2] = { addi, subi };
    int x = 0;
    print_i64(fact(10));
    print_i64(is_even(10));
    print_i64(is_odd(7));
    print_i64(apply(inc, 41));
    print_i64(apply(fp, 21));
    print_i64((*fp)(5));
    print_i64(pick(1)(10));
    print_i64(pick(0)(10));
    print_f64(half(5));
    print_i64(low(300));
    print_i64(neg(-32767));
    print_i64(nz(5));
    print_i64(nz(0));
    print_i64(wide(1LL << 40, 3, -1));
    set(&x, 9);
    print_i64(x);
    print_i64(old(10, 3));
    print_i64(ops[0](2, 3) * ops[1](10, 4));
    print_i64(fp == twice);
    print_i64(fp != inc);
    fp = &inc;
    print_i64(fp(1));
    return 0;
}

/* expect: 3628800 */
/* expect: 1 */
/* expect: 1 */
/* expect: 42 */
/* expect: 42 */
/* expect: 10 */
/* expect: 20 */
/* expect: 11 */
/* expect: 2.5 */
/* expect: 44 */
/* expect: 32767 */
/* expect: 1 */
/* expect: 0 */
/* expect: 3298534883327 */
/* expect: 9 */
/* expect: 7 */
/* expect: 30 */
/* expect: 1 */
/* expect: 1 */
/* expect: 2 */
