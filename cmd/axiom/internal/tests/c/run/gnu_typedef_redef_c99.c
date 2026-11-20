/* flags: --c-std=gnu99 */
/* The GNU dialects take a typedef declared again with the same type before
   C11 too (C11 6.7p3), as the system's headers declare size_t and va_list
   again under a GNU compiler; another type stays C3002. */
#include "host.h"
typedef unsigned long T;
typedef unsigned long T;
typedef int A[3];
typedef int A[3];
int main(void) {
    T x = 5;
    A a = { 1, 2, 3 };
    print_i64((long long)x + a[2] + (long long)sizeof(A));
    return 0;
}
/* expect: 20 */
