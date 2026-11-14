/* flags: --c-std=gnu89 */
/* expect-warning: C4145 */
/* A GNU dialect before C99 or C11 takes the keywords that the later
   standards spell in the namespace reserved to the implementation (_Bool,
   _Complex, _Static_assert, _Alignas, _Alignof, _Generic, _Noreturn,
   _Thread_local, _Atomic, _BitInt), with a warning: no program could use
   those names for its own. */
#include "host.h"

_Static_assert(sizeof(_Bool) == 1, "a bool is a byte");
struct box { char c; _Alignas(16) char d; };
static _Thread_local int per_thread = 3;
_Noreturn static void never(void) { for (;;) { } }

static int f(void) {
    _Bool b = 7;
    _Complex double z = 2.0;
    _Atomic int a = 4;
    _BitInt(12) w = 100;
    int g = _Generic(b, _Bool: 10, default: 20);
    return b + (int)__real__ z + a + (int)w + g + per_thread + (int)_Alignof(struct box);
}

int main(void) {
    if (0) never();
    print_i64(f());
    print_i64(sizeof(struct box));
    return 0;
}
/* expect: 136 */
/* expect: 32 */
