/* flags: --c-std=c23 */
/* The headers in C23: the __STDC_VERSION_*_H__ macros, <stdbool.h> with
   bool, true and false keywords (not macros), nullptr_t, unreachable(),
   and the C23 headers <stdbit.h> and <stdckdint.h>. */
#include <stdbool.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <float.h>
#include <stdarg.h>
#include <stdbit.h>
#include <stdckdint.h>
#include "host.h"

#if defined(bool) || defined(true) || defined(false) || defined(alignas) || defined(alignof)
#error "C23: keywords, not macros"
#endif
#if __STDC_VERSION_STDDEF_H__ != 202311L || __STDC_VERSION_STDINT_H__ != 202311L || __STDC_VERSION_LIMITS_H__ != 202311L
#error "versions"
#endif
#if __STDC_VERSION_FLOAT_H__ != 202311L || __STDC_VERSION_STDARG_H__ != 202311L
#error "versions"
#endif
#if __STDC_VERSION_STDBIT_H__ != 202311L || __STDC_VERSION_STDCKDINT_H__ != 202311L
#error "versions"
#endif

static int classify(int x) {
    if (x > 0) return 1;
    if (x < 0) return -1;
    if (x == 0) return 0;
    unreachable();
}

int main(void) {
    nullptr_t n = nullptr;
    int r;
    print_i64(n == nullptr);
    print_i64(classify(-4) * 10 + classify(9));
    print_i64(stdc_count_ones(0xF0u) + stdc_bit_width((unsigned char)5));
    print_i64(ckd_mul(&r, 65536, 65536) * 10 + ckd_add(&r, 1, 2) + r);
    return 0;
}

/* expect: 1 */
/* expect: -9 */
/* expect: 7 */
/* expect: 13 */
