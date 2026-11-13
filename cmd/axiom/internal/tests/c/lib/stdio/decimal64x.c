/* _Decimal64x (Annex H: decimal128's format here) with the length modifier
   DD in printf and scanf, which take it as _Decimal128 (the format check
   too: no warning, even as an error). */
/* flags: --c-std=c23 */
#include <stdio.h>
#pragma GCC diagnostic error "-Wformat"

int main(void) {
    _Decimal64x x = 1.25d64x;
    printf("%DDf %.3DDe %DDa\n", x, x * 1000, x);
    _Decimal64x y = 0;
    int n = sscanf("6.75e2", "%DDf", &y);
    printf("%d %DDg\n", n, y);
    return 0;
}
// expect: 1.250000 1.250e+03 1.25
// expect: 1 675
