/* flags: --c-std=c11 */
/* C11 features in use together: _Generic for type-generic math and
   printing macros (on qualified operands, arrays, function designators and
   expressions of every arithmetic type), anonymous structures and unions
   nested inside each other, _Static_assert at file, block and member
   scope, _Alignas and _Alignof on objects and members, _Noreturn on a
   function reached through longjmp-free control flow, u8 and u/U string
   literals and char16_t/char32_t, and <stdalign.h>/<stdnoreturn.h>. The
   workload: a small geometry library over a variant vector type, a
   type-tagged value stack, and a ring of aligned buffers. */
#include "host.h"
#include <stdint.h>
#include <stddef.h>
#include <stdalign.h>
#include <stdnoreturn.h>
#include <stdbool.h>

_Static_assert(sizeof(int32_t) == 4, "int32_t has 32 bits");
_Static_assert(_Alignof(max_align_t) >= _Alignof(double), "max_align_t is the strictest");

/* ---- _Generic ---- */

#define TYPE_ID(x) _Generic((x),                                                                                  \
    _Bool: 1, char: 2, signed char: 3, unsigned char: 4, short: 5, unsigned short: 6, int: 7, unsigned: 8,          \
    long: 9, unsigned long: 10, long long: 11, unsigned long long: 12, float: 13, double: 14, long double: 15,       \
    char *: 16, const char *: 17, int *: 18, default: 0)

static int64_t abs_ll(long long x) { return x < 0 ? -x : x; }
static double abs_d(double x) { return x < 0 ? -x : x; }
static float abs_f(float x) { return x < 0 ? -x : x; }
#define ABS(x) _Generic((x), float: abs_f, double: abs_d, default: abs_ll)(x)

static int64_t max_i(int64_t a, int64_t b) { return a > b ? a : b; }
static double max_d(double a, double b) { return a > b ? a : b; }
#define MAX(a, b) _Generic((a) + (b), float: max_d, double: max_d, default: max_i)(a, b)

static uint64_t out_hash = 1469598103934665603ull;

static void show_i(long long v) { print_i64(v); out_hash = (out_hash ^ (uint64_t)v) * 1099511628211ull; }
static void show_u(unsigned long long v) { print_u64(v); out_hash = (out_hash ^ v) * 1099511628211ull; }
static void show_d(double v) { print_f64(v); out_hash = (out_hash ^ (uint64_t)(int64_t)(v * 1024)) * 1099511628211ull; }
static void show_s(const char *s) {
    size_t n = 0;
    while (s[n]) n++;
    print_str(s, n);
    for (size_t i = 0; i < n; i++) out_hash = (out_hash ^ (unsigned char)s[i]) * 1099511628211ull;
}
#define SHOW(x) _Generic((x),                                                                                      \
    float: show_d, double: show_d,                                                                                  \
    unsigned: show_u, unsigned long: show_u, unsigned long long: show_u,                                            \
    char *: show_s, const char *: show_s,                                                                           \
    default: show_i)(x)

/* ---- anonymous members ---- */

typedef struct vec3 {
    union {
        struct { float x, y, z; };
        struct { float r, g, b; };
        float v[3];
    };
} vec3;

_Static_assert(offsetof(vec3, y) == sizeof(float), "anonymous members share the layout");
_Static_assert(offsetof(vec3, b) == offsetof(vec3, z), "aliases");

