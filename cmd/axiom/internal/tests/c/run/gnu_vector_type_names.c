/* flags: --c-std=gnu17 */
/* In a GNU dialect the attribute vector_size in a type name (of a compound
   literal, a cast, sizeof) makes the vector type, as in a declaration. */
#include "host.h"
#define VEC(n, t) __attribute__((vector_size((n) * sizeof(t)))) t
int main(void) {
    VEC(4, float) f = (VEC(4, float)){ 1.5f, 2.5f, 3.5f, 4.5f };
    VEC(2, long long) z = (VEC(2, long long)){};
    VEC(4, int) bits = (VEC(4, int))f;
    f = 2.0f * f + (VEC(4, float)){ 1, 1, 1, 1 };
    print_f64(f[0]);
    print_f64(f[3]);
    print_i64(z[0] + z[1]);
    print_i64(bits[0] == 0x3fc00000);
    print_i64(sizeof(VEC(8, short)));
    print_i64(_Alignof(VEC(2, double)));
    return 0;
}
/* expect: 4 */
/* expect: 10 */
/* expect: 0 */
/* expect: 1 */
/* expect: 16 */
/* expect: 16 */
