/* C23's strdup and strndup (7.26.2.6, 7.26.2.7): copies in memory from
   malloc; strndup copies at most n bytes and adds the terminating null. */
/* flags: --c-std=c23 */
#include <stdlib.h>
#include <string.h>
#include "print.h"

int main(void) {
    char a[5] = {'a', 'b', 'c', 'd', 'e'}; /* (no null) */
    char *p = strdup("copy me"), *q = strndup("truncate", 4), *r = strndup(a, 5), *s = strndup("ab", 10);
    print_cstr(p); /* expect: copy me */
    print_cstr(q); /* expect: trun */
    print_cstr(r); /* expect: abcde */
    print_cstr(s); /* expect: ab */
    print_i64((long long)strlen(s)); /* expect: 2 */
    free(p);
    free(q);
    free(r);
    free(s);
    return 0;
}
