/* flags: --c-std=c99 */
/* C99 features together: designated initializers, compound literals,
   variable length arrays, flexible array members, inline, _Bool, long long,
   hexadecimal floating constants, __func__, variadic macros, va_copy, for
   declarations, and the C99 headers. */
#include "host.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <limits.h>
#include <float.h>
#include <stdarg.h>
#include <iso646.h>
struct fl { int n; int a[]; };
struct pt { int x, y; };
static struct pt *mk(void) { return &(struct pt){ 3, 4 }; }
static inline int sq(int x) { return x * x; }
static int va(int n, ...) { va_list ap, aq; int s = 0, i; va_start(ap, n); va_copy(aq, ap); for (i = 0; i < n; i++) s += va_arg(ap, int); s += va_arg(aq, int) * 100; va_end(aq); va_end(ap); return s; }
static long long ll(void) { return LLONG_MAX; }
static int vlaf(int n, int m, int a[n][m]) { return a[n-1][m-1] + (int)(sizeof(*a) / sizeof(int)); }
#define DBG(...) va(__VA_ARGS__)
union u { int i; float f; };
int main(void) {
    int arr[5] = { [4] = 9, [1] = 2, 3 };
    struct pt p = { .y = 7, .x = 1 };
    struct pt q[2] = { [1].y = 5, [0] = { 8 } };
    union u un = { .f = 1.0f };
    bool b = 5;
    _Bool c = 0;
    uint8_t u8 = 255; int64_t i64 = INT64_MIN; uintptr_t up = (uintptr_t)&p; size_t sz = sizeof(intmax_t);
    long long x = ll();
    unsigned long long ux = ULLONG_MAX;
    int n = 3;
    int vla[n][n + 1];
    double h = 0x1.8p1;
    const char *fn = __func__;
    for (int i = 0; i < n; i++) for (int j = 0; j < n + 1; j++) vla[i][j] = i * 10 + j;
    print_i64(arr[0] + arr[1] * 10 + arr[2] * 100 + arr[4] * 1000);
    print_i64(p.x * 10 + p.y); print_i64(q[0].x * 100 + q[0].y * 10 + q[1].y);
    print_i64(un.i == 0x3f800000);
    print_i64(b); print_i64(c); print_i64(u8 + 1); print_i64(i64 < 0); print_i64(up != 0); print_i64(sz == 8);
    print_i64(x == 9223372036854775807LL); print_u64(ux); print_i64(INT_MAX); print_i64(SCHAR_MIN); print_i64(CHAR_BIT);
    print_i64(sizeof vla / sizeof(int)); print_i64(vla[2][3]); print_i64(vlaf(n, n + 1, vla));
    print_f64(h); print_f64(DBL_EPSILON); print_f64(FLT_MAX); print_i64(DBL_DIG); print_i64(FLT_RADIX);
    print_str(fn, 4); print_i64(sizeof(__func__));
    print_i64(mk()->y); print_i64(sq(7));
    print_i64(DBG(3, 1, 2, 3));
    print_i64((1 and 0) or (not 0)); print_i64(5 bitand 3); print_i64(5 xor 3);
    print_i64(offsetof(struct fl, a) == sizeof(int)); print_i64(sizeof(struct fl) == sizeof(int));
    print_i64(((int[]){ 1, 2, 3 })[2]);
    print_i64(INT8_MIN); print_i64(UINT16_MAX); print_i64(INT32_C(5) + UINT64_C(7)); print_i64(sizeof(int_least8_t)); print_i64(sizeof(int_fast16_t) >= 2);
    print_i64(PTRDIFF_MAX > 0); print_i64(SIZE_MAX > 0); print_i64(WCHAR_MAX > 0); print_i64(SIG_ATOMIC_MAX > 0);
    print_i64(-7 / 2); print_i64(-7 % 2);
    print_i64(true + false); print_i64(__bool_true_false_are_defined);
    return 0;
}

/* expect: 9320 */
/* expect: 17 */
/* expect: 805 */
/* expect: 1 */
/* expect: 1 */
/* expect: 0 */
/* expect: 256 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 18446744073709551615 */
/* expect: 2147483647 */
/* expect: -128 */
/* expect: 8 */
/* expect: 12 */
/* expect: 23 */
/* expect: 27 */
/* expect: 3 */
/* expect: 2.220446049250313e-16 */
/* expect: 3.4028234663852886e+38 */
/* expect: 15 */
/* expect: 2 */
/* expect: main */
/* expect: 5 */
/* expect: 4 */
/* expect: 49 */
/* expect: 106 */
/* expect: 1 */
/* expect: 1 */
/* expect: 6 */
/* expect: 1 */
/* expect: 1 */
/* expect: 3 */
/* expect: -128 */
/* expect: 65535 */
/* expect: 12 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: -3 */
/* expect: -1 */
/* expect: 1 */
/* expect: 1 */
