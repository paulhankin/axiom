/* flags: --c-std=gnu11 */
/* The GNU dialects' generic atomic builtins on objects that are not 1, 2,
   4 or 8 bytes: under the lock table of C11's atomics (cgen_atomic.ax), so
   that they are atomic with each other and with _Atomic objects, across
   threads. */
#include "host.h"

void *__axiom_c_thread_start(void (*f)(void *), void *arg);
void __axiom_c_thread_join(void *thread);

struct wide { long long a, b; };
struct odd { char c[3]; };
struct wide w;
struct odd o = { { 1, 2, 3 } };

static void work(void *arg) {
    int i;
    (void)arg;
    for (i = 0; i < 5000; i++) {
        struct wide cur, next;
        __atomic_load(&w, &cur, __ATOMIC_SEQ_CST);
        do {
            next.a = cur.a + 1;
            next.b = cur.b + 2;
        } while (!__atomic_compare_exchange(&w, &cur, &next, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST));
    }
}

int main(void) {
    struct odd x = { { 7, 8, 9 } }, r;
    struct wide v;
    void *t[4];
    int i;
    __atomic_exchange(&o, &x, &r, __ATOMIC_SEQ_CST);
    print_i64(r.c[0] * 100 + r.c[1] * 10 + r.c[2]);
    __atomic_load(&o, &r, __ATOMIC_SEQ_CST);
    print_i64(r.c[0] * 100 + r.c[1] * 10 + r.c[2]);
    x.c[0] = 4;
    __atomic_store(&o, &x, __ATOMIC_RELAXED);
    print_i64(o.c[0]);
    print_i64(__atomic_always_lock_free(sizeof w, 0) * 10 + __atomic_always_lock_free(sizeof(long long), 0));
    for (i = 0; i < 4; i++)
        t[i] = __axiom_c_thread_start(work, 0);
    for (i = 0; i < 4; i++)
        __axiom_c_thread_join(t[i]);
    __atomic_load(&w, &v, __ATOMIC_SEQ_CST);
    print_i64(v.a);
    print_i64(v.b);
    return 0;
}
/* expect: 123 */
/* expect: 789 */
/* expect: 4 */
/* expect: 1 */
/* expect: 20000 */
/* expect: 40000 */
