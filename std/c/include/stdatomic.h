/* stdatomic.h - atomics (C11 7.17, C23 7.17), for the Axiom compiler's C
   front end. The generic functions are the compiler's builtins (their
   argument is a pointer to any atomic type); atomic_flag's functions,
   atomic_thread_fence and atomic_signal_fence are static inline functions
   over them, and macros of the same names, so that a constant order stays
   one. Each operation has the order it is given when that is a constant
   (an order an operation does not take is a warning, and is made at least
   as strong as it); one that is not a constant is memory_order_seq_cst.
   memory_order_consume is memory_order_acquire. atomic_compare_exchange_weak
   fails spuriously only on AArch64 without LSE (one exclusive attempt);
   atomic_signal_fence is a barrier to the compiler only.

   Atomic objects of 1, 2, 4 and 8 bytes are lock-free (the integer types
   and pointers among them, so every ATOMIC_*_LOCK_FREE is 2), and of 16
   bytes on AArch64 and x86-64 (where the processor has cmpxchg16b:
   atomic_is_lock_free asks it unless the target's extensions include it).
   Other sizes (structures of 3 bytes, or of 32) take a lock from a table of
   spinlocks indexed by address, which a signal handler must not use while
   the interrupted thread may hold it. On WebAssembly without the atomics
   feature, a module has one thread and the operations are ordinary loads
   and stores. */
#ifndef __AXIOM_STDATOMIC_H
#define __AXIOM_STDATOMIC_H

#if __STDC_VERSION__ < 201112L
#error "<stdatomic.h> is a header of C11 and later"
#endif

#if __STDC_VERSION__ >= 202311L
#define __STDC_VERSION_STDATOMIC_H__ 202311L
#endif

/* 7.17.3: the orders (the GNU dialects' __ATOMIC_ values are the same) */
typedef enum memory_order {
    memory_order_relaxed,
    memory_order_consume,
    memory_order_acquire,
    memory_order_release,
    memory_order_acq_rel,
    memory_order_seq_cst
} memory_order;

/* 7.17.5: whether the types are lock-free (2: always) */
#define ATOMIC_BOOL_LOCK_FREE 2
#define ATOMIC_CHAR_LOCK_FREE 2
#if __STDC_VERSION__ >= 202311L
#define ATOMIC_CHAR8_T_LOCK_FREE 2
#endif
#define ATOMIC_CHAR16_T_LOCK_FREE 2
#define ATOMIC_CHAR32_T_LOCK_FREE 2
#define ATOMIC_WCHAR_T_LOCK_FREE 2
#define ATOMIC_SHORT_LOCK_FREE 2
#define ATOMIC_INT_LOCK_FREE 2
#define ATOMIC_LONG_LOCK_FREE 2
#define ATOMIC_LLONG_LOCK_FREE 2
#define ATOMIC_POINTER_LOCK_FREE 2

/* 7.17.2: initialization (ATOMIC_VAR_INIT was removed in C23) */
#if __STDC_VERSION__ < 202311L
#define ATOMIC_VAR_INIT(value) (value)
#endif
#define atomic_init(obj, value) __builtin_atomic_init((obj), (value))

/* 7.17.3.1 */
#define kill_dependency(y) (y)

/* 7.17.4: fences */
static inline void atomic_thread_fence(memory_order __axiom_order) {
    __builtin_atomic_thread_fence(__axiom_order);
}
static inline void atomic_signal_fence(memory_order __axiom_order) {
    __builtin_atomic_signal_fence(__axiom_order);
}
#define atomic_thread_fence(order) __builtin_atomic_thread_fence(order)
#define atomic_signal_fence(order) __builtin_atomic_signal_fence(order)

/* 7.17.5.1 */
#define atomic_is_lock_free(obj) __builtin_atomic_is_lock_free(obj)

/* 7.17.6: the atomic integer types, as the types of <stdint.h>,
   <stddef.h> and <uchar.h> (which this header does not declare) are */
