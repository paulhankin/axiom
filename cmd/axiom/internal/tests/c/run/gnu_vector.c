/* flags: --c-std=gnu17 */
/* GNU vector types (vector_size): the operators lane by lane, comparisons
   (-1 and 0), a scalar with a vector, subscripts, initializers and
   compound literals, casts between vectors of one size, __builtin_shuffle,
   __builtin_shufflevector, __builtin_convertvector, the conditional
   operator, sizes of 2 to 64 bytes and arguments and results. */
#include "host.h"

typedef int v4si __attribute__((vector_size(16)));
typedef unsigned v4su __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
typedef double v4df __attribute__((vector_size(32)));
typedef long long v8di __attribute__((vector_size(64)));
typedef unsigned char v8qu __attribute__((vector_size(8)));
typedef signed char v2qi __attribute__((vector_size(2)));
typedef short v8hi __attribute__((vector_size(16)));
typedef short v4hi __attribute__((vector_size(8)));
typedef float v8sf __attribute__((vector_size(32)));
typedef long long v2di __attribute__((vector_size(16)));
typedef int v16si __attribute__((vector_size(64)));

static void pv4(v4si v) {
    for (int i = 0; i < 4; i++)
        print_i64(v[i]);
}

__attribute__((noinline)) static v4si madd(v4si a, v4si b, int k) { return a * b + k; }
__attribute__((noinline)) static v4df scale(v4df a, double k) { return a * k - 0.5; }
__attribute__((noinline)) static v8di big(v8di a, v8di b) { return (a ^ b) + (a << 2); }
__attribute__((noinline)) static v2qi tiny(v2qi a) { return -a + 1; }

struct holder { char c; v4si v; };

int main(void) {
    v4si a = {1, -2, 3, -4}, b = {10, 20, 30, 40};
    pv4(madd(a, b, 1));
/* expect: 11 */
/* expect: -39 */
/* expect: 91 */
/* expect: -159 */
    pv4(b / a + b % 7);
/* expect: 13 */
/* expect: -4 */
/* expect: 12 */
/* expect: -5 */
    pv4((a & 6) | (b >> 2) ^ ~a);
/* expect: -4 */
/* expect: 6 */
/* expect: -5 */
/* expect: 13 */
    pv4(a < b);
/* expect: -1 */
/* expect: -1 */
/* expect: -1 */
/* expect: -1 */
    pv4(a == (v4si){1, 2, 3, 4});
/* expect: -1 */
/* expect: 0 */
/* expect: -1 */
/* expect: 0 */
    v4sf f = {1.5f, -2.25f, 3.0f, 0.0f};
    v4si m = f >= 1.5f;
    pv4(m);
/* expect: -1 */
/* expect: 0 */
/* expect: -1 */
/* expect: 0 */
    f = f * 2 + 1;
    print_f64(f[0] + f[1] + f[2] + f[3]);
/* expect: 8.5 */
    v4su u = (v4su)a;
    print_i64(u[1] >> 28);
/* expect: 15 */
    v4si sh = a << (v4si){1, 2, 3, 4};
    pv4(sh);
/* expect: 2 */
/* expect: -8 */
/* expect: 24 */
/* expect: -64 */
    a[2] = 100;
    a[0] += 5;
    pv4(a);
/* expect: 6 */
/* expect: -2 */
/* expect: 100 */
/* expect: -4 */
    v4si c = m ? a : b;
    pv4(c);
/* expect: 6 */
/* expect: 20 */
/* expect: 100 */
/* expect: 40 */
    v4si sel = a > 0 ? a : -a;
    pv4(sel);
/* expect: 6 */
/* expect: 2 */
/* expect: 100 */
/* expect: 4 */
    v4si s1 = __builtin_shuffle(a, (v4si){3, 2, 1, 0});
    pv4(s1);
/* expect: -4 */
/* expect: 100 */
/* expect: -2 */
/* expect: 6 */
    v4si s2 = __builtin_shuffle(a, b, (v4si){0, 4, 1, 7});
    pv4(s2);
/* expect: 6 */
/* expect: 10 */
/* expect: -2 */
/* expect: 40 */
    v8hi s3 = __builtin_shufflevector((v8hi){0, 1, 2, 3, 4, 5, 6, 7}, (v8hi){8, 9, 10, 11, 12, 13, 14, 15}, 15, 0, 14, 1,
                                      -1, 2, 8, 3);
    print_i64(s3[0] * 1000 + s3[1] * 100 + s3[2] * 10 + s3[3]);
/* expect: 15141 */
    print_i64(s3[5] * 100 + s3[6] * 10 + s3[7]);
/* expect: 283 */
    v4sf cv = __builtin_convertvector(a, v4sf);
    print_f64(cv[2] / 8);
/* expect: 12.5 */
    v4si back = __builtin_convertvector((v4sf){1.75f, -2.75f, 1e3f, 0.5f}, v4si);
    pv4(back);
/* expect: 1 */
/* expect: -2 */
/* expect: 1000 */
/* expect: 0 */
    v4df d = scale((v4df){1, 2, 3, 4}, 2.5);
    print_f64(d[0] + d[3]);
/* expect: 11.5 */
    v8di e = big((v8di){1, 2, 3, 4, 5, 6, 7, 8}, (v8di){8, 7, 6, 5, 4, 3, 2, 1});
    print_i64(e[0] + e[7] * 1000);
/* expect: 41013 */
    v2qi t = tiny((v2qi){-128, 5});
    print_i64(t[0] * 100 + t[1]);
/* expect: -12704 */
    v8qu q = {250, 251, 252, 253, 254, 255, 0, 1};
    q += 10;
    print_i64(q[0] + q[5] * 1000 + q[7] * 100000);
/* expect: 1109004 */
    v4hi h4 = (v4hi)(v8qu){1, 2, 3, 4, 5, 6, 7, 8};
    print_i64(h4[0]);
/* expect: 513 */
    v2df pd = (v2df)(v2di){0, 0x3ff0000000000000LL};
    print_f64(pd[1]);
/* expect: 1 */
    v16si w = {0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9};
    w = w * 3 + (v16si){1};
    print_i64(w[0] + w[3] * 10 + w[15] * 100 + w[8] * 1000);
/* expect: 2911 */
    v8sf s8 = {1, 2, 3, 4, 5, 6, 7, 8};
    s8 = s8 / 2 - (v8sf){0.5f};
    print_f64(s8[0] + s8[7]);
/* expect: 4 */
    print_i64(sizeof(v2qi) + sizeof(v8qu) * 10 + sizeof(v4df) * 100 + sizeof(v8di) * 1000);
/* expect: 67282 */
    print_i64(_Alignof(v4si) >= 8);
/* expect: 1 */
    struct holder hd = {1, {5, 6, 7, 8}};
    hd.v *= hd.v;
    print_i64(hd.v[3] + (int)((char *)&hd.v - (char *)&hd));
/* expect: 80 */
    v4si z = {0};
    z++;
    z = -z;
    print_i64(z[0] + z[3]);
/* expect: -2 */
    v4si nz = (v4si){0, 1, 0, 2} == 0;
    pv4(nz);
/* expect: -1 */
/* expect: 0 */
/* expect: -1 */
/* expect: 0 */
    return 0;
}
