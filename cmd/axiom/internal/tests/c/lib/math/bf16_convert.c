/* Conversions to __bf16 (a GNU dialect: bfloat16) rounded once, in every
   binary rounding direction, with their exceptions: for each finite
   positive bfloat16 value b and its successor (or the infinity), values of
   float, double, long double and (where it holds them exactly) _Decimal128
   at b, at their midpoint and on each side of it, both signs. The expected
   result is found by comparisons in double, which holds every bfloat16
   value and midpoint exactly (tininess detected after rounding); arithmetic on __bf16 (in float, rounded) is
   checked against the same rounding of the float result. The program
   prints the number of mismatches. */
/* flags: --c-std=gnu23 */
/* native-only */
#include <fenv.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#pragma STDC FENV_ACCESS ON

static const int modes[4] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};

static double value(unsigned u)
{
    /* bfloat16 is float's high half */
    unsigned w = u << 16;
    float f;
    memcpy(&f, &w, 4);
    return f;
}

static unsigned bits(__bf16 h)
{
    unsigned short s;
    memcpy(&s, &h, 2);
    return s;
}

static int bad, checked;

/* The bits of the conversion of x (lo < |x| < hi, or x == lo) in direction
   m, and whether it is inexact. */
static unsigned want(double x, unsigned i, int m, int *inexact)
{
    double ax = fabs(x);
    unsigned sign = signbit(x) ? 0x8000u : 0u;
    double lo = value(i), hi = i + 1 == 0x7F80 ? 0x1p128 : value(i + 1), mid = (lo + hi) / 2;
    *inexact = ax != lo;
    if (ax == lo)
        return sign | i;
    int up;
    if (m == 0)
        up = ax > mid || (ax == mid && (i & 1));
    else if (m == 3)
        up = 0;
    else
        up = (m == 2) != (sign != 0);
    return sign | (up ? i + 1 : i);
}

/* Is x tiny after rounding (IEEE 754-2019 7.5): below 2^-126 once rounded
   to 8 bits with an unbounded exponent in direction m? */
static int tiny(double x, int m)
{
    double ax = fabs(x);
    if (ax >= 0x1p-126)
        return 0;
    if (ax <= 0x1.fep-127)
        return 1;
    /* between the largest 8-bit value below 2^-126 and 2^-126 */
    if (m == 0)
        return ax < 0x1.ffp-127;  /* (the tie goes to 2^-126, the even one) */
    if (m == 3)
        return 1;
    return (m == 2) == (signbit(x) != 0);
}

static void check(const char *what, double x, unsigned i, int m, unsigned got, int flags)
{
    int inexact;
    unsigned w = want(x, i, m, &inexact);
    int wflags = inexact ? FE_INEXACT : 0;
    if ((w & 0x7FFF) == 0x7F80)
        wflags |= FE_OVERFLOW;
    if (inexact && tiny(x, m))
        wflags |= FE_UNDERFLOW;
    checked++;
    if (got != w || flags != wflags) {
        if (bad < 10)
            printf("%s: %a in mode %d: %#x (flags %#x), not %#x (%#x)\n", what, x, m, got, flags, w, wflags);
        bad++;
    }
}

int main(void)
{
    for (unsigned i = 0; i < 0x7F80; i++) {
        double lo = value(i), hi = i + 1 == 0x7F80 ? 0x1p128 : value(i + 1);  /* (past the largest: 2^128) */
        double mid = (lo + hi) / 2;
        double xs[5] = {lo, mid, mid - (hi - lo) / 1024, mid + (hi - lo) / 1024, mid + (hi - lo) / 0x1p30};
        for (int s = 0; s < 2; s++) {
            for (int k = 0; k < 5; k++) {
                double x = s ? -xs[k] : xs[k];
                for (int m = 0; m < 4; m++) {
                    fesetround(modes[m]);
                    int f;
                    if (k < 4 && fabs(x) <= FLT_MAX) {
                        volatile float vf = (float)x;  /* exact: mid has 9 bits */
                        feclearexcept(FE_ALL_EXCEPT);
                        volatile __bf16 r = vf;
                        f = fetestexcept(FE_ALL_EXCEPT);
                        check("float", x, i, m, bits(r), f);
                    }
                    volatile double vd = x;
                    feclearexcept(FE_ALL_EXCEPT);
                    volatile __bf16 r2 = vd;
                    f = fetestexcept(FE_ALL_EXCEPT);
                    check("double", x, i, m, bits(r2), f);
                    volatile long double vl = x;
                    feclearexcept(FE_ALL_EXCEPT);
                    volatile __bf16 r3 = vl;
                    f = fetestexcept(FE_ALL_EXCEPT);
                    check("long double", x, i, m, bits(r3), f);
                    if (lo >= 0x1p-30 && hi <= 0x1p30) {
                        volatile _Decimal128 vq = (_Decimal128)x;  /* exact here */
                        feclearexcept(FE_ALL_EXCEPT);
                        volatile __bf16 r4 = vq;
                        f = fetestexcept(FE_ALL_EXCEPT);
                        check("_Decimal128", x, i, m, bits(r4), f);
                    }
                    fesetround(FE_TONEAREST);
                }
            }
        }
        /* back: exact */
        volatile __bf16 b;
        unsigned short u = (unsigned short)i;
        memcpy((void *)&b, &u, 2);
        checked++;
        if ((double)b != lo || (float)b != (float)lo) {
            if (bad < 10)
                printf("%#x to double: %a\n", i, (double)b);
            bad++;
        }
        /* arithmetic: the float operation, rounded */
        volatile __bf16 c = (__bf16)1.25f;
        volatile float pf = (float)b * 1.25f;
        volatile __bf16 want_p = pf;
        volatile __bf16 got_p = b * c;
        checked++;
        if (bits(want_p) != bits(got_p)) {
            if (bad < 10)
                printf("%#x * 1.25: %#x, not %#x\n", i, bits(got_p), bits(want_p));
            bad++;
        }
    }
    printf("%d mismatches of %d\n", bad, checked > 100000 ? 1 : 0);
    return 0;
}
/* expect: 0 mismatches of 1 */
