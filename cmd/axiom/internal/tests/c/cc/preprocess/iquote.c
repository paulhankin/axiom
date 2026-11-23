/* -iquote: its directories are searched for "..." (after the source's
   own), not for <...>. */
#include <stdio.h>
#include "which.h"
int q = WHICH + NESTED;
#undef WHICH
#include <which.h>
int i = WHICH;
int main(void) {
    printf("%d %d\n", q, i);
    return 0;
}
