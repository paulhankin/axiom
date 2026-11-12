/* flags: -O1 */
/* A quotient and a remainder of the same operands share one divide
   instruction (isel.rem_quotient; on wasm the remainder is x - q y):
   pairs of every width and signedness, interleaved pairs, a repeated
   quotient, a remainder before its quotient, and pairs split by a store. */
#include "host.h"

static volatile int sink;

static unsigned long long mix(unsigned long long *s) {
    *s ^= *s << 13;
    *s ^= *s >> 7;
    *s ^= *s << 17;
    return *s;
}

static unsigned long long u64p(unsigned long long a, unsigned long long b) { return a / b * 1000003u + a % b; }
static unsigned long long s64p(long long a, long long b) { return (unsigned long long)(a / b) * 1000003u + (unsigned long long)(a % b); }
static unsigned u32p(unsigned a, unsigned b) { return a / b * 1009u + a % b; }
static unsigned s32p(int a, int b) { return (unsigned)(a / b) * 1009u + (unsigned)(a % b); }
static int s16p(short a, short b) { return a / b * 1009 + a % b; }
static unsigned u8p(unsigned char a, unsigned char b) { return a / b * 1009u + a % b; }

/* q1 r2 q2 r1, q q r, r q, q store r */
typedef unsigned long long U;

static U shapes(long long a, long long b, long long c, long long d) {
    long long q1 = a / b, q2 = c / d, r2 = c % d, r1 = a % b;
    long long q3 = a / b;
    long long r3 = a % b;
    long long r4 = c % b;
    long long q4 = c / b;
    long long q5 = d / b;
    sink = (int)q5;
    long long r5 = d % b;
    return (((((U)q1 * 3 + (U)q2) * 5 + (U)r2) * 7 + (U)r1) * 11 + (U)q3 * 13 + (U)r3) * 17 + (U)r4 * 19 + (U)q4 * 23 +
           (U)q5 * 29 + (U)r5;
}

int main(void) {
    unsigned long long s = 88172645463325252ULL, h = 0;
    unsigned long long t = 0;
    int i;
    for (i = 0; i < 20000; i++) {
        unsigned long long a = mix(&s), b = mix(&s), c = mix(&s);
        int b32 = (int)(unsigned)(b | 1);
        b >>= c & 63;
        if (b == 0) b = 3;
        h = h * 31 + u64p(a, b);
        if ((long long)b != -1) h = h * 31 + (unsigned long long)s64p((long long)a, (long long)b);
        h = h * 31 + u32p((unsigned)a, (unsigned)b | 1);
        if (b32 == -1) b32 = 7;
        h = h * 31 + s32p((int)a, b32);
        h = h * 31 + (unsigned)s16p((short)a, (short)(b | 1));
        h = h * 31 + u8p((unsigned char)a, (unsigned char)(b | 1));
        if ((long long)b != -1) t = t * 3 + shapes((long long)a >> 1, (long long)b, (long long)c >> 2, (long long)(b | 5));
    }
    print_u64(h);
    print_u64(t);
    return 0;
}
/* expect: 7404801130815531595 */
/* expect: 6914185141893953943 */
