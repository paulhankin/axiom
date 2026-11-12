/* <math.h> in C89: HUGE_VAL and the double functions; no C99 names. */
/* flags: --c-std=c89 */
/* (print.h is C99: its _Bool) */
void print_i64(__axiom_int64 x);
void print_f64(double x);
#include <math.h>

#ifdef NAN
#error "NAN is C99"
#endif
#ifdef isnan
#error "isnan is C99"
#endif

/* (a C89 program may use the C99 names itself) */
static int fmax(int a, int b)
{
    return a > b ? a : b;
}

int main(void)
{
    double ip;
    int e;
    print_f64(sqrt(2.0));
/* expect: 1.4142135623730951 */
    print_f64(pow(2.0, 0.5));
/* expect: 1.4142135623730951 */
    print_f64(atan2(1.0, 1.0) * 4.0);
/* expect: 3.141592653589793 */
    print_f64(fmod(7.5, 2.0));
/* expect: 1.5 */
    print_f64(modf(2.5, &ip) + ip);
/* expect: 2.5 */
    print_f64(frexp(8.0, &e));
/* expect: 0.5 */
    print_i64(e);
/* expect: 4 */
    print_f64(ldexp(1.0, 10));
/* expect: 1024 */
    print_i64(HUGE_VAL > 1e308);
/* expect: 1 */
    print_i64(fmax(2, 3));
/* expect: 3 */
    return 0;
}
