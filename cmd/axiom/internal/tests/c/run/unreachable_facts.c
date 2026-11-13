/* flags: --c-std=gnu23 */
/* What a path to unreachable() says (switches.ax: dead_default,
   known_interval; gvn.ax): the switches whose value it bounds, with a
   default no value reaches (all values in the bounds have cases), with
   values in the bounds that have none (they still go to the default), and
   with a default that is unreachable itself; a signed division of a value
   it says is not negative. Every value tried is one the program may pass. */
#include "host.h"
#include <stddef.h>

static volatile int vals[12] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -3, 41 };

[[gnu::noinline]] static int all_cases(int x) {
    if (x < 0 || x > 4)
        unreachable();
    switch (x) {
    case 0: return 10;
    case 1: return 22;
    case 2: return 31;
    case 3: return 47;
    case 4: return 5;
    default: return 99;  /* (no value gets here) */
    }
}

[[gnu::noinline]] static int gaps(int x) {
    if (x < 0 || x > 9)
        unreachable();
    switch (x) {
    case 0: return 10;
    case 1: return 22;
    case 2: return 31;
    case 3: return 47;
    case 4: return 5;
    case 6: return 60;
    case 7: return 70;
    case 8: return 80;
    default: return 99;  /* 5 and 9 */
    }
}

[[gnu::noinline]] static int dead_default(unsigned x) {
    switch (x) {
    case 3: return 1;
    case 4: return 4;
    case 5: return 9;
    case 6: return 16;
    case 7: return 25;
    default: unreachable();
    }
}

[[gnu::noinline]] static int jumps(int x, int *p) {
    if (x < 0 || x >= 6)
        unreachable();
    switch (x) {
    case 0: p[0] += 1; break;
    case 1: p[1] += 2; break;
    case 2: p[2] += 3; break;
    case 3: p[0] += 4; break;
    case 4: p[1] += 5; break;
    case 5: p[2] += 6; break;
    }
    return p[0] * 100 + p[1] * 10 + p[2];
}

[[gnu::noinline]] static int divide(int x) {
    if (x < 0)
        unreachable();
    return x / 10 * 1000 + x % 7 * 10 + x / 4;
}

int main(void) {
    int s = 0;
    for (int i = 0; i < 5; i++)
        s = s * 3 + all_cases(vals[i]);
    print_i64(s);
    s = 0;
    for (int i = 0; i < 10; i++)
        s = s * 2 + gaps(vals[i]);
    print_i64(s);
    s = 0;
    for (int i = 3; i < 8; i++)
        s = s * 2 + dead_default((unsigned)vals[i]);
    print_i64(s);
    int p[3] = { 0, 0, 0 };
    for (int i = 0; i < 6; i++)
        s = jumps(vals[i], p);
    print_i64(s);
    print_i64(divide(vals[11]) + divide(vals[9]));
    return 0;
}
/* expect: 1829 */
/* expect: 20491 */
/* expect: 141 */
/* expect: 579 */
/* expect: 4092 */
