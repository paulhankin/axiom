/* flags: --c-char=unsigned */
/* A plain char bit-field is as signed as plain char (here unsigned); a
   plain int one is signed. */
#include "host.h"

struct A { char c : 4; int i : 4; signed char s : 4; };

int main(void) {
    struct A a = { 15, 15, 15 };
    print_i64(a.c);
    print_i64(a.i);
    print_i64(a.s);
    return 0;
}
/* expect: 15 */
/* expect: -1 */
/* expect: -1 */
