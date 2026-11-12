/* flags: --c-std=gnu17 */
/* The GNU dialects' builtins: bit operations, overflow arithmetic, memory
   and math operations, classification, constants, the folded ones. */
#include "host.h"

static int g = 3;
static int side(void) { return ++g; }

int main(void) {
    unsigned x = 0x00F0u;
    print_i64(__builtin_clz(x) + 100 * __builtin_ctz(x) + 10000 * __builtin_popcount(x));
    print_i64(__builtin_clzll(1ull) + 100 * __builtin_ctzl(8ul) + 10000 * __builtin_parity(7u));
    print_i64(__builtin_ffs(0) + 10 * __builtin_ffs(12) + 1000 * __builtin_ffsll(1ll << 40));
    print_i64(__builtin_clrsb(-1) + 100 * __builtin_clrsb(1));
    print_u64(__builtin_bswap16(0x1234) + 0x10000u * 0);
    print_u64(__builtin_bswap32(0x11223344u));
    print_u64(__builtin_bswap64(0x0102030405060708ull));
    int r;
    print_i64(__builtin_add_overflow(2147483647, 1, &r));
    print_i64(r);
    print_i64(__builtin_sadd_overflow(1, 2, &r) * 10 + r);
    unsigned long long ull;
    print_i64(__builtin_umulll_overflow(1ull << 40, 1ull << 40, &ull));
    print_i64(__builtin_mul_overflow_p(1000, 1000, (short)0));
    print_i64(__builtin_sub_overflow_p(1, 2, (unsigned)0));
    char buf[8];
    __builtin_memset(buf, 'a', 7);
    buf[7] = 0;
    __builtin_memcpy(buf + 1, "xy", 2);
    __builtin_memmove(buf + 2, buf + 1, 3);
    print_str(buf, 7);
    print_f64(__builtin_fabs(-2.5) + __builtin_sqrt(16.0) + __builtin_floor(1.5) + __builtin_ceil(1.25));
    print_f64(__builtin_trunc(-1.75) + __builtin_rint(2.5) + __builtin_copysign(3.0, -1.0) + 7.0);
    print_f64(__builtin_sqrtf(2.25f));
    double inf = __builtin_inf(), nan = __builtin_nan("");
    print_i64(__builtin_isnan(nan) + 10 * __builtin_isinf(inf) + 100 * __builtin_isfinite(1.0) +
              1000 * __builtin_signbit(-0.0) + 10000 * __builtin_isnormal(1e-310));
    print_i64(__builtin_isinf_sign(-inf) + 10 * __builtin_fpclassify(0, 1, 2, 3, 4, 1e-310));
    print_i64(__builtin_isgreater(nan, 1.0) + 10 * __builtin_isless(1.0, 2.0) + 100 * __builtin_isunordered(nan, 1.0) +
              1000 * __builtin_islessgreater(1.0, 1.0));
    print_i64(__builtin_huge_valf() == inf);
    print_i64(__builtin_constant_p(3 * 4) + 10 * __builtin_constant_p(side()) + 100 * __builtin_constant_p("s"));
    print_i64(g);
    print_i64(__builtin_choose_expr(sizeof(int) == 4, 1, side()));
    char *volatile vbuf = buf;
    char *ob = vbuf;
    print_u64(__builtin_object_size(ob, 0) == (size_t)-1);
    print_i64(__builtin_object_size(ob, 2));
    print_i64(__builtin_expect(g == 3, 1) + __builtin_expect_with_probability(5, 5, 0.9));
    int *a = __builtin_alloca(10 * sizeof(int));
    for (int i = 0; i < 10; i++) a[i] = i;
    char *al = __builtin_alloca_with_align(3, 512);
    print_i64(a[9] + ((size_t)al % 64));
    __builtin_prefetch(a, 0, 3);
    print_i64(*(int *)__builtin_assume_aligned(a + 2, 4));
    print_i64(__builtin_frame_address(0) != 0);
    print_i64(__builtin_abs(-7) + 17);
    if (g == 100)
        __builtin_trap();
    return 0;
}
/* expect: 40424 */
/* expect: 10363 */
/* expect: 41030 */
/* expect: 3031 */
/* expect: 13330 */
/* expect: 1144201745 */
/* expect: 578437695752307201 */
/* expect: 1 */
/* expect: -2147483648 */
/* expect: 3 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: axxyaaa */
/* expect: 9.5 */
/* expect: 5 */
/* expect: 1.5 */
/* expect: 1111 */
/* expect: 29 */
/* expect: 110 */
/* expect: 1 */
/* expect: 101 */
/* expect: 3 */
/* expect: 1 */
/* expect: 1 */
/* expect: 0 */
/* expect: 6 */
/* expect: 9 */
/* expect: 2 */
/* expect: 1 */
/* expect: 24 */
