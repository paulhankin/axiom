/* A decimal floating operand with an imaginary one (Annex G) has no common
   type (C23 6.3.1.8p1). */
/* flags: --c-std=c23 */
/* expect-error: C3230 */
#include <complex.h>
int main(void) { _Decimal64 x = 1; double _Imaginary y = 2.0 * _Imaginary_I; return x + y != 0; }
