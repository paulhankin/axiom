/* Calls in tail position (cmd/axiom/src/tailrec.ax: sibling calls; at -O2
   they jump to the callee): mutual recursion, arguments permuted and on the
   stack (more than the registers hold, mixed integers and doubles), a
   callee with fewer or more stack arguments than the caller, structures by
   value and returned through memory or in registers (9 to 16 bytes,
   integers and floats mixed; returned as the callee left them, or changed
   first), variadic callees, calls through pointers (a threaded
   interpreter), values in callee-saved registers, several tail positions
   in loops and switches, a local whose address escaped and a
   variable-length array (which keep their calls), and void, integer,
   narrow, bool, float and double results. */
#include <stdarg.h>
#include <stdbool.h>
#include "host.h"

/* PAD keeps a function out of line (too big to inline, and not internal) */
static volatile int vs;
#define PAD vs = 1; vs = 2; vs = 3; vs = 4; vs = 5; vs = 6; vs = 7; vs = 8; vs = 9; vs = 10; \
    vs = 11; vs = 12; vs = 13; vs = 14; vs = 15; vs = 16; vs = 17; vs = 18; vs = 19; vs = 20; \
    vs = 21; vs = 22; vs = 23; vs = 24; vs = 25; vs = 26; vs = 27; vs = 28; vs = 29; vs = 30; \
    vs = 31; vs = 32; vs = 33; vs = 34; vs = 35; vs = 36; vs = 37; vs = 38; vs = 39; vs = 40;

/* mutual recursion */
static long hops;
static int is_odd(unsigned n);
static int is_even(unsigned n) {
    if (n == 0) return 1;
    hops++;
    return is_odd(n - 1);
}
static int is_odd(unsigned n) {
    if (n == 0) return 0;
    hops++;
    return is_even(n - 1);
}

/* bool results, as `a && f(x)` and `a || f(x)` */
static bool b_odd(long n);
static bool b_even(long n) { return n == 0 || b_odd(n - 1); }
static bool b_odd(long n) { return n != 0 && b_even(n - 1); }

/* permuted and overlapping arguments */
long g3(long a, long b, long c) { PAD  return a * 10000 + b * 100 + c; }
long f3(long a, long b, long c) { PAD  return g3(c, a, b); }
long f3b(long a, long b, long c) { PAD  return g3(b, b, a + c); }

/* stack arguments: 8 integers and 10 doubles (more than either ABI's registers) */
double g18(long a, long b, long c, long d, long e, long f, long g, long h,
                  double x0, double x1, double x2, double x3, double x4, double x5, double x6, double x7,
                  double x8, double x9) { PAD 
    return (double)(a + 2 * b + 3 * c + 4 * d + 5 * e + 6 * f + 7 * g + 8 * h) +
           x0 + 2 * x1 + 3 * x2 + 4 * x3 + 5 * x4 + 6 * x5 + 7 * x6 + 8 * x7 + 9 * x8 + 10 * x9;
}
double f18(long a, long b, long c, long d, long e, long f, long g, long h,
                  double x0, double x1, double x2, double x3, double x4, double x5, double x6, double x7,
                  double x8, double x9) { PAD 
    /* the stack arguments shuffled among themselves */
    return g18(h, g, f, e, d, c, b, a, x9, x8, x7, x6, x5, x4, x3, x2, x1, x0);
}
/* mixed: integers and doubles interleaved, 9 of each */
long gmix(long a, double x, long b, double y, long c, double z, long d, double w, long e,
                 long f, long g, long h, long i, double p, double q, double r, double s, double t) { PAD 
    return a + b * 3 + c * 5 + d * 7 + e * 11 + f * 13 + g * 17 + h * 19 + i * 23 +
           (long)(x * 2 + y * 4 + z * 8 + w * 16 + p * 32 + q * 64 + r * 128 + s * 256 + t * 512);
}
long fmix(long a, double x, long b, double y, long c, double z, long d, double w, long e,
                 long f, long g, long h, long i, double p, double q, double r, double s, double t) { PAD 
    return gmix(i, t, h, s, g, r, f, q, e, d, c, b, a, p, w, z, y, x);
}
/* fewer stack arguments than the caller has */
long g7(long a, long b, long c, long d, long e, long f, long g) { PAD  return a - b + c - d + e - f + g * 1000; }
long f9(long a, long b, long c, long d, long e, long f, long g, long h, long i) { PAD 
    return g7(i, h, g, f, e, d, c + b + a);
}
/* more than the caller has: an ordinary call */
long g10(long a, long b, long c, long d, long e, long f, long g, long h, long i, long j) { PAD 
    return a + b * 2 + c * 3 + d * 4 + e * 5 + f * 6 + g * 7 + h * 8 + i * 9 + j * 10;
}
long f2(long a, long b) { PAD  return g10(a, b, a, b, a, b, a, b, a, b); }

