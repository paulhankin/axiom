/* Pointer differences divide by the element's size exactly (divconst.ax
   exact_sdiv: a shift and a multiplication by the inverse of the odd part):
   sizes odd, even and powers of two, differences negative, zero and across
   the whole array, and an array of variable length (a division). */
#include "host.h"

struct S12 { int a, b, c; };
struct S24 { long long a; int b[4]; };
struct S40 { char c[40]; };
struct S3 { char c[3]; };
struct S7 { char c[7]; };

static struct S12 a12[50];
static struct S24 a24[50];
static struct S40 a40[50];
static struct S3 a3[50];
static struct S7 a7[50];
static long long a8[50];

static volatile int vi = 37, vj = 5;

static long long sum(int n) {
    char vla[n][5];
    long long s = 0;
    int i, j;
    for (i = 0; i < 50; i++) {
        for (j = 0; j < 50; j += 7) {
            s = s * 3 + (&a12[i] - &a12[j]);
            s = s * 3 + (&a24[i] - &a24[j]);
            s = s * 3 + (&a40[i] - &a40[j]);
            s = s * 3 + (&a3[i] - &a3[j]);
            s = s * 3 + (&a7[i] - &a7[j]);
            s = s * 3 + (&a8[i] - &a8[j]);
            if (i < n && j < n) s = s * 3 + (&vla[i] - &vla[j]);
            s &= 0xFFFFFFFFFFFFLL;
        }
    }
    return s;
}

int main(void) {
    struct S12 *p = &a12[vi], *q = &a12[vj];
    print_i64(p - q);
    print_i64(q - p);
    print_i64(&a7[49] - &a7[0]);
    print_i64(&a3[0] - &a3[49]);
    print_i64(&a40[vi] - &a40[vi]);
    print_i64(sum(vi));
    return 0;
}
/* expect: 32 */
/* expect: -32 */
/* expect: 49 */
/* expect: -49 */
/* expect: 0 */
/* expect: 210740802183703 */
