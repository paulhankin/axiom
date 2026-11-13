/* flags: --c-std=gnu23 */
/* __builtin_object_size of the objects an expression shows (a declared
   object, a string literal, a member, an element at a constant index; else
   unknown), __builtin_cpu_supports and __builtin_cpu_init, enumerations
   whose underlying type is __int128, case ranges of a switch on __int128,
   the attributes cold, hot, section and visibility, #pragma GCC
   visibility, aligned on a typedef, __builtin_expect, and
   __builtin_ia32_pause and the fences. */
#include "host.h"

struct S { int a; char b[10]; double d; };
struct T { int n; char tail[4]; };
static char buf[100];
enum W : __int128 { W0 = -5, W1, W2 = 0x7fffffffffffffffLL };
enum U : unsigned __int128 { U0 = 18446744073709551615ULL };
typedef int a16 __attribute__((aligned(16)));
struct A { char c; a16 v; };
#pragma GCC visibility push(hidden)
int hidden_value = 3;
#pragma GCC visibility pop
__attribute__((visibility("default"), section(".data.axiom_test"))) int placed = 4;

static int sw(__int128 v) {
    switch (v) {
    case 1 ... 10: return 1;
    case (__int128)1 << 70 ... ((__int128)1 << 70) + 5: return 2;
    case -3: return 3;
    case 20 ... 30: return 4;
    default: return 0;
    }
}

__attribute__((cold, noinline)) static int rare(int x) { return -x; }
__attribute__((hot)) static int often(int x) {
    if (__builtin_expect(x > 100, 0))
        return rare(x);
    return x * 2;
}

int main(void) {
    struct S s;
    struct T t;
    int arr[8];
    char *volatile pv = buf;
    char *p = pv;  /* (read back: where it points is not seen at any level) */
    print_u64(__builtin_object_size(buf, 0));
/* expect: 100 */
    print_u64(__builtin_object_size(buf + 10, 0) + __builtin_object_size(&buf[99], 1) * 1000);
/* expect: 1090 */
    print_u64(__builtin_object_size(buf + 200, 0));
/* expect: 0 */
    print_u64(__builtin_object_size(&s, 0) + __builtin_object_size(&s.b[2], 0) * 100);
/* expect: 1824 */
    print_u64(__builtin_object_size(&s.b[2], 1) + __builtin_object_size(s.b, 3) * 100);
/* expect: 1008 */
    print_u64(__builtin_object_size(t.tail, 1) + __builtin_object_size(&arr[3], 0) * 100);
/* expect: 2004 */
    print_u64(__builtin_object_size("hello", 0));
/* expect: 6 */
    print_i64(__builtin_object_size(p, 0) == (__typeof__(sizeof 0))-1);
/* expect: 1 */
    print_u64(__builtin_object_size(p, 2) + __builtin_dynamic_object_size((char *)&s + 4, 0));
/* expect: 20 */
    enum W w = W1;
    enum U u = U0;
    print_i64((long long)sizeof(enum W) + (int)w * 100);
/* expect: -384 */
    print_i64((long long)(W2 >> 32) + (long long)(u >> 32));
/* expect: 6442450942 */
    print_i64(sw(5) + sw(((__int128)1 << 70) + 3) * 10 + sw(-3) * 100 + sw(25) * 1000);
/* expect: 4321 */
    print_i64(sw(0) + sw(((__int128)1 << 70) + 6) + sw(10) * 10 + sw(11) * 100);
/* expect: 10 */
    __builtin_cpu_init();
#if defined(__AXIOM_ASM_X86_64__)
    print_i64(__builtin_cpu_supports("sse2") && !__builtin_cpu_supports("lse") && __builtin_cpu_supports("avx2") >= 0);
#else
    print_i64(!__builtin_cpu_supports("sse2") && __builtin_cpu_supports("x86-64-v3") == 0);
#endif
/* expect: 1 */
    print_i64(often(3) + often(200));
/* expect: -194 */
    print_i64(_Alignof(a16) + sizeof(struct A) * 100);
/* expect: 3216 */
    print_i64(hidden_value + placed * 10);
/* expect: 43 */
    __builtin_ia32_pause();
    __builtin_ia32_mfence();
    __builtin_ia32_lfence();
    __builtin_ia32_sfence();
    return 0;
}
