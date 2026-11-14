/* Objects defined with a const-qualified type are read-only (6.7.3p6), so
   the optimizer reads their values where they are loaded: integers of each
   width and signedness, floating values converted to integers (toward
   zero), pointers in a table, members of a const structure, elements of a
   const array of structures; a const volatile object is still read when the
   program runs. The values are the same at every level. */
#include "host.h"

const signed char sc = -5;
const unsigned short us = 65000;
const int ints[4] = { -1, 2, -300000, 4 };
const long long ll = -1234567890123LL;
const unsigned long long ull = 18446744073709551000ULL;
const float fl = -2.75f;
const double db = 123456.875;
static const char *const names[] = { "zero", "one", "two" };
struct pt { short x; double y; unsigned char z[3]; };
static const struct pt pts[2] = { { 7, 1.5, { 1, 2, 3 } }, { -8, -2.25, { 250, 251, 252 } } };
const volatile int cvi = 11;

int main(void) {
    print_i64(sc + us);
    print_i64(ints[0] + ints[1] + ints[2] + ints[3]);
    print_i64(ll);
    print_u64(ull);
    print_i64((int)fl);
    print_i64((long long)(db * 8));
    print_i64((unsigned)db);
    print_i64((unsigned char)(fl * -50.0f));
    print_str(names[2], 3);
    print_i64(pts[1].x * 1000 + (int)(pts[0].y * pts[1].y * 4) + pts[1].z[2] + pts[0].z[1]);
    print_i64(cvi);
    return 0;
}
/* expect: 64995 */
/* expect: -299995 */
/* expect: -1234567890123 */
/* expect: 18446744073709551000 */
/* expect: -2 */
/* expect: 987655 */
/* expect: 123456 */
/* expect: 137 */
/* expect: two */
/* expect: -7759 */
/* expect: 11 */
