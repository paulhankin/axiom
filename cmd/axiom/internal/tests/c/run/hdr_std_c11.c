/* flags: --c-std=c11 */
/* <stdbool.h>, <stdalign.h>, <stdnoreturn.h> and <iso646.h> in C11: the
   macros the standard gives them. */
#include <stdbool.h>
#include <stdalign.h>
#include <stdnoreturn.h>
#include <iso646.h>
#include <stddef.h>
#include "host.h"

#if !defined(bool) || !defined(true) || !defined(false) || !__bool_true_false_are_defined
#error "stdbool"
#endif
#if !defined(alignas) || !defined(alignof) || !__alignas_is_defined || !__alignof_is_defined
#error "stdalign"
#endif
#ifndef noreturn
#error "stdnoreturn"
#endif

static alignas(16) char buf[3];

static noreturn void stop(void);

int main(void) {
    bool b = 5;
    int x = 6;
    x and_eq 3;
    x or_eq 8;
    x xor_eq 1;
    print_i64(b);
    print_i64(alignof(max_align_t) >= alignof(double));
    print_i64(((size_t)buf & 15) == 0 and not (1 not_eq 1) and (2 bitand 3) == 2 and (compl 0) == -1);
    print_i64(x bitor 16);
    print_i64((1 xor 3) or 0);
    if (x == 0) stop();
    return 0;
}

static noreturn void stop(void) {
    for (;;) {
    }
}

/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 27 */
/* expect: 1 */