static vec3 v_add(vec3 a, vec3 b) { return (vec3){ { { a.x + b.x, a.y + b.y, a.z + b.z } } }; }
static vec3 v_scale(vec3 a, float k) {
    for (int i = 0; i < 3; i++) a.v[i] *= k;
    return a;
}
static float v_dot(vec3 a, vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
static vec3 v_cross(vec3 a, vec3 b) {
    vec3 r;
    r.x = a.y * b.z - a.z * b.y;
    r.y = a.z * b.x - a.x * b.z;
    r.z = a.x * b.y - a.y * b.x;
    return r;
}

/* a variant: anonymous union of anonymous structs with a tag */
struct value {
    enum { V_INT, V_REAL, V_PAIR, V_TEXT } tag;
    union {
        long long i;
        double d;
        struct { int first, second; };
        struct { const char *text; size_t len; };
    };
};

static struct value v_int(long long i) { return (struct value){ .tag = V_INT, .i = i }; }
static struct value v_real(double d) { return (struct value){ .tag = V_REAL, .d = d }; }
static struct value v_pair(int a, int b) { return (struct value){ .tag = V_PAIR, .first = a, .second = b }; }
static struct value v_text(const char *s) {
    struct value v = { .tag = V_TEXT, .text = s };
    while (s[v.len]) v.len++;
    return v;
}

#define V(x) _Generic((x), int: v_int, long long: v_int, double: v_real, float: v_real, char *: v_text, \
                      const char *: v_text)(x)

static double as_real(struct value v) {
    switch (v.tag) {
    case V_INT: return (double)v.i;
    case V_REAL: return v.d;
    case V_PAIR: return v.first + v.second / 1000.0;
    case V_TEXT: return (double)v.len;
    }
    return 0;
}

/* a typed stack */
struct stack {
    _Alignas(16) struct value items[32];
    int n;
};

static void push(struct stack *s, struct value v) { s->items[s->n++] = v; }
static struct value pop(struct stack *s) { return s->items[--s->n]; }

static noreturn void fail_forever(volatile int *flag) {
    for (;;)
        if (*flag) *flag = 0;
}

/* ---- alignment ---- */

struct aligned_bufs {
    char tag;
    alignas(64) unsigned char a[48];
    char gap;
    _Alignas(32) unsigned char b[16];
    _Alignas(double) char c;
};

_Static_assert(_Alignof(struct aligned_bufs) == 64, "the strictest member");
_Static_assert(offsetof(struct aligned_bufs, a) == 64, "a is at 64");
_Static_assert(offsetof(struct aligned_bufs, b) == 128, "b is at 128");

static struct aligned_bufs ring[3];
static alignas(128) char big_aligned[256];