/* structures */
struct big { long a, b, c, d; };
struct two { long a, b; };
struct dd { double x, y; };
struct big mkbig(long k) { PAD  struct big r = { k, k + 1, k + 2, k + 3 }; return r; }
struct big fwdbig(long k) { PAD  return mkbig(k * 2); }
struct two mktwo(long a, long b) { PAD  struct two r = { a, b }; return r; }
struct two fwdtwo(long a, long b) { PAD  return mktwo(b, a); }
struct dd mkdd(double x, double y) { PAD  struct dd r = { x, y }; return r; }
struct dd fwddd(double x) { PAD  return mkdd(x, x * 3); }
long sumbig(struct big b) { PAD  return b.a + b.b * 10 + b.c * 100 + b.d * 1000; }
long passbig(long k) { PAD  struct big b = { k, k, k, k }; return sumbig(b); }
long sumtwo(struct two t, long k) { PAD  return t.a * 100 + t.b + k; }
long passtwo(struct two t, long k) { PAD  return sumtwo(t, k + 1); }

/* variadic callees */
static long vsum(int n, ...) {
    va_list ap;
    va_start(ap, n);
    long s = 0;
    for (int i = 0; i < n; i++) s = s * 10 + va_arg(ap, int);
    va_end(ap);
    return s;
}
long callv(int a, int b, int c) { PAD  return vsum(3, c, b, a); }
static double vdsum(int n, ...) {
    va_list ap;
    va_start(ap, n);
    double s = 0;
    for (int i = 0; i < n; i++) s = s * 2 + va_arg(ap, double);
    va_end(ap);
    return s;
}
double callvd(double a, double b) { PAD  return vdsum(2, b, a); }

/* calls through pointers: a threaded interpreter whose handlers tail-call the next one */
enum { OP_ADD, OP_SUB, OP_DBL, OP_JNZ, OP_HALT };
struct vm { const int *code; long acc; long n; };
typedef long (*handler)(struct vm *, int);
static long op_add(struct vm *m, int pc);
static long op_sub(struct vm *m, int pc);
static long op_dbl(struct vm *m, int pc);
static long op_jnz(struct vm *m, int pc);
static long op_halt(struct vm *m, int pc);
static const handler table[] = { op_add, op_sub, op_dbl, op_jnz, op_halt };
static long next(struct vm *m, int pc) { return table[m->code[pc]](m, pc); }
static long op_add(struct vm *m, int pc) { m->acc += m->code[pc + 1]; return next(m, pc + 2); }
static long op_sub(struct vm *m, int pc) { m->n -= 1; return next(m, pc + 1); }
static long op_dbl(struct vm *m, int pc) { m->acc = (m->acc * 2) % 1000003; return next(m, pc + 1); }
static long op_jnz(struct vm *m, int pc) { return next(m, m->n != 0 ? m->code[pc + 1] : pc + 2); }
static long op_halt(struct vm *m, int pc) { (void)pc; return m->acc; }

/* values in callee-saved registers when the call is made */
long mix6(long a, long b, long c, long d, long e, long f) { PAD  return a ^ (b << 1) ^ (c << 2) ^ (d << 3) ^ (e << 4) ^ (f << 5); }
static long opaque(long x) { return x * 7 + 1; }
long saved(long x) { PAD 
    long a = opaque(x), b = opaque(a), c = opaque(b), d = opaque(c), e = opaque(d);
    long f = opaque(e + a);
    return mix6(f, e, d, c, b, a + x);
}

/* several tail positions, in a switch and in a loop */
static int step(int s, int k);
static int st_a(int k) { return k <= 0 ? 1 : step(1, k - 1); }
static int st_b(int k) { return k <= 0 ? 2 : step(2, k - 1); }
static int st_c(int k) { return k <= 0 ? 3 : step(0, k - 3); }
static int step(int s, int k) {
    for (int i = 0; i < 3; i++) {
        if (k % 7 == i) return st_c(k - i);
        if (k % 11 == i) break;
    }
    switch (s) {
    case 0: return st_a(k);
    case 1: return st_b(k);
    default: return st_c(k);
    }
}

