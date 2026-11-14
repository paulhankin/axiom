/* flags: --c-std=gnu17 */
/* va_arg of an empty structure (a GNU extension: a structure with no
   members, or with only a zero-length array, has size 0). A call passes
   such an argument in no register and no stack slot, so va_arg reads
   nothing and leaves the arguments after it where they are: before the
   registers run out, and after (from the stack). */
#include <stdarg.h>
#include "host.h"

struct none {};
struct zero { char x[0]; };
struct three { char x[3]; };

static long long f(int n, ...) {
    va_list ap;
    va_start(ap, n);
    long long s = 0;
    for (int i = 0; i < n; i++) {
        struct none e = va_arg(ap, struct none);
        struct zero z = va_arg(ap, struct zero);
        (void)e;
        (void)z;
        s = s * 10 + va_arg(ap, int);
    }
    struct three t = va_arg(ap, struct three);
    double d = va_arg(ap, double);
    va_end(ap);
    return s * 1000 + t.x[0] + t.x[2] + (long)(d * 10);
}

int main(void) {
    struct none e;
    struct zero z;
    struct three t = {{1, 2, 3}};
    print_i64(f(1, e, z, 7, t, 2.5));
    print_i64(f(8, e, z, 1, e, z, 2, e, z, 3, e, z, 4, e, z, 5, e, z, 6, e, z, 7, e, z, 8, t, 2.5));
    print_i64(sizeof(struct none) + sizeof(struct zero));
    return 0;
}
/* expect: 7029 */
/* expect: 12345678029 */
/* expect: 0 */
