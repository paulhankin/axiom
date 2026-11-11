/* flags: --c-std=c23 */
/* nullptr and nullptr_t (C23 6.2.5, 6.3.2.4, 6.4.4.6): a scalar type with
   one value, the null pointer; conversions to pointers and bool, and from
   null pointer constants; comparisons, ?:, arguments, results, objects
   and static initializers. */
#include <stddef.h>
#include "host.h"

static nullptr_t zs = 0;
static nullptr_t zn = nullptr;
static int *ps = nullptr;
static bool bs = nullptr;
static void (*fs)(void) = nullptr;

static nullptr_t give(void) { return nullptr; }
static int is_null(int *p) { return p == nullptr; }
static int take(nullptr_t z) { return z == nullptr; }

int main(void) {
    int x = 1;
    int *p = &x;
    nullptr_t z = nullptr;
    print_i64(sizeof(nullptr_t) == sizeof(void *));
    print_i64(_Generic(nullptr, nullptr_t: 1, void *: 2, default: 3));
    print_i64(_Generic(z, nullptr_t: 1, default: 3));
    typeof(nullptr) w = 0;
    print_i64(w == z);
    print_i64(p == nullptr);
    print_i64(nullptr != p);
    print_i64(is_null(nullptr));
    print_i64(is_null(z));
    print_i64(take(nullptr) + take(0));
    print_i64(give() == nullptr);
    print_i64(nullptr == 0);
    print_i64(z == (void *)0);
    bool b = z;
    print_i64(b);
    print_i64((bool)nullptr);
    print_i64(!nullptr);
    print_i64(nullptr ? 1 : 2);
    print_i64(z || x);
    int *q = x ? p : nullptr;
    print_i64(q == p);
    q = x ? nullptr : p;
    print_i64(q == nullptr);
    print_i64(zs == zn);
    print_i64(ps == nullptr && !bs && fs == nullptr);
    void *v = (void *)nullptr;
    print_i64(v == 0);
    (void)nullptr;
    char *s = nullptr;
    s = z;
    print_i64(s == 0);
    return 0;
}

/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 0 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 2 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 0 */
/* expect: 0 */
/* expect: 1 */
/* expect: 2 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
