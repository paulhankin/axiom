/* Arrays: multidimensional, of structures, sizeof, decay, initializers. */
#include "host.h"

struct pt { int x, y; };

static int m[3][4] = { { 1, 2, 3, 4 }, { 5, 6, 7, 8 }, { 9, 10, 11, 12 } };
static struct pt pts[3] = { { 1, 2 }, { 3, 4 }, { 5, 6 } };
static int e[] = { 1, 2, 3, 4, 5 };
static int z[5];
static char s[] = "hello";
static char s2[8] = "abc";
static int flat[2][3] = { 1, 2, 3, 4, 5 };

static int trace(int (*a)[4], int n) {
    int t = 0, i;
    for (i = 0; i < n; i++) t += a[i][i];
    return t;
}

int main(void) {
    int i, j;
    int local[2][2] = { { 1, 2 }, { 3 } };
    struct pt lp[2] = { { 7, 8 }, { 9 } };
    char ls[] = "xyz";
    int sum = 0;
    for (i = 0; i < 3; i++)
        for (j = 0; j < 4; j++) sum += m[i][j];
    print_i64(sum);
    print_i64(trace(m, 3));
    print_i64(sizeof m);
    print_i64(sizeof m[0]);
    print_i64(sizeof e / sizeof e[0]);
    print_i64(sizeof s);
    print_i64(sizeof s2);
    print_i64(s2[3]);
    print_i64(s[4]);
    print_str(s, 5);
    print_i64(pts[2].y + pts[0].x);
    print_i64(z[4]);
    print_i64(flat[1][1] + flat[1][2]);
    print_i64(local[1][0] + local[1][1]);
    print_i64(lp[1].x + lp[1].y);
    print_i64(sizeof ls);
    print_i64(ls[2]);
    print_i64(*m[1]);
    print_i64(**m);
    print_i64(*(*(m + 2) + 1));
    {
        int (*row)[4] = m + 1;
        print_i64((*row)[3]);
        print_i64(row[1][0]);
    }
    {
        struct pt *pp = pts;
        pp++;
        print_i64(pp->x);
        print_i64((pp + 1)->y);
    }
    return 0;
}

/* expect: 78 */
/* expect: 18 */
/* expect: 48 */
/* expect: 16 */
/* expect: 5 */
/* expect: 6 */
/* expect: 8 */
/* expect: 0 */
/* expect: 111 */
/* expect: hello */
/* expect: 7 */
/* expect: 0 */
/* expect: 5 */
/* expect: 3 */
/* expect: 9 */
/* expect: 4 */
/* expect: 122 */
/* expect: 5 */
/* expect: 1 */
/* expect: 10 */
/* expect: 8 */
/* expect: 9 */
/* expect: 3 */
/* expect: 6 */
