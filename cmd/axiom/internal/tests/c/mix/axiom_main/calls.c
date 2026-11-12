/* The functions the Axiom program calls; some call it back. */
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

int ax_triple(int);
int8_t ax_small(int8_t);
bool ax_less(int32_t, int32_t);

int c_add(int a, int b) { return a + b; }
int8_t c_neg8(int8_t x) { return (int8_t)-x; }
uint16_t c_hi(uint32_t x) { return (uint16_t)(x >> 16); }
bool c_is_odd(long long x) { return x % 2 != 0; }
double c_mix(float a, double b, long long c, unsigned char d) { return a + b + (double)c + d; }

long long c_sum(int n, ...) {
    va_list ap;
    long long s = 0;
    va_start(ap, n);
    while (n-- > 0)
        s += va_arg(ap, long long);
    va_end(ap);
    return s;
}

int c_apply(int (*f)(int), int x) { return f(x); }

/* 1 + 3*4: an Axiom function, and an i8 through it */
int c_calls_axiom(int x) { return ax_triple(x) + ax_small(2); }

static int add7(int x) { return x + 7; }
int (*c_make_adder(void))(int) { return add7; }

void c_sort3(int32_t *xs) {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j + 1 < 3 - i; j++)
            if (ax_less(xs[j + 1], xs[j])) {
                int32_t t = xs[j];
                xs[j] = xs[j + 1];
                xs[j + 1] = t;
            }
}
