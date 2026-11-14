/* flags: --c-std=gnu17 */
/* A comparison of vectors gives the signed integer vector of its shape,
   each lane all ones or zero; in a GNU dialect it is assigned (and passed,
   returned, used to initialize) to an unsigned vector of that shape too,
   and so are '~' of one and '&', '|', '^' of two, as the common compilers
   do. */
#include "host.h"
typedef unsigned short u16x8 __attribute__((vector_size(16)));
typedef unsigned int u32x4 __attribute__((vector_size(16)));
typedef unsigned long long u64x2 __attribute__((vector_size(16)));
static u32x4 below(u32x4 a, u32x4 b) { return a < b; }
static unsigned lanes(u32x4 m) { return (m[0] & 1) + (m[1] & 1) + (m[2] & 1) + (m[3] & 1); }
int main(void) {
    u16x8 h = { 1, 9, 3, 9, 5, 9, 7, 9 };
    u16x8 nine = h == 9;
    u32x4 a = { 1, 5, 2, 8 };
    u32x4 b = { 4, 4, 4, 4 };
    u32x4 out = ~(a < b);
    u32x4 both = (a < b) & (a != 2);
    u64x2 w = { 3, 0 };
    u64x2 nz = w != 0;
    print_u64(nine[1]);
    print_u64(nine[2]);
    print_u64(lanes(below(a, b)));
    print_u64(lanes(out));
    print_u64(lanes(both));
    print_u64(lanes(a > 1 | a == 1));
    print_u64(nz[0]);
    print_u64(nz[1]);
    return 0;
}
/* expect: 65535 */
/* expect: 0 */
/* expect: 2 */
/* expect: 2 */
/* expect: 1 */
/* expect: 4 */
/* expect: 18446744073709551615 */
/* expect: 0 */
