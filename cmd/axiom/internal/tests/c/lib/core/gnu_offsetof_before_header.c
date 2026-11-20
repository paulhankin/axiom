/* flags: --c-std=gnu17 */
/* In a GNU dialect a definition of offsetof that the program makes before it
   includes <stddef.h> (a fallback of its own, for when no header defined it)
   is replaced by the header's, as the common compilers' <stddef.h> replaces
   it; under an ISO standard it stays C1060. */
#ifndef offsetof
#define offsetof(type, field) ((size_t) & ((type *)0)->field)
#endif
#include "print.h"
#include <stddef.h>

struct S { char c; double d; int a[3]; };
enum { AT = offsetof(struct S, a[1]) };

int main(void) {
    print_i64(offsetof(struct S, d));
    print_i64(AT);
    return 0;
}
/* expect: 8 */
/* expect: 20 */
