/* flags: --c-std=gnu11 */
/* Packed structures, unions and members (the attribute packed), their
   layout, and accesses to their members at any address: through '.',
   '->', arrays in them, nested structures, bit-fields, initializers. The
   bit-fields' layout is the target's (Windows x64 keeps units of their
   type): the layout numbers expected are System V's and AAPCS64's, or
   Windows's, confirmed against the platforms' compilers. */
#include "host.h"
#include <stddef.h>

#define LAYOUT(v, sysv, win) print_i64((v) == (__AXIOM_BITFIELD_LAYOUT__ == 2 ? (win) : (sysv)) ? 1 : -(long long)(v))

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
    LAYOUT(sizeof(struct P) * 10 + _Alignof(struct P), 351, 381);
    LAYOUT(offsetof(struct P, in) * 1000 + offsetof(struct P, l) * 100 + offsetof(struct P, arr) * 10 +
               offsetof(struct P, tail),
           2104, 2107);
    print_i64(sizeof(union U) * 10 + _Alignof(union U));
    print_i64(sizeof(struct M) * 100 + offsetof(struct M, i) * 10 + offsetof(struct M, s));
    print_i64(sizeof(struct PA) * 10 + _Alignof(struct PA));
    LAYOUT(sizeof(struct H) * 10 + offsetof(struct H, p), 361, 391);
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
/* expect: 1 */
/* expect: 1 */
/* expect: 81 */
/* expect: 816 */
/* expect: 84 */
/* expect: 1 */
/* expect: 139306 */
/* expect: 9373 */
/* expect: 9219 */
/* expect: 143 */
/* expect: 1432778512 */
