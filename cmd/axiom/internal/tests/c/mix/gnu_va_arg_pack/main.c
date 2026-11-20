/* __builtin_va_arg_pack and __builtin_va_arg_pack_len in an extern inline
   wrapper of a variadic function (as the system's headers wrap printf and
   open for _FORTIFY_SOURCE): the wrapper forwards the caller's variable
   arguments, which only inlining each call gives it, so it is not used,
   and the calls are the external function's (the other unit's) with their
   own arguments; the wrapper's checks of a call (a function with the
   attribute error, in dead code once inlined) are not reported. */
/* uses: ../print.ax */
/* flags: --c-std=gnu11 */
#include "../print.h"
#if !__has_builtin(__builtin_va_arg_pack) || !__has_builtin(__builtin_va_arg_pack_len)
#error "no __builtin_va_arg_pack"
#endif

extern int sum(int n, ...);
extern int checked_sum(int n, ...);
extern void too_many(void) __attribute__((__error__("sum takes at most 4 numbers")));

extern __inline __attribute__((__always_inline__, __gnu_inline__)) int sum(int n, ...) {
    if (__builtin_va_arg_pack_len() > 4)
        too_many();
    return checked_sum(n, __builtin_va_arg_pack());
}

int main(void) {
    print_i64(sum(3, 1, 2, 3));
    print_i64(sum(1, 40));
    return 0;
}
/* expect: 6 */
/* expect: 40 */
