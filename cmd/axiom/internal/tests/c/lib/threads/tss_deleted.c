/* flags: --c-std=c11 */
/* A key used after tss_delete (7.26.6.3 makes it undefined; here defined):
   tss_get gives null and tss_set thrd_error, in the thread that set a value
   and in another, and a new key in the same slot does not see the old
   values. thrd_sleep without a signal: 0, *remaining untouched. */
#include <stdio.h>
#include <threads.h>

static tss_t k;
static int v = 4;

static int other(void *arg) {
    (void)arg;
    return tss_get(k) == NULL && tss_set(k, &v) == thrd_error;
}

int main(void) {
    tss_t k2;
    thrd_t t;
    int r = 0;
    struct timespec d = { 0, 1000000 }, rem = { 7, 8 };
    tss_create(&k, NULL);
    tss_set(k, &v);
    printf("%d\n", *(int *)tss_get(k));
    tss_delete(k);
    printf("%d %d\n", tss_get(k) == NULL, tss_set(k, &v) == thrd_error);
    thrd_create(&t, other, NULL);
    thrd_join(t, &r);
    printf("%d\n", r);
    tss_create(&k2, NULL);
    printf("%d\n", tss_get(k2) == NULL);
    printf("%d %ld %ld\n", thrd_sleep(&d, &rem), (long)rem.tv_sec, rem.tv_nsec);
    return 0;
}
/* expect: 4 */
/* expect: 1 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 0 7 8 */
