/* flags: --c-std=gnu17 */
/* In a GNU dialect a definition of NULL that the program makes before it
   includes a header that defines NULL is replaced by the header's, as the
   common compilers' headers replace it (7.1.3p2 makes the program's
   definition undefined: under an ISO standard it stays C1060). */
#define NULL 0
#include "print.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    print_i64(sizeof(NULL) == sizeof(void *));
    print_i64(_Generic(NULL, void *: 1, default: 0));
    return 0;
}
/* expect: 1 */
/* expect: 1 */
