/* flags: --c-std=c23 */
/* expect-error: C3230 */
#include <tgmath.h>
int main(void) { _Decimal64 x = 1; double d = 2; return (int)pow(x, d); }
