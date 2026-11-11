/* flags: --c-std=c23 */
/* expect-warning: C4050 */
/* [[deprecated]] (6.7.13): a use of the name of an object, function,
   typedef name, enumeration constant, member or tag declared with it is
   warned about (with the message, if it has one); its declarations are not,
   and a later declaration can add the attribute. */
#include "host.h"

[[deprecated]] static int old = 1;
[[deprecated("use twice")]] static int once(int x) { return x; }
[[deprecated]] typedef int Old;
enum Color { RED [[deprecated]] = 3, GREEN };
struct [[deprecated("a newer one")]] Point { int x; [[deprecated]] int y; };
static int late(void);
[[deprecated]] static int late(void);
static int late(void) { return 5; }

int main(void) {
    Old o = old;
    struct Point p = { 1, 2 };
    print_i64(o + once(2) + RED + p.y + late());
    return 0;
}
/* expect: 13 */
