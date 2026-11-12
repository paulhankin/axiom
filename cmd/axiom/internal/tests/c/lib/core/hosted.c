/* A hosted program (C17 4p6): __STDC_HOSTED__ is 1, and the library's
   headers are there. */
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <stdlib.h>
#include "print.h"

int main(void) {
    print_i64(__STDC_HOSTED__); /* expect: 1 */
#ifdef EXIT_SUCCESS
    print_cstr("found"); /* expect: found */
#endif
    return 0;
}
