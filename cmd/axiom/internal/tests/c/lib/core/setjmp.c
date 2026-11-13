/* setjmp and longjmp (C17 7.13): setjmp returns 0, then longjmp's value
   (1 for 0); longjmp from nested calls, through a function pointer, from
   within a loop, with two buffers; objects not modified after setjmp
   (7.13.2.1p3) and volatile ones that were have their values. */
/* wasm-flags: --features=+exception-handling */
#include <setjmp.h>
#include "print.h"

static jmp_buf outer, inner;
static int depth;

static void down(int n, int v) {
    if (n == 0)
        longjmp(outer, v);
    depth++;
    down(n - 1, v);
}

static void (*volatile jump)(int, int) = down;

static int once(int v) {
    int a = v * 7;
    long long b = (long long)v << 40;
    volatile int changed = 0;
    int r = setjmp(outer);
    if (r == 0) {
        changed = v + 1;
        jump(20, v);
        return -1000;
    }
    return r * 1000000 + (a + (int)(b >> 40)) * 100 + changed;
}

static int nested(void) {
    volatile int trail = 0;
    if (setjmp(outer) != 0)
        return trail;
    trail = 1;
    if (setjmp(inner) == 0) {
        trail = 2;
        longjmp(inner, 1);
    }
    trail = trail * 10 + 3;
    longjmp(outer, 1);
}

int main(void) {
    volatile int i;
    int r;
    print_i64(once(5)); /* expect: 5004006 */
    print_i64(once(0)); /* expect: 1000001 */
    print_i64(depth); /* expect: 40 */
    print_i64(nested()); /* expect: 23 */
    /* jumping back into the same frame, around a loop */
    i = 0;
    r = setjmp(outer);
    if (r < 100) {
        i++;
        longjmp(outer, r + 25);
    }
    print_i64(r); /* expect: 100 */
    print_i64(i); /* expect: 4 */
    switch (setjmp(outer)) {
    case 0:
        print_cstr("first"); /* expect: first */
        longjmp(outer, 2);
    case 2:
        print_cstr("second"); /* expect: second */
        break;
    default:
        print_cstr("wrong");
    }
    if (!setjmp(outer))
        down(3, 0);
    else
        print_cstr("0 is 1"); /* expect: 0 is 1 */
    return 0;
}
