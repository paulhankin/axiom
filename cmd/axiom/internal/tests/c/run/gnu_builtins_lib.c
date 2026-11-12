/* flags: --c-std=gnu11 */
/* native-only */
/* The GNU builtins that are calls of the C library's functions of their
   names (strlen, memcmp, pow, labs ...), and fma (an IR operation natively;
   the library's on WebAssembly). */
#include "host.h"

int main(void) {
    char buf[8] = "axxyaaa";
    print_i64(__builtin_strlen(buf) + 100 * (__builtin_memcmp(buf, "axxya", 5) == 0));
    print_i64(__builtin_strcmp("abc", "abd") < 0);
    print_f64(__builtin_pow(2.0, 10.0) + __builtin_fmax(1.0, 2.0) + __builtin_round(2.5));
    print_f64(__builtin_fma(2.0, 3.0, 1.0));
    print_i64(__builtin_labs(-8L) + __builtin_llabs(-9LL));
    char *p = __builtin_strchr(buf, 'y');
    print_i64(p - buf);
    return 0;
}
/* expect: 107 */
/* expect: 1 */
/* expect: 1029 */
/* expect: 7 */
/* expect: 17 */
/* expect: 3 */
