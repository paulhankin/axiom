/* Initializers: brace elision, zero fill, nested aggregates, unions (their
   first member), character arrays from strings, scalars in braces,
   automatic objects from run-time values, incomplete arrays. */
#include "host.h"

struct in { int a[2]; char c; };
struct out { struct in i; int tail[2]; };
union u { short s; char c[4]; };

static struct out so = { 1, 2, 'x', 3 };
static struct out so2 = { { { 4 }, 'y' }, { 5, 6 } };
static int grid[2][3] = { { 1 }, { 4, 5 } };
static int elided[][2] = { 1, 2, 3, 4, 5 };
static char words[][6] = { "one", "three", { 'h', 'i' } };
static union u un = { 0x0102 };
static int scalar = { 42 };
static struct in ins[] = { 1, 2, 'a', { 3, 4 }, 'b' };

static int f(int x) { return x * 3; }

int main(void) {
    int k = 2;
    int a[4] = { f(1), f(k), k };
    struct in li = { { k, k + 1 }, 'z' };
    struct out lo = { li, { 8 } };
    char s[10] = "hi";
    char t[3] = "abc";
    struct in copy = li;
    union u lu = { 7 };
    int sc = { f(2) };
    print_i64(so.i.a[0] + so.i.a[1] * 10 + so.i.c * 100 + so.tail[0] * 100000 + so.tail[1]);
    print_i64(so2.i.a[0] + so2.i.a[1] + so2.i.c + so2.tail[0] * so2.tail[1]);
    print_i64(grid[0][0] + grid[0][1] + grid[0][2] + grid[1][0] * grid[1][1] + grid[1][2]);
    print_i64(sizeof elided / sizeof elided[0]);
    print_i64(elided[2][0] + elided[2][1]);
    print_i64(sizeof words);
    print_str(words[1], 5);
    print_i64(words[2][1] + words[2][2]);
    print_i64(un.s);
    print_i64(scalar);
    print_i64(sizeof ins / sizeof ins[0]);
    print_i64(ins[1].a[1] + ins[1].c);
    print_i64(a[0] + a[1] + a[2] + a[3]);
    print_i64(li.a[1] + li.c);
    print_i64(lo.i.a[0] + lo.tail[0] + lo.tail[1]);
    print_i64(s[1] + s[2] + s[9]);
    print_i64(t[2]);
    print_i64(copy.c);
    print_i64(lu.s);
    print_i64(sc);
    {
        int n[5] = { 0 };
        int m[] = { 1, 2, 3, 4, 5, 6, 7 };
        int i, z = 0;
        for (i = 0; i < 5; i++) z += n[i];
        print_i64(z);
        print_i64(sizeof m);
    }
    return 0;
}

/* expect: 312021 */
/* expect: 155 */
/* expect: 21 */
/* expect: 3 */
/* expect: 5 */
/* expect: 18 */
/* expect: three */
/* expect: 105 */
/* expect: 258 */
/* expect: 42 */
/* expect: 3 */
/* expect: 4 */
/* expect: 11 */
/* expect: 125 */
/* expect: 10 */
/* expect: 105 */
/* expect: 99 */
/* expect: 122 */
/* expect: 7 */
/* expect: 6 */
/* expect: 0 */
/* expect: 28 */
