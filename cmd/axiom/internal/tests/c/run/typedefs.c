/* Typedefs, enums, qualifiers, tags in nested scopes, anonymous members. */
#include "host.h"

typedef unsigned int uint;
typedef int arr3[3];
typedef struct list { int v; struct list *next; } list;
typedef int (*fnp)(int);
typedef const char *cstr;
enum { ZERO, ONE, TWO, TEN = 10, ELEVEN };
typedef enum shape { CIRCLE = 1, SQUARE = 4 } shape;

struct outer {
    int kind;
    union {
        int i;
        float f;
    };
    struct {
        short a, b;
    };
};

static int neg(int x) { return -x; }

int main(void) {
    uint u = 3;
    arr3 a = { 1, 2, 3 };
    list n2 = { 2, 0 }, n1 = { 1, &n2 };
    fnp f = neg;
    cstr s = "text";
    shape sh = SQUARE;
    const volatile int cv = 9;
    struct outer o;
    print_u64(u * 2);
    print_i64(a[2] + sizeof(arr3));
    print_i64(n1.next->v);
    print_i64(f(4));
    print_i64(s[1]);
    print_i64(ELEVEN + TWO);
    print_i64(sh * CIRCLE);
    print_i64(cv);
    o.kind = 1;
    o.i = 5;
    o.a = 6;
    o.b = 7;
    print_i64(o.kind + o.i + o.a * o.b);
    print_i64(sizeof(struct outer));
    {
        struct list { char c; } inner = { 'x' };
        typedef long uint;
        uint big = 5;
        print_i64(inner.c);
        print_i64(big);
        print_i64(sizeof(struct list));
    }
    {
        enum shape e2 = CIRCLE;
        int TEN = 3;
        print_i64(e2 + TEN);
    }
    return 0;
}

/* expect: 6 */
/* expect: 15 */
/* expect: 2 */
/* expect: -4 */
/* expect: 101 */
/* expect: 13 */
/* expect: 4 */
/* expect: 9 */
/* expect: 48 */
/* expect: 12 */
/* expect: 120 */
/* expect: 5 */
/* expect: 1 */
/* expect: 4 */
