/* Numeric programs: Kahan and plain summation of a harmonic series,
   Newton's square roots and a Simpson integration in double and in long
   double (printed to 15 significant digits, which every long double format
   of the targets gives alike: binary64, the x87's, binary128), a
   Gauss-Jordan solution of a 6x6 system, the extreme values of <float.h>
   by %a and %g, and strtod's and printf's round trip of 1000 values at
   17 digits. */
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static long double newton(long double x) {
    long double r = x > 1 ? x : 1;
    int i;
    for (i = 0; i < 100; i++)
        r = (r + x / r) / 2;
    return r;
}

static double f(double x) {
    return exp(-x * x);
}

int main(void) {
    double plain = 0, kahan = 0, c = 0, a[6][7];
    long double lsum = 0;
    int i, j, k, bad = 0;
    for (i = 1; i <= 1000000; i++) {
        double t = 1.0 / i, y = t - c, s = kahan + y;
        c = (s - kahan) - y;
        kahan = s;
        plain += t;
        lsum += 1.0L / i;
    }
    printf("harmonic: plain %.15g kahan %.15g long double %.12Lg\n", plain, kahan, lsum);
    printf("plain differs from kahan: %d; kahan matches long double: %d\n", plain != kahan,
           fabs(kahan - (double)lsum) < 1e-11);
    printf("sqrt: %.12Lg %.12Lg %.15g\n", newton(2.0L), newton(1e10L), sqrt(3.0));
    {
        int n = 1000;
        double h = 4.0 / n, s = f(-2) + f(2);
        for (i = 1; i < n; i++)
            s += f(-2 + i * h) * (i % 2 ? 4 : 2);
        printf("integral of exp(-x^2) over [-2, 2]: %.12f (erf: %.12f)\n", s * h / 3, sqrt(acos(-1.0)) * erf(2.0));
    }
    /* a linear system with the solution 1, 2, ..., 6 */
    for (i = 0; i < 6; i++) {
        double rhs = 0;
        for (j = 0; j < 6; j++) {
            a[i][j] = 1.0 / (i + j + 1) + (i == j ? 1 : 0);
            rhs += a[i][j] * (j + 1);
        }
        a[i][6] = rhs;
    }
    for (k = 0; k < 6; k++) {
        double p = a[k][k];
        for (j = k; j < 7; j++)
            a[k][j] /= p;
        for (i = 0; i < 6; i++)
            if (i != k) {
                double m = a[i][k];
                for (j = k; j < 7; j++)
                    a[i][j] -= m * a[k][j];
            }
    }
    printf("solution:");
    for (i = 0; i < 6; i++)
        printf(" %.12f", a[i][6]);
    printf("\n");
    printf("DBL: %a %a %a %g\n", DBL_MAX, DBL_MIN, DBL_TRUE_MIN, DBL_EPSILON);
    printf("FLT: %a %a %g %g\n", (double)FLT_MAX, (double)FLT_MIN, (double)FLT_EPSILON, (double)FLT_TRUE_MIN);
    {
        unsigned long long s = 7;
        for (i = 0; i < 1000; i++) {
            double x, y;
            char buf[40];
            s = s * 6364136223846793005ULL + 1442695040888963407ULL;
            x = ldexp((double)(s >> 11), (int)(s % 200) - 150);
            snprintf(buf, sizeof buf, "%.17g", x);
            y = strtod(buf, NULL);
            bad += x != y;
        }
        printf("round trips: %d bad\n", bad);
    }
    return 0;
}

// expect: harmonic: plain 14.392726722865 kahan 14.3927267228657 long double 14.3927267229
// expect: plain differs from kahan: 1; kahan matches long double: 1
// expect: sqrt: 1.41421356237 100000 1.73205080756888
// expect: integral of exp(-x^2) over [-2, 2]: 1.764162781523 (erf: 1.764162781525)
// expect: solution: 1.000000000000 2.000000000000 3.000000000000 4.000000000000 5.000000000000 6.000000000000
// expect: DBL: 0x1.fffffffffffffp+1023 0x1p-1022 0x0.0000000000001p-1022 2.22045e-16
// expect: FLT: 0x1.fffffep+127 0x1p-126 1.19209e-07 1.4013e-45
// expect: round trips: 0 bad