int main(void) {
    {
        _Static_assert(TYPE_ID('a') == 7, "a character constant is an int");
        const volatile short cvs = 3;
        char arr[4] = "abc";
        const char *cp = arr;
        show_i(TYPE_ID(cvs));         /* lvalue conversion drops the qualifiers */
        show_i(TYPE_ID(arr));         /* arrays decay */
        show_i(TYPE_ID(cp));
        show_i(TYPE_ID(&arr[0]));
        show_i(TYPE_ID(cvs + 1));
        show_i(TYPE_ID((unsigned char)1 + (unsigned char)2));
        show_i(TYPE_ID(1u + 1LL));
        show_i(TYPE_ID(1ull * 2));
        show_i(TYPE_ID(1.0f * 2));
        show_i(TYPE_ID(1.0f * 2.0));
        show_i(TYPE_ID((_Bool)5));
        show_i(TYPE_ID(main));        /* function designator: a pointer, so default */
        show_i(TYPE_ID(1 ? (char)1 : (short)2));
        show_i(TYPE_ID(+(signed char)1));
        long double ld = 1;
        show_i(TYPE_ID(ld));
    }
    SHOW(ABS(-5));
    SHOW(ABS(-2.5));
    SHOW(ABS(-1.25f));
    SHOW(ABS(-9000000000LL));
    SHOW(MAX(3, 7));
    SHOW(MAX(3, 7.5));
    SHOW(MAX(-1, -2));
    SHOW(42u);
    SHOW("text through _Generic");
    SHOW(18446744073709551615ull);
    {
        vec3 a = { { { 1, 2, 3 } } }, b = { .v = { 4, 5, 6 } };
        vec3 c = v_cross(a, b);
        SHOW(c.x);
        SHOW(c.g);
        SHOW(c.v[2]);
        SHOW(v_dot(a, b));
        vec3 d = v_scale(v_add(a, c), 0.5f);
        SHOW(d.r + d.g + d.b);
        vec3 path = { 0 };
        for (int i = 0; i < 100; i++) {
            vec3 step = { .x = (float)(i % 7) - 3, .y = (float)(i % 5) * 0.5f, .z = -(float)(i % 3) };
            path = v_add(path, v_scale(step, 0.25f));
        }
        SHOW(path.x * 1000 + path.y * 100 + path.z);
    }
    {
        struct stack s = { .n = 0 };
        push(&s, V(10));
        push(&s, V(2.5));
        push(&s, V("hello"));
        push(&s, v_pair(3, 250));
        push(&s, V(5000000000LL));
        push(&s, V(0.125f));
        double total = 0;
        int tags = 0;
        while (s.n) {
            struct value v = pop(&s);
            total += as_real(v);
            tags = tags * 4 + (int)v.tag;
        }
        SHOW(total);
        SHOW(tags);
        SHOW((long long)((uintptr_t)&s.items[0] % 16));
        SHOW((long long)_Alignof(struct stack));
    }
    {
        SHOW((long long)((uintptr_t)&ring[1] % 64));
        SHOW((long long)((uintptr_t)ring[2].b % 32));
        SHOW((long long)((uintptr_t)big_aligned % 128));
        SHOW((long long)sizeof(struct aligned_bufs));
        alignas(16) int local[3] = { 1, 2, 3 };
        SHOW((long long)((uintptr_t)local % 16 + local[2]));
        SHOW((long long)alignof(char));
        SHOW((long long)alignof(int[5]) == (long long)alignof(int));
    }
    {
        const char *s8 = u8"utf-8: é中";
        SHOW(s8);
        size_t n = 0;
        while (s8[n]) n++;
        SHOW((long long)n);
        const unsigned short *s16 = u"é中\U0001F600";
        unsigned sum16 = 0;
        int len16 = 0;
        for (; s16[len16]; len16++) sum16 += s16[len16];
        SHOW(len16);
        SHOW(sum16);
        const unsigned int *s32 = U"\U0001F600x";
        SHOW(s32[0]);
        SHOW((long long)(sizeof U"ab" / sizeof(unsigned int)));
        SHOW((long long)u'é' + (long long)U'\U0001F600');
    }
    volatile int flag = 0;
    if (flag) fail_forever(&flag);
    print_u64(out_hash);
    return 0;
}

/* expect: 5 */
/* expect: 16 */
/* expect: 17 */
/* expect: 16 */
/* expect: 7 */
/* expect: 7 */
/* expect: 11 */
/* expect: 12 */
/* expect: 13 */
/* expect: 14 */
/* expect: 1 */
/* expect: 0 */
/* expect: 7 */
/* expect: 7 */
/* expect: 15 */
/* expect: 5 */
/* expect: 2.5 */
/* expect: 1.25 */
/* expect: 9000000000 */
/* expect: 7 */
/* expect: 7.5 */
/* expect: -1 */
/* expect: 42 */
/* expect: text through _Generic */
/* expect: 18446744073709551615 */
/* expect: -3 */
/* expect: 6 */
/* expect: -3 */
/* expect: 32 */
/* expect: 3 */
/* expect: 1225.25 */
/* expect: 5000000020.875 */
/* expect: 1204 */
/* expect: 0 */
/* expect: 16 */
/* expect: 0 */
/* expect: 0 */
/* expect: 0 */
/* expect: 192 */
/* expect: 3 */
/* expect: 1 */
/* expect: 1 */
/* expect: utf-8: é中 */
/* expect: 12 */
/* expect: 4 */
/* expect: 132435 */
/* expect: 128512 */
/* expect: 3 */
/* expect: 128745 */
/* expect: 1974616929660489066 */
