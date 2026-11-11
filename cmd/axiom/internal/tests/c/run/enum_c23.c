/* flags: --c-std=c23 */
/* C23 enumerations: a fixed underlying type (the constants have the
   enumerated type; forward declarations), and values outside int (the
   enumeration's type is wide enough; its constants then have that type). */
#include "host.h"

enum Small : unsigned char { S0, S1 = 200, S2 };
enum Wide : long long { W0 = -5, W1 = 0x7FFFFFFFFFFFLL };
enum Fwd : short;
enum Fwd : short { F0 = -3, F1 };
enum Big { B0 = 1, B1 = 0x100000000, B2 };
enum Mid { M0 = 2147483647, M1 };
enum Neg { N0 = -1, N1 = 0xFFFFFFFF };
enum Plain { P0, P1 = 7 };
enum Flag : bool { OFF, ON };

static enum Fwd fwd_value(enum Fwd *p) { return *p; }

int main(void) {
    enum Small s = S2;
    enum Fwd f = F1;
    enum Flag fl = ON;
    print_i64(sizeof(enum Small));
    print_i64(S2);
    print_i64(sizeof S1);
    print_i64(s + 1);
    print_i64(sizeof(enum Wide));
    print_i64(W1);
    print_i64(W0 < 0);
    print_i64(sizeof(enum Fwd));
    print_i64(fwd_value(&f));
    print_i64(sizeof(enum Big));
    print_i64(B2);
    print_i64(sizeof B0);
    print_i64(sizeof(enum Mid));
    print_i64(M1);
    print_i64(sizeof M0);
    print_i64(sizeof(enum Neg));
    print_i64(N1);
    print_i64(N0);
    print_i64(sizeof P1);
    print_i64(sizeof(enum Flag));
    print_i64(fl + ON);
    return 0;
}
/* expect: 1 */
/* expect: 201 */
/* expect: 1 */
/* expect: 202 */
/* expect: 8 */
/* expect: 140737488355327 */
/* expect: 1 */
/* expect: 2 */
/* expect: -2 */
/* expect: 8 */
/* expect: 4294967297 */
/* expect: 8 */
/* expect: 4 */
/* expect: 2147483648 */
/* expect: 4 */
/* expect: 8 */
/* expect: 4294967295 */
/* expect: -1 */
/* expect: 4 */
/* expect: 1 */
/* expect: 2 */
