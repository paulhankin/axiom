/* getenv (C17 7.22.4.6): a set variable's value, a null pointer for one
   that is not set; asked for again, the same string while the value is
   the same. */
#include <stdlib.h>
#include "print.h"

int main(void) {
    char *p = getenv("PATH"), *q = getenv("PATH");
    print_bool(p != NULL && p[0] != 0); /* expect: true */
    print_bool(p == q); /* expect: true */
    print_bool(getenv("AXIOM_SURELY_UNSET_VARIABLE_42") == NULL); /* expect: true */
    print_bool(getenv("") == NULL); /* expect: true */
    return 0;
}