/* a local whose address escapes: the call keeps it */
static int *kept;
static void keep(int *p) { kept = p; }
int peek(int k) { PAD  return *kept + k; }
int escaped(int n) { PAD 
    int x = n * 3;
    keep(&x);
    return peek(1);
}
/* a variable-length array: the call keeps it too */
int vsum_arr(const int *a, int n) { PAD  int s = 0; for (int i = 0; i < n; i++) s += a[i]; return s; }
int with_vla(int n) { PAD 
    int a[n];
    for (int i = 0; i < n; i++) a[i] = i * i;
    return vsum_arr(a, n);
}

/* results of each kind */
static long counter;
void bump(long k) { PAD  counter += k; }
void bump2(long k) { PAD  bump(k * 2); }
float half(float x) { PAD  return x / 2; }
float halfp(float x) { PAD  return half(x + 1); }
double third(double x) { PAD  return x / 3; }
double thirdp(double x) { PAD  return third(x * 9); }
signed char narrow(int x) { PAD  return (signed char)x; }
signed char narrowp(int x) { PAD  return narrow(x + 1); }
unsigned short unarrow(int x) { PAD  return (unsigned short)x; }
unsigned short unarrowp(int x) { PAD  return unarrow(x - 2); }
bool gt(long a, long b) { PAD  return a > b; }
bool gtp(long a, long b) { PAD  return gt(b, a); }

/* structures returned in registers (9 to 16 bytes, integers and floats mixed) */
struct ld { long a; double x; };
struct dl { double x; long a; };
struct i3 { int a, b, c; };
struct f3 { float x, y, z; };
struct c9 { char c[9]; };
struct ld mkld(long a, double x) { PAD struct ld r = { a, x }; return r; }
struct ld fwdld(double x, long a) { PAD return mkld(a * 2, x + 0.5); }
struct dl mkdl(double x, long a) { PAD struct dl r = { x, a }; return r; }
struct dl fwddl(long a, double x) { PAD return mkdl(x * 3, a - 1); }
struct i3 mki3(int a, int b, int c) { PAD struct i3 r = { a, b, c }; return r; }
struct i3 fwdi3(int a, int b, int c) { PAD return mki3(c, a, b); }
struct f3 mkf3(float x, float y, float z) { PAD struct f3 r = { x, y, z }; return r; }
struct f3 fwdf3(float x, float y, float z) { PAD return mkf3(z, y, x); }
struct c9 mkc9(int k) { PAD struct c9 r; for (int i = 0; i < 9; i++) r.c[i] = (char)(k + i); return r; }
struct c9 fwdc9(int k) { PAD return mkc9(k + 1); }
/* in a recursion: each level returns what the next one returns */
static struct ld ld_down(long n, double x);
static struct ld ld_up(long n, double x) { return n == 0 ? mkld(7, x) : ld_down(n - 1, x + 1); }
static struct ld ld_down(long n, double x) { return n == 0 ? mkld(8, x) : ld_up(n - 1, x * 0.5); }
/* not the callee's registers: the caller changes them */
struct ld notfwd(long a) { PAD struct ld r = mkld(a, 1.0); r.a += 1; return r; }

