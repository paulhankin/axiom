/* flags: --c-std=c23 */
/* C23 constexpr objects: named constants usable in integer constant
   expressions (array sizes, case labels, static assertions, other
   constants' initializers), members of constexpr structures, floating
   constants, null pointers, strings, at file and block scope. */
#include "host.h"

constexpr int K = 6;
constexpr double D = 2.5;
constexpr unsigned char UC = 255;
constexpr int *NP = nullptr;
constexpr char NAME[] = "axiom";
struct P { int x, y; struct { short lo, hi; } in; };
constexpr struct P CP = { 3, 4, { .hi = 9 } };
constexpr int K2 = K * CP.y;
static int arr[K];
static int arr2[CP.in.hi];
static int sized = K2;
enum E { E0 = K + 1 };

int main(void) {
    constexpr int local = K * 2;
    static constexpr long long sl = 1LL << 40;
    int la[local];
    print_i64(sizeof arr / sizeof arr[0]);
    print_i64(sizeof arr2 / sizeof arr2[0]);
    print_i64(sizeof la / sizeof la[0]);
    print_i64(sized + E0);
    print_i64(D * 2);
    print_i64(CP.x + CP.in.lo + CP.in.hi);
    print_i64(UC + 1);
    print_i64(NP == 0);
    print_i64(sizeof NAME + NAME[1]);
    print_i64(sl >> 38);
    switch (5) {
    case K - 1:
        print_i64(51);
        break;
    default:
        print_i64(0);
    }
    static_assert(K == 6 && CP.y == 4);
    return 0;
}
/* expect: 6 */
/* expect: 9 */
/* expect: 12 */
/* expect: 31 */
/* expect: 5 */
/* expect: 12 */
/* expect: 256 */
/* expect: 1 */
/* expect: 126 */
/* expect: 4 */
/* expect: 51 */
