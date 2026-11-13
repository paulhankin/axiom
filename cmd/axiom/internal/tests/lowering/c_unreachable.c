/* What unreachable() lets the code leave out (switches.ax, gvn.ax): the
   test that leads to it (none of the functions compares), a table's range
   check when the value is bounded, or the default is unreachable, and a
   jump table's; a division of a value that is not negative is unsigned
   (no sign fix-up: sar). */
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding --c-std=gnu23 | \bcmp\b | 0
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding --c-std=gnu23 | \bud2\b | 0
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding --c-std=gnu23 | \bsar\b | 0
// lower: --target=aarch64 -O2 --c-freestanding --c-std=gnu23 | \bcmp\b | 0
#include <stddef.h>
int bounded(unsigned x) { if (x > 10) unreachable(); return x > 20; }
int table(unsigned x) {
    if (x >= 4)
        unreachable();
    switch (x) { case 0: return 10; case 1: return 22; case 2: return 31; case 3: return 47; default: return 99; }
}
int no_default(int x) {
    switch (x) {
    case 0: return 10; case 1: return 22; case 2: return 31; case 3: return 47; case 4: return 5;
    default: unreachable();
    }
}
void jumps(int x, int *p) {
    if (x < 0 || x > 5)
        unreachable();
    switch (x) {
    case 0: p[0]++; break; case 1: p[3]++; break; case 2: p[1] += 5; break;
    case 3: p[2]--; break; case 4: p[7] = 3; break; case 5: p[5]++; break;
    }
}
int divide(int x) { if (x < 0) unreachable(); return x / 10 + x % 7; }
