/* C11 K.3.1.1p4: __STDC_WANT_LIB_EXT1__ must be defined identically for
   every inclusion of Annex K's headers in a translation unit: here it is
   not defined for the first and defined as 1 for the second, a
   diagnostic as #error gives. */
#include <stdio.h>
#define __STDC_WANT_LIB_EXT1__ 1
#include <string.h>

int main(void) {
    return 0;
}
// check-only
// expect-error: C1040
