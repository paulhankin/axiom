/* GNU89's `extern inline` (the attribute gnu_inline under C99 and later) is
   a definition only for inlining, even after a declaration without inline
   (as the system's headers declare a library function and then define a
   wrapper of it so): no external definition is made here (the other unit's
   is the function, which links without a second definition). */
/* uses: ../print.ax */
/* flags: --c-std=gnu11 */
#include "../print.h"

int calls;
extern int twice(int x);
extern __inline __attribute__((__always_inline__, __gnu_inline__)) int twice(int x) {
    calls++;
    return x * 2 + 1000;
}
extern int thrice(int x);
extern __inline __attribute__((__gnu_inline__)) int thrice(int x) { return x * 3 + 1000; }
extern __inline __attribute__((__gnu_inline__)) int never_used(int x) { return x; }

int via_other(int x);

int main(void) {
    print_i64(twice(5));
    print_i64(calls);
    print_i64(via_other(5));
    return 0;
}
/* expect: 1010 */
/* expect: 1 */
/* expect: 25 */
