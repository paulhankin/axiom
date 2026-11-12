/* flags: --c-std=gnu11 */
/* Packed structures, unions and members (the attribute packed), their
   layout, and accesses to their members at any address: through '.',
   '->', arrays in them, nested structures, bit-fields, initializers. */
#include "host.h"
#include <stddef.h>

struct inner { short s; int i; };
struct __attribute__((packed)) P {
    char c;
    struct inner in;
    long long l;
    int arr[3];
    unsigned b1 : 3, b2 : 13, b3 : 17;
    char tail;
};
union __attribute__((packed)) U { signed char c; int i; long long l; };
struct M { char c; int i __attribute__((packed)); short s; };
struct __attribute__((packed, aligned(4))) PA { char c; int i; };
struct H { char c; struct P p; };
static struct P gp = { 1, { 2, 3 }, 4, { 5, 6, 7 }, 7, 8191, 131071, 9 };

static int sum(struct P *p) {
    return p->c + p->in.s + p->in.i + (int)p->l + p->arr[0] + p->arr[1] + p->arr[2] + p->b1 + p->b2 + p->b3 + p->tail;
}

int main(void) {
    print_i64(sizeof(struct P) * 10 + _Alignof(struct P));
    print_i64(offsetof(struct P, in) * 1000 + offsetof(struct P, l) * 100 + offsetof(struct P, arr) * 10 +
              offsetof(struct P, tail));
    print_i64(sizeof(union U) * 10 + _Alignof(union U));
    print_i64(sizeof(struct M) * 100 + offsetof(struct M, i) * 10 + offsetof(struct M, s));
    print_i64(sizeof(struct PA) * 10 + _Alignof(struct PA));
    print_i64(sizeof(struct H) * 10 + offsetof(struct H, p));
    print_i64(sum(&gp));
    struct H h;
    h.c = 0;
    h.p = gp;
    h.p.in.i += 100;
    h.p.arr[2] *= 3;
    h.p.b2 -= 1;
    h.p.b3 = 5;
    h.p.l <<= 8;
    print_i64(sum(&h.p));
    print_i64(h.p.l + h.p.b2 + h.p.b3);
    struct P *pp = &h.p;
    pp->tail = 'z';
    print_i64(pp->tail + pp->arr[2]);
    union U un;
    un.l = 0x1122334455667788LL;
    print_i64(un.i + un.c);
    return 0;
}
/* expect: 351 */
/* expect: 2104 */
/* expect: 81 */
/* expect: 816 */
/* expect: 84 */
/* expect: 361 */
/* expect: 139306 */
/* expect: 9373 */
/* expect: 9219 */
/* expect: 143 */
/* expect: 1432778512 */