typedef _Atomic _Bool atomic_bool;
typedef _Atomic char atomic_char;
typedef _Atomic signed char atomic_schar;
typedef _Atomic unsigned char atomic_uchar;
typedef _Atomic short atomic_short;
typedef _Atomic unsigned short atomic_ushort;
typedef _Atomic int atomic_int;
typedef _Atomic unsigned int atomic_uint;
typedef _Atomic long atomic_long;
typedef _Atomic unsigned long atomic_ulong;
typedef _Atomic long long atomic_llong;
typedef _Atomic unsigned long long atomic_ullong;
#if __STDC_VERSION__ >= 202311L
typedef _Atomic unsigned char atomic_char8_t;
#endif

/* 8 and 16 bits; 64 bits (int64_t); at least 32 bits (int_least32_t); the fast types */
typedef _Atomic signed char atomic_int_least8_t;
typedef _Atomic unsigned char atomic_uint_least8_t;
typedef _Atomic short atomic_int_least16_t;
typedef _Atomic unsigned short atomic_uint_least16_t;
typedef _Atomic signed char atomic_int_fast8_t;
typedef _Atomic unsigned char atomic_uint_fast8_t;
/* (the types <stdint.h> has, from the compiler's macros: the platform's) */
typedef _Atomic __AXIOM_INT_FAST16_TYPE__ atomic_int_fast16_t;
typedef _Atomic __AXIOM_UINT_FAST16_TYPE__ atomic_uint_fast16_t;
typedef _Atomic unsigned short atomic_char16_t;
typedef _Atomic __AXIOM_INT64_TYPE__ atomic_int_least64_t;
typedef _Atomic __AXIOM_UINT64_TYPE__ atomic_uint_least64_t;
typedef _Atomic __AXIOM_INT64_TYPE__ atomic_int_fast64_t;
typedef _Atomic __AXIOM_UINT64_TYPE__ atomic_uint_fast64_t;
typedef _Atomic __AXIOM_INTMAX_TYPE__ atomic_intmax_t;
typedef _Atomic __AXIOM_UINTMAX_TYPE__ atomic_uintmax_t;
#if __AXIOM_INT_WIDTH__ == 32 || (__AXIOM_LONG_WIDTH__ != 32 && __AXIOM_INT_WIDTH__ == 64)
typedef _Atomic int atomic_int_least32_t;
typedef _Atomic unsigned int atomic_uint_least32_t;
typedef _Atomic unsigned int atomic_char32_t;
#else
typedef _Atomic long atomic_int_least32_t;
typedef _Atomic unsigned long atomic_uint_least32_t;
typedef _Atomic unsigned long atomic_char32_t;
#endif
typedef _Atomic __AXIOM_INT_FAST32_TYPE__ atomic_int_fast32_t;
typedef _Atomic __AXIOM_UINT_FAST32_TYPE__ atomic_uint_fast32_t;

/* as wide as a pointer: the platform's (<stddef.h>, <stdint.h>) */
typedef _Atomic __AXIOM_PTRDIFF_TYPE__ atomic_intptr_t;
typedef _Atomic __AXIOM_SIZE_TYPE__ atomic_uintptr_t;
typedef _Atomic __AXIOM_SIZE_TYPE__ atomic_size_t;
typedef _Atomic __AXIOM_PTRDIFF_TYPE__ atomic_ptrdiff_t;

/* as wchar_t (<stddef.h>) */
#if __AXIOM_WCHAR_WIDTH__ == 16
#if __AXIOM_WCHAR_SIGNED__
typedef _Atomic short atomic_wchar_t;
#else
typedef _Atomic unsigned short atomic_wchar_t;
#endif
#elif __AXIOM_INT_WIDTH__ == 32 || (__AXIOM_LONG_WIDTH__ != 32 && __AXIOM_INT_WIDTH__ >= 32)
#if __AXIOM_WCHAR_SIGNED__
typedef _Atomic int atomic_wchar_t;
#else
typedef _Atomic unsigned int atomic_wchar_t;
#endif
#else
#if __AXIOM_WCHAR_SIGNED__
typedef _Atomic long atomic_wchar_t;
#else
typedef _Atomic unsigned long atomic_wchar_t;
#endif
#endif

