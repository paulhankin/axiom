/* flags: --target=x86_64 */
/* check-only */
/* The implementation-defined choices of --target=x86_64 by default: those of
   the x86-64 System V ABI (Linux). Checked when compiled (the test is not
   run). */
#include <float.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>

_Static_assert(CHAR_BIT == 8 && sizeof(short) == 2 && sizeof(int) == 4 && sizeof(long long) == 8, "the widths");
_Static_assert((CHAR_MIN < 0) == 1 && ((char)-1 < 0) == 1, "plain char's signedness");
_Static_assert(sizeof(long) == 8 && LONG_MAX == 9223372036854775807L, "long");
_Static_assert(sizeof(void *) == 8 && sizeof(size_t) == 8 && sizeof(ptrdiff_t) == 8, "pointers, size_t");
_Static_assert(sizeof(intptr_t) == 8 && sizeof(uintptr_t) == 8, "intptr_t");
_Static_assert(sizeof(wchar_t) == 4 && sizeof(L'a') == 4, "wchar_t's width");
_Static_assert(((wchar_t)-1 < 0) == 1 && (WCHAR_MIN < 0) == 1, "wchar_t's signedness");
_Static_assert(_Alignof(long long) == 8 && _Alignof(double) == 8, "alignment");
_Static_assert(sizeof(long double) == 16 && _Alignof(long double) == 16 && LDBL_MANT_DIG == 64, "long double: the x87's 80-bit format");
_Static_assert(sizeof(int64_t) == 8 && INT64_MAX == 9223372036854775807LL, "int64_t");

/* The exact types of the typedefs, as the platform's C ABI and C library
   have them (a declaration with them is compatible with the platform's) */
#define IS(x, t) _Generic((x), t: 1, default: 0)
_Static_assert(IS((size_t)0, unsigned long) && IS(sizeof(int), unsigned long) && IS((ptrdiff_t)0, long), "size_t, ptrdiff_t");
_Static_assert(IS((char *)0 - (char *)0, long), "a pointer difference");
_Static_assert(IS((intptr_t)0, long) && IS((uintptr_t)0, unsigned long), "intptr_t");
_Static_assert(IS((int64_t)0, long) && IS((uint64_t)0, unsigned long) && IS(INT64_C(1), long), "int64_t");
_Static_assert(IS((intmax_t)0, long) && IS((uintmax_t)0, unsigned long) && IS(INTMAX_MAX, long), "intmax_t");
_Static_assert(IS((int_fast16_t)0, long) && IS((int_fast32_t)0, long) && IS((uint_fast32_t)0, unsigned long), "int_fastN_t");
_Static_assert(IS(SIZE_MAX, unsigned long) && IS(PTRDIFF_MIN, long) && IS(UINT64_MAX, unsigned long), "the limits' types");
_Static_assert(IS(WINT_MAX, unsigned int) && IS((wchar_t)0, int) && IS(L'a', int), "wint_t's limits, wchar_t");
_Static_assert(_Alignof(max_align_t) == 16, "max_align_t");

int main(void) { return 0; }
