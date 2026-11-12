/* flags: --c-std=gnu11 */
/* The __sync and __atomic builtins on integers and pointers of 1, 2, 4
   and 8 bytes (every memory order is sequentially consistent). */
#include "host.h"

static int i = 5;
static unsigned char b = 0xF0;
static long long ll = 1;
static short sh = 7;
static int arr[4];
static int *ip = arr;
struct __attribute__((aligned(4))) pair { short a, b; };

#define P(e) print_i64(e)

int main(void) {
    P(__sync_fetch_and_add(&i, 3));
    P(__sync_add_and_fetch(&i, 2));
    P(__sync_fetch_and_sub(&i, 1));
    P(__sync_sub_and_fetch(&i, 1));
    P(__sync_fetch_and_or(&b, 0x0F));
    P(__sync_and_and_fetch(&b, 0x3C));
    P(__sync_xor_and_fetch(&b, 0xFF));
    P(__sync_fetch_and_nand(&sh, 3));
    P(sh);
    P(__sync_nand_and_fetch(&sh, -1));
    P(__sync_bool_compare_and_swap(&ll, 1, 40));
    P(__sync_bool_compare_and_swap(&ll, 1, 50));
    P(__sync_val_compare_and_swap(&ll, 40, 41));
    P(ll);
    P(__sync_lock_test_and_set(&i, 77));
    P(i);
    __sync_lock_release(&i);
    __sync_synchronize();
    P(i);
    __atomic_store_n(&i, 12, __ATOMIC_SEQ_CST);
    P(__atomic_load_n(&i, __ATOMIC_ACQUIRE));
    P(__atomic_exchange_n(&i, 13, __ATOMIC_ACQ_REL));
    P(i);
    int expected = 13;
    P(__atomic_compare_exchange_n(&i, &expected, 20, 0, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED));
    P(i);
    expected = 99;
    P(__atomic_compare_exchange_n(&i, &expected, 30, 1, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED));
    P(expected);
    P(__atomic_fetch_add(&ll, 9, __ATOMIC_RELAXED));
    P(__atomic_add_fetch(&ll, 1, __ATOMIC_RELAXED));
    P(__atomic_fetch_and(&ll, 6, 0));
    P(__atomic_xor_fetch(&ll, 3, 0));
    P(__atomic_fetch_nand(&ll, 1, 0));
    P(ll);
    int *old = __atomic_fetch_add(&ip, 2, __ATOMIC_SEQ_CST);
    P(old == arr);
    P(ip == (int *)((char *)arr + 2));
    char flag = 0;
    P(__atomic_test_and_set(&flag, 0));
    P(__atomic_test_and_set(&flag, 0));
    __atomic_clear(&flag, 0);
    P(flag);
    struct pair p = { 1, 2 }, q = { 3, 4 }, r;
    __atomic_load(&p, &r, 0);
    P(r.a * 10 + r.b);
    __atomic_store(&p, &q, 0);
    P(p.a * 10 + p.b);
    struct pair x = { 5, 6 };
    __atomic_exchange(&p, &x, &r, 0);
    P(r.a * 1000 + r.b * 100 + p.a * 10 + p.b);
    struct pair want = { 5, 6 }, nv = { 7, 8 };
    P(__atomic_compare_exchange(&p, &want, &nv, 0, 0, 0));
    P(p.a * 10 + p.b);
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
    __atomic_signal_fence(__ATOMIC_SEQ_CST);
    P(__atomic_always_lock_free(sizeof(long long), 0));
    P(__atomic_is_lock_free(16, 0));
    return 0;
}
/* expect: 5 */
/* expect: 10 */
/* expect: 10 */
/* expect: 8 */
/* expect: 240 */
/* expect: 60 */
/* expect: 195 */
/* expect: 7 */
/* expect: -4 */
/* expect: 3 */
/* expect: 1 */
/* expect: 0 */
/* expect: 40 */
/* expect: 41 */
/* expect: 8 */
/* expect: 77 */
/* expect: 0 */
/* expect: 12 */
/* expect: 12 */
/* expect: 13 */
/* expect: 1 */
/* expect: 20 */
/* expect: 0 */
/* expect: 20 */
/* expect: 41 */
/* expect: 51 */
/* expect: 51 */
/* expect: 1 */
/* expect: 1 */
/* expect: -2 */
/* expect: 1 */
/* expect: 1 */
/* expect: 0 */
/* expect: 1 */
/* expect: 0 */
/* expect: 12 */
/* expect: 34 */
/* expect: 3456 */
/* expect: 1 */
/* expect: 78 */
/* expect: 1 */
/* expect: 0 */
