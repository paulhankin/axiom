/* Vectors (a GNU dialect) under FENV_ACCESS ON: their arithmetic rounds in
   the dynamic mode and raises its exceptions, lane by lane (float, double
   and _Float16 lanes), and their relational operators signal "invalid" for
   a NaN lane while == and != do not (F.9.3); at -O0 and -O2 alike (the
   optimizer neither folds nor drops them). */
/* flags: --c-std=gnu23 */
/* native-only */
#include "print.h"
#include <fenv.h>
#include <math.h>
#include <string.h>

#pragma STDC FENV_ACCESS ON

typedef float v4 __attribute__((vector_size(16)));
typedef double v2 __attribute__((vector_size(16)));
typedef _Float16 v8h __attribute__((vector_size(16)));

static int inv(void)
{
    int r = fetestexcept(FE_INVALID) != 0;
    feclearexcept(FE_ALL_EXCEPT);
    return r;
}

int main(void)
{
    volatile float qn = NAN, one = 1, three = 3, big = 3e38f;
    volatile _Float16 hone = 1, hthree = 3, hbig = 60000;
    v4 a = {1, qn, 3, 4}, b = {2, 2, 2, 2};
    v2 c = {1.0, (double)qn}, d = {2.0, 2.0};
    feclearexcept(FE_ALL_EXCEPT);
    __typeof__(a < b) r1 = a < b;
    int lt = inv();
    __typeof__(a == b) r2 = a == b;
    int eq = inv();
    __typeof__(c >= d) r3 = c >= d;
    int ge = inv();
    __typeof__(c != d) r4 = c != d;
    int ne = inv();
    print_bool(lt && !eq && ge && !ne);
    print_bool(r1[0] == -1 && r1[1] == 0 && r2[1] == 0 && r3[1] == 0 && r4[1] == -1);
    v4 x = {one, one, one, one}, y = {three, three, three, three};
    fesetround(FE_UPWARD);
    v4 u = x / y;
    fesetround(FE_DOWNWARD);
    v4 dn = x / y;
    fesetround(FE_TONEAREST);
    print_bool(u[0] > dn[0] && u[3] > dn[3]);
    feclearexcept(FE_ALL_EXCEPT);
    v4 g = {big, big, 1, 1};
    v4 o = g + g;
    print_bool(fetestexcept(FE_OVERFLOW) && fetestexcept(FE_INEXACT) && isinf(o[0]));
    feclearexcept(FE_ALL_EXCEPT);
    v4 e = x + x;
    print_bool(!fetestexcept(FE_INEXACT) && e[0] == 2);
    v8h hx = {hone, hone, hone, hone, hone, hone, hone, hone}, hy = {hthree, hthree, hthree, hthree, hthree, hthree, hthree, hthree};
    fesetround(FE_UPWARD);
    v8h hu = hx / hy;
    fesetround(FE_DOWNWARD);
    v8h hd = hx / hy;
    fesetround(FE_TONEAREST);
    unsigned short ub, db;
    memcpy(&ub, &hu, 2);
    memcpy(&db, &hd, 2);
    print_bool(ub == 0x3556 && db == 0x3555);
    v8h hg = {hbig, hbig, hbig, hbig, hbig, hbig, hbig, hbig};
    feclearexcept(FE_ALL_EXCEPT);
    v8h ho = hg + hg;
    print_bool(fetestexcept(FE_OVERFLOW) && fetestexcept(FE_INEXACT) && isinf((double)ho[0]));
    return 0;
}
// expect: true
// expect: true
// expect: true
// expect: true
// expect: true
// expect: true
// expect: true
