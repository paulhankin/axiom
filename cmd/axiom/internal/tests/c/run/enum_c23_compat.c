/* flags: --c-std=c23 */
/* C23 6.2.7p1: enumerations with the same tag declared in different scopes
   (or defined again in the same one) are compatible when their constants
   have the same names and values, in any order, and their fixed underlying
   types, if they have them, are compatible. */
#include "host.h"

enum E : long { A, B } ee = B;
enum K { K1 = 5, K2 } kk = K2;
enum E : long { A, B };

static long f(enum E *p) { return *p; }

int main(void) {
    enum E : long { A, B } e = A;
    enum E *p1 = &ee;
    long r = f(p1) * 10;
    p1 = &e;
    r += f(p1);
    enum K { K2 = 6, K1 = 5 } k = K1;
    enum K *p4 = &kk;
    r = r * 10 + *p4;
    p4 = &k;
    r = r * 10 + *p4;
    print_i64(r);
    print_i64(_Generic(e, enum E: 1, default: 0));
    return 0;
}
/* expect: 1065 */
/* expect: 1 */
