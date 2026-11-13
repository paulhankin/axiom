/* flags: --c-std=gnu17 */
/* __bf16's complex type and vectors of __bf16 (a GNU dialect): the parts
   and the lanes computed in float and rounded to bfloat16 (each vector
   operation; a complex value when it is stored), passed and returned as
   _Float16's are, converted with __builtin_convertvector. */
#include "host.h"

typedef __bf16 v8bf __attribute__((vector_size(16)));
typedef float v8f __attribute__((vector_size(32)));

static _Complex __bf16 mul(_Complex __bf16 a, _Complex __bf16 b) { return a * b; }
static _Complex __bf16 dv(_Complex __bf16 a, _Complex __bf16 b) { return a / b; }
static v8bf fma8(v8bf a, v8bf b) { return a * b + a; }

static void pc(_Complex __bf16 z) {
    print_f64((double)__real__ z);
    print_f64((double)__imag__ z);
}

int main(void) {
    _Complex __bf16 z = 1.5f + 2.25if;
    _Complex __bf16 w = 3.0f - 0.1if;
    pc(mul(z, w));
    pc(dv(z, w));
    pc(z + w);
    print_i64(sizeof z * 10 + _Alignof(_Complex __bf16));
    print_bool(z == (_Complex __bf16)(1.5f + 2.25if));
    v8bf a = {1.0f, 2.5f, 3.1f, -4.0f, 5.0f, 6.0f, 7.0f, 1e30f}, b = {0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 1e30f};
    v8bf c = fma8(a, b), e = -a, f = a / b;
    for (int i = 0; i < 8; i += 3) {
        print_f64((double)c[i]);
        print_f64((double)e[i]);
        print_f64((double)f[i]);
    }
    v8f g = __builtin_convertvector(c, v8f);
    print_f64((double)g[1]);
    print_i64(sizeof(v8bf));
    return 0;
}
/* expect: 4.71875 */
/* expect: 6.59375 */
/* expect: 0.474609375 */
/* expect: 0.765625 */
/* expect: 4.5 */
/* expect: 2.15625 */
/* expect: 42 */
/* expect: true */
/* expect: 1.1015625 */
/* expect: -1 */
/* expect: 10 */
/* expect: -5.59375 */
/* expect: 4 */
/* expect: -10 */
/* expect: 11.875 */
/* expect: -7 */
/* expect: 10 */
/* expect: 3 */
/* expect: 16 */
