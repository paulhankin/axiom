/* The memory orders of C's atomics choose the instructions (the C11
   mappings): on x86-64 only a sequentially consistent store (xchg) and
   fence (mfence) need more than a plain access; on AArch64 a relaxed load
   or store is LDR or STR, an acquire load LDAR (LDAPR with RCpc), a release
   store STLR, a read-modify-write's exclusive pair or LSE instruction has
   the acquire and release forms its order needs, an acquire fence is DMB
   ISHLD and a release or stronger one DMB ISH; a signal fence and a relaxed
   one are no instruction. A weak compare-exchange is one exclusive
   attempt (no loop back). */
// lower-selfhost: --target=native -O2 --c-freestanding | \bxchg\b | 1
// lower-selfhost: --target=native -O2 --c-freestanding | \bmfence\b | 1
// lower-selfhost: --target=native -O2 --c-freestanding | \block xadd\b | 4
// lower: --target=aarch64 -O2 --c-freestanding | \bldar\s | 2
// lower: --target=aarch64 -O2 --c-freestanding | \bstlr\s | 2
// lower: --target=aarch64 -O2 --c-freestanding | \bldxr\s | 2
// lower: --target=aarch64 -O2 --c-freestanding | \bldaxr\s | 4
// lower: --target=aarch64 -O2 --c-freestanding | \bstxr\s | 3
// lower: --target=aarch64 -O2 --c-freestanding | \bstlxr\s | 3
// lower: --target=aarch64 -O2 --c-freestanding | \bdmb\s+ishld\b | 1
// lower: --target=aarch64 -O2 --c-freestanding | \bdmb\s+ish\n | 2
// lower: --target=aarch64 --cpu=neoverse-n1 -O2 --c-freestanding | \bldapr\s | 1
// lower: --target=aarch64 --cpu=neoverse-n1 -O2 --c-freestanding | \bldadd\s | 1
// lower: --target=aarch64 --cpu=neoverse-n1 -O2 --c-freestanding | \bldadda\s | 1
// lower: --target=aarch64 --cpu=neoverse-n1 -O2 --c-freestanding | \bldaddl\s | 1
// lower: --target=aarch64 --cpu=neoverse-n1 -O2 --c-freestanding | \bldaddal\s | 1
// lower: --target=aarch64 --cpu=neoverse-n1 -O2 --c-freestanding | \bcasal\s | 1
// lower: --target=aarch64 --cpu=neoverse-n1 -O2 --c-freestanding | \bcasa\s | 1
#include <stdatomic.h>
atomic_int x;
atomic_long y;
int ld_relaxed(void) { return atomic_load_explicit(&x, memory_order_relaxed); }
int ld_acquire(void) { return atomic_load_explicit(&x, memory_order_acquire); }
int ld_seq_cst(void) { return atomic_load(&x); }
void st_relaxed(int v) { atomic_store_explicit(&x, v, memory_order_relaxed); }
void st_release(int v) { atomic_store_explicit(&x, v, memory_order_release); }
void st_seq_cst(int v) { atomic_store(&x, v); }
long add_relaxed(long v) { return atomic_fetch_add_explicit(&y, v, memory_order_relaxed); }
long add_acquire(long v) { return atomic_fetch_add_explicit(&y, v, memory_order_acquire); }
long add_release(long v) { return atomic_fetch_add_explicit(&y, v, memory_order_release); }
long add_seq_cst(long v) { return atomic_fetch_add(&y, v); }
_Bool cas_weak(int *e, int d) {
    return atomic_compare_exchange_weak_explicit(&x, e, d, memory_order_acquire, memory_order_relaxed);
}
_Bool cas_strong(int *e, int d) { return atomic_compare_exchange_strong(&x, e, d); }
void fence_relaxed(void) { atomic_thread_fence(memory_order_relaxed); }
void fence_acquire(void) { atomic_thread_fence(memory_order_acquire); }
void fence_release(void) { atomic_thread_fence(memory_order_release); }
void fence_seq_cst(void) { atomic_thread_fence(memory_order_seq_cst); }
void fence_signal(void) { atomic_signal_fence(memory_order_seq_cst); }
