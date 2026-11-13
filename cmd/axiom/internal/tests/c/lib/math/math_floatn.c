/* The functions of _Float16 and _Float128 (ISO/IEC TS 18661-3, C23 Annex
   H; std/c/lib/math/m_f16.c, m_f128.c): binary128's correctly rounded
   exact functions, its elementary functions in its precision, strtof128
   and strfromf128 (whatever long double is), binary16's functions through
   float, strtof16 and strfromf16 with its subnormals and overflow, and the
   _Float32 and _Float64 names of float's and double's functions. */
/* flags: --c-std=c17 */
#define __STDC_WANT_IEC_60559_TYPES_EXT__ 1
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
static void p128(const char *what, _Float128 x) { char b[80]; strfromf128(b, sizeof b, "%.33g", x); printf("%s %s\n", what, b); }
static void p16(const char *what, _Float16 x) { char b[40]; strfromf16(b, sizeof b, "%.6g", x); printf("%s %s\n", what, b); }
int main(void) {
  char *e;
  _Float128 q = strtof128("1.000000000000000000000000000000001", &e);
  p128("parse", q);
  p128("sqrt2", sqrtf128(2));
  p128("exp1", expf128(1));
  p128("sin1", sinf128(1));
  p128("pow", powf128(3, 0.5F128));
  p128("floor", floorf128(-2.5F128));
  p128("fma", fmaf128(1.0F128/3, 3, -1));
  p128("max", FLT128_MAX);
  p128("nextup0", nextupf128(0));
  char b[64]; strfromf128(b, sizeof b, "%a", (_Float128)1/3); printf("%s\n", b);
  strfromf128(b, 5, "%f", (_Float128)-123.5); printf("[%s] %d\n", b, strfromf128(b, 5, "%f", (_Float128)-123.5));
  _Float16 h = strtof16("0.1", &e);
  p16("h", h); p16("sqrth", sqrtf16(h)); p16("fmah", fmaf16(h, h, h)); p16("nexth", nextafterf16(1, 2)); p16("huge", strtof16("1e9", 0));
  p16("sub", strtof16("6e-8", 0));
  strfromf16(b, sizeof b, "%a", h); printf("%s\n", b);
  printf("%g %g %d\n", (double)sinf32(1), (double)cosf64(1), (int)lroundf128(2.5F128));
  return 0;
}
/* expect: parse 1 */
/* expect: sqrt2 1.4142135623730950488016887242097 */
/* expect: exp1 2.71828182845904523536028747135266 */
/* expect: sin1 0.841470984807896506652502321630299 */
/* expect: pow 1.73205080756887729352744634150587 */
/* expect: floor -3 */
/* expect: fma -4.81482486096808963263994485646232e-35 */
/* expect: max 1.18973149535723176508575932662801e+4932 */
/* expect: nextup0 6.47517511943802511092443895822765e-4966 */
/* expect: 0x1.5555555555555555555555555555p-2 */
/* expect: [-123] 11 */
/* expect: h 0.0999756 */
/* expect: sqrth 0.316162 */
/* expect: fmah 0.109985 */
/* expect: nexth 1.00098 */
/* expect: huge inf */
/* expect: sub 5.96046e-08 */
/* expect: 0x1.998p-4 */
/* expect: 0.841471 0.540302 3 */
