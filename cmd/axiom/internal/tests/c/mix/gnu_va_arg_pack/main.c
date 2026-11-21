/* __builtin_va_arg_pack and __builtin_va_arg_pack_len in an extern inline
   wrapper of a variadic function (as the system's headers wrap printf and
   open for _FORTIFY_SOURCE): each call of the function inlines the
   wrapper, whose call that ends in __builtin_va_arg_pack () gets the
   call's variable arguments there (evaluated once, with the others), and
   __builtin_va_arg_pack_len () is their number. A wrapper's check of a
   call (a function with the attribute error, or one defined nowhere) in
   code that inlining leaves never run is not reported, and is not linked,
   at every optimization level; the function's address is the external
   function's (the other unit's). */
/* uses: ../print.ax */
/* flags: --c-std=gnu11 */
#include "../print.h"
#if !__has_builtin(__builtin_va_arg_pack) || !__has_builtin(__builtin_va_arg_pack_len)
#error "no __builtin_va_arg_pack"
#endif

struct pt { int x, y; };

extern int sum(int n, ...);
extern int checked_sum(int n, ...);
extern void too_many(void) __attribute__((__error__("sum takes at most 4 numbers")));

extern __inline __attribute__((__always_inline__, __gnu_inline__)) int sum(int n, ...) {
    if (__builtin_va_arg_pack_len() > 4)
        too_many();
    return checked_sum(n, __builtin_va_arg_pack());
}

/* the number of variable arguments, a constant at each call */
extern int count(int first, ...);
extern __inline __attribute__((__gnu_inline__)) int count(int first, ...) {
    return __builtin_va_arg_pack_len() * 100 + first;
}

/* a wrapper that forwards to another: the outer call's variable arguments
   reach the inner wrapper's call */
extern int sum2(int n, ...);
extern __inline __attribute__((__gnu_inline__)) int sum2(int n, ...) {
    int k = n + 1;
    return sum(k - 1, __builtin_va_arg_pack());
}

/* variable arguments of several types, a structure among them */
extern double mix(int n, ...);
extern double mixed(const char *how, ...);
extern __inline __attribute__((__gnu_inline__)) double mixed(const char *how, ...) {
    return mix(how[0] - '0', __builtin_va_arg_pack());
}

/* __builtin_constant_p of a parameter: decided once the call is inlined;
   the code it leaves never run calls a function defined nowhere */
extern void never_defined(void);
extern int pick(int mode, ...);
extern __inline __attribute__((__gnu_inline__)) int pick(int mode, ...) {
    if (__builtin_constant_p(mode) && mode > 100)
        never_defined();
    if (__builtin_va_arg_pack_len() < 1)
        return checked_sum(0);
    return checked_sum(mode, __builtin_va_arg_pack());
}

/* no result */
extern void note(int n, ...);
extern int noted;
extern __inline __attribute__((__gnu_inline__)) void note(int n, ...) {
    if (n == 0)
        return;
    noted = checked_sum(n, __builtin_va_arg_pack());
}

static int calls;
static int next(void) { return ++calls; }

int main(void) {
    print_i64(sum(3, 1, 2, 3));
    print_i64(sum(1, 40));
    print_i64(sum(0));
    print_i64(count(7));
    print_i64(count(7, 1, 2.5, "x"));
    print_i64(sum2(2, 5, 6));
    print_i64(sum(2, next(), next()));
    print_i64(calls);
    struct pt p = { 3, 4 };
    print_i64((long long)(mixed("4", 0.5, 7LL, "abc", p) * 2));
    volatile int m = 2;
    print_i64(pick(m, 8, 9));
    print_i64(pick(2, 8, 9));
    print_i64(pick(50));
    note(0, 5);
    print_i64(noted);
    note(1, 5);
    print_i64(noted);
    int (*fp)(int, ...) = sum;
    print_i64(fp(2, 1, 1));
    return 0;
}
/* expect: 1006 */
/* expect: 1040 */
/* expect: 1000 */
/* expect: 7 */
/* expect: 307 */
/* expect: 1011 */
/* expect: 1003 */
/* expect: 2 */
/* expect: 89 */
/* expect: 1017 */
/* expect: 1017 */
/* expect: 1000 */
/* expect: 0 */
/* expect: 1005 */
/* expect: 2 */
