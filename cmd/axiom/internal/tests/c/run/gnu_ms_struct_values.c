/* flags: --c-std=gnu17 */
/* x86-64-only */
/* The values of bit-fields laid out by the attribute ms_struct (x86-64):
   each in its unit from the least significant bit, read back through the
   member and seen in the object's bytes; a static initializer gives them
   the same places. */
#include "host.h"

struct __attribute__((ms_struct)) M { char a : 4; short b : 9; short c : 7; char d; int e : 20; unsigned f : 12; };
static struct M s = { -3, 200, -9, 'x', -70000, 4000 };

static void show(struct M *m) {
    const unsigned char *p = (const unsigned char *)m;
    unsigned long long lo = 0;
    for (int i = 0; i < 8; i++)
        lo |= (unsigned long long)p[i] << (8 * i);
    unsigned long long hi = 0;
    for (int i = 8; i < 12; i++)
        hi |= (unsigned long long)p[i] << (8 * (i - 8));
    print_i64(sizeof *m);
    print_i64(m->a + m->b + m->c + m->d + m->e + (int)m->f);
    print_i64((long long)(lo & 0xffffffffff0fULL));
    print_i64((long long)hi);
}

int main(void) {
    struct M v;
    __builtin_memset(&v, 0, sizeof v);
    v.a = -3; v.b = 200; v.c = -9; v.d = 'x'; v.e = -70000; v.f = 4000;
    show(&v);
    show(&s);
    return 0;
}
/* expect: 12 */
/* expect: -65692 */
/* expect: 519402160141 */
/* expect: 4195282576 */
/* expect: 12 */
/* expect: -65692 */
/* expect: 519402160141 */
/* expect: 4195282576 */
