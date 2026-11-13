/* __STDC_WANT_LIB_EXT1__ defined as 0: the names are not declared (C11
   K.3.1.1p2), so a program may use them itself. */
#define __STDC_WANT_LIB_EXT1__ 0
#include <stdio.h>
#include <stdlib.h>

static int set_constraint_handler_s(int x) {
    return x + 1;
}

int main(void) {
    printf("%d\n", set_constraint_handler_s(1));
    return 0;
}
// expect: 2
