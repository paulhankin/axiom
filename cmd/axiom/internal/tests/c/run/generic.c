/* flags: --c-std=c11 */
/* Generic selections (6.5.1.1): the controlling expression's type after
   lvalue conversion (qualifiers dropped) and the conversions of arrays and
   functions to pointers; compatible types; default; the result is the
   selected expression, with its type and value category; only it is
   evaluated. */
#include "host.h"

#define TYPE(x) _Generic((x), char: 1, signed char: 2, unsigned char: 3, short: 4, unsigned short: 5, int: 6, \
    unsigned: 7, long: 8, unsigned long: 9, long long: 10, unsigned long long: 11, float: 12, double: 13, \
    long double: 14, _Bool: 15, int *: 16, const int *: 17, char *: 18, void (*)(void): 19, default: 0)

enum E { E0, E1 };
struct S { int a; } s;
static void fn(void) {}
static int calls;
enum { IS_LONG = _Generic(1L, long: 1, default: 0) };
_Static_assert(IS_LONG && _Generic('a', int: 1), "a generic selection in a constant expression");
static int count(void) { return ++calls; }

int main(void) {
    const int ci = 1;
    volatile short vs = 2;
    int arr[3] = { 4, 5, 6 };
    const int carr[2] = { 7, 8 };
    char c = 'a';
    int x = 0;
    print_i64(TYPE(c));
    print_i64(TYPE('a'));
    print_i64(TYPE(ci));
    print_i64(TYPE(vs));
    print_i64(TYPE(vs + 1));
    print_i64(TYPE(arr));
    print_i64(TYPE(carr));
    print_i64(TYPE("str"));
    print_i64(TYPE(fn));
    print_i64(TYPE(&fn));
    print_i64(TYPE(1.0f));
    print_i64(TYPE(1.0L));
    print_i64(TYPE((_Bool)2));
    print_i64(TYPE(E1));
    enum E e = E1;
    print_i64(TYPE(e)); /* (its compatible type is unsigned int here) */
    print_i64(TYPE(s));
    print_i64(TYPE(0x7fffffffu + 0LL));
    print_i64(_Generic(ci, const int: 1, int: 2));
    print_i64(_Generic(s, struct S: 3, default: 4));
    print_i64(_Generic(arr, int[3]: 5, int *: 6));
    /* not evaluated: the controlling expression, the other associations */
    print_i64(_Generic(count(), int: 10, default: count()));
    print_i64(_Generic(x++, int: _Generic(1.0, double: 20, default: count()), default: count()));
    print_i64(calls + x);
    /* an lvalue when its selected expression is one */
    _Generic(x, int: x, default: c) = 42;
    print_i64(x);
    int *p = &_Generic(1, int: arr[2], default: arr[0]);
    print_i64(*p);
    /* a function designator */
    print_i64(_Generic(1, int: count, default: fn)());
    print_i64(sizeof _Generic(c, char: arr, default: x));
    return 0;
}

/* expect: 1 */
/* expect: 6 */
/* expect: 6 */
/* expect: 4 */
/* expect: 6 */
/* expect: 16 */
/* expect: 17 */
/* expect: 18 */
/* expect: 19 */
/* expect: 19 */
/* expect: 12 */
/* expect: 14 */
/* expect: 15 */
/* expect: 6 */
/* expect: 7 */
/* expect: 0 */
/* expect: 10 */
/* expect: 2 */
/* expect: 3 */
/* expect: 6 */
/* expect: 10 */
/* expect: 20 */
/* expect: 0 */
/* expect: 42 */
/* expect: 6 */
/* expect: 1 */
/* expect: 12 */
