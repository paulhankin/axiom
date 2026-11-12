/* Loop counters narrower than a pointer (int, short, unsigned), which -O2
   widens where their steps cannot overflow (signed arithmetic), and the
   addresses computed from them: the results must be those of the
   arithmetic as C defines it, at the edges of the types too. */
#include "host.h"
#include <limits.h>

static int a[64];
static unsigned char bytes[300];

/* a counter that is also a value, compared with a bound read in the loop */
static long sum_to(const int *n) {
    long s = 0;
    for (int i = 0; i < *n; i++) s += a[i] * (long)i;
    return s;
}

/* t[i + j] with i invariant, and the counter's value after the loop */
static int mul_add(unsigned *t, const unsigned *x, int nx, const unsigned *y, int ny) {
    int j = 0;
    for (int i = 0; i < nx; i++) {
        unsigned long long carry = 0;
        for (j = 0; j < ny; j++) {
            unsigned long long p = (unsigned long long)x[i] * y[j] + t[i + j] + carry;
            t[i + j] = (unsigned)p;
            carry = p >> 32;
        }
        t[i + j] = (unsigned)carry;
    }
    return j;
}

/* counters near INT_MAX that do not overflow */
static long near_max(int lo, int hi) {
    long s = 0;
    for (int i = lo; i < hi; i++) s += a[i - lo] + (i & 7);
    for (int i = lo; i <= hi - 1; i++) s += i - lo;
    return s;
}

/* downward and by steps other than one */
static long steps(int n) {
    long s = 0;
    for (int i = n - 1; i >= 0; i--) s = (s * 3 + a[i]) % 1000003;
    for (int i = n - 1; i >= 0; i -= 3) s += a[i] * 5;
    for (int i = 1; i < n; i += 7) s -= a[i - 1] + a[2 * i - 2 > 63 ? 63 : 2 * i - 2];
    return s;
}

/* negative offsets from the counter, and a negative start */
static long offsets(int n) {
    long s = 0;
    for (int i = -8; i < n; i++) s += a[i + 8] * (i < 0 ? -1 : 1);
    for (int i = 5; i < n; i++) s += a[i - 5] - a[(i - 5) / 2];
    return s;
}

/* unsigned counters wrap: not widened as if they did not */
static unsigned long long wraps(void) {
    unsigned long long s = 0;
    unsigned n = 0;
    for (unsigned u = UINT_MAX - 3; u != 4; u++) {
        s = s * 31 + bytes[u & 255] + u;
        n++;
    }
    s += n;
    for (unsigned char c = 250; c != 3; c++) s = s * 7 + bytes[c] + c;
    for (unsigned short h = 65530; h != 6; h++) s = s * 5 + bytes[h % 300];
    return s;
}

/* narrow signed counters (their steps are int arithmetic, then narrowed) */
static long narrow(void) {
    long s = 0;
    for (short h = -300; h < 300; h += 3) s += bytes[h + 300 < 300 ? h + 300 : 299] + h;
    for (signed char c = -100; c < 100; c++) s += a[(c + 128) & 63] * c;
    return s;
}

/* a counter whose sign extension feeds an address both as itself and plus
   an invariant, in a loop entered zero times too */
static long both(int n, int k) {
    long s = 0;
    for (int i = 0; i < n; i++) s += a[i] + a[i + k];
    return s;
}

/* (the arguments through volatile objects: not constants the optimizer folds) */
static volatile int vk[8] = { 40, 0, 4, 5, 20, 64, 50, 30 };

int main(void) {
    for (int i = 0; i < 64; i++) a[i] = i * 37 % 101 - 50;
    for (int i = 0; i < 300; i++) bytes[i] = (unsigned char)(i * 7 + 3);
    int n = vk[0];
    print_i64(sum_to(&n));
    n = vk[1];
    print_i64(sum_to(&n));
    unsigned t[16] = { 0 };
    unsigned x[4] = { 0xffffffffu, 2, 0x80000000u, 7 };
    unsigned y[5] = { 0xffffffffu, 0xfffffffeu, 3, 0, 0x12345678u };
    print_i64(mul_add(t, x, vk[2], y, vk[3]));
    unsigned long long h = 0;
    for (int i = 0; i < 16; i++) h = h * 1000003 + t[i];
    print_u64(h);
    print_i64(mul_add(t, x, vk[2], y, vk[1]));
    print_i64(near_max(INT_MAX - vk[4], INT_MAX));
    print_i64(near_max(INT_MIN, INT_MIN + vk[7]));
    print_i64(steps(vk[5]));
    print_i64(steps(vk[1]));
    print_i64(offsets(vk[6]));
    print_u64(wraps());
    print_i64(narrow());
    print_i64(both(vk[7], vk[4]));
    print_i64(both(vk[1], vk[4]));
    print_i64(both(vk[3] * 2, vk[3]));
    return 0;
}
/* expect: 1759 */
/* expect: 0 */
/* expect: 5 */
/* expect: 10088380062427203382 */
/* expect: 0 */
/* expect: 234 */
/* expect: 484 */
/* expect: -748371 */
/* expect: 0 */
/* expect: 163 */
/* expect: 1955474794338849389 */
/* expect: 21005 */
/* expect: -19 */
/* expect: 0 */
/* expect: -62 */
