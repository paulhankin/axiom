/* long double in the language, whatever its format (double's, the x87's
   or binary128: the output is the same): sizes and layout, constants
   folded as run time computes them, static initial values, conversions
   to and from every arithmetic type, _Bool, increments and compound
   assignments, comparisons with NaNs, _Generic, structures and unions
   holding long doubles, and calls: many arguments, structures by value,
   results, variadic arguments, function pointers. */
#include <float.h>
#include <stdarg.h>
#include <stddef.h>
#include "host.h"

typedef long double LD;

struct S {
    char c;
    LD x;
    short s;
};
struct P {
    LD a, b;
};
union U {
    LD x;
    int i;
};

static LD over = 1.0L + LDBL_EPSILON; /* (more than double holds, where long double is wider) */
static const LD third = 1.0L / 3;
static struct S st = { 'a', 2.5L, 7 };
static LD arr[4] = { 1.0L, -0.0L, 0.5L, LDBL_MAX * 2 };
static double dbl = 1.0L / 3;
static float flt = 1.0L / 3;
static int itr = 7.9L;
volatile LD v_one = 1.0L, v_three = 3.0L, v_eps = LDBL_EPSILON, v_zero = 0.0L;

static LD many(int i1, LD a, double d1, LD b, int i2, LD c, LD d, LD e, LD f, LD g, LD h, LD i, LD j, float f1, LD k) {
    return i1 + a + d1 + b * 2 + i2 + c * 3 + d * 4 + e * 5 + f * 6 + g * 7 + h * 8 + i * 9 + j * 10 + f1 + k * 11;
}

static struct S twice(struct S s) {
    s.x *= 2;
    s.c += 1;
    s.s *= 2;
    return s;
}

static struct P swap(struct P p) {
    struct P r;
    r.a = p.b;
    r.b = p.a;
    return r;
}

static union U neg(union U u) {
    u.x = -u.x;
    return u;
}

static LD sum(int n, ...) {
    va_list ap;
    LD s = 0;
    int k;
    va_start(ap, n);
    for (k = 0; k < n; k++) {
        if (k % 3 == 0)
            s += va_arg(ap, LD);
        else if (k % 3 == 1)
            s += va_arg(ap, double);
        else
            s += va_arg(ap, int);
    }
    va_end(ap);
    return s;
}

static LD sum_list(int n, va_list ap) {
    LD s = 0;
    while (n-- > 0)
        s += va_arg(ap, struct P).a;
    return s;
}

static LD sum_p(int n, ...) {
    va_list ap;
    LD s;
    va_start(ap, n);
    s = sum_list(n, ap);
    va_end(ap);
    return s;
}

static LD apply(LD (*f)(LD, LD), LD a, LD b) {
    return f(a, b);
}

static LD mul(LD a, LD b) {
    return a * b;
}

int main(void) {
    LD x = 1.5L, n;
    union U u;
    struct S s;
    struct P p = { 1.25L, -3.0L };
    /* layout */
    print_i64(sizeof(LD) >= sizeof(double) && _Alignof(LD) == sizeof(LD) && offsetof(struct S, x) == sizeof(LD));
    print_i64(sizeof(struct S) == 3 * sizeof(LD) && _Alignof(max_align_t) >= _Alignof(LD));
    /* constants and run time agree; the extra precision is kept */
    print_i64(third == v_one / v_three && over - 1 == v_eps && v_one + v_eps != 1 && v_one + v_eps / 2 == 1);
    print_i64(1.0L + LDBL_EPSILON / 2 == 1.0L && (LDBL_MAX * 2 > LDBL_MAX) && -LDBL_MAX * 2 < -LDBL_MAX);
    print_i64(LDBL_MIN / 2 > 0 && LDBL_TRUE_MIN / 2 == 0 && LDBL_TRUE_MIN / LDBL_EPSILON == LDBL_MIN);
    print_i64(dbl == 1.0 / 3 && flt == 1.0f / 3 && itr == 7 && arr[3] > LDBL_MAX && st.x == 2.5L && st.s == 7);
    /* _Bool, !, ?: and && */
    print_i64(!arr[1] && (_Bool)0.5L && (arr[1] ? 0 : 1) && (v_zero || v_one) && !(v_zero && v_one));
    /* increments, compound assignment, mixed operands */
    x++;
    ++x;
    x--;
    x += 0.25;
    x *= 2;
    x /= 0.5f;
    x -= 1;
    print_i64((long long)x);
    print_i64(x == 10.0L);
    /* to and from every arithmetic type */
    print_i64((signed char)-100.75L);
    print_i64((unsigned char)200.5L);
    print_i64((short)-30000.9L);
    print_i64((unsigned short)65535.99L);
    print_i64((int)-2147483648.0L);
    print_i64((long long)-9223372036854775808.0L);
    print_u64((unsigned long long)9223372036854775808.0L);
    print_u64((unsigned long long)(v_one * 18446744073709549568.0L));
    print_i64((_Bool)v_zero + (_Bool)(v_zero - 1) * 2);
    print_f64((double)(v_one / 8));
    print_f64((float)(v_one / 3) == 1.0f / 3);
    print_i64((LD)(unsigned long long)-1 > 1e19L && (LD)(long long)-1 == -1);
    print_i64(LDBL_MANT_DIG < 64 || (LD)0x7fffffffffffffffLL - 0x7ffffffffffffffeLL == 1);
    print_i64((LD)1e-45f == 1e-45f && (LD)4.9406564584124654e-324 == 4.9406564584124654e-324);
    /* NaNs and infinities */
    n = v_zero / v_zero;
    print_i64(n != n && !(n == n) && !(n < 1) && !(n >= 1) && (n ? 1 : 0) && !(v_one < n));
    print_i64(-(v_one / v_zero) < -LDBL_MAX && (LD)(1.0 / 0.0 * v_one) == v_one / v_zero);
    /* the type of mixed operations */
    print_i64(_Generic(1.0L + 1, long double: 1, default: 0) + _Generic(1.0L * 1.0f, long double: 2, default: 0));
    print_i64(_Generic(v_one, long double: 1, double: 2, default: 3) + _Generic((double)1.0L, double: 10, default: 0));
    /* structures and unions */
    s = twice(st);
    print_i64(s.c == 'b' && s.x == 5.0L && s.s == 14);
    p = swap(p);
    print_f64((double)p.a);
    print_f64((double)p.b);
    u.x = 0.75L;
    u = neg(u);
    print_f64((double)u.x);
    /* calls */
    print_f64((double)many(1, 0.5L, 0.25, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 0.125f, 1));
    print_f64((double)sum(7, 1.5L, 2.25, 3, 4.0L, 0.5, -2, v_one / 4));
    print_f64((double)sum_p(3, p, p, p));
    print_f64((double)apply(mul, 1.5L, -4.0L));
    print_i64(sum(1, 1.0L + LDBL_EPSILON) - 1 == LDBL_EPSILON);
    return 0;
}

/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 10 */
/* expect: 1 */
/* expect: -100 */
/* expect: 200 */
/* expect: -30000 */
/* expect: 65535 */
/* expect: -2147483648 */
/* expect: -9223372036854775808 */
/* expect: 9223372036854775808 */
/* expect: 18446744073709549568 */
/* expect: 2 */
/* expect: 0.125 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 3 */
/* expect: 11 */
/* expect: 1 */
/* expect: -3 */
/* expect: 1.25 */
/* expect: -0.75 */
/* expect: 68.875 */
/* expect: 9.5 */
/* expect: -9 */
/* expect: -6 */
/* expect: 1 */
