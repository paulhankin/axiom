/* flags: --c-char=unsigned */
/* Plain char unsigned: its range, promotions and character constants. */
#include "host.h"
#include <limits.h>

static int f(char c) { return c; }

int main(void) {
    char c = -1;
    char s[] = "\xff\x80";
    print_i64(c);
    print_i64(c < 0);
    print_i64(CHAR_MIN);
    print_i64(CHAR_MAX);
    print_i64('\xff');
    print_i64(s[0] + s[1]);
    print_i64(f(200));
    print_i64((char)-56);
    print_i64(c == 255);
    print_i64((int)(char)300);
    return 0;
}

/* expect: 255 */
/* expect: 0 */
/* expect: 0 */
/* expect: 255 */
/* expect: 255 */
/* expect: 383 */
/* expect: 200 */
/* expect: 200 */
/* expect: 1 */
/* expect: 44 */
