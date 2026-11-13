/* Complex arithmetic (C99 6.2.5, 6.3.1.6-8, 6.5): the types in every
   specifier order, their size and alignment, conversions between real and
   complex types and between complex types, the operators (unary + and -,
   ! , + - * / with real and complex operands, == and !=, && ||, ?:,
   assignment and compound assignment), casts, I, CMPLX and creal, cimag
   and conj, and complex objects in arrays, structures and pointers. */
#include <complex.h>
#include "host.h"

static void pc(double complex z) {
    print_f64(creal(z));
    print_f64(cimag(z));
}

typedef struct { char c; float complex f; double complex d; } S;

int main(void) {
    _Complex double a = 1.5 + 2.0 * I;
    double _Complex b = CMPLX(-0.5, 4.0);
    float complex f = 2.0f - 3.0f * I;
    complex float g = f;
    long double complex l = CMPLXL(1.0, -1.0);
    _Complex long double l2 = l;
    double long _Complex l3 = l2;
    /* sizes and alignments: two of the real type */
    print_i64(sizeof(float complex) == 2 * sizeof(float));
    print_i64(sizeof(double complex) == 2 * sizeof(double));
    print_i64(sizeof l3 == 2 * sizeof(long double));
    print_i64(_Alignof(double complex) == _Alignof(double));
    print_i64(_Alignof(float complex) == _Alignof(float));
    {
        double complex arr[3] = { 1, 2.0 * I, CMPLX(3, 4) };
        double *p = (double *)arr;
        print_f64(p[0] + p[3] * 10 + p[4] * 100 + p[5] * 1000);
        print_i64((int)(sizeof arr));
    }
    /* + - * / of complex values */
    pc(a + b);
    pc(a - b);
    pc(a * b);
    pc(a / b);
    pc(-a);
    pc(+b);
    /* with a real operand */
    pc(a + 1);
    pc(2 - a);
    pc(a * 3.0f);
    pc(0.5 * b);
    pc(a / 2);
    pc(1.0 / b);
    /* float complex arithmetic, and mixed with double */
    {
        float complex h = f * g + f / g;
        print_f64(crealf(h));
        print_f64(cimagf(h));
        pc(f + a);
        print_i64(_Generic(f + a, double complex: 1, default: 0));
        print_i64(_Generic(f + 1.0, double complex: 1, default: 0));
        print_i64(_Generic(f + 1, float complex: 1, default: 0));
        print_i64(_Generic(l + f, long double complex: 1, default: 0));
        print_i64(_Generic(1.0f + 2LL, float: 1, default: 0));
    }
    /* conversions */
    {
        double d = a;
        int i = b;
        _Bool t = CMPLX(0, 0.25), u = CMPLX(0, 0), w = CMPLX(-0.0, 0);
        float complex fc = 7;
        double complex dc = f;
        unsigned char uc = CMPLX(300.5, 9);
        print_f64(d);
        print_i64(i);
        print_i64(t);
        print_i64(u);
        print_i64(w);
        pc(fc);
        pc(dc);
        print_i64(uc);
        pc((double complex)3);
        print_f64((double)b);
        print_i64((long)CMPLX(-7.9, 1));
        print_f64((float)(double complex)1e300);
    }
    /* equality, truth, logical operators */
    print_i64(a == a);
    print_i64(a != a);
    print_i64(a == b);
    print_i64(CMPLX(2, 0) == 2);
    print_i64(2.0f != CMPLX(2, 0));
    print_i64(CMPLX(2, 1) == 2);
    print_i64(!a);
    print_i64(!CMPLX(0, 0));
    print_i64(a && 0);
    print_i64(CMPLX(0, 1) || 0);
    if (CMPLX(0, -1)) print_i64(11);
    {
        int n = 0;
        double complex z = CMPLX(0, 3);
        while (z) {
            z -= CMPLX(0, 1);
            n++;
        }
        print_i64(n);
    }
    /* ?: */
    {
        int k;
        for (k = 0; k < 2; k++) {
            pc(k ? a : b);
            pc(k ? a : 5);
            pc(k ? f : 1.0);
        }
    }
    /* assignment and compound assignment */
    {
        double complex z = 1;
        double r = 10;
        float complex fz = 1;
        z += a;
        pc(z);
        z -= 1;
        pc(z);
        z *= a;
        pc(z);
        z /= b;
        pc(z);
        z *= 2;
        pc(z);
        z /= 4.0f;
        pc(z);
        r += a;  /* the real part only remains */
        print_f64(r);
        r *= CMPLX(0, 1);
        print_f64(r);
        fz += a;
        print_f64(crealf(fz));
        print_f64(cimagf(fz));
        pc(z = b);
        pc(a = b = CMPLX(9, -9));
    }
    /* creal, cimag, conj and I */
    print_f64(cimag(I));
    print_f64(creal(I));
    print_i64(_Generic(_Complex_I, float complex: 1, default: 0)); /* (I is _Imaginary_I with Annex G) */
    pc(conj(a));
    print_f64(crealf(conjf(f)) + cimagf(conjf(f)));
    print_f64(creall(conjl(l)) - cimagl(conjl(l)));
    print_f64(creal(2.5));
    print_f64(cimag(2.5));
    print_f64(cimag(I * I));
    print_f64(creal(I * I));
    /* in structures, through pointers */
    {
        S s = { 'x', 1 + I, CMPLX(2, 3) };
        S t2;
        S *ps = &t2;
        double complex *pz = &s.d;
        t2 = s;
        ps->d *= ps->f;
        *pz += 1;
        pc(t2.d);
        pc(s.d);
        print_f64(crealf(t2.f) + cimagf(t2.f));
        print_i64((int)sizeof(S));
    }
    return 0;
}

