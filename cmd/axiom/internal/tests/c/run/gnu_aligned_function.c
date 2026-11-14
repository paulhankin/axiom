/* flags: --c-std=gnu17 */
/* In a GNU dialect the attribute aligned(n) on a function says where its
   code starts, a multiple of n, and __alignof__ of the function is n (1
   for one without it). The attribute may be on a declaration before the
   definition. (WebAssembly's functions have no addresses in memory: there
   only __alignof__.) */
#include "host.h"
void first(void) __attribute__((aligned(128)));
void first(void) {}
__attribute__((aligned(64))) static int second(int x) { return x + 1; }
int plain(void) { return 0; }
int main(void) {
    print_i64(__alignof__(first));
    print_i64(__alignof__(second));
    print_i64(__alignof__(plain));
#ifdef __wasm__
    print_i64(0);
    print_i64(0);
#else
    print_i64((__UINTPTR_TYPE__)first % 128);
    print_i64((__UINTPTR_TYPE__)second % 64);
#endif
    print_i64(second(1));
    return 0;
}
/* expect: 128 */
/* expect: 64 */
/* expect: 1 */
/* expect: 0 */
/* expect: 0 */
/* expect: 2 */
