/* native-only */
/* linux-only */
/* flags: --c-std=gnu23 */
/* expect: 1 1 1 1 1 1 1 1 1 1 */
/* __int128 across the platform's C calling convention, against the system's
   C compiler (c/host_native.c's h_q_ functions): alone, among other
   arguments so that a pair of registers is not left half used (and on
   AArch64 starts at an even register), beyond the registers (on the stack,
   aligned to 16), in structures, as variadic arguments both ways, and
   through function pointers. */
#include "host.h"

typedef __int128 i128;
typedef unsigned __int128 u128;
typedef struct { i128 x; } h_q1;
typedef struct { char c; i128 x; } h_cq;
typedef struct { long l; u128 x; } h_lq;
i128 h_q_many(int i1, i128 a, long l1, i128 b, int i2, i128 c, i128 d, int i3, i128 e, double f, i128 g);
u128 h_q_shift(u128 x, int k);
h_q1 h_q_q1(h_q1 s);
h_cq h_q_cq(h_cq s);
h_lq h_q_lq(h_lq s);
i128 h_q_va(int n, ...);
i128 h_q_call(i128 (*f)(i128, int, i128));
i128 h_q_vcall(i128 (*f)(int, ...));

static i128 many(int i1, i128 a, long l1, i128 b, int i2, i128 c, i128 d, int i3, i128 e, double f, i128 g) {
    return i1 + a * 2 + l1 * 3 + b * 4 + i2 * 5 + c * 6 + d * 7 + i3 * 8 + e * 9 + (i128)f * 10 + g * 11;
}

static i128 cb(i128 a, int k, i128 b) {
    return a / k + b;
}

static i128 vcb(int n, ...) {
    __builtin_va_list ap;
    i128 s = 0;
    __builtin_va_start(ap, n);
    while (n-- > 0) {
        s = s * 3 + __builtin_va_arg(ap, i128);
        s += __builtin_va_arg(ap, int);
    }
    __builtin_va_end(ap);
    return s;
}

static char out[40];
static int nout;

static void ok(int c) {
    if (nout) out[nout++] = ' ';
    out[nout++] = c ? '1' : '0';
}

int main(void) {
    i128 big = ((i128)0x123456789abcdef << 64) | 0xfedcba9876543210;
    i128 r = h_q_many(1, big, -2, -big, 3, big >> 3, (i128)-5 << 100, 4, 6, 2.0, big + 1);
    ok(r == many(1, big, -2, -big, 3, big >> 3, (i128)-5 << 100, 4, 6, 2.0, big + 1));
    ok(h_q_shift((u128)big, 68) == (((u128)big << 68) | ((u128)big >> 60)));
    h_q1 q = h_q_q1((h_q1){ big });
    ok(q.x == big * 3 - 1);
    h_cq c = h_q_cq((h_cq){ 'a', -big });
    ok(c.c == 'q' && c.x == -big + 'a');
    h_lq l = h_q_lq((h_lq){ 3, (u128)big });
    ok(l.l == -1 && l.x == (u128)big * 3);
    ok(h_q_va(3, big, 1, (i128)-1, 2, (i128)1 << 100, 3) == vcb(3, big, 1, (i128)-1, 2, (i128)1 << 100, 3));
    ok(h_q_call(cb) == cb((i128)1 << 100, 7, -((i128)3 << 70)) + 1);
    ok(h_q_vcall(vcb) == vcb(3, (i128)5 << 64, 1, (i128)-7, 2, (i128)1 << 120, 3));
    i128 (*volatile pm)(int, i128, long, i128, int, i128, i128, int, i128, double, i128) = h_q_many;
    ok(pm(9, 1, 2, 3, 4, 5, 6, 7, 8, 9.5, 10) == many(9, 1, 2, 3, 4, 5, 6, 7, 8, 9.5, 10));
    /* (the stack's pairs after the registers are used up) */
    ok(h_q_many(1, 2, 3, 4, 5, 6, 7, 8, (i128)1 << 90, 0.0, -((i128)1 << 91)) ==
       many(1, 2, 3, 4, 5, 6, 7, 8, (i128)1 << 90, 0.0, -((i128)1 << 91)));
    print_str(out, (size_t)nout);
    return 0;
}
