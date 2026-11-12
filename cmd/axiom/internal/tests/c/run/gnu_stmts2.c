/* flags: --c-std=gnu17 */
/* More of the GNU dialects: attributes in odd places, cleanups with a
   variable length array in scope and in a statement expression, computed
   goto in a function that is inlined, label differences, a switch over
   case ranges of a char, enumerators and labels with attributes. */
#include "host.h"

static int seen;
static void clean(int *p) { seen = seen * 10 + *p; }

static int step(int k) {
    static const int off[] = { &&zero - &&zero, &&one - &&zero, &&two - &&zero };
    goto *(&&zero + off[k]);
zero:
    return 10;
one:
    return 11;
two:
    return 12;
}

enum colour { RED __attribute__((deprecated)) = 1, GREEN [[gnu::unused]] = 2, BLUE };

int main(void) {
    int n = 3;
    {
        int a __attribute__((cleanup(clean))) = 1;
        int v[n];
        v[0] = 5;
        int b __attribute__((cleanup(clean))) = 2;
        (void)a;
        (void)b;
        n = v[0];
    }
    print_i64(seen * 10 + n);
    seen = 0;
    int r = ({ int c __attribute__((cleanup(clean))) = 7; c + 1; });
    print_i64(r * 100 + seen);
    print_i64(step(0) + step(1) * 100 + step(2) * 10000);
    int * __attribute__((unused)) p, (__attribute__((unused)) *q);
    p = q = &n;
    struct { int x : 4 __attribute__((packed)); } bf = { 3 };
    print_i64(*p + *q + bf.x + BLUE + GREEN);
    __attribute__((aligned)) char big = 1;
    [[gnu::aligned(32)]] int a32 = 2;
    print_i64((((size_t)&big % 16) == 0) + (((size_t)&a32 % 32) == 0) * 10 + big + a32);
    goto done;
done: __attribute__((unused, cold));
    return 0;
}
/* expect: 215 */
/* expect: 807 */
/* expect: 121110 */
/* expect: 18 */
/* expect: 14 */
