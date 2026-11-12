/* flags: --c-std=gnu11 */
/* The GNU dialects' initializers and types: designator ranges, old-style
   member designators, flexible array members initialized in static objects,
   zero-length arrays, structures without members, compound literals of
   constants initializing static objects, arithmetic on void and function
   pointers, sizeof of void and of a function. */
#include "host.h"

struct flex { int n; int d[]; };
static struct flex f3 = { 3, { 10, 20, 30 } };
static struct flex f0 = { 0 };
struct named { char *s; char text[]; };
static struct named nm = { 0, "hello" };
struct pt { int x, y; };
static struct pt pts[5] = { [0 ... 2] = { 1, 2 }, [3].x = 7, [4] = { y: 9 } };
static int grid[3][4] = { [0 ... 2][1 ... 2] = 5 };
static struct pt from_lit = (struct pt){ 4, 6 };
static int arr_lit = (int){ 42 };
struct empty {};
struct zl { int len; char data[0]; };

static int one(void) { return 1; }

int main(void) {
    print_i64(sizeof(struct flex) + f3.d[0] + f3.d[2] + f3.n + f0.n);
    print_str(nm.text, 5);
    int s = 0;
    for (int i = 0; i < 5; i++)
        s = s * 10 + pts[i].x;
    print_i64(s);
    print_i64(pts[4].y + pts[1].y);
    int g = 0;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 4; j++)
            g += grid[i][j];
    print_i64(g);
    print_i64(from_lit.x * 10 + from_lit.y + arr_lit);
    struct empty e;
    print_i64(sizeof(struct empty) + sizeof e);
    print_i64(sizeof(struct zl));
    char buf[sizeof(struct zl) + 4];
    struct zl *z = (struct zl *)buf;
    z->len = 4;
    z->data[3] = 'x';
    print_i64(buf[sizeof(int) + 3]);
    int local[6] = { [1 ... 4] = 3 };
    print_i64(local[0] + local[1] + local[4] + local[5]);
    struct pt q = { y: 1, x: 2 };
    print_i64(q.x * 10 + q.y);
    void *p = buf;
    p += 2;
    print_i64((char *)p - buf);
    int (*fp)(void) = one;
    print_i64(sizeof(void) + sizeof(one) + sizeof(*fp));
    print_i64((char *)(fp + 1) - (char *)fp);
    return 0;
}
/* expect: 47 */
/* expect: hello */
/* expect: 11170 */
/* expect: 11 */
/* expect: 30 */
/* expect: 88 */
/* expect: 0 */
/* expect: 4 */
/* expect: 120 */
/* expect: 6 */
/* expect: 21 */
/* expect: 2 */
/* expect: 3 */
/* expect: 1 */
