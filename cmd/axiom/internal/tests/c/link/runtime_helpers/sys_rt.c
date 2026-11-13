#include <stdatomic.h>
static _Atomic int counter;
int sys_atomics(int n) {
    for (int i = 0; i < n; i++)
        atomic_fetch_add(&counter, 2);
    return atomic_load(&counter);
}
long long sys_int128(long long a, long long b) {
    __int128 x = (__int128)a * b;
    double d = (double)x;
    __int128 y = (__int128)d;
    return (long long)(y / 7);
}
double sys_complex(double a, double b) {
    double _Complex z = a + b * 1.0i;
    volatile double _Complex w = z;
    double _Complex p = w * w;
    return __real__ p;
}
