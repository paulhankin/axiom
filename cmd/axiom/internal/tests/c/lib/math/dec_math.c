/* <math.h>'s decimal floating functions (C23 7.12, the d32, d64 and d128
   suffixes): elementary values (correctly rounded here), their exact cases
   with the preferred quantum, special values and errors (errno), the
   quantum functions, the encodings (DPD and BID), total order, payloads,
   fromfp, the narrowing operations, the classification macros, the
   decimal rounding direction. */
/* flags: --c-std=c23 */
#include <errno.h>
#include <fenv.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#define S(x) _Generic((x), _Decimal32: s32, _Decimal64: s64, _Decimal128: s128)(x)
static void s32(_Decimal32 x) { printf("%Ha ", x); }
static void s64(_Decimal64 x) { printf("%Da ", x); }
static void s128(_Decimal128 x) { printf("%DDa ", x); }
static void nl(void) { printf("\n"); }
static void err(void) { printf("%s ", errno == EDOM ? "EDOM" : errno == ERANGE ? "ERANGE" : "-"); errno = 0; }

int main(void) {
    volatile _Decimal64 one = 1, two = 2, mone = -1, zero = 0;
    S(expd64(one)); S(logd64(two)); S(log10d64(two)); S(log2d64(3.0dd)); S(log1pd64(1E-20dd));
/* expect: 2.718281828459045 0.6931471805599453 0.3010299956639812 1.584962500721156 1.000000000000000e-20  */
    nl();
    S(exp2d64(0.5dd)); S(exp10d64(-1.5dd)); S(expm1d64(1E-10dd)); S(exp2m1d32(1E-3df)); S(exp10m1d128(0.5dl));
/* expect: 1.414213562373095 0.03162277660168379 1.000000000050000e-10 0.0006933875 2.162277660168379331998893544432719  */
    nl();
    S(sind64(one)); S(cosd64(one)); S(tand64(one)); S(sind64(1E300dd)); S(cosd128(1E6000dl));
/* expect: 0.8414709848078965 0.5403023058681397 1.557407724654902 -0.9857504251603770 -0.6888260645008393834757008024508130  */
    nl();
    S(asind64(0.5dd)); S(acosd64(0.5dd)); S(atand64(one)); S(atan2d64(one, mone)); S(atan2pid64(one, mone));
/* expect: 0.5235987755982989 1.047197551196598 0.7853981633974483 2.356194490192345 0.7500000000000000  */
    nl();
    S(sinpid64(0.25dd)); S(cospid64(1.5dd)); S(tanpid32(0.75df)); S(asinpid64(one)); S(acospid64(mone));
/* expect: 0.7071067811865475 0 -1.000000 0.5 1  */
    nl();
    S(sinhd64(one)); S(coshd64(one)); S(tanhd64(0.5dd)); S(asinhd64(one)); S(acoshd64(two)); S(atanhd64(0.5dd));
/* expect: 1.175201193643801 1.543080634815244 0.4621171572600098 0.8813735870195430 1.316957896924817 0.5493061443340548  */
    nl();
    S(powd64(two, 0.5dd)); S(powrd64(two, 10)); S(cbrtd64(two)); S(rsqrtd64(two)); S(hypotd64(one, two));
/* expect: 1.414213562373095 1024 1.259921049894873 0.7071067811865475 2.236067977499790  */
    nl();
    S(erfd64(0.5dd)); S(erfcd64(5.0dd)); S(lgammad64(10.5dd)); S(tgammad64(4.5dd)); S(tgammad64(-1.5dd));
/* expect: 0.5204998778130465 1.537459794428035e-12 13.94062521940376 11.63172839656745 2.363271801207355  */
    nl();
    S(erfcd128(100.dl)); S(tgammad128(1000.dl)); S(lgammad32(1E5df));
/* expect: 6.405961424921732039021339148586394e-4346 4.023872600770937735437024339230040e+2564 1051288  */
    nl();
    /* exact results, with the quanta C23 prefers */
    S(expd64(zero)); S(logd64(one)); S(log10d64(1000.dd)); S(exp10d64(5.dd)); S(exp2d32(10.df)); S(sqrtd64(1.44dd));
/* expect: 1 0 3 100000 1024 1.2  */
    nl();
    S(powd64(two, 10)); S(powd64(1.5dd, 2)); S(powd64(0.5dd, -2)); S(pownd64(1.10dd, 3)); S(rootnd64(32, 5));
/* expect: 1024 2.25 4 1.331000 2  */
    nl();
    S(cbrtd64(27.000dd)); S(hypotd64(3.0dd, 4.00dd)); S(compoundnd64(0.1dd, 2)); S(tgammad64(5.dd)); S(lgammad64(2.dd));
/* expect: 3.0 5.00 1.210000000000000 24 0  */
    nl();
    /* special values and errors */
    errno = 0;
    S(logd64(zero)); err(); S(logd64(mone)); err(); S(sqrtd64(mone)); err(); S(tgammad64(mone)); err();
/* expect: -inf ERANGE nan EDOM nan EDOM nan EDOM  */
    nl();
    S(lgammad64(zero)); err(); S(powd64(zero, mone)); err(); S(powd64(-8, 1 / 3.0dd)); err(); S(expd64(1E6dd)); err();
/* expect: inf ERANGE inf ERANGE nan EDOM inf ERANGE  */
    nl();
    S(expd64(-1E6dd)); err(); S(atan2d64(0.0dd, -0.0dd)); S(powd64(-1, 1E400dl)); S(powd64(mone / zero, -3)); S(atanhd64(one)); err();
/* expect: 0e-398 ERANGE 3.141592653589793 1 -0 inf ERANGE  */
    nl();
    /* rounding functions, remainders */
    S(ceild64(-0.5dd)); S(floord64(2.50dd)); S(roundd64(2.5dd)); S(roundevend64(2.5dd)); S(truncd64(-7.9dd)); S(rintd64(1.5E3dd));
/* expect: -0 2 3 2 -7 1.5e+03  */
    nl();
    S(fmodd64(7.5dd, 2)); S(remainderd64(7.5dd, 2)); S(fmodd64(-1E300dd, 3)); S(remainderd128(1E100dl, 7.dl));
/* expect: 1.5 -0.5 -1 -3  */
    nl();
    printf("%ld %ld %lld %lld ", lrintd64(2.5dd), lroundd64(2.5dd), llrintd64(-3.5dd), llroundd64(-3.5dd));
    printf("%d %ld ", ilogbd64(123.dd), llogbd64(1E-5dd));
    S(logbd64(123.45dd)); S(scalbnd64(1.5dd, 3)); S(ldexpd32(2.0df, -2)); S(scalblnd128(1.dl, 7000));
/* expect: 2 3 -4 -4 2 -5 2 1.5e+03 0.020 inf  */
    nl();
    {
        int e;
        _Decimal64 ip, fr = frexpd64(123.45dd, &e);
        S(fr);
        printf("%d ", e);
        fr = modfd64(-12.375dd, &ip);
        S(fr);
        S(ip);
/* expect: 0.12345 3 -0.375 -12  */
        nl();
    }
    S(nextupd64(one)); S(nextdownd64(one)); S(nextafterd32(0, mone)); S(nexttowardd64(one, 2.dl)); S(nextupd32(DEC32_MAX));
/* expect: 1.000000000000001 0.9999999999999999 -1e-101 1.000000000000001 inf  */
    nl();
    S(fabsd64(-2.50dd)); S(copysignd64(2.50dd, -0.dd)); S(fdimd64(5, 7)); S(fmaxd64(1.0dd, 1.00dd)); S(fmind64(1.0dd, 1.00dd));
    S(fmaxd64(one, zero / zero)); S(fmaximumd64(one, zero / zero)); S(fminimum_magd64(-3, 2)); S(fmaximum_numd64(zero / zero, two));
/* expect: 2.50 -2.50 0 1.0 1.00 1 nan 2 2  */
    nl();
    S(fmad64(two, 3, 0.5dd)); S(fmad32(1E-7df, 1E-7df, 1.df)); S(quantumd64(1.230dd)); printf("%lld ", llquantexpd64(1.230dd));
    S(quantized64(3.14159dd, 0.01dd)); S(quantized64(5.dd, 1.00dd)); printf("%d %d ", samequantumd64(1.0dd, 2.0dd), samequantumd64(1.0dd, 1.00dd));
/* expect: 6.5 1.000000 0.001 -3 3.14 5.00 1 0  */
    nl();
    /* narrowing */
    S(d32addd64(1.0000001dd, 1)); S(d32muld128(1.234567890dl, 1)); S(d64divd128(1.dl, 3.dl)); S(d32sqrtd64(two)); S(d32fmad64(two, 3, 1));
/* expect: 2.000000 1.234568 0.3333333333333333 1.414214 7  */
    nl();
    /* encodings: DPD and BID */
    {
        _Decimal64 x = 1.dd, y, enc;
        unsigned long long u;
        encodedecd64(&enc, &x);
        memcpy(&u, &enc, 8);
        printf("%llx ", u);
        decodedecd64(&y, &enc);
        S(y);
        _Decimal128 big = 9.999999999999999999999999999999999E6144dl, eb, back;
        encodedecd128(&eb, &big);
        decodedecd128(&back, &eb);
        printf("%d ", memcmp(&back, &big, 16) == 0);
        encodebind64(&enc, &x);
        printf("%d ", memcmp(&enc, &x, 8) == 0);
/* expect: 2238000000000001 1 1 1  */
        nl();
    }
    /* total order, payloads, canonical encodings */
    {
        _Decimal64 a = 1.0dd, b = 1.00dd, nz = -0.dd, pz = 0.dd, n1 = nand64("123"), r;
        printf("%d %d %d %d ", totalorderd64(&b, &a), totalorderd64(&a, &b), totalorderd64(&nz, &pz),
               totalordermagd64(&nz, &pz));
        S(getpayloadd64(&n1)); S(getpayloadd64(&a));
        printf("%d ", setpayloadd64(&r, 456.dd));
        S(getpayloadd64(&r));
        printf("%d %d ", setpayloadsigd64(&r, 7.dd), issignaling(r));
        printf("%d ", setpayloadd64(&r, 1.5dd));
        _Decimal32 nc, cc;
        unsigned u = 0x6cb89680; /* 10000000 * 10^0 in the second form: non-canonical, zero */
        memcpy(&nc, &u, 4);
        printf("%d %d ", iscanonical(nc), iszero(nc));
        canonicalized32(&cc, &nc);
        printf("%d ", iscanonical(cc));
        S(cc);
/* expect: 1 0 1 1 123 -1 0 456 0 1 1 0 1 1 0  */
        nl();
    }
    /* fromfp */
    S(fromfpd64(2.5dd, FP_INT_TONEAREST, 8)); S(fromfpd64(2.5dd, FP_INT_TONEARESTFROMZERO, 8)); S(fromfpd64(300, FP_INT_UPWARD, 8));
    S(ufromfpd64(-1, FP_INT_UPWARD, 8)); S(ufromfpxd64(255.5dd, FP_INT_DOWNWARD, 8));
/* expect: 2 3 nan nan 255  */
    nl();
    /* classification */
    printf("%d %d %d %d %d %d %d %d ", fpclassify(1E-398dd) == FP_SUBNORMAL, fpclassify(1E-383dd) == FP_NORMAL,
           isnan(DEC64_SNAN), issignaling(DEC64_SNAN), isinf(HUGE_VAL_D128), signbit(-0.dd), iszero(0E50dd),
           isnormal(DEC32_MIN));
    printf("%d %d %d ", isless(one, two), isunordered(one, zero / zero), isgreaterequal(1.0dd, 1.00dd));
/* expect: 1 1 1 1 1 1 1 1 1 1 1  */
    nl();
    /* the decimal rounding direction */
    printf("%d ", fe_dec_getround() == FE_DEC_TONEAREST);
    fe_dec_setround(FE_DEC_DOWNWARD);
    S(one / 3); S(expd32(1));
    fe_dec_setround(FE_DEC_UPWARD);
    S(one / 3); S(expd32(1));
    fe_dec_setround(FE_DEC_TONEARESTFROMZERO);
    S((_Decimal32)0.00000125dd + 1);
    printf("%d %d ", fe_dec_setround(17) != 0, fe_dec_getround() == FE_DEC_TONEARESTFROMZERO);
    fe_dec_setround(FE_DEC_TONEAREST);
    S((_Decimal32)0.00000125dd + 1);
/* expect: 1 0.3333333333333333 2.718281 0.3333333333333334 2.718282 1.000001 1 1 1.000001  */
    nl();
    return 0;
}
