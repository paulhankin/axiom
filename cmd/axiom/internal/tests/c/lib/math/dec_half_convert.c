/* Conversions between the decimal floating types and _Float16 (binary16),
   rounded once (std/c/rt/decimal.ax's binary16), in every binary rounding
   direction to _Float16 and every decimal one from it, with their
   exceptions: for each finite binary16 value h and its successor, values of
   each decimal type at h, at their midpoint and on each side of it, and
   between h and the midpoint (as
   _Decimal128 computes them exactly, then rounded to the type), both signs.
   The expected results are found by comparisons in _Decimal128, which holds
   every binary16 value and midpoint exactly. The program prints the number
   of mismatches. */
/* flags: --c-std=c23 */
/* native-only */
#include <fenv.h>
#include <stdio.h>
#include <string.h>

#pragma STDC FENV_ACCESS ON

typedef _Decimal128 D;

static const int modes[4] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
static const int dmodes[5] = {FE_DEC_TONEAREST, FE_DEC_DOWNWARD, FE_DEC_UPWARD, FE_DEC_TOWARDZERO,
                              FE_DEC_TONEARESTFROMZERO};

static _Float16 half(unsigned u)
{
    unsigned short s = (unsigned short)u;
    _Float16 h;
    memcpy(&h, &s, 2);
    return h;
}

static unsigned bits(_Float16 h)
{
    unsigned short s;
    memcpy(&s, &h, 2);
    return s;
}

static int bad, checked;

/* x (of format k: 32, 64, 128, as a _Decimal128 exactly), whose magnitude
   is between the binary16 values lo (bits i) and hi (bits i + 1, or
   infinity), whose midpoint is mid (neg: x is negative): its conversion in
   each direction */
static void check(int k, D x, unsigned i, D lo, D hi, D mid, int neg)
{
    volatile _Decimal32 x32 = (_Decimal32)x;
    volatile _Decimal64 x64 = (_Decimal64)x;
    volatile _Decimal128 x128 = x;
    for (int m = 0; m < 4; m++) {
        /* the direction for the magnitude */
        int mm = m;
        if (neg && m == 1)
            mm = 2;
        else if (neg && m == 2)
            mm = 1;
        unsigned want;
        int exact = 0;
        D ax = neg ? -x : x;
        if (ax == lo) {
            want = i;
            exact = 1;
        } else if (ax == hi) {
            want = i + 1;
            exact = 1;
        } else if (mm == 1 || mm == 3) {
            want = i;
        } else if (mm == 2) {
            want = i + 1;
        } else if (ax < mid) {
            want = i;
        } else if (ax > mid) {
            want = i + 1;
        } else {
            want = (i & 1) ? i + 1 : i;
        }
        int wflags = 0;
        if (!exact) {
            wflags |= FE_INEXACT;
            if (want == 0x7C00)
                wflags |= FE_OVERFLOW;
            /* (tiny after rounding: below the least normal value, 2^-14) */
            if (i + 1 < 0x3FF)
                wflags |= FE_UNDERFLOW;
        }
        if (neg)
            want |= 0x8000;
        fesetround(modes[m]);
        feclearexcept(FE_ALL_EXCEPT);
        _Float16 r;
        if (k == 32)
            r = (_Float16)x32;
        else if (k == 64)
            r = (_Float16)x64;
        else
            r = (_Float16)x128;
        int got = fetestexcept(FE_ALL_EXCEPT);
        fesetround(FE_TONEAREST);
        int mask = FE_INEXACT | FE_OVERFLOW | FE_INVALID | FE_DIVBYZERO;
        if (i + 1 < 0x3FF || i >= 0x400)
            mask |= FE_UNDERFLOW;
        checked++;
        if (bits(r) != want || (got & mask) != wflags) {
            if (bad < 10)
                printf("d%d -> f16: %#x in mode %d gives %#x flags %#x, not %#x flags %#x\n", k, (unsigned)bits(half(i)),
                       m, bits(r), got, want, wflags);
            bad++;
        }
    }
}

int main(void)
{
    for (unsigned i = 0; i < 0x7C00; i++) {
        /* (the subnormals, the first normal binades and the last, and
           every 13th value between) */
        if (i > 0x900 && i < 0x7B00 && i % 13 != 0)
            continue;
        D lo = (D)half(i);
        D hi = i == 0x7BFF ? 65536.DL : (D)half(i + 1);
        D mid = (lo + hi) / 2;
        D eps = (hi - lo) * 1E-20DL;
        D xs[5] = {lo, mid, mid + eps, mid - eps, (lo + mid) / 2};
        for (int neg = 0; neg < 2; neg++) {
            for (int j = 0; j < 5; j++) {
                D x = neg ? -xs[j] : xs[j];
                /* each decimal type: the value as it holds it, if it is
                   still between the two (to nearest in decimal) */
                D x32 = (D)(_Decimal32)x;
                D x64 = (D)(_Decimal64)x;
                D a = neg ? -x32 : x32, b = neg ? -x64 : x64;
                if (a >= lo && a <= hi)
                    check(32, x32, i, lo, hi, mid, neg);
                if (b >= lo && b <= hi)
                    check(64, x64, i, lo, hi, mid, neg);
                check(128, x, i, lo, hi, mid, neg);
            }
        }
    }
    /* binary16 to a decimal type: exact in _Decimal128, rounded once in
       each decimal direction to the others, as from that exact value */
    for (unsigned i = 0; i < 0x10000; i++) {
        if ((i & 0x7FFF) > 0x900 && (i & 0x7FFF) < 0x7B00 && i % 11 != 0)
            continue;
        _Float16 h = half(i);
        if (h != h)
            continue;
        volatile _Float16 vh = h;
        feclearexcept(FE_ALL_EXCEPT);
        D e = (D)vh;
        if (fetestexcept(FE_ALL_EXCEPT) != 0 || (_Float16)e != h) {
            if (bad < 10)
                printf("f16 -> d128: %#x not exact\n", i);
            bad++;
        }
        for (int m = 0; m < 5; m++) {
            fe_dec_setround(dmodes[m]);
            feclearexcept(FE_ALL_EXCEPT);
            volatile _Decimal32 w32 = (_Decimal32)e;
            int f1 = fetestexcept(FE_ALL_EXCEPT);
            feclearexcept(FE_ALL_EXCEPT);
            volatile _Decimal32 g32 = (_Decimal32)vh;
            int f2 = fetestexcept(FE_ALL_EXCEPT);
            feclearexcept(FE_ALL_EXCEPT);
            volatile _Decimal64 w64 = (_Decimal64)e;
            int f3 = fetestexcept(FE_ALL_EXCEPT);
            feclearexcept(FE_ALL_EXCEPT);
            volatile _Decimal64 g64 = (_Decimal64)vh;
            int f4 = fetestexcept(FE_ALL_EXCEPT);
            fe_dec_setround(FE_DEC_TONEAREST);
            checked += 2;
            if (memcmp((const void *)&w32, (const void *)&g32, 4) != 0 || f1 != f2 ||
                memcmp((const void *)&w64, (const void *)&g64, 8) != 0 || f3 != f4) {
                if (bad < 10)
                    printf("f16 -> d32/d64: %#x in mode %d\n", i, m);
                bad++;
            }
        }
    }
    printf("%d mismatches of %d\n", bad, checked > 100000 ? 1 : 0);
    return 0;
}
/* expect: 0 mismatches of 1 */