/* 7.17.7: operations on atomic types */
#define atomic_store(obj, desired) __builtin_atomic_store((obj), (desired), memory_order_seq_cst)
#define atomic_store_explicit(obj, desired, order) __builtin_atomic_store((obj), (desired), (order))
#define atomic_load(obj) __builtin_atomic_load((obj), memory_order_seq_cst)
#define atomic_load_explicit(obj, order) __builtin_atomic_load((obj), (order))
#define atomic_exchange(obj, desired) __builtin_atomic_exchange((obj), (desired), memory_order_seq_cst)
#define atomic_exchange_explicit(obj, desired, order) __builtin_atomic_exchange((obj), (desired), (order))
#define atomic_compare_exchange_strong(obj, expected, desired) \
    __builtin_atomic_compare_exchange((obj), (expected), (desired), memory_order_seq_cst, memory_order_seq_cst)
#define atomic_compare_exchange_strong_explicit(obj, expected, desired, success, failure) \
    __builtin_atomic_compare_exchange((obj), (expected), (desired), (success), (failure))
#define atomic_compare_exchange_weak(obj, expected, desired) \
    __builtin_atomic_compare_exchange_weak((obj), (expected), (desired), memory_order_seq_cst, memory_order_seq_cst)
#define atomic_compare_exchange_weak_explicit(obj, expected, desired, success, failure) \
    __builtin_atomic_compare_exchange_weak((obj), (expected), (desired), (success), (failure))
#define atomic_fetch_add(obj, operand) __builtin_atomic_fetch_add((obj), (operand), memory_order_seq_cst)
#define atomic_fetch_add_explicit(obj, operand, order) __builtin_atomic_fetch_add((obj), (operand), (order))
#define atomic_fetch_sub(obj, operand) __builtin_atomic_fetch_sub((obj), (operand), memory_order_seq_cst)
#define atomic_fetch_sub_explicit(obj, operand, order) __builtin_atomic_fetch_sub((obj), (operand), (order))
#define atomic_fetch_or(obj, operand) __builtin_atomic_fetch_or((obj), (operand), memory_order_seq_cst)
#define atomic_fetch_or_explicit(obj, operand, order) __builtin_atomic_fetch_or((obj), (operand), (order))
#define atomic_fetch_xor(obj, operand) __builtin_atomic_fetch_xor((obj), (operand), memory_order_seq_cst)
#define atomic_fetch_xor_explicit(obj, operand, order) __builtin_atomic_fetch_xor((obj), (operand), (order))
#define atomic_fetch_and(obj, operand) __builtin_atomic_fetch_and((obj), (operand), memory_order_seq_cst)
#define atomic_fetch_and_explicit(obj, operand, order) __builtin_atomic_fetch_and((obj), (operand), (order))

/* 7.17.8: the atomic flag (one byte, 0 when clear) */
typedef struct atomic_flag {
    _Atomic unsigned char __axiom_set;
} atomic_flag;
#define ATOMIC_FLAG_INIT { 0 }

static inline _Bool atomic_flag_test_and_set(volatile atomic_flag *__axiom_f) {
    return __builtin_atomic_exchange(&__axiom_f->__axiom_set, 1, memory_order_seq_cst) != 0;
}
static inline _Bool atomic_flag_test_and_set_explicit(volatile atomic_flag *__axiom_f, memory_order __axiom_order) {
    return __builtin_atomic_exchange(&__axiom_f->__axiom_set, 1, __axiom_order) != 0;
}
static inline void atomic_flag_clear(volatile atomic_flag *__axiom_f) {
    __builtin_atomic_store(&__axiom_f->__axiom_set, 0, memory_order_seq_cst);
}
static inline void atomic_flag_clear_explicit(volatile atomic_flag *__axiom_f, memory_order __axiom_order) {
    __builtin_atomic_store(&__axiom_f->__axiom_set, 0, __axiom_order);
}
#define atomic_flag_test_and_set_explicit(f, order) \
    (__builtin_atomic_exchange(&(f)->__axiom_set, 1, (order)) != 0)
#define atomic_flag_clear_explicit(f, order) __builtin_atomic_store(&(f)->__axiom_set, 0, (order))

#endif
