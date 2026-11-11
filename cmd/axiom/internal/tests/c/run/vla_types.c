/* Variable length array types: size expressions evaluated once each, sizeof of
   a VLA expression evaluating its operand (and of another not), three
   dimensions, parameters with static and qualifiers in their brackets, [*]
   in a prototype, pointer arithmetic on pointers to VLAs (+, -, ++, --, +=,
   -=, differences), arrays of structures, recursion, typedefs of VLA types
   (and of arrays of them), sizes of other integer types, a conditional
   between pointers to VLAs, _Alignof. */
#include "host.h"
static int calls;
static int next(int x) { calls = calls * 10 + x; return x; }
struct P { double x, y; char tag; };
static long long sum3(int a, int b, int c, int v[a][b][c]) {
    long long s = 0;
    for (int i = 0; i < a; i++)
        for (int j = 0; j < b; j++)
            for (int k = 0; k < c; k++) s += v[i][j][k] * (i + 2 * j + 3 * k + 1);
    return s;
}
static int qual(int n, int a[const static n], int b[restrict n]) { return a[n - 1] + b[0]; }
static int star_proto(int n, int (*p)[*]);
static int star_proto(int n, int (*p)[n]) { return p[1][n - 1]; }
static int rec(int depth) {
    char buf[depth + 1];
    buf[depth] = (char)depth;
    if (depth == 0) return 0;
    return buf[depth] + rec(depth - 1);
}
int main(void) {
    int a[next(2)][next(3)];
    print_i64(calls / 10 + calls % 10 + (calls > 9) * 100);
    print_i64(sizeof a / sizeof a[0][0]);
    int i = 1;
    print_i64(sizeof(i++));
    print_i64(i);
    int (*x)[i + 2] = a;
    print_i64(sizeof x[i++]);
    print_i64(i);
    print_i64(sizeof(x[0]) + sizeof *x);
    int v[2][3][4];
    for (int p = 0; p < 2; p++) for (int q = 0; q < 3; q++) for (int r = 0; r < 4; r++) v[p][q][r] = p * 100 + q * 10 + r;
    int n2 = 2, n3 = 3, n4 = 4;
    print_i64(sum3(n2, n3, n4, v));
    int dyn[n2][n3][n4];
    for (int p = 0; p < n2; p++) for (int q = 0; q < n3; q++) for (int r = 0; r < n4; r++) dyn[p][q][r] = v[p][q][r];
    print_i64(sum3(n2, n3, n4, dyn));
    print_i64(sizeof dyn + sizeof dyn[1] + sizeof dyn[1][2]);
    int (*pp)[n3][n4] = dyn;
    pp += 1;
    print_i64(pp[0][2][3]);
    print_i64(pp - dyn);
    pp--;
    print_i64((*pp)[1][1]);
    int (*row)[n4] = dyn[1];
    row++;
    row += 1;
    print_i64(**row);
    row -= 2;
    print_i64(row[1][0] + (&dyn[1][2] - row));
    struct P ps[n2 + 1];
    ps[2].x = 1.5;
    ps[2].tag = 'z';
    print_i64(sizeof ps);
    print_i64((long long)(ps[2].x * 2) + ps[2].tag);
    int w[5] = {1, 2, 3, 4, 5};
    print_i64(qual(5, w, w));
    int m2[3][n4];
    m2[1][3] = 77;
    print_i64(star_proto(n4, m2));
    print_i64(rec(50));
    int n = 4;
    typedef int T[n];
    n++;
    T t1;
    print_i64(sizeof(T) + sizeof t1 + n);
    {
        typedef T U[n];
        n = 1;
        U u;
        print_i64(sizeof u);
    }
    unsigned char uc = 200;
    char big[uc];
    print_i64(sizeof big);
    long long ll = 3;
    double d[ll];
    print_i64(sizeof d);
    enum { E = 7 } e = E;
    short sh[e];
    print_i64(sizeof sh);
    int c = 1;
    int (*sel)[n4] = c ? (int (*)[n4])m2 : dyn[0];
    print_i64(sel[1][3]);
    print_i64(sizeof(int (*)[n4]) == sizeof(int *));
    print_i64(_Alignof(int[n4]));
    print_i64(sizeof(double[n2][n3]));
    return 0;
}

/* expect: 105 */
/* expect: 6 */
/* expect: 4 */
/* expect: 1 */
/* expect: 12 */
/* expect: 2 */
/* expect: 24 */
/* expect: 12818 */
/* expect: 12818 */
/* expect: 160 */
/* expect: 123 */
/* expect: 1 */
/* expect: 11 */
/* expect: 120 */
/* expect: 112 */
/* expect: 72 */
/* expect: 125 */
/* expect: 6 */
/* expect: 77 */
/* expect: 1275 */
/* expect: 37 */
/* expect: 80 */
/* expect: 200 */
/* expect: 24 */
/* expect: 14 */
/* expect: 77 */
/* expect: 1 */
/* expect: 4 */
/* expect: 48 */
