/* The variables of a C program in its debug information (run.mjs, "debug
   information"; dwarf.mjs reads it): block scopes (lexical blocks, a name
   declared again inside), typedefs, a block-scope static, structure and
   typedef declared in a block (and one in such a structure's members), a
   variable length array (its bound),
   a thread-local object, and values a debugger reads at -O1 and above
   (a parameter, a variable assigned once, one whose address is taken; one
   assigned again and again, a structure taken apart, a pointer into a
   frame object, a variable length array), a function inlined (its
   inline instance, its variables), and one called through a pointer with
   constants (its call site's parameters; its own parameters' entry
   values); its macros, one from a header of its own (.debug_macro). */
#include "vars_macros.h"                     /* line: macro-include */
#define VARS_DEPTH 3                         /* line: macro-def */
typedef int count_t;
typedef count_t *count_ptr;
_Thread_local int per_thread = 3;
static volatile int sink;

static void use(int *p) { sink = *p; }

int shadow(count_t n) {
    count_t x = n + 1;
    count_ptr px = &x;
    for (int i = 0; i < n; i++) {
        int x = i * 2;                           /* line: inner-x */
        typedef long wide_t;
        struct pair { int a, b; } pr = { x, i };
        static int calls;
        wide_t w = x + calls++;
        struct holder { struct held { int h; } in; } hd = { { i } };
        sink = (int)w + pr.a + pr.b + hd.in.h;
    }
    {
        int x = 99;
        sink = x;
    }
    return *px;
}

int vla(int n) {
    int v[n];
    for (int i = 0; i < n; i++) {
        v[i] = i * i;
    }
    sink = v[n - 1];                             /* line: vla-use */
    return v[0];
}

int once(int a, int b) {
    int s = a * b + per_thread;
    int t = s - a;
    use(&t);
    sink = s;                                    /* line: once-use */
    return s + t;
}

struct pt { int x; long y; };
int *volatile keep;

int again(int a, int b) {
    int r = a + 1;
    sink = r;
    r = r * b;
    sink = r;
    struct pt q = { a, b };
    q.x += 3;
    sink = q.x;
    int arr[4] = { a, b, 3, 4 };
    keep = arr;                                  /* (in memory: its address escapes) */
    int *pa = &arr[2];
    sink = *pa;
    for (int i = 0; i < a; i++) {
        r = r ^ i;
        q.x += i;
        sink = r + q.x;
    }
    return (int)(r + q.x + q.y) + arr[1];
}

static int scaled(int v, int k) {
    int m = v * k;
    sink = m;
    return m + 1;
}

static void nothing(void) {}
static void (*volatile nothing_fp)(void) = nothing;

static int entry_of(int a, long b) {
    sink = a;
    sink = (int)b;
    nothing_fp();                                /* (a and b are not kept past it) */
    return 7;
}
static int (*volatile entry_fp)(int, long) = entry_of;

#undef VARS_DEPTH                            /* line: macro-undef */

int main(int argc, char **argv) {
    (void)argv;
    per_thread++;
    int z = scaled(argc, 3);                     /* line: scaled-call */
    return shadow(argc + 2) + vla(argc + 3) + once(argc, 5) + again(argc, 7) + z + entry_fp(4, 5L) + VARS_TWICE(1) == 0;
}
