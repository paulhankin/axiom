/* flags: --c-std=c23 */
/* maybe_unused (6.7.13) on every kind of entity it may appertain to,
   labels too; the program is unchanged. */
#include "host.h"

[[maybe_unused]] static int obj;
[[maybe_unused]] static int fn(void) { return 1; }
[[maybe_unused]] typedef int T;
struct [[maybe_unused]] S { [[maybe_unused]] int m; };
enum [[maybe_unused]] E { A [[maybe_unused]] = 4 };

static int param([[maybe_unused]] int unused, int used [[maybe_unused]]) { return used; }

int main(void) {
    [[maybe_unused]] int local;
[[maybe_unused]] spare:
    print_i64(param(1, 2) + A);
    return 0;
}
/* expect: 6 */
