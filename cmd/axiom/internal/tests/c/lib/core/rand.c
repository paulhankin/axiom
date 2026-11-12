/* rand and srand (C17 7.22.2): the library's generator (<stdlib.h>: a
   64-bit linear congruential generator, the high 31 bits), whose sequence
   starts as after srand(1), and starts again with the same seed. */
#include <stdlib.h>
#include "print.h"

int main(void) {
    int i, ok = 1, first[5];
    print_i64(RAND_MAX); /* expect: 2147483647 */
    for (i = 0; i < 5; i++)
        first[i] = rand();
    print_i64(first[0]); /* expect: 908834774 */
    print_i64(first[1]); /* expect: 1093944153 */
    print_i64(first[4]); /* expect: 1708211034 */
    srand(1);
    for (i = 0; i < 5; i++)
        ok &= rand() == first[i];
    print_bool(ok); /* expect: true */
    srand(42);
    print_i64(rand()); /* expect: 1220265334 */
    print_i64(rand()); /* expect: 484179026 */
    srand(0);
    print_i64(rand()); /* expect: 167951807 */
    for (i = 0; i < 100000; i++) {
        int r = rand();
        if (r < 0 || r > RAND_MAX)
            ok = 0;
    }
    print_bool(ok); /* expect: true */
    return 0;
}
