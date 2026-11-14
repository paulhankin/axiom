/* flags: --c-std=gnu17 */
/* expect-warning: C4150 */
/* A GNU dialect initializes a character array from a string literal longer
   than the array (6.7.9p2 does not allow it) with the array's length of its
   characters, the rest dropped, with the warning C4150. A string exactly as
   long as the array, without room for the null character, is valid C and
   has no diagnostic. */
#include "host.h"
static const char rows[2][3] = { "pqrst", "uvw" }; /* expect-warning: C4150 */
struct s { char tag[2]; int n; };
int main(void) {
    char local[4] = "abcdefg"; /* expect-warning: C4150 */
    struct s v = { "xyz", 9 }; /* expect-warning: C4150 */
    char exact[3] = "abc";
    print_str(&rows[0][0], 6);
    print_str(local, 4);
    print_str(v.tag, 2);
    print_i64(v.n);
    print_str(exact, 3);
    return 0;
}
/* expect: pqruvw */
/* expect: abcd */
/* expect: xy */
/* expect: 9 */
/* expect: abc */
