/* flags: --c-std=gnu11 */
/* expect-warning: C4141 */
/* A GNU dialect converts a pointer to a qualified type to a pointer to a
   less qualified one without a cast (6.5.16.1p1 asks for the qualifiers),
   with a warning: in an assignment, an initialization, an argument and a
   return. The object is then accessed through the new pointer; it is not
   const itself, so writing it is defined. */
#include "host.h"

static int data[3] = { 1, 2, 3 };
static const int *view = data;

static int *unconst(const int *p) { return p; }          /* return */
static void put(int *p, int v) { *p = v; }
static int sum(volatile int *p, int n) {
    int s = 0;
    for (int i = 0; i < n; i++) s += p[i];
    return s;
}

int main(void) {
    int *w = view;                                       /* initialization */
    w[0] = 10;
    int *x;
    x = view + 1;                                        /* assignment */
    *x = 20;
    put(view + 2, 30);                                   /* argument */
    *unconst(view) += 1;
    const volatile int *cv = data;
    print_i64(sum(cv, 3));                               /* const dropped, volatile kept */
    void *p;
    int i = 0;
    p = i ? 0 : (const void *)0;                         /* the ?: has type const void * */
    print_bool(p == 0);
    p = !i ? (const void *)data : 0;
    print_bool(p == 0);
    print_i64(data[0] + data[1] + data[2]);
    return 0;
}
/* expect: 61 */
/* expect: true */
/* expect: false */
/* expect: 61 */
