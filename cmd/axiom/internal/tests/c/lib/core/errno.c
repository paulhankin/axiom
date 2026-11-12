/* <errno.h> (C17 7.5): errno is a modifiable lvalue of type int, 0 at
   start-up; EDOM, ERANGE and EILSEQ are distinct positive constants. */
#include <errno.h>
#include "print.h"

int main(void) {
    int *p = &errno;
    print_i64(errno); /* expect: 0 */
    errno = 34;
    print_i64(*p); /* expect: 34 */
    errno++;
    print_i64(errno); /* expect: 35 */
    print_bool(EDOM > 0 && ERANGE > 0 && EILSEQ > 0); /* expect: true */
    print_bool(EDOM != ERANGE && ERANGE != EILSEQ && EDOM != EILSEQ); /* expect: true */
    print_i64(sizeof errno == sizeof(int)); /* expect: 1 */
#if defined(EDOM) && defined(ERANGE) && defined(EILSEQ) && defined(ENOMEM) && defined(EINVAL)
    print_cstr("macros"); /* expect: macros */
#endif
    return 0;
}
