/* 6.7.3p8: a const array type's elements are const; the array has the
   element type's size, and a pointer to its element is a pointer to const. */
#include "host.h"

typedef int A[3];
const A a = { 4, 5, 6 };

int main(void) {
    const int *p = a;
    print_i64(sizeof a / sizeof a[0]);
    print_i64(p[2]);
    return 0;
}

/* expect: 3 */
/* expect: 6 */
