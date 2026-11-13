/* flags: --c-std=gnu17 */
/* __declspec (a GNU dialect: the Windows spelling of attributes), as the
   GNU attributes that do what it says: align on objects, structures and
   members, thread (a storage-class specifier), selectany (weak), noinline,
   noreturn, nothrow, deprecated (with its message) and restrict (malloc),
   dllexport and dllimport (no effect), several in one, commas too. */
/* expect-warning: C4050 */
#include "host.h"

__declspec(dllexport) int exported(int x) { return x + 1; }
__declspec(selectany) int shared = 5;
__declspec(thread) int tls = 7;
struct __declspec(align(32)) A { char c; };
struct B { char c; __declspec(align(16)) int x; };
__declspec(align(64)) static char buf[3];
__declspec(noinline) static int twice(int x) { return x * 2; }
__declspec(noreturn) void stop(void);
__declspec(deprecated("use new_api")) int old_api(void) { return 1; }
__declspec(restrict) void *make(void);
__declspec(noinline dllimport) int imported(void);
__declspec(noinline, nothrow) static int three(void) { return 3; }

int main(void) {
    print_i64(exported(1));
    print_i64(shared + tls);
    print_i64(_Alignof(struct A) * 100 + sizeof(struct A));
    print_i64(__builtin_offsetof(struct B, x));
    print_i64((unsigned long)buf % 64);
    print_i64(twice(3) + three());
    print_i64(old_api());
    return 0;
}
/* expect: 2 */
/* expect: 12 */
/* expect: 3232 */
/* expect: 16 */
/* expect: 0 */
/* expect: 9 */
/* expect: 1 */
