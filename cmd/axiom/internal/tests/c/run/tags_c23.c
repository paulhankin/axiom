/* flags: --c-std=c23 */
/* C23's tag compatibility (6.2.7p1, 6.7.2.3): a structure, union or
   enumeration defined again in the same scope with the same content is the
   same type; structures and unions with the same tag and content declared
   in different scopes are compatible types. */
#include "host.h"

struct Point { int x, y; };
struct Point { int x, y; };
union Num { int i; float f; };
union Num { int i; float f; };
enum Color { RED, GREEN = 5, BLUE };
enum Color { RED, GREEN = 5, BLUE };
enum Small : unsigned char { S0, S1 };
enum Small : unsigned char { S0, S1 };
struct List { int v; struct List *next; };

static int sum(struct Point *p) { return p->x + p->y; }

static int length(struct List *l) {
    struct List { int v; struct List *next; };  /* (compatible with the outer one) */
    struct List *q = l;
    int n = 0;
    for (; q; q = q->next) {
        n += q->v;
    }
    return n;
}

int main(void) {
    struct Point p = { 1, 2 };
    print_i64(sum(&p));
    {
        struct Point { int x, y; } q = { 10, 20 };
        print_i64(sum(&q));
        p = q;
        print_i64(p.x);
    }
    struct List c = { 3, 0 }, b = { 2, &c }, a = { 1, &b };
    print_i64(length(&a));
    print_i64(BLUE + S1);
    union Num n = { .f = 1.0f };
    print_i64(n.i == 0x3f800000);
    return 0;
}
/* expect: 3 */
/* expect: 30 */
/* expect: 10 */
/* expect: 6 */
/* expect: 7 */
/* expect: 1 */
