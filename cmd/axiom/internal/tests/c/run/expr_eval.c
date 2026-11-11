/* The evaluation of expressions (6.5): division and remainder truncate
   toward zero; signed overflow and shift counts out of range are undefined
   (they compile and run); && || ?: and , evaluate their operands in order,
   and only those they need; a compound assignment, ++ and -- evaluate their
   operand once; assignments yield the value stored (converted); the
   results of comparisons and ! are int 0 or 1; unsigned arithmetic wraps. */
#include <limits.h>
#include "host.h"

static int trace[16];
static int nt;
static int mark(int v) { trace[nt++] = v; return v; }
static int idx(int i) { mark(100 + i); return i; }

volatile int vbig = 40;
volatile int vneg = -1;

int main(void) {
    int a[4] = { 0, 0, 0, 0 };
    int i = 0;
    print_i64(7 / 2);
    print_i64(-7 / 2);
    print_i64(7 / -2);
    print_i64(-7 % 2);
    print_i64(7 % -2);
    print_i64(-7 / -2 * -2 + -7 % -2);
    print_u64(0u - 1u);
    print_i64((unsigned char)(250 + 10));
    print_i64(INT_MAX + vneg * -1 < 0); /* undefined: wraps here */
    print_i64((1 << vbig) == (1 << vbig)); /* undefined: does not stop anything */
    print_i64((int)(1u << 31 >> 31));
    print_i64(-16 >> 2);
    print_i64(1 && mark(1) && mark(0) && mark(2));
    print_i64(0 || mark(3) || mark(4));
    print_i64(mark(5) ? mark(6) : mark(7));
    print_i64((mark(8), mark(9)));
    for (int k = 0; k < nt; k++)
        print_i64(trace[k]);
    nt = 0;
    a[idx(1)] += 5;
    a[idx(2)]++;
    --a[idx(3)];
    print_i64(nt);
    print_i64(a[1] * 100 + a[2] * 10 + a[3]);
    unsigned char uc;
    print_i64(uc = 300);
    double d;
    print_f64(d = i = 7.9);
    short s = -1;
    print_i64((unsigned short)s == 65535);
    print_i64((s == -1) + (s != 0) + !s + !!s);
    print_i64(sizeof(1 == 1) == sizeof(int));
    int x = 5;
    x += x *= 2;
    print_i64(i);
    print_i64(x >= 10);
    long long big = 1;
    big <<= 40;
    print_i64(big >> 39);
    unsigned u = 1;
    u -= 2;
    print_u64(u);
    u *= u;
    print_u64(u);
    i = 3;
    i *= 2.5;
    print_i64(i);
    i = 10;
    i /= 0.5;
    print_i64(i);
    char ch = 'A';
    ch += 1.5;
    print_i64(ch);
    _Bool b = 0;
    b += 2;
    print_i64(b);
    b -= 1;
    print_i64(b);
    b--;
    print_i64(b);
    print_i64(-(unsigned short)1 < 0);
    print_i64(~0u == UINT_MAX);
    print_i64(-INT_MIN == INT_MIN);
    return 0;
}
/* expect: 3 */
/* expect: -3 */
/* expect: -3 */
/* expect: -1 */
/* expect: 1 */
/* expect: -7 */
/* expect: 4294967295 */
/* expect: 4 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: -4 */
/* expect: 0 */
/* expect: 1 */
/* expect: 6 */
/* expect: 9 */
/* expect: 1 */
/* expect: 0 */
/* expect: 3 */
/* expect: 5 */
/* expect: 6 */
/* expect: 8 */
/* expect: 9 */
/* expect: 3 */
/* expect: 509 */
/* expect: 44 */
/* expect: 7 */
/* expect: 1 */
/* expect: 3 */
/* expect: 1 */
/* expect: 7 */
/* expect: 1 */
/* expect: 2 */
/* expect: 4294967295 */
/* expect: 1 */
/* expect: 7 */
/* expect: 20 */
/* expect: 66 */
/* expect: 1 */
/* expect: 0 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
