/* A generated maze of labels and gotos into and out of while, for and do
   loops, switch bodies and if arms, forwards and backwards. */
#include "host.h"
static long long f0(long long x, long long y) {
    long long s = 0; int steps = 0; int c0 = 0, c1 = 0, c2 = 0, c3 = 0, c4 = 0, c5 = 0, c6 = 0, c7 = 0, c8 = 0, c9 = 0;
    if (x & 1) goto L5;
    if (y & 2) goto L2;
    for (c0 = 0; c0 < 2; c0++) { s += c0;
L0:
    if (++steps > 300) return s;
    s = s * 7 + x - 8; x ^= s >> 3; y += 1;
    if (s & 64) continue; if (s & 128) break; }
    for (c1 = 0; c1 < 2; c1++) { s += c1;
L1:
    if (++steps > 300) return s;
    s = s * 6 + x - 0; x ^= s >> 3; y += 2;
    if ((s ^ y) & 4) goto L3;
    if (s & 64) continue; if (s & 128) break; }
    for (c2 = 0; c2 < 2; c2++) { s += c2;
L2:
    if (++steps > 300) return s;
    s = s * 4 + x - 8; x ^= s >> 3; y += 1;
    switch ((int)(s & 3)) { case 0: goto L0; case 1: s++; break; case 2: goto L5; }
    if (s & 64) continue; if (s & 128) break; }
    for (c3 = 0; c3 < 2; c3++) { s += c3;
L3:
    if (++steps > 300) return s;
    s = s * 4 + x - 4; x ^= s >> 3; y += 2;
    if (s & 64) continue; if (s & 128) break; }
    for (c4 = 0; c4 < 2; c4++) { s += c4;
L4:
    if (++steps > 300) return s;
    s = s * 7 + x - 1; x ^= s >> 3; y += 2;
    if (s & 64) continue; if (s & 128) break; }
    for (c5 = 0; c5 < 2; c5++) { s += c5;
L5:
    if (++steps > 300) return s;
    s = s * 2 + x - 0; x ^= s >> 3; y += 1;
    switch ((int)(s & 3)) { case 0: goto L6; case 1: s++; break; case 2: goto L1; }
    if (s & 64) continue; if (s & 128) break; }
    for (c6 = 0; c6 < 2; c6++) { s += c6;
L6:
    if (++steps > 300) return s;
    s = s * 7 + x - 2; x ^= s >> 3; y += 3;
    if (s & 64) continue; if (s & 128) break; }
    return s;
}
static long long f1(long long x, long long y) {
    long long s = 0; int steps = 0; int c0 = 0, c1 = 0, c2 = 0, c3 = 0, c4 = 0, c5 = 0, c6 = 0, c7 = 0, c8 = 0, c9 = 0;
    if (x & 1) goto L1;
    if (y & 2) goto L5;
    for (c0 = 0; c0 < 2; c0++) { s += c0;
L0:
    if (++steps > 300) return s;
    s = s * 2 + x - 6; x ^= s >> 3; y += 3;
    if ((s ^ y) & 4) goto L3;
    if (s & 64) continue; if (s & 128) break; }
    c1 = 0; do {
L1:
    if (++steps > 300) return s;
    s = s * 2 + x - 4; x ^= s >> 3; y += 1;
    } while (++c1 < 2);
    c2 = 0; do {
L2:
    if (++steps > 300) return s;
    s = s * 7 + x - 9; x ^= s >> 3; y += 5;
    if ((s ^ y) & 4) goto L0;
    } while (++c2 < 2);
    for (c3 = 0; c3 < 2; c3++) { s += c3;
L3:
    if (++steps > 300) return s;
    s = s * 3 + x - 4; x ^= s >> 3; y += 4;
    if (s & 64) continue; if (s & 128) break; }
    c4 = 0; do {
L4:
    if (++steps > 300) return s;
    s = s * 4 + x - 7; x ^= s >> 3; y += 2;
    if ((s ^ y) & 4) goto L0;
    } while (++c4 < 2);
    c5 = 0; do {
L5:
    if (++steps > 300) return s;
    s = s * 7 + x - 6; x ^= s >> 3; y += 1;
    } while (++c5 < 2);
    return s;
}
static long long f2(long long x, long long y) {
    long long s = 0; int steps = 0; int c0 = 0, c1 = 0, c2 = 0, c3 = 0, c4 = 0, c5 = 0, c6 = 0, c7 = 0, c8 = 0, c9 = 0;
    if (x & 1) goto L0;
    if (y & 2) goto L7;
    switch ((int)(y & 3)) { case 0: s += 3;
L0:
    if (++steps > 300) return s;
    s = s * 1 + x - 2; x ^= s >> 3; y += 5;
    switch ((int)(s & 3)) { case 0: goto L1; case 1: s++; break; case 2: goto L3; }
    break; case 1: s ^= 5; goto L5; default: s += 1; }
    switch ((int)(y & 3)) { case 0: s += 3;
L1:
    if (++steps > 300) return s;
    s = s * 5 + x - 5; x ^= s >> 3; y += 5;
    break; case 1: s ^= 5; goto L1; default: s += 1; }
    for (c2 = 0; c2 < 2; c2++) { s += c2;
L2:
    if (++steps > 300) return s;
    s = s * 5 + x - 5; x ^= s >> 3; y += 3;
    if ((s ^ y) & 8) goto L1;
    if (s & 64) continue; if (s & 128) break; }
    for (c3 = 0; c3 < 2; c3++) { s += c3;
L3:
    if (++steps > 300) return s;
    s = s * 3 + x - 8; x ^= s >> 3; y += 5;
    if (s & 64) continue; if (s & 128) break; }
    c4 = 0; do {
L4:
    if (++steps > 300) return s;
    s = s * 3 + x - 4; x ^= s >> 3; y += 5;
    if ((s ^ y) & 4) goto L5;
    } while (++c4 < 2);
    switch ((int)(y & 3)) { case 0: s += 3;
L5:
    if (++steps > 300) return s;
    s = s * 2 + x - 1; x ^= s >> 3; y += 2;
    break; case 1: s ^= 5; goto L2; default: s += 1; }
    c6 = 0; while (c6 < 3) { c6++;
L6:
    if (++steps > 300) return s;
    s = s * 6 + x - 0; x ^= s >> 3; y += 1;
    }
    switch ((int)(y & 3)) { case 0: s += 3;
L7:
    if (++steps > 300) return s;
    s = s * 2 + x - 9; x ^= s >> 3; y += 3;
    break; case 1: s ^= 5; goto L6; default: s += 1; }
    c8 = 0; do {
L8:
    if (++steps > 300) return s;
    s = s * 2 + x - 0; x ^= s >> 3; y += 1;
    } while (++c8 < 2);
    return s;
}
int main(void) {
    long long t = 0;
    for (long long a = 0; a < 10; a++) for (long long b = 0; b < 6; b++) { t = t * 31 + f0(a, b) + f1(b, a) * 3 + f2(a + b, a - b); }
    print_i64(t);
    return 0;
}

/* expect: -1181935893303191961 */
