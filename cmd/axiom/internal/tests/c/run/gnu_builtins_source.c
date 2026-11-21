/* flags: --c-std=gnu17 */
/* The GNU dialects' __builtin_LINE, __builtin_FILE and __builtin_FUNCTION
   (the presumed line and file of the call, the function it is in),
   __builtin___clear_cache (the instruction cache made to see what was
   written: nothing where it always does, on AArch64 the caches' lines
   cleaned), __builtin_speculation_safe_value (no speculation is tracked:
   the value) and __builtin_bswap128. */
#include "host.h"
#if !__has_builtin(__builtin_LINE) || !__has_builtin(__builtin_FILE) || !__has_builtin(__builtin_FUNCTION) || \
    !__has_builtin(__builtin___clear_cache) || !__has_builtin(__builtin_speculation_safe_value) || \
    !__has_builtin(__builtin_bswap128)
#error "a builtin is missing"
#endif

static size_t len(const char *s) {
    size_t n = 0;
    while (s[n]) n++;
    return n;
}

static void str(const char *s) { print_str(s, len(s)); }

static const char *which(void) { return __builtin_FUNCTION(); }
static const char *at_file = __builtin_FUNCTION();

static int line_of_call(int line) { return line; }

int main(void) {
    print_i64(__builtin_LINE());
    print_i64(line_of_call(__builtin_LINE()));
    str(which());
    str(__builtin_FUNCTION());
    print_i64(len(at_file));
    const char *f = __builtin_FILE();
    for (const char *q = f; *q; q++)
        if (*q == '/' || *q == '\\') f = q + 1;
    str(f);
#line 500 "elsewhere.c"
    print_i64(__builtin_LINE());
    str(__builtin_FILE());
    print_i64(_Generic(__builtin_FILE(), const char *: 1, default: 0) + _Generic(__builtin_LINE(), int: 2, default: 0));

    static unsigned char code[64];
    for (int i = 0; i < 64; i++) code[i] = (unsigned char)i;
    __builtin___clear_cache((char *)code, (char *)code + sizeof code);
    print_i64(code[63]);

    volatile int k = 41;
    int v = __builtin_speculation_safe_value(k + 1);
    int *p = __builtin_speculation_safe_value(&v, (int *)0);
    print_i64(*p + __builtin_speculation_safe_value(k, 0));

    unsigned __int128 x = ((unsigned __int128)0x0102030405060708ULL << 64) | 0x090a0b0c0d0e0f10ULL;
    unsigned __int128 y = __builtin_bswap128(x);
    print_u64((unsigned long long)(y >> 64));
    print_u64((unsigned long long)y);
    print_bool(__builtin_bswap128(y) == x);
    return 0;
}
/* expect: 29 */
/* expect: 30 */
/* expect: which */
/* expect: main */
/* expect: 0 */
/* expect: gnu_builtins_source.c */
/* expect: 500 */
/* expect: elsewhere.c */
/* expect: 3 */
/* expect: 63 */
/* expect: 83 */
/* expect: 1157159078456920585 */
/* expect: 578437695752307201 */
/* expect: true */
