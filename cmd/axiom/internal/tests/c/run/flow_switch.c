/* switch statements: case labels inside a loop in the switch body (the device
   that unrolls a copy), cases inside nested statements and blocks, default
   first, long long and unsigned controlling expressions with wide values,
   and a _Noreturn function. */
#include "host.h"
_Noreturn static void stop(void) { for (;;) { } }
static int duff(int *to, const int *from, int count) {
    int n = (count + 7) / 8, s = 0;
    switch (count % 8) {
    case 0: do { *to++ = *from++; s++;
    case 7:      *to++ = *from++; s++;
    case 6:      *to++ = *from++; s++;
    case 5:      *to++ = *from++; s++;
    case 4:      *to++ = *from++; s++;
    case 3:      *to++ = *from++; s++;
    case 2:      *to++ = *from++; s++;
    case 1:      *to++ = *from++; s++;
            } while (--n > 0);
    }
    return s;
}
static int tags(void) {
    struct S { int a; } x = { 1 };
    {
        struct S { int b, c; } y = { 2, 3 };
        x.a += y.b + y.c + (int)sizeof(struct S);
    }
    return x.a + (int)sizeof(struct S);
}
static int blockfn(void) {
    int twice(int);
    return twice(21);
}
int twice(int x) { return x * 2; }
static int labels(int n) {
    int r = 0;
    goto start;
    {
    back:
        r += 100;
        if (r > 300) return r;
    }
start:
    {
        int k = n;
        r += k;
        goto back;
    }
}
static int sw_default_first(int x) {
    switch (x) {
    default: return -1;
    case 1: return 10;
    case 2: return 20;
    }
}
static int sw_ll(long long x) {
    switch (x) {
    case 0x100000000LL: return 1;
    case -0x100000000LL: return 2;
    case 1: return 3;
    case 0x7fffffffffffffffLL: return 4;
    case -0x7fffffffffffffffLL - 1: return 5;
    }
    return 0;
}
static int sw_u(unsigned x) {
    switch (x) { case -1: return 1; case 0x80000000u: return 2; }
    return 0;
}
static int sw_nested(int x) {
    int r = 0;
    switch (x) {
        while (r < 10) {
            case 1: r += 1;
            if (r == 5) break;
            case 2: r += 2;
        }
        r += 1000;
        break;
        case 3: { case 4: r = 44; }
    }
    return r;
}
int main(void) {
    int a[20], b[20];
    for (int i = 0; i < 20; i++) a[i] = i * 3;
    print_i64(duff(b, a, 13));
    print_i64(b[12] + b[0]);
    print_i64(duff(b, a, 16));
    print_i64(tags());
    print_i64(blockfn());
    print_i64(labels(7));
    print_i64(sw_default_first(1) + sw_default_first(5));
    print_i64(sw_ll(0x100000000LL) * 10000 + sw_ll(-0x100000000LL) * 1000 + sw_ll(1) * 100 + sw_ll(0x7fffffffffffffffLL) * 10 + sw_ll(-0x7fffffffffffffffLL - 1));
    print_i64(sw_ll(2) + sw_ll((long long)1 << 32 | 1));
    print_i64(sw_u(0xffffffffu) * 10 + sw_u(0x80000000u));
    print_i64(sw_nested(1) + sw_nested(2) * 10000 + sw_nested(3) * 1000000);
    print_i64(sw_nested(4));
    if (a[0] == 1) stop();
    return 0;
}

/* expect: 13 */
/* expect: 36 */
/* expect: 16 */
/* expect: 18 */
/* expect: 42 */
/* expect: 321 */
/* expect: 9 */
/* expect: 12345 */
/* expect: 0 */
/* expect: 12 */
/* expect: 54111012 */
/* expect: 44 */
