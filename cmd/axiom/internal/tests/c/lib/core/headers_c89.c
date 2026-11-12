/* The library's headers in C89 (and C95) mode: only C89's names; what
   later standards added is not declared (a C89 program may use those
   names: here as its own functions). */
/* flags: --c-std=c89 */
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <stdlib.h>
/* (print.h has _Bool, which C89 lacks) */
void print_i64(__axiom_int64 x);

/* (later standards' names, free in C89) */
static int isblank(int c) { return c == 1; }
static int strtoll(int x) { return x + 1; }
static int aligned_alloc(int x) { return x * 2; }
static int _Exit_ = 0;
static int strdup(int x) { return x - 1; }
static int quick_exit(int x) { return x; }

int main(void) {
    div_t d;
    ldiv_t l;
    d = div(7, -2);
    l = ldiv(-7L, 2L);
    print_i64(d.quot); /* expect: -3 */
    print_i64(d.rem); /* expect: 1 */
    print_i64(l.quot); /* expect: -3 */
    print_i64(l.rem); /* expect: -1 */
    print_i64(isblank(1) + strtoll(1) + aligned_alloc(1) + strdup(1) + quick_exit(1) + _Exit_); /* expect: 6 */
    assert(d.quot == -3);
    print_i64(abs(-5) + (int)labs(-6L)); /* expect: 11 */
    return EXIT_SUCCESS;
}
