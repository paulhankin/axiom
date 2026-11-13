/* The other side of atomic_foreign.c, built by the system's C compiler:
   compare-exchange loops on the 24-byte atomic object (which that compiler
   does with calls of __atomic_load and __atomic_compare_exchange, which
   the program defines) from threads of the system's library; and
   increments of a 16-byte counter (calls of __atomic_fetch_add_16). */
#include <pthread.h>
#include <stdatomic.h>
struct s24 { long long a, b, c; };
extern _Atomic struct s24 shared;
extern _Atomic unsigned __int128 wide_count;
void ours_add(int n);
static void *run(void *arg) {
    int i;
    for (i = 0; i < 2000; i++) {
        struct s24 cur = atomic_load(&shared), next;
        do {
            next = cur;
            next.a += 1;
            next.c += 2;
        } while (!atomic_compare_exchange_weak(&shared, &cur, next));
        atomic_fetch_add(&wide_count, (unsigned __int128)1 << 64 | 1);
        if (arg)
            ours_add(1);
    }
    return 0;
}
int foreign_run(void) {
    pthread_t t[4];
    int i;
    for (i = 0; i < 4; i++)
        if (pthread_create(&t[i], 0, run, (void *)(long)(i & 1)) != 0)
            return -1;
    for (i = 0; i < 4; i++)
        pthread_join(t[i], 0);
    return 0;
}
