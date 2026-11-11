/* flags: --c-std=c99 */
/* Compound literals (C99 6.5.2.5): at file scope (static, their addresses
   constants), in blocks (automatic, initialized each time they are
   evaluated), as lvalues, arrays of deduced length, const, structures
   returned and passed, with designators. */
#include "host.h"
struct P { int x, y; };
static int *gp = (int[]){ 4, 5, 6 };
static struct P *gq = &(struct P){ .y = 8 };
static const char *gs = (const char[]){ "hey" };
static int sum(const int *p, int n) { int s = 0; while (n--) s += *p++; return s; }
static struct P mk(int a) { return (struct P){ a, a * 2 }; }
int main(void) {
    int i, tot = 0;
    print_i64(gp[0] + gp[2]); print_i64(gq->x + gq->y); print_i64(gs[1]);
    print_i64(sum((int[]){ 1, 2, 3, 4 }, 4));
    print_i64(sizeof((int[]){ 1, 2, 3 }));
    int *p = (int[3]){ [2] = 7 };
    print_i64(p[0] + p[2]);
    struct P q = (struct P){ 3, 4 };
    print_i64(q.x * q.y);
    (struct P){ 1, 2 }.x = 5;
    print_i64(((struct P){ 9, 10 }).y);
    print_i64(mk(4).y);
    for (i = 0; i < 3; i++) {
        int *r = (int[2]){ i, i * 10 };
        tot += r[0] + r[1];
        r[0] = 100;
    }
    print_i64(tot);
    struct P *pp = &(struct P){ 1, 1 };
    pp->x = 42; print_i64(pp->x);
    const int *c = (const int[]){ 11, 12 };
    print_i64(c[1]);
    char *s = (char[]){ "abc" };
    s[0] = 'z'; print_i64(s[0]); print_i64(sizeof((char[]){ "abc" }));
    print_i64((int){ 77 });
    int v = 3;
    print_i64((int){ v } + 1);
    int *e = &(int){ 0 }; *e += 6; print_i64(*e);
    return 0;
}
/* expect: 10 */
/* expect: 8 */
/* expect: 101 */
/* expect: 10 */
/* expect: 12 */
/* expect: 7 */
/* expect: 12 */
/* expect: 10 */
/* expect: 8 */
/* expect: 33 */
/* expect: 42 */
/* expect: 12 */
/* expect: 122 */
/* expect: 4 */
/* expect: 77 */
/* expect: 4 */
/* expect: 6 */