/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 4321 */
/* expect: 48 */
/* expect: 1 */
/* expect: 6 */
/* expect: 2 */
/* expect: -2 */
/* expect: -8.75 */
/* expect: 5 */
/* expect: 0.4461538461538462 */
/* expect: -0.4307692307692308 */
/* expect: -1.5 */
/* expect: -2 */
/* expect: -0.5 */
/* expect: 4 */
/* expect: 2.5 */
/* expect: 2 */
/* expect: 0.5 */
/* expect: -2 */
/* expect: 4.5 */
/* expect: 6 */
/* expect: -0.25 */
/* expect: 2 */
/* expect: 0.75 */
/* expect: 1 */
/* expect: -0.03076923076923077 */
/* expect: -0.24615384615384617 */
/* expect: -4 */
/* expect: -12 */
/* expect: 3.5 */
/* expect: -1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1.5 */
/* expect: 0 */
/* expect: 1 */
/* expect: 0 */
/* expect: 0 */
/* expect: 7 */
/* expect: 0 */
/* expect: 2 */
/* expect: -3 */
/* expect: 255 */
/* expect: 3 */
/* expect: 0 */
/* expect: -0.5 */
/* expect: -7 */
/* expect: Infinity */
/* expect: 1 */
/* expect: 0 */
/* expect: 0 */
/* expect: 1 */
/* expect: 0 */
/* expect: 0 */
/* expect: 0 */
/* expect: 1 */
/* expect: 0 */
/* expect: 1 */
/* expect: 11 */
/* expect: 3 */
/* expect: -0.5 */
/* expect: 4 */
/* expect: 5 */
/* expect: 0 */
/* expect: 1 */
/* expect: 0 */
/* expect: 1.5 */
/* expect: 2 */
/* expect: 1.5 */
/* expect: 2 */
/* expect: 2 */
/* expect: -3 */
/* expect: 2.5 */
/* expect: 2 */
/* expect: 1.5 */
/* expect: 2 */
/* expect: -1.75 */
/* expect: 6 */
/* expect: 1.5307692307692307 */
/* expect: 0.24615384615384617 */
/* expect: 3.0615384615384613 */
/* expect: 0.49230769230769234 */
/* expect: 0.7653846153846153 */
/* expect: 0.12307692307692308 */
/* expect: 11.5 */
/* expect: 0 */
/* expect: 2.5 */
/* expect: 2 */
/* expect: -0.5 */
/* expect: 4 */
/* expect: 9 */
/* expect: -9 */
/* expect: 1 */
/* expect: 0 */
/* expect: 1 */
/* expect: 9 */
/* expect: 9 */
/* expect: 5 */
/* expect: 0 */
/* expect: 2.5 */
/* expect: 0 */
/* expect: 0 */
/* expect: -1 */
/* expect: -1 */
/* expect: 5 */
/* expect: 3 */
/* expect: 3 */
/* expect: 2 */
/* expect: 32 */
