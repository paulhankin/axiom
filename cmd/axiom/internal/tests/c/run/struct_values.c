/* flags: --c-std=c11 */
/* Structure values (6.5.2.3, 6.5.15, 6.5.16.1): assignment (also chained)
   and its value, ?: with structure operands, member access on structure
   values that are not lvalues (f().x, (c ? s : t).x, (s = t).x), arrays in
   such values (f().a[i]: an object with temporary lifetime, C11 6.2.4p8),
   comma, and values passed and returned. */
#include "host.h"

struct P { int x, y; };
struct A { int a[4]; char tag; };
struct Big { long long v[6]; };

static struct P mk(int x, int y) { struct P p = { x, y }; return p; }
static struct A mka(int k) { struct A r = { { k, k + 1, k + 2, k + 3 }, 'z' }; return r; }
static struct Big mkbig(int k) { struct Big b; for (int i = 0; i < 6; i++) b.v[i] = k * 10 + i; return b; }
static int sum(struct P p) { return p.x + p.y; }
static int calls;
static struct P counted(void) { calls++; return mk(calls, 0); }

int main(void) {
    struct P s = mk(1, 2), t = mk(3, 4), u;
    volatile int one = 1;
    print_i64(mk(5, 6).y);
    print_i64((one ? s : t).x);
    print_i64((!one ? s : t).y);
    u = one ? t : s;
    print_i64(u.x * 10 + u.y);
    print_i64((u = s).y);
    u = s = t;
    print_i64(u.x + s.x);
    print_i64(mka(7).a[2]);
    print_i64(mka(1).tag);
    int *p = mka(4).a;
    print_i64(p != 0);
    print_i64(sizeof mka(0).a);
    print_i64(mkbig(3).v[5]);
    print_i64((one ? mkbig(1) : mkbig(2)).v[0]);
    print_i64(sum((one, t)));
    print_i64(sum(one ? mk(10, 20) : s));
    struct Big b = mkbig(9);
    b = one ? b : mkbig(0);
    print_i64(b.v[3]);
    print_i64(counted().x + counted().x);
    print_i64(calls);
    const struct P c = { 8, 9 };
    u = c;
    print_i64(u.y);
    print_i64((one ? c : s).x);
    return 0;
}
/* expect: 6 */
/* expect: 1 */
/* expect: 4 */
/* expect: 34 */
/* expect: 2 */
/* expect: 6 */
/* expect: 9 */
/* expect: 122 */
/* expect: 1 */
/* expect: 16 */
/* expect: 35 */
/* expect: 10 */
/* expect: 7 */
/* expect: 30 */
/* expect: 93 */
/* expect: 3 */
/* expect: 2 */
/* expect: 9 */
/* expect: 8 */
