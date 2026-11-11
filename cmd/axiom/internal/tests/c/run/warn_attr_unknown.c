/* flags: --c-std=c23 */
/* expect-warning: C4052 */
/* An attribute without a prefix that the standard does not define is
   ignored, with a warning; one with a prefix is ignored. */
#include "host.h"

[[unknown]] static int a = 1;
[[vendor::unknown(1, [2], {3})]] static int b = 2;

int main(void) {
    [[maybe_unused]] int c;
    [[also_unknown]] print_i64(a + b);
    return 0;
}
/* expect: 3 */
