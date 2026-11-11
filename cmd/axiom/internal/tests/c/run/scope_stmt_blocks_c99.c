/* flags: --c-std=c99 */
/* C99 6.8.4p3, 6.8.5p5: a selection or iteration statement is a block, its
   controlling expression included: what it declares there ends with it. */
#include "host.h"

int main(void) {
    enum { a = 1 };
    int n = 0;
    if (sizeof(enum { a = 2 }) > 0 && a == 2) n += 1;
    print_i64(a);
    switch (sizeof(enum { a = 3 })) { default: n += a; }
    while (n < 10 && sizeof(enum { a = 4 })) n += a;
    do n++; while (sizeof(enum { a = 5 }) == 0);
    print_i64(a);
    print_i64(n);
    return 0;
}

/* expect: 1 */
/* expect: 1 */
/* expect: 13 */
