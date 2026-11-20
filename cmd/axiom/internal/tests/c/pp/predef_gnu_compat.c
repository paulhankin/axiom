/* --c-gnu-compat (axiom cc's default in a GNU dialect): the GNU compilers' macros, version 9.4.0 (__VERSION__: c/cc/configure), C99's inline semantics after C89, the lock-free atomic types and the __sync builtins' sizes, IEC 60559 arithmetic; no other compiler's. */
/* flags: --target=x86_64 --c-std=gnu11 --c-gnu-compat */
__GNUC__ __GNUC_MINOR__ __GNUC_PATCHLEVEL__ __GNUC_STDC_INLINE__ __GNUC_GNU_INLINE__ __clang__ _MSC_VER
__GCC_HAVE_SYNC_COMPARE_AND_SWAP_1 __GCC_HAVE_SYNC_COMPARE_AND_SWAP_8 __GCC_HAVE_SYNC_COMPARE_AND_SWAP_16 __GCC_ATOMIC_INT_LOCK_FREE __GCC_ATOMIC_POINTER_LOCK_FREE
__GCC_IEC_559 __GCC_IEC_559_COMPLEX __FINITE_MATH_ONLY__ __AXIOM_GNU__ __AXIOM__
