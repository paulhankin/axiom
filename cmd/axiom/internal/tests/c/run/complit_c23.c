/* flags: --c-std=c23 */
/* C23 compound literals: with static (one object, initialized once),
   constexpr (a constant), register, static thread_local, and the empty
   initializer. */
#include "host.h"

static int *counter(void) { return &(static int){ 10 }; }

int main(void) {
    int a[(constexpr int){ 3 }];
    int *p = (static int[]){ 1, 2, 3 };
    int r = (register int){ 5 };
    const int *q = &(static thread_local const int){ 7 };
    int *z = (int[2]){};
    int i;
    for (i = 0; i < 3; i++) {
        (*counter())++;
    }
    print_i64(*counter());
    print_i64(sizeof a / sizeof a[0]);
    static_assert((constexpr int){ 4 } == 4);
    print_i64(p[2] + r + *q);
    print_i64(z[0] + z[1]);
    print_i64((constexpr unsigned char){ 200 } + 1);
    return 0;
}
/* expect: 13 */
/* expect: 3 */
/* expect: 15 */
/* expect: 0 */
/* expect: 201 */
