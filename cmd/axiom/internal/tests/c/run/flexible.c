/* flags: --c-std=c99 */
/* Flexible array members (C99 6.7.2.1p18): the structure's size leaves
   them out (but keeps the padding their alignment asks for), they are
   reached through a pointer to storage that is large enough, and an
   initializer of the structure gives values to the members before them. */
#include "host.h"

struct V { short n; int data[]; };
struct C { char tag; char text[]; };

static union { struct V v; int room[8]; } store = { { 3 } };

int main(void) {
    struct V *p = &store.v;
    struct C c = { 'x' };
    int i;
    print_i64(sizeof(struct V));
    print_i64(sizeof(struct C));
    print_i64(sizeof c);
    for (i = 0; i < p->n; i++) {
        p->data[i] = (i + 1) * 10;
    }
    print_i64(p->data[0] + p->data[1] + p->data[2]);
    print_i64(store.room[3]);
    print_i64(p->n + c.tag);
    return 0;
}
/* expect: 4 */
/* expect: 1 */
/* expect: 1 */
/* expect: 60 */
/* expect: 30 */
/* expect: 123 */
