/* A GNU dialect accepts a translation unit without declarations (here the
   second unit, which conditional inclusion leaves empty); an ISO standard
   does not (c/parse/syntax-empty-translation-unit.c). */
/* flags: --c-std=gnu17 */
/* units: units/empty_unit.c */
#include "host.h"
int main(void) {
    print_i64(7);
    return 0;
}

/* expect: 7 */
