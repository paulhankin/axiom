/* flags: --c-std=gnu17 --c-warn-format=2 */
/* The format attribute's checks at level 2: more arguments than the format
   uses (C4132), a format that is not a string literal (C4134), an argument
   of the right size and kind but another type, or a flag the conversion
   ignores (C4135). */
/* expect-warning: C4132 */
/* expect-warning: C4134 */
/* expect-warning: C4135 */
#include "host.h"

static int seen;
__attribute__((format(printf, 1, 2))) static void say(const char *fmt, ...) { seen += fmt[0]; }

int main(void) {
    const char *f = "%d";
    say("%d", 1, 2); /* C4132 */
    say(f, 1); /* C4134 */
    say("%u", -1); /* C4135 */
    say("%#d", 1); /* C4135 */
    print_i64(seen);
/* expect: 148 */
    return 0;
}
