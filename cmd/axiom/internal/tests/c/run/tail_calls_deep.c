/* native-only */
/* flags: -O2 */
/* Ten million calls in tail position in a row (cmd/axiom/src/tailrec.ax:
   sibling calls), which as calls would need far more stack than a thread
   has: mutual recursion with integer, bool, double and stack arguments, a
   threaded interpreter whose handlers call the next one through a table,
   and forwarding through a function pointer. At -O2 each is a jump, and the
   stack does not grow. (tail_calls.c runs the same shapes at every level,
   not as deep; tail_calls_regs_deep.c structures returned in registers.) */
#include <stdbool.h>
#include "host.h"

#define N 10000000

static int is_odd(unsigned n);
static int is_even(unsigned n) { return n == 0 ? 1 : is_odd(n - 1); }
static int is_odd(unsigned n) { return n == 0 ? 0 : is_even(n - 1); }

static bool b_odd(long n);
static bool b_even(long n) { return n == 0 || b_odd(n - 1); }
static bool b_odd(long n) { return n != 0 && b_even(n - 1); }

/* nine integer and ten double arguments: some on the stack, permuted on each hop */
static double pong(long n, long a, long b, long c, long d, long e, long f, long g, long h,
                   double x0, double x1, double x2, double x3, double x4, double x5, double x6, double x7,
                   double x8, double x9);
static double ping(long n, long a, long b, long c, long d, long e, long f, long g, long h,
                   double x0, double x1, double x2, double x3, double x4, double x5, double x6, double x7,
                   double x8, double x9) {
    if (n == 0) return (double)(a + 2 * b + 3 * c + 4 * d + 5 * e + 6 * f + 7 * g + 8 * h) + x0 + 2 * x9 + 3 * x8;
    return pong(n - 1, h, a, b, c, d, e, f, g, x9, x0, x1, x2, x3, x4, x5, x6, x7, x8);
}
static double pong(long n, long a, long b, long c, long d, long e, long f, long g, long h,
                   double x0, double x1, double x2, double x3, double x4, double x5, double x6, double x7,
                   double x8, double x9) {
    if (n == 0) return (double)(a - b + c - d + e - f + g - h) + x1 - x2;
    return ping(n - 1, b, c, d, e, f, g, h, a, x1, x2, x3, x4, x5, x6, x7, x8, x9, x0);
}

/* a threaded interpreter: each handler calls the next through the table */
struct vm { const unsigned char *code; long acc; long n; };
typedef long (*handler)(struct vm *, long);
static long op_inc(struct vm *m, long pc);
static long op_dec(struct vm *m, long pc);
static long op_loop(struct vm *m, long pc);
static long op_halt(struct vm *m, long pc);
static const handler table[] = { op_inc, op_dec, op_loop, op_halt };
static long op_inc(struct vm *m, long pc) { m->acc += 3; return table[m->code[pc + 1]](m, pc + 1); }
static long op_dec(struct vm *m, long pc) { m->n -= 1; return table[m->code[pc + 1]](m, pc + 1); }
static long op_loop(struct vm *m, long pc) {
    long to = m->n != 0 ? 0 : pc + 1;
    return table[m->code[to]](m, to);
}
static long op_halt(struct vm *m, long pc) { return m->acc + pc; }

/* through a function pointer passed along */
typedef long (*stepf)(long, long, void *);
static long count_down(long n, long acc, void *self) {
    if (n == 0) return acc;
    return ((stepf)self)(n - 1, acc + (n & 7), self);
}

int main(void) {
    print_i64(is_even(N));
    print_bool(b_even(N + 1));
    print_f64(ping(N, 1, 2, 3, 4, 5, 6, 7, 8, 0.5, 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5));
    static const unsigned char prog[] = { 0, 1, 2, 3 };
    struct vm m = { prog, 0, N / 3 };
    print_i64(table[0](&m, 0));
    print_i64(count_down(N, 0, (void *)count_down));
    return 0;
}
/* expect: 1 */
/* expect: false */
/* expect: 249 */
/* expect: 10000002 */
/* expect: 35000000 */
