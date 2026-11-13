/* flags: --c-std=c23 */
/* wasm-only */
/* The format checks know the decimal floating types' length modifiers (C23
   7.23.6.1p7, 7.23.6.2p11): H, D and DD with a, e, f and g take
   _Decimal32, _Decimal64 and _Decimal128 (not promoted), and pointers to
   them in scanf; with another conversion they are no specification. */
/* expect-error: C4130 */
/* expect-error: C4133 */
#include <stdio.h>
#pragma GCC diagnostic error "-Wformat"
int main(void) {
    _Decimal64 d = 1.5dd;
    _Decimal32 h = 2.5df;
    printf("%Df %Ha %DDe %HG\n", d, h, 3.0dl, h);
    if (sscanf("1.5 2.5", "%Df %Hf", &d, &h) != 2)
        return 1;
    printf("%Df\n", 1.0);
    printf("%Dd\n", 1);
    return 0;
}
