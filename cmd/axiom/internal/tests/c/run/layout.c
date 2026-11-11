/* Layout: the size, alignment and member offsets of structures and unions
   (each member at its alignment, the largest alignment the structure's,
   tail padding), nested and in arrays. */
#include "host.h"

#define OFF(s, m) ((int)((char *)&(s).m - (char *)&(s)))

struct a { char c; int i; };
struct b { char c; long long ll; char d; };
struct c { short s; char c[3]; };
struct d { char c; struct c in; double x; };
struct e { char c; union { char c; long long ll; } u; char t; };
struct f { int n; double tail[]; };
struct g { struct a arr[3]; char z; };
union h { char c[5]; int i; };
struct i { float f; char c; };
struct j { char a, b, c; };

int main(void) {
    struct a a;
    struct b b;
    struct c c;
    struct d d;
    struct e e;
    struct g g;
    print_i64(sizeof(struct a) * 100 + _Alignof(struct a));
    print_i64(OFF(a, i));
    print_i64(sizeof(struct b) * 100 + _Alignof(struct b));
    print_i64(OFF(b, ll) * 100 + OFF(b, d));
    print_i64(sizeof(struct c) * 100 + _Alignof(struct c));
    print_i64(OFF(c, c));
    print_i64(sizeof(struct d) * 100 + _Alignof(struct d));
    print_i64(OFF(d, in) * 100 + OFF(d, x));
    print_i64(sizeof(struct e) * 100 + _Alignof(struct e));
    print_i64(OFF(e, u) * 100 + OFF(e, t));
    print_i64(sizeof(struct f) * 100 + _Alignof(struct f));
    print_i64(sizeof(struct g) * 100 + _Alignof(struct g));
    print_i64(OFF(g, arr[2].i) * 100 + OFF(g, z));
    print_i64(sizeof(union h) * 100 + _Alignof(union h));
    print_i64(sizeof(struct i) * 100 + _Alignof(struct i));
    print_i64(sizeof(struct j) * 100 + _Alignof(struct j));
    print_i64(sizeof(struct j[3]));
    return 0;
}

/* expect: 804 */
/* expect: 4 */
/* expect: 2408 */
/* expect: 816 */
/* expect: 602 */
/* expect: 2 */
/* expect: 1608 */
/* expect: 208 */
/* expect: 2408 */
/* expect: 816 */
/* expect: 808 */
/* expect: 2804 */
/* expect: 2024 */
/* expect: 804 */
/* expect: 804 */
/* expect: 301 */
/* expect: 9 */
