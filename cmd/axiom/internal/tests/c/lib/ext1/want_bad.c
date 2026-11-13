/* __STDC_WANT_LIB_EXT1__ must be 0 or 1 (C11 K.3.1.1p2-p3): another
   value is an error. */
#define __STDC_WANT_LIB_EXT1__ 2
#include <stdlib.h>

int main(void) {
    return 0;
}
// check-only
// expect-error: C1040
