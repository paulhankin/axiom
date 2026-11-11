/* Structures and unions by value through function pointers, with arrays
   and mixed members, nested calls, and large ones (passed by address). */
#include "host.h"

struct s3 { char c[3]; };
struct mix { char c; short s; double d; };
union both { long long ll; double d; };
struct huge { int v[20]; };

static struct s3 rev(struct s3 x) {
    struct s3 r;
    r.c[0] = x.c[2];
    r.c[1] = x.c[1];
    r.c[2] = x.c[0];
    return r;
}

static struct mix scale(struct mix m, int k) {
    m.c *= k;
    m.s *= k;
    m.d *= k;
    return m;
}

static union both as_bits(double d) {
    union both b;
    b.d = d;
    return b;
}

static struct huge inc_all(struct huge h) {
    int i;
    for (i = 0; i < 20; i++) h.v[i]++;
    return h;
}

static int sum(struct huge h) {
    int i, s = 0;
    for (i = 0; i < 20; i++) s += h.v[i];
    return s;
}

int main(void) {
    struct s3 (*fr)(struct s3) = rev;
    struct huge (*fh)(struct huge) = inc_all;
    struct s3 a = { { 'a', 'b', 'c' } };
    struct mix m = { 2, 300, 0.5 };
    struct huge h;
    int i;
    struct s3 b = fr(a);
    print_i64(b.c[0] * 10000 + b.c[1] * 100 + b.c[2]);
    m = scale(scale(m, 2), 3);
    print_i64(m.c + m.s);
    print_f64(m.d);
    print_i64(as_bits(1.0).ll == 0x3FF0000000000000LL);
    for (i = 0; i < 20; i++) h.v[i] = i;
    print_i64(sum(h));
    print_i64(sum(fh(fh(h))));
    print_i64(sum(h));
    print_i64(fr(fr(a)).c[0]);
    return 0;
}

/* expect: 999897 */
/* expect: 1812 */
/* expect: 3 */
/* expect: 1 */
/* expect: 190 */
/* expect: 230 */
/* expect: 190 */
/* expect: 97 */
