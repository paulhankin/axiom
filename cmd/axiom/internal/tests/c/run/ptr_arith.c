/* Pointers: arithmetic, comparison, difference, conversions, void *, null. */
#include "host.h"

static int sum(const int *p, int n) {
    int s = 0;
    const int *end = p + n;
    while (p < end) s += *p++;
    return s;
}

int main(void) {
    int a[10];
    int *p = a, *q;
    void *v;
    char *c;
    int i;
    for (i = 0; i < 10; i++) a[i] = i * i;
    print_i64(sum(a, 10));
    q = &a[7];
    print_i64(q - p);
    print_i64(p - q);
    print_i64(*(q - 2));
    print_i64(q[-3]);
    print_i64(2[a]);
    print_i64(*(2 + p));
    print_i64(p < q);
    print_i64(p >= q);
    print_i64(p == &a[0]);
    print_i64(q != p);
    v = q;
    q = v;
    print_i64(*q);
    c = (char *)a;
    print_i64((int *)(c + sizeof(int) * 3) == &a[3]);
    p = 0;
    print_i64(p == 0);
    print_i64(!p);
    print_i64(p ? 1 : 2);
    p = a + 9;
    p -= 4;
    print_i64(*p);
    p += 2;
    print_i64(*p);
    --p;
    print_i64(*p);
    p++;
    print_i64(*p);
    {
        int **pp = &p;
        **pp = 100;
        print_i64(a[7]);
        print_i64(*pp - a);
    }
    {
        long diff = &a[9] - &a[1];
        unsigned u = 3;
        print_i64(diff);
        print_i64(*(a + u));
        print_i64(a[u + 1]);
    }
    return 0;
}

/* expect: 285 */
/* expect: 7 */
/* expect: -7 */
/* expect: 25 */
/* expect: 16 */
/* expect: 4 */
/* expect: 4 */
/* expect: 1 */
/* expect: 0 */
/* expect: 1 */
/* expect: 1 */
/* expect: 49 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 2 */
/* expect: 25 */
/* expect: 49 */
/* expect: 36 */
/* expect: 49 */
/* expect: 100 */
/* expect: 7 */
/* expect: 8 */
/* expect: 9 */
/* expect: 16 */
