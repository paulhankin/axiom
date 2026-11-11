/* flags: --c-std=c23 */
/* native-only */
/* Variable arguments of C23 types (the C library's snprintf, which the
   native tests link): a _BitInt is not promoted (one as wide as int is
   passed as one), nullptr_t is passed as a null void *. */
#include "host.h"

int snprintf(char *buf, size_t n, const char *fmt, ...);

int main(void) {
    char buf[100];
    _BitInt(32) a = -5;
    unsigned _BitInt(64) b = 18446744073709551615uwb;
    _BitInt(64) c = -(_BitInt(64))0x7FFFFFFFFFFFFFFFwb;
    int n = snprintf(buf, sizeof buf, "%d %llu %lld %p", a, b, c, nullptr);
    print_str(buf, (size_t)n);
    return 0;
}

/* expect: -5 18446744073709551615 -9223372036854775807 (nil) */
