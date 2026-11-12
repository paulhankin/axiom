/* flags: --c-std=gnu11 */
/* expect-warning: C4050 */
/* expect-warning: C4051 */
/* expect-warning: C4052 */
/* The GNU attributes, in their places: aligned, packed, section, used,
   unused, weak, alias, visibility, noreturn, noinline, always_inline, cold,
   hot, const, pure, malloc, format, nonnull, warn_unused_result,
   deprecated, constructor, destructor (with priorities), cleanup,
   transparent_union, mode, and [[gnu::...]] in a GNU dialect. */
#include "host.h"

typedef int si __attribute__((mode(SI)));
typedef unsigned int u8m __attribute__((__mode__(__QI__)));
typedef int dim __attribute__((mode(DI)));
static int counter __attribute__((section(".data.counters"), used)) = 4;
static int unused_one __attribute__((unused));
int weak_defined(void) __attribute__((weak));
int weak_defined(void) { return 11; }
static int target_fn(int x) { return x * 3; }
static int alias_fn(int) __attribute__((alias("target_fn")));
int hidden_fn(void) __attribute__((visibility("hidden")));
int hidden_fn(void) { return 2; }
__attribute__((noreturn)) static void stop(void) { for (;;) { } }
__attribute__((noinline, cold)) static int slow(int x) { return x + 1; }
static inline __attribute__((always_inline, hot)) int fast(int x) { return x * 2; }
__attribute__((const)) static int sq(int x) { return x * x; }
__attribute__((pure)) static int rd(const int *p) { return *p; }
__attribute__((malloc)) static void *mk(void) { static char b[4]; return b; }
__attribute__((format(printf, 1, 2))) static int fmt(const char *f, ...) { (void)f; return 1; }
__attribute__((nonnull(1))) static int deref(int *p) { return *p; }
__attribute__((warn_unused_result)) static int important(void) { return 5; }
__attribute__((deprecated("use new"))) static int old_api(void) { return 6; }
[[gnu::noinline]] static int scoped(void) { return 7; }
__attribute__((nosuch_attribute)) static int unknown_attr = 1;

static int order[8];
static int norder;
__attribute__((constructor)) static void c_default(void) { order[norder++] = 3; }
__attribute__((constructor(101))) static void c_first(void) { order[norder++] = 1; }
__attribute__((constructor(200))) static void c_second(void) { order[norder++] = 2; }
__attribute__((destructor(150))) static void d_last(void) { print_i64(1500); }
__attribute__((destructor)) static void d_first(void) { print_i64(65535); }

typedef union { int *ip; long *lp; } __attribute__((transparent_union)) intptr_u;
static int take(intptr_u u) { return *u.ip; }

static int cleaned;
static void bye(int *p) { cleaned += *p; }

int main(void) {
    print_i64(sizeof(si) * 100 + sizeof(u8m) * 10 + sizeof(dim));
    print_i64(counter + unused_one);
    print_i64(weak_defined());
    print_i64(alias_fn(4));
    print_i64(hidden_fn());
    if (counter == 0)
        stop();
    print_i64(slow(1) + fast(2) + sq(3) + rd(&counter) + (mk() != 0) + fmt("%d", 1));
    print_i64(deref(&counter));
    int keep = important();
    print_i64(keep);
    important();
    print_i64(old_api());
    print_i64(scoped() + unknown_attr);
    print_i64(order[0] * 100 + order[1] * 10 + order[2]);
    int v = 9;
    print_i64(take(&v));
    {
        int c __attribute__((cleanup(bye))) = 3;
        (void)c;
    }
    for (int i = 0; i < 3; i++) {
        int d __attribute__((cleanup(bye))) = 10;
        if (i == 1)
            continue;
        (void)d;
    }
    print_i64(cleaned);
    struct __attribute__((packed)) { char c; long long l; } pk = { 1, 0x123456789LL };
    print_i64(sizeof(pk) * 1000 + _Alignof(__typeof__(pk)));
    print_i64(pk.l);
    int aligned_local __attribute__((aligned(64))) = 1;
    print_i64(((size_t)&aligned_local % 64) + aligned_local);
    return 0;
}
/* expect: 418 */
/* expect: 4 */
/* expect: 11 */
/* expect: 12 */
/* expect: 2 */
/* expect: 21 */
/* expect: 4 */
/* expect: 5 */
/* expect: 6 */
/* expect: 8 */
/* expect: 123 */
/* expect: 9 */
/* expect: 33 */
/* expect: 9001 */
/* expect: 4886718345 */
/* expect: 1 */
/* expect: 65535 */
/* expect: 1500 */
