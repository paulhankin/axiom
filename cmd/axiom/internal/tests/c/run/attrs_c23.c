/* flags: --c-std=c23 */
/* C23 attributes on declarations: the standard ones (deprecated,
   maybe_unused, nodiscard, noreturn and _Noreturn, unsequenced,
   reproducible, with their __x__ spellings) are accepted, others ignored;
   none changes what the program does. thread_local and alignas are
   keywords. */
#include "host.h"

[[deprecated]] static int old_value = 1;
[[deprecated("use new_value")]] static int older = 2;
[[maybe_unused]] static int unused;
[[__maybe_unused__]] static int unused2;
[[nodiscard]] static int value(void) { return 3; }
[[nodiscard("check it")]] static int value2(void) { return 4; }
[[noreturn]] static void stop(void) { for (;;) { } }
[[_Noreturn]] static void stop2(void) { for (;;) { } }
static int pure(int x) [[unsequenced]] { return x + 1; }
static int pure2(int x) [[reproducible]] { return x + 2; }
[[vendor::anything(1, 2, "x")]] static int vendor = 5;
[[unknown_attribute]] static int unknown = 6;
struct [[deprecated]] Tagged { [[maybe_unused]] int a; };
enum Colors { RED [[deprecated]], GREEN };
thread_local int tl = 7;
alignas(16) static int al = 8;
[[vendor::anything]];

int main(void) {
    [[maybe_unused]] int local = 0;
    struct Tagged t = { 9 };
    if (local) {
        stop();
        stop2();
    }
    print_i64(old_value + older + unused + unused2);
    print_i64(value() + value2());
    print_i64(pure(1) + pure2(1));
    print_i64(vendor + unknown + t.a + GREEN);
    print_i64(tl + al);
    return 0;
}
/* expect: 3 */
/* expect: 7 */
/* expect: 5 */
/* expect: 21 */
/* expect: 15 */
