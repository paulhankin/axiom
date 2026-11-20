/* flags: --c-std=gnu17 */
/* A function whose parameter is a transparent union is compatible with one
   whose parameter has a member's type, as the GNU compilers take it (the
   system's headers declare getsockname so, which programs pass where a
   function of a struct sockaddr * is wanted): it converts without a cast,
   and is called through the pointer with the member's value. */
#include "host.h"
struct addr { int family; };
struct addr6 { int family; int scope; };
typedef union { struct addr *a; struct addr6 *a6; } any_addr __attribute__((transparent_union));

static int family_of(int fd, any_addr p) { return fd + p.a->family; }
static int call(int (*f)(int, struct addr *), struct addr *a) { return f(1, a); }
/* a declaration with the member's type, then the definition with the union */
int lookup(int fd, struct addr6 *p);
int lookup(int fd, any_addr p) { return fd * 10 + p.a6->scope; }

int main(void) {
    struct addr a = { 41 };
    struct addr6 b = { 6, 3 };
    int (*g)(int, struct addr6 *) = family_of;
    print_i64(call(family_of, &a));
    print_i64(g(2, &b));
    print_i64(lookup(4, &b));
    return 0;
}
/* expect: 42 */
/* expect: 8 */
/* expect: 43 */
