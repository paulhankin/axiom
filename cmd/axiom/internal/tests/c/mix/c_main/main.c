/* A C program with Axiom modules beside it (docs/c.md, "C and Axiom in one
   program"): the Axiom runtime is running before main (its allocator, its
   I/O, its threads), the modules' export fns are C functions, callbacks go
   both ways. */
/* uses: ../print.ax */
#include "../print.h"
#include <stddef.h>
#include <stdint.h>

/* rt.ax */
void *rt_alloc(size_t n);
void rt_free(void *p, size_t n);
int32_t rt_apply(int32_t (*f)(int32_t), int32_t x);
int64_t rt_sum_threads(int32_t n);
void (*rt_hook(void))(int64_t);
int64_t rt_sum_list(const int64_t *xs, size_t n);

static int32_t sq(int32_t x) { return x * x; }

int main(int argc, char **argv) {
    print_i64(argc); /* expect: 1 */
    print_bool(argv[argc] == 0); /* expect: true */
    int64_t *xs = rt_alloc(100 * sizeof(int64_t));
    for (int i = 0; i < 100; i++)
        xs[i] = i;
    print_i64(rt_sum_list(xs, 100)); /* expect: 4950 */
    rt_free(xs, 100 * sizeof(int64_t));
    print_i64(rt_apply(sq, 9)); /* expect: 81 */
    rt_hook()(-17);
    /* expect: hook */
    /* expect: -17 */
    print_i64(rt_sum_threads(4)); /* expect: 6 */
    print_cstr("done"); /* expect: done */
    return 0;
}
