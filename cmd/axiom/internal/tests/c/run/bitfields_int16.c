/* flags: --c-int=16 --c-long=32 */
/* Bit-fields with a 16-bit int: units of 2 bytes for int, 4 for long; an
   unsigned bit-field as wide as int promotes to unsigned int, a narrower
   one to int; a long bit-field keeps its type. */
#include "host.h"

struct A { unsigned a : 3; int b : 13; unsigned c : 16; int d : 9; long e : 20; };
struct B { char x; int y : 12; };

int main(void) {
    struct A s = { 0, -4096, 0, 255, 0 };
    print_i64(sizeof(struct A));
    print_i64(sizeof(struct B));
    print_i64(s.b);
    s.b = -4097;
    print_i64(s.b);
    print_i64(s.d);
    print_i64(s.c - 1);
    print_i64(s.a - 1);
    print_i64(s.e - 1);
    s.c = 65535;
    s.c++;
    print_i64(s.c);
    s.e = 524287;
    print_i64(s.e);
    s.e += 1;
    print_i64(s.e);
    return 0;
}
/* expect: 8 */
/* expect: 4 */
/* expect: -4096 */
/* expect: 4095 */
/* expect: 255 */
/* expect: 65535 */
/* expect: -1 */
/* expect: -1 */
/* expect: 0 */
/* expect: 524287 */
/* expect: -524288 */
