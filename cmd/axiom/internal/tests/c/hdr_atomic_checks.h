/* hdr_atomic_checks.h - checks of <stdatomic.h> (C11 and later) against
   <stdint.h> and <stddef.h>: each atomic typedef is the atomic version of
   the type it stands for, the lock-free macros say what the
   implementation does, and atomic types are as large as their non-atomic
   versions. Included by hdr_checks.h (CHECK). */
#ifndef HDR_ATOMIC_CHECKS_H
#define HDR_ATOMIC_CHECKS_H
#if __STDC_VERSION__ >= 201112L
#include <stdatomic.h>
#include <stdint.h>
/* A is _Atomic(T): compatible pointers */
#define HA_SAME(A, T) CHECK(_Generic((A *)0, _Atomic(T) *: 1, default: 0))
HA_SAME(atomic_bool, _Bool);
HA_SAME(atomic_char, char);
HA_SAME(atomic_schar, signed char);
HA_SAME(atomic_uchar, unsigned char);
HA_SAME(atomic_short, short);
HA_SAME(atomic_ushort, unsigned short);
HA_SAME(atomic_int, int);
HA_SAME(atomic_uint, unsigned int);
HA_SAME(atomic_long, long);
HA_SAME(atomic_ulong, unsigned long);
HA_SAME(atomic_llong, long long);
HA_SAME(atomic_ullong, unsigned long long);
HA_SAME(atomic_char16_t, uint_least16_t);
HA_SAME(atomic_char32_t, uint_least32_t);
HA_SAME(atomic_wchar_t, wchar_t);
HA_SAME(atomic_int_least8_t, int_least8_t);
HA_SAME(atomic_uint_least8_t, uint_least8_t);
HA_SAME(atomic_int_least16_t, int_least16_t);
HA_SAME(atomic_uint_least16_t, uint_least16_t);
HA_SAME(atomic_int_least32_t, int_least32_t);
HA_SAME(atomic_uint_least32_t, uint_least32_t);
HA_SAME(atomic_int_least64_t, int_least64_t);
HA_SAME(atomic_uint_least64_t, uint_least64_t);
HA_SAME(atomic_int_fast8_t, int_fast8_t);
HA_SAME(atomic_uint_fast8_t, uint_fast8_t);
HA_SAME(atomic_int_fast16_t, int_fast16_t);
HA_SAME(atomic_uint_fast16_t, uint_fast16_t);
HA_SAME(atomic_int_fast32_t, int_fast32_t);
HA_SAME(atomic_uint_fast32_t, uint_fast32_t);
HA_SAME(atomic_int_fast64_t, int_fast64_t);
HA_SAME(atomic_uint_fast64_t, uint_fast64_t);
HA_SAME(atomic_intptr_t, intptr_t);
HA_SAME(atomic_uintptr_t, uintptr_t);
HA_SAME(atomic_size_t, size_t);
HA_SAME(atomic_ptrdiff_t, ptrdiff_t);
HA_SAME(atomic_intmax_t, intmax_t);
HA_SAME(atomic_uintmax_t, uintmax_t);
#if __STDC_VERSION__ >= 202311L
HA_SAME(atomic_char8_t, unsigned char);
CHECK(ATOMIC_CHAR8_T_LOCK_FREE == 2 && __STDC_VERSION_STDATOMIC_H__ == 202311L);
#else
CHECK(ATOMIC_VAR_INIT(3) == 3);
#endif
CHECK(ATOMIC_BOOL_LOCK_FREE == 2 && ATOMIC_CHAR_LOCK_FREE == 2 && ATOMIC_CHAR16_T_LOCK_FREE == 2);
CHECK(ATOMIC_CHAR32_T_LOCK_FREE == 2 && ATOMIC_WCHAR_T_LOCK_FREE == 2 && ATOMIC_SHORT_LOCK_FREE == 2);
CHECK(ATOMIC_INT_LOCK_FREE == 2 && ATOMIC_LONG_LOCK_FREE == 2 && ATOMIC_LLONG_LOCK_FREE == 2);
CHECK(ATOMIC_POINTER_LOCK_FREE == 2);
CHECK(sizeof(atomic_llong) == sizeof(long long) && _Alignof(atomic_llong) == sizeof(long long));
CHECK(sizeof(atomic_flag) == 1 && sizeof(atomic_intptr_t) == sizeof(intptr_t));
CHECK(memory_order_relaxed == 0 && memory_order_seq_cst == 5);
#endif
#endif
