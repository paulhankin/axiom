/* flags: --c-std=gnu17 */
/* The decimal floating types and their constants before C23: an extension
   of the GNU dialects. */
#include "host.h"
#include "../dec_show.h"

int main(void) {
    _Decimal64 x = 1.20dd;
    _Decimal32 y = 0.5df;
    _Decimal128 z = 3.dl;
    dec_show(x * (_Decimal64)y + (_Decimal64)z);
    return 0;
}
/* expect: 3.600 */
