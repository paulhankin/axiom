/* flags: --c-std=gnu17 */
/* The GNU dialects before C23 take C23's enumeration constants outside int's
   range (a flag in the top bit, 1u << 31, as the system's headers define
   them): the enumerated type is the first of unsigned int, long, ... that
   holds the values, and the constants then have it. */
#include "host.h"

enum Flags { F_IN = 1, F_OUT = 4, F_EDGE = 1u << 31 };
enum Big { B_SMALL = -1, B_LARGE = 0x100000000, B_NEXT };
enum Fits { X_A = 1, X_B = 0x7fffffff };

int main(void) {
    print_i64(sizeof(enum Flags));
    print_u64(F_EDGE);
    print_bool(F_EDGE > 0);
    print_bool(_Generic(F_EDGE, unsigned int: 1, default: 0));
    print_i64(sizeof(enum Big));
    print_i64(B_NEXT);
    print_bool(B_SMALL < 0);
    print_bool(_Generic(X_B, int: 1, default: 0));
    unsigned int ev = F_IN | F_EDGE;
    print_bool((ev & F_EDGE) != 0);
    return 0;
}
/* expect: 4 */
/* expect: 2147483648 */
/* expect: true */
/* expect: true */
/* expect: 8 */
/* expect: 4294967297 */
/* expect: true */
/* expect: true */
/* expect: true */
