/* flags: --c-std=c11 */
/* Comparisons of long double with zero, which the compiler makes inline
   from the bits where long double is wider than double (cgen_ld.ax:
   sign_of): zeros of both signs, subnormals, infinities, a NaN, and in the
   x87 format the encodings it does not have (unnormals, pseudo-infinities,
   pseudo-NaNs: unordered; a pseudo-denormal: its value), as the library's
   comparison gives them. The printed value is what that comparison gives,
   per target format. */
#include "host.h"
union u { long double d; unsigned char b[16]; unsigned long long q[2]; };
static int classify(long double x) { return (x < 0) * 1 + (x > 0) * 2 + (x == 0) * 4 + (x != 0) * 8 + (x <= 0) * 16 + (x >= 0) * 32 + (0 < x) * 64 + (0 >= x) * 128; }
int main(void) {
    long double v[] = { 0.0L, -0.0L, 1.0L, -1.0L, 1e-4950L, -1e-4950L, 1.0L / 0.0L, -1.0L / 0.0L, 0.0L / 0.0L, 3.5L, -2e300L };
    long long h = 0;
    for (unsigned i = 0; i < sizeof v / sizeof v[0]; i++) h = h * 257 + classify(v[i]);
#if __AXIOM_LDBL_WIDTH__ == 80
    /* encodings the x87 format does not have: an unnormal, a pseudo-infinity, a pseudo-NaN, a pseudo-denormal */
    { union u x; unsigned long long enc[4][2] = { { 0x4000000000000000ULL, 0x3fff }, { 0, 0x7fff }, { 1, 0xffff }, { 0x8000000000000000ULL, 0x8000 } };
      for (int k = 0; k < 4; k++) { x.q[0] = enc[k][0]; x.q[1] = enc[k][1]; h = h * 257 + classify(x.d); } }
#endif
    print_i64(h == (__AXIOM_LDBL_WIDTH__ == 80 ? 2665218569030625069LL : __AXIOM_LDBL_WIDTH__ == 128 ? 8726501153589053052LL : h));
    return 0;
}
/* expect: 1 */