int main(void) {
    print_i64(is_even(1000));
    print_i64(is_odd(1001));
    print_i64(hops);
    print_bool(b_even(1000));
    print_bool(b_odd(1000));
    print_i64(f3(1, 2, 3));
    print_i64(f3b(1, 2, 3));
    print_f64(f18(1, 2, 3, 4, 5, 6, 7, 8, 0.5, 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5));
    print_i64(fmix(1, 0.5, 2, 1.5, 3, 2.5, 4, 3.5, 5, 6, 7, 8, 9, 4.5, 5.5, 6.5, 7.5, 8.5));
    print_i64(f9(1, 2, 3, 4, 5, 6, 7, 8, 9));
    print_i64(f2(3, 4));
    struct big b = fwdbig(5);
    print_i64(b.a + b.b + b.c + b.d);
    struct two t = fwdtwo(7, 8);
    print_i64(t.a * 10 + t.b);
    struct dd d = fwddd(1.5);
    print_f64(d.x + d.y);
    print_i64(passbig(3));
    struct two t2 = { 4, 5 };
    print_i64(passtwo(t2, 6));
    print_i64(callv(1, 2, 3));
    print_f64(callvd(1.0, 2.0));
    static const int prog[] = { OP_ADD, 3, OP_DBL, OP_SUB, OP_JNZ, 0, OP_HALT };
    struct vm m = { prog, 0, 500 };
    print_i64(next(&m, 0));
    print_i64(saved(3));
    print_i64(step(0, 1000));
    print_i64(escaped(14));
    print_i64(with_vla(10));
    bump2(21);
    print_i64(counter);
    print_f64(halfp(2.5f));
    print_f64(thirdp(2.0));
    print_i64(narrowp(127));
    print_i64(unarrowp(1));
    print_bool(gtp(1, 2));
    print_bool(gtp(2, 1));
    /* the callees called directly too (a second caller each) */
    print_i64(g3(4, 5, 6));
    print_f64(g18(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1));
    print_i64(gmix(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1));
    print_i64(g7(1, 2, 3, 4, 5, 6, 7));
    print_i64(g10(1, 2, 3, 4, 5, 6, 7, 8, 9, 10));
    print_i64(mkbig(1).d);
    print_i64(mktwo(1, 2).b);
    print_f64(mkdd(1, 2).y);
    struct big bb = { 1, 2, 3, 4 };
    print_i64(sumbig(bb));
    print_i64(sumtwo(t2, 3));
    print_i64(mix6(1, 2, 3, 4, 5, 6));
    int here = 9;
    keep(&here);
    print_i64(peek(0));
    static const int arr[] = { 1, 2, 3 };
    print_i64(vsum_arr(arr, 3));
    bump(1);
    print_i64(counter);
    print_f64(half(1.0f));
    print_f64(third(3.0));
    print_i64(narrow(300));
    print_i64(unarrow(70000));
    print_bool(gt(3, 2));
    struct ld r1 = fwdld(2.25, 5);
    print_i64(r1.a);
    print_f64(r1.x);
    struct dl r2 = fwddl(9, 1.5);
    print_f64(r2.x);
    print_i64(r2.a);
    struct i3 r3 = fwdi3(1, 2, 3);
    print_i64(r3.a * 100 + r3.b * 10 + r3.c);
    struct f3 r4 = fwdf3(1.5f, 2.5f, 3.5f);
    print_f64(r4.x * 100 + r4.y * 10 + r4.z);
    struct c9 r5 = fwdc9(40);
    print_i64(r5.c[0] + r5.c[8] * 1000);
    struct ld r6 = ld_up(1001, 1.0);
    print_i64(r6.a);
    print_f64(r6.x);
    struct ld r7 = notfwd(4);
    print_i64(r7.a);
    return 0;
}
/* expect: 1 */
/* expect: 1 */
/* expect: 2001 */
/* expect: true */
/* expect: false */
/* expect: 30102 */
/* expect: 20204 */
/* expect: 312.5 */
/* expect: 1844 */
/* expect: 6003 */
/* expect: 195 */
/* expect: 46 */
/* expect: 87 */
/* expect: 6 */
/* expect: 3333 */
/* expect: 412 */
/* expect: 321 */
/* expect: 5 */
/* expect: 281004 */
/* expect: 290181 */
/* expect: 3 */
/* expect: 43 */
/* expect: 285 */
/* expect: 42 */
/* expect: 1.75 */
/* expect: 6 */
/* expect: -128 */
/* expect: 65535 */
/* expect: true */
/* expect: false */
/* expect: 40506 */
/* expect: 91 */
/* expect: 1121 */
/* expect: 6997 */
/* expect: 385 */
/* expect: 4 */
/* expect: 2 */
/* expect: 2 */
/* expect: 4321 */
/* expect: 408 */
/* expect: 185 */
/* expect: 9 */
/* expect: 6 */
/* expect: 43 */
/* expect: 0.5 */
/* expect: 1 */
/* expect: 44 */
/* expect: 4464 */
/* expect: true */
/* expect: 10 */
/* expect: 2.75 */
/* expect: 4.5 */
/* expect: 8 */
/* expect: 312 */
/* expect: 376.5 */
/* expect: 49041 */
/* expect: 8 */
/* expect: 2 */
/* expect: 5 */
