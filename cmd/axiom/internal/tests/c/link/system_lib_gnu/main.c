/* --c-lib=system in a GNU dialect: the system's headers define the GNU
   keywords away for a compiler that is not their own (__attribute__(x) as
   nothing); here they are keywords, and the program's attributes after
   those headers keep their meaning. __int128_t and __uint128_t name
   __int128 and unsigned __int128. */
/* flags: --c-lib=system --c-std=gnu17 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct __attribute__((packed)) rec { char c; int i; long long l; };
struct __attribute__((aligned(32))) wide { int x; };
static int ctor_ran;
__attribute__((constructor)) static void ctor(void) { ctor_ran = 1; }

int main(void) {
    printf("%zu %zu %zu\n", sizeof(struct rec), _Alignof(struct rec), _Alignof(struct wide)); /* expect: 13 1 32 */
    printf("%d\n", ctor_ran); /* expect: 1 */
    __extension__ long long v = 5;
    printf("%lld\n", v); /* expect: 5 */
    /* (the dialect's built-in type names, which the system's headers use) */
    __uint128_t w = (__uint128_t)1 << 100;
    __int128_t n = -7;
    printf("%d %d %zu\n", (int)(w >> 98), (int)n, sizeof(__uint128_t)); /* expect: 4 -7 16 */
    return 0;
}
