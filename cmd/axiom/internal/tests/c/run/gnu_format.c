/* flags: --c-std=gnu17 */
/* The format attribute's checks at the default level (--c-warn-format=1):
   an argument of a type the conversion does not take (C4130), too few
   arguments (C4131), a conversion specification that is not one (C4133);
   format_arg, the scanf archetype and %n$ positions. A warning does not
   change the program. */
/* expect-warning: C4130 */
/* expect-warning: C4131 */
/* expect-warning: C4133 */
#include <stdarg.h>
#include "host.h"

static int seen;
__attribute__((format(printf, 2, 3))) static void say(int tag, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    seen += tag;
    va_end(ap);
}
__attribute__((format(scanf, 1, 2))) static int get(const char *fmt, ...) { return fmt[0]; }
__attribute__((format_arg(1))) static const char *tr(const char *s) { return s; }

int main(void) {
    int n = 0;
    long l = 0;
    say(1, "%d %s %5.2f %ld %zu %p %%", 1, "x", 2.0, 3L, sizeof n, (void *)&n); /* (correct) */
    say(1, "%2$s %1$d", 7, "y"); /* (correct) */
    say(1, tr("%d"), 4); /* (correct) */
    get("%d %ld %s %c", &n, &l, (char[8]){0}, (char[1]){0}); /* (correct) */
    say(2, "%s", 5); /* C4130 */
    say(4, "%d %d", 1); /* C4131 */
    say(8, "%y", 1); /* C4133 */
    say(16, tr("%f"), "z"); /* C4130 */
    get("%d", &l); /* C4130 */
    print_i64(seen);
/* expect: 33 */
    return 0;
}
