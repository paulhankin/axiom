/* flags: --c-std=c11 */
/* Switches whose cases each give a constant are table lookups at -O2
   (switches.ax: lookup_tables): returns of constants, a variable set to a
   constant in each case, holes in the span (the default's value), negative
   and repeated case values, a default that falls through to the join, an
   8-bit switch value and 64-bit results, and values below, inside and
   above the span. */
#include "host.h"
static int ret4(int x) {
    switch (x) {
    case 0: return 11;
    case 1: return 23;
    case 2: return 37;
    case 3: return 41;
    default: return -1;
    }
}
static long long holes(int x) {
    long long r;
    switch (x) {
    case -3: r = 100; break;
    case -1: r = -7; break;
    case 2: r = 0x7fffffffffffLL; break;
    case 6: r = 5; break;
    default: r = 9; break;
    }
    return r * 2;
}
static int two(unsigned char c, int *second) {
    int a = 0, b = 1;
    switch (c) {
    case 'a': a = 1; b = 10; break;
    case 'b': a = 2; b = 20; break;
    case 'd': a = 4; b = 40; break;
    case 'e': a = 5; b = 50; break;
    }
    *second = b;
    return a;
}
static short fall(int x) {
    short v = 3;
    switch (x) {
    case 10: v = 30; break;
    case 11: v = 31; break;
    case 12: v = 32; break;
    case 14: v = 34; break;
    default: break;
    }
    return v;
}
int main(void) {
    long long h = 0;
    for (int x = -10; x < 270; x++) {
        int s = 0;
        h = h * 31 + ret4(x);
        h = h * 31 + holes(x);
        h = h * 31 + two((unsigned char)x, &s) + s * 7;
        h = h * 31 + fall(x);
    }
    print_i64(h);
    return 0;
}
/* expect: -311009072011159511 */
