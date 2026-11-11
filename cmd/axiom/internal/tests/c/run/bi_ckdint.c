/* flags: --c-std=c23 */
/* <stdckdint.h>: ckd_add, ckd_sub and ckd_mul over signed and unsigned
   operands and results of every width, at the edges of their ranges
   (hashes of the results and the overflow flags), and a few by value. */
#include <stdckdint.h>
#include <stdint.h>
#include <limits.h>
#include "host.h"
int main(void) {
    long long vs[] = { 0, 1, -1, 127, -128, 255, 32767, -32768, 65535, INT_MAX, INT_MIN, UINT_MAX, LLONG_MAX, LLONG_MIN, 3037000499LL, -3037000500LL, 4294967296LL };
    unsigned long long us[] = { 0, 1, 255, 65535, 4294967295ULL, 9223372036854775808ULL, 18446744073709551615ULL, 4294967296ULL };
    int n = sizeof vs / sizeof vs[0], m = sizeof us / sizeof us[0], i, j;
    unsigned long long h = 0;
    for (i = 0; i < n; i++) for (j = 0; j < n; j++) {
        long long a = vs[i], b = vs[j];
        signed char sc; unsigned char uc; short ss; int si; unsigned ui; long long sl; unsigned long long ul;
        h = h * 31 + ckd_add(&sc, a, b) + 2 * ckd_sub(&uc, a, b) + 4 * ckd_mul(&ss, a, b) + 8 * ckd_add(&si, a, b)
            + 16 * ckd_mul(&ui, a, b) + 32 * ckd_sub(&sl, a, b) + 64 * ckd_mul(&ul, a, b) + 128 * ckd_mul(&sl, a, b);
        h = h * 7 + (unsigned char)sc + uc + (unsigned short)ss + (unsigned)si + ui + (unsigned long long)sl + ul;
    }
    print_u64(h);
    for (i = 0; i < m; i++) for (j = 0; j < n; j++) {
        unsigned long long a = us[i]; long long b = vs[j];
        long long sl; unsigned long long ul; int si; unsigned short us_;
        h = h * 31 + ckd_add(&sl, a, b) + 2 * ckd_sub(&ul, a, b) + 4 * ckd_mul(&sl, a, b) + 8 * ckd_mul(&ul, b, a)
            + 16 * ckd_sub(&si, b, a) + 32 * ckd_add(&us_, a, b);
        h = h * 7 + (unsigned long long)sl + ul + (unsigned)si + us_;
    }
    print_u64(h);
    { int r; print_i64(ckd_add(&r, INT_MAX, 1)); print_i64(r); print_i64(ckd_mul(&r, 46341, 46341)); print_i64(r);
      unsigned u8; print_i64(ckd_sub(&u8, 0, 1)); print_u64(u8); }
    return 0;
}

/* expect: 2782917764167081968 */
/* expect: 10831235208492142042 */
/* expect: 1 */
/* expect: -2147483648 */
/* expect: 1 */
/* expect: -2147479015 */
/* expect: 1 */
/* expect: 4294967295 */
