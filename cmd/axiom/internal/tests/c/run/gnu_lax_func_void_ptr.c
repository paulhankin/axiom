/* flags: --c-std=gnu17 */
/* expect-warning: C4140 */
/* A GNU dialect converts a pointer to a function to void * and back
   without a cast (6.5.16.1p1 asks for one), with a warning: in an
   initialization (static and automatic, a member's too), an assignment,
   an argument and a return. Calls through the pointer converted back reach
   the function. */
#include "host.h"

static int counter;
static void bump(void) { counter += 1; }
static int twice(int x) { return 2 * x; }

struct holder { void *code; };

static void *table[] = { bump, &twice };               /* static, an address constant */
static struct holder held = { bump };
static struct holder lit[] = { ((struct holder){ bump }), bump };

static void *as_void(int (*f)(int)) { return f; }      /* return */
static int call_int(void *p, int x) {
    int (*f)(int) = p;                                  /* initialization from void * */
    return f(x);
}
static void call_void(void *p) {
    void (*f)(void);
    f = p;                                              /* assignment from void * */
    f();
}

int main(void) {
    void *v = bump;                                     /* automatic */
    void (*back)(void) = v;
    back();
    call_void(table[0]);
    call_void(held.code);
    call_void(lit[0].code);
    call_void(lit[1].code);
    call_void(bump);                                    /* argument */
    print_i64(counter);
    print_i64(call_int(table[1], 21));
    print_i64(call_int(as_void(twice), 5));
    print_bool(table[0] == (void *)bump);
    print_bool(as_void(twice) == table[1]);
    return 0;
}
/* expect: 6 */
/* expect: 42 */
/* expect: 10 */
/* expect: true */
/* expect: true */
